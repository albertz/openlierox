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

#include "Color.h"

namespace DeprecatedGUI {
class CWidget;
};

// Do not confuse this with game script - these vars mainly for GUI frontend and options
class CScriptableVars	// Singletone
{
public:

	// It's called ScriptCallback but it's used only for widgets, so contains link to widget that raised an event.
	typedef void ( * ScriptCallback_t ) ( const std::string & param, DeprecatedGUI::CWidget * source );
	typedef Uint32 Color_t;

	enum ScriptVarType_t
	{
		SVT_BOOL,
		SVT_INT,
		SVT_FLOAT,
		SVT_STRING,
		SVT_COLOR,		// uint32 actually, used only in XML files
		SVT_CALLBACK	// Cannot be referenced from XML files directly, only as string
	};

	// Pointer to any in-game var - var should be global or static
	struct ScriptVarPtr_t
	{
		ScriptVarType_t type;
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
		ScriptVarPtr_t(): type(SVT_CALLBACK) { b = NULL; }; // Invalid value initially (all pointer are NULL in union)
		ScriptVarPtr_t( bool * v, bool def = false ): type(SVT_BOOL) { b = v; bdef = def; };
		ScriptVarPtr_t( int * v, int def = 0 ): type(SVT_INT) { i = v; idef = def; };
		ScriptVarPtr_t( float * v, float def = 0.0 ): type(SVT_FLOAT) { f = v; fdef = def; };
		ScriptVarPtr_t( std::string * v, const char * def = "" ): type(SVT_STRING) { s = v; sdef = def; };
		ScriptVarPtr_t( Color_t * v, Color_t def = MakeColour(255,0,255) ): type(SVT_COLOR) { cl = v; cldef = def; };
		ScriptVarPtr_t( ScriptCallback_t v ): type(SVT_CALLBACK) { cb = v; };
	};

	// Helper wrapper for common var types (used in GUI skinning)
	struct ScriptVar_t
	{
		ScriptVarType_t type;
		// Cannot do union because of std::string
		bool b;
		int i;
		float f;
		std::string s;
		Color_t c;	// color
		// No callback here - we cannot assign callbacks to each other
		ScriptVar_t(): type(SVT_BOOL), b(false), i(0), f(0.0), s(""), c(0) { };
		ScriptVar_t( bool v ): type(SVT_BOOL), b(v), i(0), f(0.0), s(""), c(0) { };
		ScriptVar_t( int v ): type(SVT_INT), b(false), i(v), f(0.0), s(""), c(0) { };
		ScriptVar_t( float v ): type(SVT_FLOAT), b(false), i(0), f(v), s(""), c(0) { };
		ScriptVar_t( const std::string & v ): type(SVT_STRING), b(false), i(0), f(0.0), s(v), c(0) { };
		ScriptVar_t( const char * v ): type(SVT_STRING), b(false), i(0), f(0.0), s(v), c(0) { };
		ScriptVar_t( Color_t v ): type(SVT_COLOR), b(false), i(0), f(0.0), s(""), c(v) { };
	};

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
	
	static void SetVarByString(const CScriptableVars::ScriptVarPtr_t& var, const std::string& str);

	// Convert "tLXOptions -> iNetworkPort " to "iNetworkPort", not used anywhere
	static std::string StripClassName( const std::string & c );

	// Allows registering vars with daisy-chaining
	class VarRegisterHelper
	{
		friend class CScriptableVars;

		std::map< std::string, ScriptVarPtr_t > & m_vars;	// Reference to CScriptableVars::m_vars
		std::string m_prefix;

		VarRegisterHelper( CScriptableVars * parent, const std::string & prefix ): 
			m_vars( parent->m_vars ), m_prefix(prefix) {};

		std::string Name( const std::string & c )
		{
			if( m_prefix != "" ) 
				return m_prefix + "." + c;
			return c;
		};

		public:

		operator bool () { return true; };	// To be able to write static expressions

		VarRegisterHelper & operator() ( bool & v, const std::string & c, bool def = false )
			{ m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); return *this; };

		VarRegisterHelper & operator() ( int & v, const std::string & c, int def = 0 )
			{ m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); return *this; };

		VarRegisterHelper & operator() ( float & v, const std::string & c, float def = 0.0 )
			{ m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); return *this; };

		VarRegisterHelper & operator() ( std::string & v, const std::string & c, const char * def = "" )
			{ m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); return *this; };

		VarRegisterHelper & operator() ( Color_t & v, const std::string & c, Color_t def = MakeColour(255,0,255) )
			{ m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); return *this; };

		VarRegisterHelper & operator() ( ScriptCallback_t v, const std::string & c )
			{ m_vars[Name(c)] = ScriptVarPtr_t( v ); return *this; };
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
};

#endif
