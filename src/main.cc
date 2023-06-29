#include "mehxml.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#if defined(_WIN32)
#include <windows.h>
#endif

constexpr unsigned char MAX_ARGS = 8;
constexpr unsigned char MAX_RETURNS = 4;
constexpr unsigned char MAX_FUNCS = 128;

// Function Structure
//------------------------------------------------------------------------
struct arg_t
{
    std::string name;
    std::string type;
    std::string desc;
    bool optional;
};

typedef struct arg_t return_t;

struct function_t
{
    std::string module;
    std::string name;
    std::string desc;
    arg_t args[MAX_ARGS];
    return_t returns[MAX_RETURNS];
};

// Utility Functions
//------------------------------------------------------------------------
std::string trim_spaces(const std::string& line)
{
    std::string result = line;
    result.erase(0, result.find_first_not_of(" \t"));
    return result;
}

// Temporary Logging Functions
//------------------------------------------------------------------------
void debug_print(const char* format)
{
#if defined(_WIN32)
    OutputDebugStringA(format);
#endif
    printf("%s", format);
}

template<typename... Args>
void debug_print(const char* format, const Args&... args)
{
    constexpr std::size_t BufferSize = 256;
    char buffer[BufferSize];
    std::snprintf(buffer, BufferSize, format, args...);
    #if defined(_WIN32)
        OutputDebugStringA(buffer);
    #endif
    printf("%s", buffer);
}

// Data Extraction
//------------------------------------------------------------------------
arg_t get_arg(const std::string& line)
{
    arg_t arg;
    arg.name = "";
    arg.type = "";
    arg.desc = "";
    arg.optional = false;

    int colon_pos = line.find_first_of(':');

    // Get the name
    arg.name = line.substr(0, colon_pos);

    // Get the type
    arg.type = line.substr(colon_pos + 1);
    size_t next_pos = arg.type.find(' ');
    if (next_pos != std::string::npos)
    {
        arg.type = arg.type.substr(0, next_pos);
        
        // Check if it's optional
        std::string optPrefix = "<opt>";
        if (arg.name.substr(0, optPrefix.size()) == optPrefix) {
            arg.name.erase(0, optPrefix.size());
            arg.optional = true;
        }

        // Get the description
        arg.desc = line.substr(colon_pos + arg.type.length()+2, line.length());
    }

    return arg;
}

function_t extract(const std::string& comment)
{
    function_t fn;
    fn.name = "";
    fn.desc = "";
    
    bool isDesc = true;
    std::istringstream iss(comment.c_str());
    std::string line;
    std::string buf;

    unsigned char argc = 0;
    unsigned char retc = 0;

    while (std::getline(iss, line))
    {
        line = trim_spaces(line);

        if (line.find("@function") != std::string::npos) // Get function name
        {
            isDesc = false;
            fn.name = line.substr(10, line.length() - 10);
        } else if (line.find("@param") != std::string::npos) // Get param
        {
            isDesc = false;
            line = line.substr(7, line.length() - 7); // remove @param
            fn.args[argc++] = get_arg(line);
        } else if (line.find("@return") != std::string::npos) // Get returns
        {
            isDesc = false;
            line = line.substr(8, line.length() - 8);
            fn.returns[retc++] = get_arg(line);
        } else // Get description
        {
            // Remove empty lines
            if (line.empty() || !isDesc)
                continue;

            fn.desc += line + "\\n";
        }
    }

    if (!fn.desc.empty())
        fn.desc = fn.desc.substr(0, fn.desc.length() - 2);

    return fn;
}

int main(int argc, char* argv[])
{
    const char* usage = "Usage: .\\docgen.exe -f,--file <header_file> (-o,--out outfile.xml)";

    if (argc < 2) {
        printf("%s\n", usage);
        return 0;
    }

    std::string input_file;
    std::string output_file = "api.xml";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-f" && i + 1 < argc) {
            input_file = argv[i + 1];
            i++;
        } else if (arg == "-o" && i + 1 < argc) {
            output_file = argv[i + 1];
            i++;
        } else
        {
            printf("%s\n", usage);
            return 0;
        }
    }

    if (input_file.empty())
    {
        printf("%s\n", usage);
        return 0;
    }

    std::ifstream file(input_file);
    std::string line;

    bool in_comment = false;
    int finding_name_for = -1;

    function_t funcs[MAX_FUNCS];
    unsigned char num_funcs = 0;

    std::string current_module = "";
    std::string func_str = "";
    
    // Get our functions
    while (std::getline(file, line))    
    {
        int module_pos = line.find("@module");
        if (module_pos != std::string::npos)
        {
            current_module = line.substr(module_pos + 8, line.length());
            continue;
        }

        if (line.find("/***") != std::string::npos)
        {
            in_comment = true;
            finding_name_for = -1;
        } else if (line.find("*/") != std::string::npos)
        {
            function_t func = extract(func_str);
            func.module = current_module;
            funcs[num_funcs] = func;

            in_comment = false;
            func_str = "";

            if (func.name.empty())
                finding_name_for = num_funcs;

            num_funcs++;
        } else if (finding_name_for > -1)
        {
            int opening_bracket_pos = line.find('(');

            if (opening_bracket_pos != std::string::npos)
            {
                // Find the position of the space character preceding the opening bracket
                size_t space_pos = line.rfind(' ', opening_bracket_pos);
    
                if (space_pos != std::string::npos)
                {
                    // Extract the function name
                    std::string function_name = line.substr(space_pos + 1, opening_bracket_pos - space_pos - 1);
                    funcs[finding_name_for].name = function_name;

                    finding_name_for = -1;
                }
            }
        } else if (in_comment)
        {
            func_str += line += "\n";
        }
    }

    file.close();

    // mehxml, previously known as shitty xml. now has been improved.
    mehxml::node api("api");

    std::map<std::string, mehxml::node> modules;

    // Generate the XML Data
    for (const function_t& fn : funcs)
    {
        if (fn.name.empty())
            continue;

        // If we have specified "@module name" and it's not in the map, create a new node for it.
        if (!fn.module.empty() && !modules.count(fn.module))
        {
            mehxml::node module_node("module");
            module_node.set("name", fn.module);
            modules.insert({fn.module, module_node});
        }

        mehxml::node function("function");
        function.set("name", fn.name);

        if (!fn.desc.empty())
            function.set("desc", fn.desc);

        // Params
        for (const arg_t& arg : fn.args)
        {
            if (arg.name.empty())
                continue;

            mehxml::node input("input");
            input.set("name", arg.name);
            input.set("type", arg.type);

            if (!arg.desc.empty())
                input.set("desc", arg.desc);

            if (arg.optional)
                input.set("optional", "true");

            function.append(input);
        }

        // Returns
        for (const return_t& ret : fn.returns)
        {
            if (ret.name.empty())
                continue;

            mehxml::node output("output");
            output.set("name", ret.name);
            output.set("type", ret.type);

            if (!ret.desc.empty())
                output.set("desc", ret.desc);

            function.append(output);
        }

        // No modules.
        if (fn.module.empty())
        {
            api.append(function);
            continue;
        }


        // We are using modules
        modules.at(fn.module).append(function);
    }

    // If we're using modules, append them to the root node (api)
    for (const std::pair<std::string, mehxml::node>& module : modules)
        api.append(module.second);

    std::ofstream out(output_file);
    out << api.to_str();
    out.close();

    printf("Generated %d functions! Outputting to \"%s\"\n", num_funcs, output_file.c_str());

    return 0;
}