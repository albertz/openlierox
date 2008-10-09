/** LuaInterface.cpp
 *
 * LuaInterface class which provides interface between C++ and C lua, providing many
 * 'dirty work' functions etc...
 *
 * Jonanin 2008
 */

#include "LuaInterface.h"

//TODO: Error reporting through printf isn't good enough?
// probably need also someway of notifything the user their
// lua did not work? (eventually)

//TODO: Documentation of each function... more comments.. etc

//TODO: BETTER way to do CallLua - its kind of awkward right now { kindof fixed }

//TODO: Actually do the TODOs!

LuaInterface::LuaInterface() {
    LuaVM = lua_open();

    //TODO: Don't open all standard libraries by default...
    luaL_openlibs(LuaVM);
}

LuaInterface::~LuaInterface() {
    lua_close(LuaVM);
}

bool LuaInterface::ExecuteFile(const char * file_path) {

    if (luaL_dofile(LuaVM, file_path) != 0) {
        char buffer[50];
        sprintf(buffer, "Error loading/executing lua file at %s", file_path);
        PrintDebug("ExecuteFile", buffer);
        return false;
    }
    return true;
}

bool LuaInterface::ExecuteString(const char * str) {

    if (luaL_dostring(LuaVM, str) != 0) {
        PrintDebug("ExecuteString", "Error executing lua string.");
        return false;
    }
    return true;
}


bool LuaInterface::CallLua(int index, const char * func_name, LuaArgument * args, int nargs, int nresults, vector<LuaArgument> * return_args) {

    lua_getfield(LuaVM, index, func_name);
    if (!lua_isfunction(LuaVM, -1)) {
        lua_pop(LuaVM, -1);
        char buffer[75];
        sprintf(buffer, "Error calling lua function, the var name %s at index %i was not a function!", func_name, index);
        PrintDebug("CallLua", buffer);
        return false;
    }

    for (int i = 0; i < nargs; i++) {
        (args[i]).PushToStack(LuaVM);
    }

    int err = lua_pcall(LuaVM, nargs, nresults, NULL);

    if (err) {
        char buffer[50];
        sprintf(buffer, "Error calling lua function! Lua says: %s", lua_tostring(LuaVM, -1));
        PrintDebug("CallLua", buffer);
        return false;
    }

    vector<LuaArgument> retargs;
    LuaArgument k;

    for (int i = -nresults; i < 0; i++) {
        /* TODO: actually check if the argument exists
            more error checking here */

        k.SetFromStack(LuaVM, i);

        retargs.push_back(k);
   }

    *return_args = retargs;

    return true;

}

void LuaInterface::PrintDebug(const char * func, const char * str) {
    printf("LuaInterface::%s() - %s \n", func, str);
}
