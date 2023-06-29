# docgen

A simple way to generate API documentation for your C/C++ Lua API. Made with C++11

I made this because I needed documentation for a project of mine, I tried LDoc & Doxygen but neither of them worked the way I wanted them too. I saw someone else was looking for something similar so I've decided to pop it on here in case anyone else needs it.

## How to use
Go above one of your functions and add a comment like this:
```cpp
/***
    Executes an SQLite statement on the open database.

    @function exec
    @param query:string The SQL Statement to execute
    @param <opt>callback:function The Lua function to call for each result row
    @param <opt>arg:any The argument to pass to the callback

    @return result:int ErrorCode
*/
u32 exec(sol::this_state ts, const std::string_view sql, opt<sol::function> callback, opt<sol::lua_value> arg);
```

The descriptions are optional for both the arguments and return values.  

`@function` is for overwriting the function name, it will find it by default.

`@param` takes the following: "name:type optional_description". Where you see `<opt>`, that's just a way for us to specify that parameter is optional. It will be removed.  

`@return` is exactly the same as the param. And yes, you can have multiple.

Run the program `.\docgen.exe -f input_file.h -o outfile.xml`.  
(**The `-o` flag is optional**)

The output will look like this:
```xml
<api>
    <function name="exec" desc="Executes an SQLite statement on the open database.">
        <input name="query" type="string" desc="The SQL Statement to execute" />
        <input name="callback" type="function" desc="The Lua function to call for each result row" optional="true" />
        <input name="arg" type="any" desc="The argument to pass to the callback" optional="true" />
        <output name="result" type="int" desc="ErrorCode" />
    </function>
</api>
```

## Modules
To specify a module, add `@module myModule` anywhere, any comment after that will be using that module. The XML will be different, it will be separated into nodes, example:
```xml
<api>
    <module name="module1">
        ...
    </module>
    <module name="module2">
        ...
    </module>
</api>
```

## Notes:
Probably error prone, it hasn't been designed to avoid your mistakes.  
Arguments are limited to 8 per function  
Returns are limited to 4 per function  
Functions are limited to 128 per file  
**Note: I might change the above so it's more dynamic**

This is because it was meant to just be for my project, if you want me to increase it then just let me know.

## TODO's:
- [x] Add `@module`. This will be placed in the file and the comments after will be inside of that module. This will be optional.