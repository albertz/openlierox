// 2008
// Jon 'jonanin' Morton

#include "luainc.h"
#include "Common.h"
#include "LuaInterface.h"
#include "LuaArgument.h"
#include <list>

int main() {



    LuaInterface lu;
    lu.ExecuteFile("test.lua");

    vector<LuaArgument> retargs;

    // arguments for lua function
    LuaArgument largs[] = { LuaArgument(5) };
    // index, function name,  args array,  number of args, number of results (return args), return args vector
    lu.CallLua(LUA_GLOBALSINDEX, "doStuff", largs, 1, 4, &retargs);

    int i = 0;
    for (vector<LuaArgument>::iterator it = retargs.begin(); it < retargs.end(); it++) {
        printf("arg %i: ", i++);
        it->PrintDebug();
        printf("\n");
    }
}
