#ifndef MEHXML_H
#define MEHXML_H

#include <string>
#include <vector>
#include <sstream>

// ̶T̶h̶e̶ ̶s̶h̶i̶t̶t̶i̶e̶s̶t̶ ̶X̶M̶L̶ ̶"̶i̶m̶p̶l̶e̶m̶e̶n̶t̶a̶t̶i̶o̶n̶"̶ ̶y̶o̶u̶'̶v̶e̶ ̶e̶v̶e̶r̶ ̶s̶e̶e̶n̶,̶ ̶e̶n̶j̶o̶y̶!̶
// Not so bad anymore, it's a little nicer than it was previously. It gets the job done quickly, what else do you want from me...

namespace mehxml // previously known as "shittyxml", it's had a few upgrades.
{
    class node {
    public:
        node(const std::string& name) : name(name) {}

        void append(const node& node) {
            children.push_back(node);
        }

        void set(const std::string& name, const std::string& value)
        {
            inlined += name + "=\"" + value + "\" ";
        }

        std::string to_str(int indent = 0) const
        {
            std::string result;
            std::string indentation(indent, ' ');

            result += indentation + "<" + name;

            if (!inlined.empty())
            {
                result += " " + inlined;
            }

            bool hasChildren = !children.empty();

            if (hasChildren)
            {
                if (result.substr(result.length()-1) == " ") // For nodes with attributes & children, it removes the space before the closing bracket
                    result.pop_back();
                result += ">\n";

                for (const auto& child : children)
                    result += child.to_str(indent + 4) + "\n";

                result += indentation + "</" + name + ">";
            }
            else
            {
                result += "/>";
            }

            return result;
        }

    private:  
        std::string name;
        std::string inlined; // for inline attribute names
        std::vector<node> children;
    };
}

#endif // MEHXML_H