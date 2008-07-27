/** LuaArgument.cpp
 *
 * LuaArgument provides an easy way to deal with passing arguments between lua and C++.
 * (for example, arguments to call lua arguments, return arguments from lua, etc)
 *
 * Jonanin 2008
 */

#include "LuaArgument.h"

// TODO: add support for CFunction, nil,

// TODO documentation !!!

// FIXME: I THINK I know how pointers work... but DC makes an idiot out of me every time!!!

LuaArgument::LuaArgument() {}

LuaArgument::LuaArgument (int arg) {
    SetInt(arg);
}

LuaArgument::LuaArgument (double arg) {
    SetDouble(arg);
}

LuaArgument::LuaArgument (const string& arg) {
    SetString(arg);
}

LuaArgument::LuaArgument (map<LuaArgument, LuaArgument>& arg) {
    SetTable(arg);
}

LuaArgument::LuaArgument (bool arg) {
    SetBool(arg);
}

void LuaArgument::SetInt (int arg) {
    m_int   = arg;
    m_type  = ARG_INT;
    sortint = arg;
}

void LuaArgument::SetDouble (double arg) {
    m_double = arg;
    m_type   = ARG_DOUBLE;
    sortint  = (int)arg;
}

void LuaArgument::SetString (const string& arg) {
    m_string = arg;
    m_type  = ARG_STRING;
    sortint = arg.length() * 2; // arbitrary, doesn't really mean anything: but we need a way to sort for map.
}

void LuaArgument::SetTable (map<LuaArgument, LuaArgument>& arg) {
    m_table = map<LuaArgument, LuaArgument>(arg);
    m_type  = ARG_TABLE;
    sortint = m_table.size() * 2; // arbitrary, doesn't really mean anything: but we need a way to sort for map.
}

void LuaArgument::SetBool (bool arg) {
    m_bool  = arg;
    m_type  = ARG_BOOL;
    sortint = ((int)arg) * 100; // arbitrary, doesn't really mean anything: but we need a way to sort for map.
}

void LuaArgument::Clear () {
    m_type   = ARG_UNKNOWN;
    m_bool   = false;
    m_int    = 0;
    m_double = 0;
    m_string.clear();
    m_table.clear();
    sortint = 0;
}

/* required for map */
bool LuaArgument::operator< (const LuaArgument m1) const {
    return m1.sortint < sortint;
}

void LuaArgument::SetFromStack (lua_State * L, int index) {
        Clear();

        int rtype = lua_type(L, index);

        switch (rtype) {

            case LUA_TBOOLEAN:
                SetBool(lua_toboolean(L, index));
                break;
            case LUA_TSTRING:
                lua_pushvalue(L, index);
                SetString(lua_tostring(L, -1));
                lua_pop(L, 1);
                break;

            case LUA_TNUMBER:
                // all numbers in lua are represented as doubles, so we cant actually get
                // an integer. You have to do your own conversion for that !!
                SetDouble(lua_tonumber(L, index));
                break;

            case LUA_TTABLE:
                {



                    map<LuaArgument, LuaArgument> temptable;

                    lua_pushnil(L);
                    while (lua_next(L, index - 1) != 0) {
                        LuaArgument k;
                        LuaArgument v;
                        v.SetFromStack(L, -1);
                        k.SetFromStack(L, -2);
                        lua_pop(L, 1);
                        temptable.insert(make_pair(k,v));
                    }
                    SetTable(temptable);
                }
                break;

            default:
                // hmmm
                m_type = ARG_UNKNOWN;
                break;
    }


}

// does anything need to be done here
LuaArgument::~LuaArgument ( ) {}

void LuaArgument::PushToStack (lua_State * L) {

    switch(m_type) {

        case ARG_BOOL:
            lua_pushboolean(L, m_bool);
             break;

        case ARG_DOUBLE:
            lua_pushnumber(L, m_double);
            break;

        case ARG_INT:
            lua_pushinteger(L, m_int);
            break;

        case ARG_STRING:
            lua_pushstring(L, m_string.c_str());
            break;

        case ARG_TABLE:
            {
                lua_newtable(L);
                map<LuaArgument, LuaArgument>::iterator it;
                for (it = m_table.begin(); it != m_table.end(); it++) {
                    LuaArgument k = it->first;  // for some reason I need to copy..
                    LuaArgument v = it->second; // making the function const doens't work either
                    k.PushToStack(L);  // Push key onto stack
                    v.PushToStack(L);  // Push value onto stack
                    lua_rawset(L, -3); // add the pair to the table, so t[k] = v
                }
            }
            break;

        case ARG_UNKNOWN:
            // erm... is there something that should be done?!
            break;
    }
}

void LuaArgument::PrintDebug() {

    switch(m_type) {

        case ARG_BOOL:
            printf( (m_bool ? "true" : "false") );
            break;

        case ARG_DOUBLE:
            printf("%g", m_double);
            break;

        case ARG_INT:
            printf("%i", m_int);
            break;

        case ARG_STRING:
            printf( m_string.c_str() );
            break;

        case ARG_TABLE:
            {
                /* FIXME: (minor) prints extra comma at end */
                printf("{");
                map<LuaArgument, LuaArgument>::iterator it;
                for (it = m_table.begin(); it != m_table.end(); it++) {
                    LuaArgument k = it->first;
                    LuaArgument v = it->second;
                    k.PrintDebug();
                    printf(" - ");
                    v.PrintDebug();
                    printf(", ");
                }
                printf("}");

            }
            break;

        case ARG_UNKNOWN:
            printf( "(unknown)" );
            break;

        default:
            printf("(default)");

    }
}
