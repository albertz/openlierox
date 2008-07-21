// 2008
// Jon 'jonanin Morton

#include "luainc.h"

//TODO: more error checking......etc...

/* testing getting varaibles from lua*/
int getLuaGlobalInt(lua_State *L, const char *var) {

    // get the global from the lua (and onto stack)
    lua_getglobal(L, var);

    // check if the first element on the stack
    // is an number
    if (!lua_isnumber(L, -1)) {
        return 0;
    }

    int num = lua_tointeger(L, -1);

    // pop it off the stack so the stack is left how it was!
    lua_pop(L, 1);

    return num;
}

/* testing calling C++ functions from lua */
int L_say(lua_State *L) {

    // this will get the first argument from the function that was called.
    // in this case, a string
    const char *whattosay = luaL_optstring(L, 1, "it didn't work! D:");

    printf("+ testing calling C++ functions from lua...\n");
    printf("    lua says: %s \n", whattosay);

    return 0;
}

int main() {
    printf("\n");
    // lua has libraries, like io (io.print, etc)... so here we
    // create or own library! yay!
     const static struct luaL_reg awesomelib [] = {
        {"say", &L_say},
        {NULL, NULL}
     };

    lua_State *L = lua_open();
    luaL_openlibs(L);
    luaL_register(L, "awesome", awesomelib); /*** CALL C++ FUNCTION FROM LUA TEST ***/
    luaL_dofile(L, "../../test.lua");


    /*** GLOBAL VAR TEST ***/
    printf("+ testing getting global var from lua...\n");
    int var = getLuaGlobalInt(L, "myvar");
    printf("    global integer: %i \n", var);


    /*** CALL LUA FUNCTION TEST ***/
    printf("+ testing calling lua function from C++...\n");
    // get the function and push it onto the stack
    lua_getglobal(L, "addTwoNumbers");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        printf("    Uh oh we were trying to call something that wasn't a function! abandon ship!");
        return 0;
    }
    // these are our arguments for the function, as always, they go on the stack
    lua_pushnumber(L, 10);
    lua_pushnumber(L, 1337);
    // 2 arguments, 1 result.
    if (lua_pcall(L, 2, 1, 0) != 0) {
        printf("    Erm. error calling addTwoNumbers: %s", lua_tostring(L, -1)); // errors are pushed on to the stack
        return 0;
    }
    if (!lua_isnumber(L, -1)) {
        lua_pop(L, 1);
        printf("    ... the result wasn't a number and it should be! lets blame it on microsoft");
        return 0;
    }

    double result = lua_tonumber(L, -1);
    printf("    called addTwoNumbers(10, 1337) and got result: %f\n", result);
    lua_pop(L, 1);




    printf("\n I am done. have a nice day.\n");
    lua_close(L);
    return 0;
}
