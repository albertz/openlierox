/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#ifndef __CSCRIPTABLEVARS_H__
#define __CSCRIPTABLEVARS_H__

#include <string>
#include <map>
#include <vector>
#include <list>
#include <cassert>
#include "Color.h"

namespace DeprecatedGUI {
class CWidget;
}

// It's called ScriptCallback but it's used only for widgets, so contains link to widget that raised an event.
typedef void ( * ScriptCallback_t ) ( const std::string & param, DeprecatedGUI::CWidget * source );
typedef Uint32 Color_t;

// These typenr are also used for network, so don't change them.
enum ScriptVarType_t
{
	SVT_BOOL = 0,
	SVT_INT = 1,
	SVT_FLOAT = 2,
	SVT_STRING = 3,
	SVT_COLOR = 4,		// uint32 actually, used only in XML files
	SVT_CALLBACK	// Cannot be referenced from XML files directly, only as string
};


// Helper wrapper for common var types (used in GUI skinning)
struct ScriptVar_t
{
	ScriptVarType_t type;
	bool isUnsigned;  // True for values that are unsigned and where negative values mean infinity

	// Cannot do union because of std::string
	bool b;
	int i;
	float f;
	std::string s;
	Color_t c;	// color

	// No callback here - we cannot assign callbacks to each other
	ScriptVar_t(): type(SVT_BOOL), isUnsigned(false), b(false), i(0), f(0.0), s(""), c(0) { };
	ScriptVar_t( bool v ): type(SVT_BOOL), isUnsigned(false), b(v), i(0), f(0.0), s(""), c(0) { };
	ScriptVar_t( int v ): type(SVT_INT), isUnsigned(false), b(false), i(v), f(0.0), s(""), c(0) { };
	ScriptVar_t( float v ): type(SVT_FLOAT), isUnsigned(false), b(false), i(0), f(v), s(""), c(0) { };
	ScriptVar_t( const std::string & v ): type(SVT_STRING), isUnsigned(false), b(false), i(0), f(0.0), s(v), c(0) { };
	ScriptVar_t( const char * v ): type(SVT_STRING), isUnsigned(false), b(false), i(0), f(0.0), s(v), c(0) { };
	ScriptVar_t( Color_t v ): type(SVT_COLOR), isUnsigned(false), b(false), i(0), f(0.0), s(""), c(v) { };
	
	operator bool() const { assert(type == SVT_BOOL); return b; }
	operator int() const { assert(type == SVT_INT); return i; }
	operator float() const { assert(type == SVT_FLOAT); return f; }
	operator std::string() const { assert(type == SVT_STRING); return s; }
	operator Color_t() const { assert(type == SVT_COLOR); return c; }
	bool operator==(const ScriptVar_t& var) const {
		if(var.type != type) return false;
		switch(type) {
			case SVT_BOOL: return var.b == b;
			case SVT_INT: return var.i == i;
			case SVT_FLOAT: return var.f == f;
			case SVT_STRING: return var.s == s;
			case SVT_COLOR: return var.c == c;
			default: assert(false); return false;
		}
	}
	
	std::string toString() const;
	bool fromString( const std::string & str);
};

// Pointer to any in-game var - var should be global or static
struct ScriptVarPtr_t
{
	ScriptVarType_t type;
	bool isUnsigned;  // True for values that are unsigned and where negative values mean infinity

	// we use union to save some memory
	union
	{
		bool * b;	// Pointer to static var
		int * i;
		float * f;
		std::string * s;
		Color_t * cl;
		ScriptCallback_t cb;
	};
	union	// Is there any point in doing that union?
	{
		bool bdef;	// Default value for that var for config file loading / saving
		int idef;
		float fdef;
		const char * sdef;	// Not std::string to keep this structure plain-old-data
		Color_t cldef;
		// No default value for skin callback, 'cause it's not saved into cfg file
	};
	ScriptVarPtr_t(): type(SVT_CALLBACK), isUnsigned(false) { b = NULL; } // Invalid value initially (all pointer are NULL in union)
	ScriptVarPtr_t( bool * v, bool def = false ): type(SVT_BOOL), isUnsigned(false) { b = v; bdef = def; }
	ScriptVarPtr_t( int * v, int def = 0 ): type(SVT_INT), isUnsigned(false) { i = v; idef = def; }
	ScriptVarPtr_t( float * v, float def = 0.0 ): type(SVT_FLOAT), isUnsigned(false) { f = v; fdef = def; }
	ScriptVarPtr_t( std::string * v, const char * def = "" ): type(SVT_STRING), isUnsigned(false) { s = v; sdef = def; }
	ScriptVarPtr_t( Color_t * v, Color_t def = MakeColour(255,0,255) ): type(SVT_COLOR), isUnsigned(false) { cl = v; cldef = def; }
	ScriptVarPtr_t( ScriptCallback_t v ): type(SVT_CALLBACK), isUnsigned(false) { cb = v; }
	ScriptVarPtr_t( ScriptVar_t * v, const ScriptVar_t * def ) : type(v->type), isUnsigned(v->isUnsigned)  {
		switch(type) {
			case SVT_BOOL: b = &v->b; bdef = def->b; break;
			case SVT_INT: i = &v->i; idef = def->i; break;
			case SVT_FLOAT: f = &v->f; fdef = def->f; break;
			case SVT_STRING: s = &v->s; sdef = def->s.c_str(); break;
			case SVT_COLOR: cl = &v->c; cldef = def->c; break;
			default: assert(false);
		}
	}

	// These funcs will assert() if you try to call them on Callback varptr
	std::string toString() const;
	bool fromString( const std::string & str) const;	// const 'cause we don't change pointer itself, only data it points to
};


// Do not confuse this with game script - these vars mainly for GUI frontend and options
class CScriptableVars	// Singletone
{
public:
	
	static CScriptableVars & Init();	// Called automatically
	static void DeInit();	// Should be called from main()

	static const std::map< std::string, ScriptVarPtr_t > & Vars()
	{
		Init();
		return m_instance->m_vars;
	};
	
	typedef std::map< std::string, ScriptVarPtr_t > :: const_iterator iterator;

	static iterator begin()
	{ 
		Init();
		return m_instance->m_vars.begin();
	};
	static iterator end()
	{ 
		Init();
		return m_instance->m_vars.end();
	};
	
	static ScriptVarPtr_t GetVar( const std::string & name );	// Case-insensitive search, returns NULL on fail
	static ScriptVarPtr_t GetVar( const std::string & name, ScriptVarType_t type );	// Case-insensitive search, returns NULL on fail
	static std::string DumpVars();	// For debug output
	
	static void SetVarByString(const ScriptVarPtr_t& var, const std::string& str);

	// Convert "tLXOptions -> iNetworkPort " to "iNetworkPort", not used anywhere
	static std::string StripClassName( const std::string & c );

	static std::string GetDescription( const std::string & name );
	static std::string GetLongDescription( const std::string & name );

	static bool GetMinMaxValues( const std::string & name, int * minVal, int * maxVal );
	static bool GetMinMaxValues( const std::string & name, float * minVal, float * maxVal );
	static int GetGroup( const std::string & name );

	// Allows registering vars with daisy-chaining
	class VarRegisterHelper
	{
		friend class CScriptableVars;

		std::map< std::string, ScriptVarPtr_t > & m_vars;	// Reference to CScriptableVars::m_vars
		std::map< std::string, std::pair< std::string, std::string > > & m_descriptions;	// Reference to CScriptableVars::m_descriptions
		std::map< std::string, std::pair< ScriptVar_t, ScriptVar_t > > & m_minmax; // Reference to CScriptableVars::m_minmax
		std::map< std::string, int > & m_groups;	// Reference to CScriptableVars::m_groups
		std::string m_prefix;

		VarRegisterHelper( CScriptableVars * parent, const std::string & prefix ): 
			m_vars( parent->m_vars ), m_descriptions( parent->m_descriptions ), m_minmax( parent->m_minmax ), m_groups(parent->m_groups), m_prefix(prefix) {};

		std::string Name( const std::string & c )
		{
			if( m_prefix != "" ) 
				return m_prefix + "." + c;
			return c;
		};

		public:

		operator bool () { return true; };	// To be able to write static expressions

		VarRegisterHelper & operator() ( bool & v, const std::string & c, bool def = false, 
											const std::string & descr = "", const std::string & descrLong = "", int group = -1 )
			{ 
				m_vars[Name(c)] = ScriptVarPtr_t( &v, def );
				m_descriptions[Name(c)] = std::make_pair( descr, descrLong );
				m_groups[Name(c)] = group;
				return *this; 
			};

		VarRegisterHelper & operator() ( int & v, const std::string & c, int def = 0, 
										const std::string & descr = "", const std::string & descrLong = "", int group = -1,
										bool unsig = false, int minval = 0, int maxval = 0 )
			{ 
				ScriptVarPtr_t tmp = ScriptVarPtr_t( &v, def );
				tmp.isUnsigned = unsig;
				m_vars[Name(c)] = tmp;
				m_descriptions[Name(c)] = std::make_pair( descr, descrLong ); 
				m_minmax[Name(c)] = std::make_pair( ScriptVar_t(minval), ScriptVar_t(maxval) );
				m_groups[Name(c)] = group;
				return *this; 
			};

		VarRegisterHelper & operator() ( float & v, const std::string & c, float def = 0.0f, 
											const std::string & descr = "", const std::string & descrLong = "", int group = -1,
											bool unsig = false, float minval = 0.0f, float maxval = 0.0f )
			{ 
				ScriptVarPtr_t tmp = ScriptVarPtr_t( &v, def );
				tmp.isUnsigned = unsig;
				m_vars[Name(c)] = tmp; 
				m_descriptions[Name(c)] = std::make_pair( descr, descrLong ); 
				m_minmax[Name(c)] = std::make_pair( ScriptVar_t(minval), ScriptVar_t(maxval) );
				m_groups[Name(c)] = group;
				return *this; 
			};

		VarRegisterHelper & operator() ( std::string & v, const std::string & c, const char * def = "", 
											const std::string & descr = "", const std::string & descrLong = "", int group = -1 )
			{ 
				m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); 
				m_descriptions[Name(c)] = std::make_pair( descr, descrLong ); 
				m_groups[Name(c)] = group;
				return *this; 
			};

		VarRegisterHelper & operator() ( Color_t & v, const std::string & c, Color_t def = MakeColour(255,0,255), 
											const std::string & descr = "", const std::string & descrLong = "", int group = -1 )
			{
				m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); 
				m_descriptions[Name(c)] = std::make_pair( descr, descrLong ); 
				m_groups[Name(c)] = group;
				return *this; 
			};

		VarRegisterHelper & operator() ( ScriptCallback_t v, const std::string & c, 
											const std::string & descr = "", const std::string & descrLong = "", int group = -1 )
			{ 
				m_vars[Name(c)] = ScriptVarPtr_t( v ); 
				m_descriptions[Name(c)] = std::make_pair( descr, descrLong ); 
				m_groups[Name(c)] = group;
				return *this; 
			};
		
		VarRegisterHelper & operator() ( ScriptVar_t& v, const std::string & c, const ScriptVar_t& def, 
											const std::string & descr = "", const std::string & descrLong = "", int group = -1,
											ScriptVar_t minval = ScriptVar_t(0), ScriptVar_t maxval = ScriptVar_t(0) )
			{ 
				m_vars[Name(c)] = ScriptVarPtr_t( &v, &def ); 
				m_descriptions[Name(c)] = std::make_pair( descr, descrLong ); 
				m_minmax[Name(c)] = std::make_pair( minval, maxval );
				m_groups[Name(c)] = group;
				return *this; 
			};
	};

	static VarRegisterHelper RegisterVars( const std::string & base = "" )
	{
		Init();
		return VarRegisterHelper( m_instance, base );
	};

	// De-registers all variables with names starting with "base."
	static void DeRegisterVars( const std::string & base );

private:
	friend class VarRegisterHelper;

	static CScriptableVars * m_instance;
	std::map< std::string, ScriptVarPtr_t > m_vars;	// All in-game variables and callbacks
	std::map< std::string, std::pair< std::string, std::string > > m_descriptions;	// Description for vars - short and long
	std::map< std::string, std::pair< ScriptVar_t, ScriptVar_t > > m_minmax;	// Min and max values to make slider widget in GUI
	std::map< std::string, int > m_groups;	// Grouping of variables in GUI
};

#endif
