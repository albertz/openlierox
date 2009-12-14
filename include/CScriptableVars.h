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

#include <SDL.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <cassert>
#include <iostream>
#include "Color.h"
#include "StringUtils.h"


// Groups for options ( I came up with six groups, and named them pretty lame, TODO: fix that )
enum GameInfoGroup
{
	GIG_Invalid = -1,
	GIG_General = 0,
	GIG_Advanced,
	GIG_Score,
	GIG_Weapons,
	GIG_Bonus,
	GIG_Other,
	
	GIG_GameModeSpecific_Start, // All following options are game-mode specific
	
	GIG_Tag,
	GIG_HideAndSeek,
	GIG_CaptureTheFlag,
	GIG_Race,
	
	GIG_Size
};

// And their descriptions - don't forget to edit them in Options.cpp if you change GameInfoGroup_t
extern const char * GameInfoGroupDescriptions[][2];


enum AdvancedLevel {
	ALT_Basic = 0,
	ALT_Advanced,
	ALT_VeryAdvanced,
	ALT_Dev,
	ALT_DevKnownUnstable,
	__AdvancedLevelType_Count,
	
	ALT_OnlyViaConfig = 1000,
};
std::string AdvancedLevelDescription(AdvancedLevel l);
std::string AdvancedLevelShortDescription(AdvancedLevel l);

namespace DeprecatedGUI {
class CWidget;
}

// It's called ScriptCallback but it's used only for widgets, so contains link to widget that raised an event.
typedef void ( * ScriptCallback_t ) ( const std::string & param, DeprecatedGUI::CWidget * source );

// These typenr are also used for network, so don't change them.
enum ScriptVarType_t
{
	SVT_BOOL = 0,
	SVT_INT = 1,
	SVT_FLOAT = 2,
	SVT_STRING = 3,
	SVT_COLOR = 4,		// uint32 actually, used only in XML files
	SVT_CALLBACK,	// Cannot be referenced from XML files directly, only as string
	SVT_DYNAMIC
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
	Color c;	// color

	// No callback here - we cannot assign callbacks to each other
	ScriptVar_t(): type(SVT_BOOL), isUnsigned(false), b(false), i(0), f(0.0), s(""), c() { }
	ScriptVar_t( bool v ): type(SVT_BOOL), isUnsigned(false), b(v), i(0), f(0.0), s(""), c() { }
	ScriptVar_t( int v ): type(SVT_INT), isUnsigned(false), b(false), i(v), f(0.0), s(""), c() { }
	ScriptVar_t( float v ): type(SVT_FLOAT), isUnsigned(false), b(false), i(0), f(v), s(""), c() { }
	ScriptVar_t( const std::string & v ): type(SVT_STRING), isUnsigned(false), b(false), i(0), f(0.0), s(v), c() { }
	ScriptVar_t( const char * v ): type(SVT_STRING), isUnsigned(false), b(false), i(0), f(0.0), s(v), c() { }
	ScriptVar_t( Color v ): type(SVT_COLOR), isUnsigned(false), b(false), i(0), f(0.0), s(""), c(v) { }
	
	operator bool() const { assert(type == SVT_BOOL); return b; }
	operator int() const { assert(type == SVT_INT); return i; }
	operator float() const { assert(type == SVT_FLOAT); return f; }
	operator std::string() const { assert(type == SVT_STRING); return s; }
	operator Color() const { assert(type == SVT_COLOR); return c; }
	bool operator==(const ScriptVar_t& var) const {
		if(var.type != type) return false;
		switch(type) {
			case SVT_BOOL: return var.b == b;
			case SVT_INT: return var.i == i;
			case SVT_FLOAT: return var.f == f;
			case SVT_STRING: return var.s == s;
			case SVT_COLOR: return var.c == c;
			default: assert(false);
		}
		return false;
	}
	bool operator<(const ScriptVar_t& var) const {
		if(isNumeric() && var.isNumeric()) return getNumber() < var.getNumber();
		if(var.type != type) return type < var.type;
		switch(type) {
			case SVT_BOOL: return b < var.b;
			case SVT_INT: return i < var.i;
			case SVT_FLOAT: return f < var.f;
			case SVT_STRING: return s < var.s;
			case SVT_COLOR: return c < var.c;
			default: assert(false);
		}
		return false;
	}
	
	bool isNumeric() const { return type == SVT_INT || type == SVT_FLOAT; }
	// TODO: float has the same size as int, so we should convert to double here to avoid data loss with big ints
	float getNumber() const { if(type == SVT_INT) return (float)i; if(type == SVT_FLOAT) return f; return 0.0f; }
	
	std::string toString() const;
	bool fromString( const std::string & str);
	friend std::ostream& operator<< (std::ostream& o, const ScriptVar_t& svt);
};


template<typename T> ScriptVarType_t GetType();
template<> inline ScriptVarType_t GetType<bool>() { return SVT_BOOL; }
template<> inline ScriptVarType_t GetType<int>() { return SVT_INT; }
template<> inline ScriptVarType_t GetType<float>() { return SVT_FLOAT; }
template<> inline ScriptVarType_t GetType<std::string>() { return SVT_STRING; }
template<> inline ScriptVarType_t GetType<Color>() { return SVT_COLOR; }

struct _DynamicVar {
	virtual ~_DynamicVar() {}
	virtual ScriptVarType_t type() = 0;
	virtual ScriptVar_t asScriptVar() = 0;
	virtual void fromScriptVar(const ScriptVar_t&) = 0;
};

template < typename T >
struct DynamicVar : _DynamicVar {
	virtual ~DynamicVar() {}
	virtual ScriptVarType_t type() { return GetType<T>(); }
	virtual T get() = 0;
	virtual void set(const T&) = 0;
	virtual ScriptVar_t asScriptVar() { return ScriptVar_t(get()); }
	virtual void fromScriptVar(const ScriptVar_t& var) { set(var); }
};

struct UndefColor {
	Uint8 r, g, b, a;
	UndefColor& operator=(const Color& c) { r = c.r; g = c.g; b = c.b; a = c.a; return *this; }
	operator Color() const { return Color(r,g,b,a); }
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
		Color * cl;
		ScriptCallback_t cb;
		_DynamicVar* dynVar;
	};
	union	// Is there any point in doing that union?
	{
		bool bdef;	// Default value for that var for config file loading / saving
		int idef;
		float fdef;
		const char * sdef;	// Not std::string to keep this structure plain-old-data
		UndefColor cldef;
		// No default value for skin callback, 'cause it's not saved into cfg file
	};
	ScriptVarPtr_t(): type(SVT_CALLBACK), isUnsigned(false) { b = NULL; } // Invalid value initially (all pointer are NULL in union)
	ScriptVarPtr_t( bool * v, bool def = false ): type(SVT_BOOL), isUnsigned(false) { b = v; bdef = def; }
	ScriptVarPtr_t( int * v, int def = 0 ): type(SVT_INT), isUnsigned(false) { i = v; idef = def; }
	ScriptVarPtr_t( float * v, float def = 0.0 ): type(SVT_FLOAT), isUnsigned(false) { f = v; fdef = def; }
	ScriptVarPtr_t( std::string * v, const char * def = "" ): type(SVT_STRING), isUnsigned(false) { s = v; sdef = def; }
	ScriptVarPtr_t( Color * v, Color def = Color(255,0,255) ): type(SVT_COLOR), isUnsigned(false) { cl = v; cldef = def; }
	ScriptVarPtr_t( ScriptCallback_t v ): type(SVT_CALLBACK), isUnsigned(false) { cb = v; }
	
	ScriptVarPtr_t( DynamicVar<bool>* v, bool def ): type(SVT_DYNAMIC), isUnsigned(false), bdef(def) { dynVar = v; }
	ScriptVarPtr_t( DynamicVar<int>* v, int def ): type(SVT_DYNAMIC), isUnsigned(false), idef(def) { dynVar = v; }
	ScriptVarPtr_t( DynamicVar<float>* v, float def ): type(SVT_DYNAMIC), isUnsigned(false), fdef(def) { dynVar = v; }
	ScriptVarPtr_t( DynamicVar<std::string>* v, const std::string& def ): type(SVT_DYNAMIC), isUnsigned(false), sdef(def.c_str()) { dynVar = v; }
	ScriptVarPtr_t( DynamicVar<Color>* v, Color def ): type(SVT_DYNAMIC), isUnsigned(false) { dynVar = v; cldef = def; }
	
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

	void setDefault() const;

	bool operator==(const ScriptVar_t* var) const {
		if(var == NULL) return b == NULL;
		if(var->type != type) return false;
		switch(type) {
			case SVT_BOOL: return b == &var->b;
			case SVT_INT: return i == &var->i;
			case SVT_FLOAT: return f == &var->f;
			case SVT_STRING: return s == &var->s;
			case SVT_COLOR: return cl == &var->c;
			default: assert(false);
		}
		return false;
	}
	
	// These funcs will assert() if you try to call them on Callback varptr
	std::string toString() const;
	bool fromString( const std::string & str) const;	// const 'cause we don't change pointer itself, only data it points to
	friend std::ostream& operator<< (std::ostream& o, const ScriptVarPtr_t& svt);
};

template<typename T> T* getPointer(ScriptVarPtr_t& varPtr);
template<> inline bool* getPointer<bool>(ScriptVarPtr_t& varPtr) { if(varPtr.type != SVT_BOOL) return NULL; return varPtr.b; }
template<> inline int* getPointer<int>(ScriptVarPtr_t& varPtr) { if(varPtr.type != SVT_INT) return NULL; return varPtr.i; }
template<> inline float* getPointer<float>(ScriptVarPtr_t& varPtr) { if(varPtr.type != SVT_FLOAT) return NULL; return varPtr.f; }
template<> inline std::string* getPointer<std::string>(ScriptVarPtr_t& varPtr) { if(varPtr.type != SVT_STRING) return NULL; return varPtr.s; }
template<> inline Color* getPointer<Color>(ScriptVarPtr_t& varPtr) { if(varPtr.type != SVT_COLOR) return NULL; return varPtr.cl; }



struct RegisteredVar {
	RegisteredVar() : group(GIG_Invalid) {}

	RegisteredVar( bool & v, const std::string & c, bool def = false, 
									const std::string & descr = "", const std::string & descrLong = "",
									GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic )
	{
		var = ScriptVarPtr_t( &v, def );
		shortDesc = descr; longDesc = descrLong;
		group = g;
		advancedLevel = l;
	}
	
	RegisteredVar( int & v, const std::string & c, int def = 0, 
									const std::string & descr = "", const std::string & descrLong = "",
									GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic,
									bool unsig = false, int minval = 0, int maxval = 0 )
	{ 
		var = ScriptVarPtr_t( &v, def );
		var.isUnsigned = unsig;
		shortDesc = descr; longDesc = descrLong;
		min = ScriptVar_t(minval); max = ScriptVar_t(maxval);
		group = g;
		advancedLevel = l;
	}
	
	RegisteredVar( float & v, const std::string & c, float def = 0.0f, 
									const std::string & descr = "", const std::string & descrLong = "",
									GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic,
									bool unsig = false, float minval = 0.0f, float maxval = 0.0f )
	{ 
		var = ScriptVarPtr_t( &v, def );
		var.isUnsigned = unsig;
		shortDesc = descr; longDesc = descrLong;
		min = ScriptVar_t(minval); max = ScriptVar_t(maxval);
		group = g;
		advancedLevel = l;
	}
	
	RegisteredVar( std::string & v, const std::string & c, const char * def = "", 
									const std::string & descr = "", const std::string & descrLong = "",
									GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic )
	{ 
		var = ScriptVarPtr_t( &v, def ); 
		shortDesc = descr; longDesc = descrLong;
		group = g;
		advancedLevel = l;
	}
	
	RegisteredVar( Color & v, const std::string & c, Color def = Color(255,0,255), 
									const std::string & descr = "", const std::string & descrLong = "",
									GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic )
	{
		var = ScriptVarPtr_t( &v, def ); 
		shortDesc = descr; longDesc = descrLong;
		group = g;
		advancedLevel = l;
	}
	
	RegisteredVar( ScriptCallback_t v, const std::string & c, 
									const std::string & descr = "", const std::string & descrLong = "",
									GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic )
	{ 
		var = ScriptVarPtr_t( v ); 
		shortDesc = descr; longDesc = descrLong;
		group = g;
		advancedLevel = l;
	}
	
	RegisteredVar( ScriptVar_t& v, const std::string & c, const ScriptVar_t& def, 
									const std::string & descr = "", const std::string & descrLong = "",
									GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic,
									ScriptVar_t minval = ScriptVar_t(), ScriptVar_t maxval = ScriptVar_t(), bool unsignedValue = false )
	{ 
		var = ScriptVarPtr_t( &v, &def );
		var.isUnsigned = unsignedValue;
		shortDesc = descr; longDesc = descrLong;
		group = g;
		advancedLevel = l;
		if( v.type == SVT_INT && minval.type == SVT_INT && maxval.type == SVT_INT ) {
			min = minval.i; max = maxval.i;
		} else if( v.type == SVT_FLOAT && minval.type == SVT_FLOAT && maxval.type == SVT_FLOAT ) {
			min = minval.f; max = maxval.f;
		}
	}
	
	template<typename T>
	RegisteredVar( DynamicVar<T>* v, const std::string & c, const T& def,
									const std::string & descr = "", const std::string & descrLong = "",
									GameInfoGroup g = GIG_Invalid, AdvancedLevel l = ALT_Basic )
	{ 
		var = ScriptVarPtr_t( v, def );
		shortDesc = descr; longDesc = descrLong;
		group = g;
		advancedLevel = l;
	}
	
	
	ScriptVarPtr_t var;
	std::string shortDesc;
	std::string longDesc;
	ScriptVar_t min;
	ScriptVar_t max;
	GameInfoGroup group;
	AdvancedLevel advancedLevel;
	
	bool haveMinMax() const { return min.isNumeric() && max.isNumeric() && min < max; }
};


// Do not confuse this with game script - these vars mainly for GUI frontend and options
class CScriptableVars	// Singletone
{
public:
	
	static CScriptableVars & Init();	// Called automatically
	static void DeInit();	// Should be called from main()

	typedef std::map< std::string, RegisteredVar, stringcaseless > VarMap;
	typedef VarMap::const_iterator const_iterator;

	static const VarMap & Vars() { Init(); return m_instance->m_vars; }
	
	static const_iterator begin() { Init(); return m_instance->m_vars.begin(); }
	static const_iterator end() { Init(); return m_instance->m_vars.end(); }
	static const_iterator lower_bound(const std::string& name) { Init(); return m_instance->m_vars.lower_bound(name); }
	static const_iterator upper_bound(const std::string& name) { Init(); return m_instance->m_vars.upper_bound(name + std::string("\177\177\177")); }
	
	static RegisteredVar* GetVar( const std::string & name );	// Case-insensitive search, returns NULL on fail
	static RegisteredVar* GetVar( const std::string & name, ScriptVarType_t type );	// Case-insensitive search, returns NULL on fail
	template<typename T>
	static T* GetVarP(const std::string& name) {
		RegisteredVar* var = GetVar(name, GetType<T>());
		if(var) return getPointer<T>(var->var);
		return NULL;
	}
	
	static bool haveSomethingWith(const std::string& start) {
		CScriptableVars::const_iterator it = CScriptableVars::lower_bound(start);
		if(it == end()) return false;
		return strCaseStartsWith(it->first, start);
				
	}
	
	static std::string DumpVars();	// For debug output
	
	static void SetVarByString(const ScriptVarPtr_t& var, const std::string& str);

	// Convert "tLXOptions -> iNetworkPort " to "iNetworkPort", not used anywhere
	static std::string StripClassName( const std::string & c );

	// Allows registering vars with daisy-chaining
	class VarRegisterHelper
	{
		friend class CScriptableVars;

		VarMap& m_vars;	// Reference to CScriptableVars::m_vars
		std::string m_prefix;

		VarRegisterHelper( CScriptableVars * parent, const std::string & prefix ): 
			m_vars( parent->m_vars ), m_prefix(prefix) {}

		std::string Name( const std::string & c )
		{
			if( m_prefix != "" ) 
				return m_prefix + "." + c;
			return c;
		}

		public:

		operator bool () { return true; }	// To be able to write static expressions

		VarRegisterHelper & operator() ( bool & v, const std::string & c, bool def = false, 
											const std::string & descr = "", const std::string & descrLong = "", GameInfoGroup group = GIG_Invalid, AdvancedLevel level = ALT_Basic )
			{
				m_vars[Name(c)] = RegisteredVar(v, c, def, descr, descrLong, group, level);
				return *this; 
			}

		VarRegisterHelper & operator() ( int & v, const std::string & c, int def = 0, 
										const std::string & descr = "", const std::string & descrLong = "", GameInfoGroup group = GIG_Invalid, AdvancedLevel level = ALT_Basic,
										bool unsig = false, int minval = 0, int maxval = 0 )
			{
				m_vars[Name(c)] = RegisteredVar(v, c, def, descr, descrLong, group, level, unsig, minval, maxval);
				return *this; 
			}

		VarRegisterHelper & operator() ( float & v, const std::string & c, float def = 0.0f, 
											const std::string & descr = "", const std::string & descrLong = "", GameInfoGroup group = GIG_Invalid, AdvancedLevel level = ALT_Basic,
											bool unsig = false, float minval = 0.0f, float maxval = 0.0f )
			{
				m_vars[Name(c)] = RegisteredVar(v, c, def, descr, descrLong, group, level, unsig, minval, maxval);
				return *this; 
			}

		VarRegisterHelper & operator() ( std::string & v, const std::string & c, const char * def = "", 
											const std::string & descr = "", const std::string & descrLong = "", GameInfoGroup group = GIG_Invalid, AdvancedLevel level = ALT_Basic )
			{
				m_vars[Name(c)] = RegisteredVar(v, c, def, descr, descrLong, group, level);
				return *this; 
			}

		VarRegisterHelper & operator() ( Color & v, const std::string & c, Color def = Color(255,0,255), 
											const std::string & descr = "", const std::string & descrLong = "", GameInfoGroup group = GIG_Invalid, AdvancedLevel level = ALT_Basic )
			{
				m_vars[Name(c)] = RegisteredVar(v, c, def, descr, descrLong, group, level);
				return *this; 
			}

		VarRegisterHelper & operator() ( ScriptCallback_t v, const std::string & c, 
											const std::string & descr = "", const std::string & descrLong = "", GameInfoGroup group = GIG_Invalid, AdvancedLevel level = ALT_Basic )
			{
				m_vars[Name(c)] = RegisteredVar(v, c, descr, descrLong, group, level);
				return *this; 
			}
		
		VarRegisterHelper & operator() ( ScriptVar_t& v, const std::string & c, const ScriptVar_t& def, 
											const std::string & descr = "", const std::string & descrLong = "", GameInfoGroup group = GIG_Invalid, AdvancedLevel level = ALT_Basic,
											ScriptVar_t minval = ScriptVar_t(), ScriptVar_t maxval = ScriptVar_t(), bool unsignedValue = false )
			{ 
				m_vars[Name(c)] = RegisteredVar(v, c, def, descr, descrLong, group, level, minval, maxval, unsignedValue );
				return *this; 
			}
		
		template<typename T>
		VarRegisterHelper & operator() ( DynamicVar<T>* v, const std::string & c, const T& def,
										const std::string & descr = "", const std::string & descrLong = "", GameInfoGroup group = GIG_Invalid, AdvancedLevel level = ALT_Basic )
			{
				m_vars[Name(c)] = RegisteredVar(v, c, def, descr, descrLong, group, level);
				return *this; 
			}
		
	};

	static VarRegisterHelper RegisterVars( const std::string & base = "" )
	{
		Init();
		return VarRegisterHelper( m_instance, base );
	}

	// De-registers all variables with names starting with "base."
	static void DeRegisterVars( const std::string & base );

private:
	friend class VarRegisterHelper;

	static CScriptableVars * m_instance;
	VarMap m_vars;	// All in-game variables and callbacks	
};

#endif
