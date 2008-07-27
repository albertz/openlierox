#ifndef _LUA_ARGUMENT_H_
#define _LUA_ARGUMENT_H_


#include "Common.h"


class LuaArgument {

    public:

        enum ArgumentType {
            ARG_INT,
            ARG_DOUBLE,
            ARG_STRING,
            ARG_BOOL,
            ARG_TABLE,
            ARG_UNKNOWN
        };

        LuaArgument ( );
        LuaArgument ( int arg );                            // ARG_INT
        LuaArgument ( double arg );                         // ARG_DOUBLE
        LuaArgument ( bool arg );                           // ARG_BOOL
        LuaArgument ( const string& arg );                  // ARG_STRING
        LuaArgument ( map<LuaArgument, LuaArgument>& arg ); // ARG_TABLE
        LuaArgument ( const LuaArgument& m1 );

        LuaArgument& operator= ( const LuaArgument& m1 );

        void SetInt    ( int arg );
        void SetDouble ( double arg );
        void SetBool   ( bool arg );
        void SetString ( const string& arg );
        void SetTable  ( map<LuaArgument, LuaArgument>& arg );

        void Clear ( );

        void PushToStack ( lua_State * L );
        void SetFromStack( lua_State * L, int index );

        void PrintDebug ( );

        ~LuaArgument ( );

        // number used for sorting (required by map)
        int sortint;

        bool operator< (LuaArgument m1) const;

    private:

        ArgumentType m_type;

        string                          m_string;
        bool                            m_bool;
        int                             m_int;
        double                          m_double;
        map<LuaArgument, LuaArgument>   m_table;

};






#endif // _LUA_ARGUMENT_H_
