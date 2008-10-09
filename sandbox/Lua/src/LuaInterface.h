#ifndef _LUA_INTERFACE_H_
#define _LUA_INTERFACE_H_

#include "Common.h"
#include "LuaArgument.h"

class LuaInterface {

    public:

        LuaInterface();
        ~LuaInterface();

        // doin' the lua
        bool ExecuteFile (const char * file_path);
        bool ExecuteString (const char * str);

        bool CallLua (int index, const char * func_name, LuaArgument * args, int nargs, int nresults, vector<LuaArgument> * return_args);

    private:

        void PrintDebug (const char * func, const char * str);
        lua_State *LuaVM;

};


#endif
