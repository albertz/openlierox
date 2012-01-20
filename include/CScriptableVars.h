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
#include <boost/type_traits/alignment_of.hpp>

#include "Color.h"
#include "StringUtils.h"
#include "Ref.h"
#include "PreInitVar.h"
#include "util/CustomVar.h"
#include "CVec.h"
#include "CodeAttributes.h"
#include "StaticAssert.h"


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
	SVT_COLOR = 4,
	SVT_VEC2 = 5,
	SVT_CUSTOM = 10,
	SVT_CALLBACK,	// Cannot be referenced from XML files directly, only as string
	SVT_DYNAMIC
};

static const ScriptVarType_t SVT_INVALID = ScriptVarType_t(-1);

template<typename T> struct GetType;
template<> struct GetType<bool> { typedef bool type; static const ScriptVarType_t value = SVT_BOOL; };
template<> struct GetType<int> { typedef int type; static const ScriptVarType_t value = SVT_INT; };
template<> struct GetType<float> { typedef float type; static const ScriptVarType_t value = SVT_FLOAT; };
template<> struct GetType<std::string> { typedef std::string type; static const ScriptVarType_t value = SVT_STRING; };
template<> struct GetType<Color> { typedef Color type; static const ScriptVarType_t value = SVT_COLOR; };
template<> struct GetType<CVec> { typedef CVec type; static const ScriptVarType_t value = SVT_VEC2; };
template<> struct GetType<CustomVar::Ref> { typedef CustomVar::Ref type; static const ScriptVarType_t value = SVT_CUSTOM; };
template<> struct GetType<const char*> : GetType<std::string> {};


// workaround to warning: dereferencing type-punned pointer will break strict-aliasing rules
// also get the correctly aligned ptr
template<typename T>
T* pointer_cast_and_align(const char* p) {
	size_t p2 = (size_t)p + boost::alignment_of<T>::value;
	p2 -= p2 % boost::alignment_of<T>::value;
	return (T*)p2;
}

// Plain-old-data struct for non-POD classes/struct
// You must call init/uninit here yourself!
template<typename T>
struct PODForClass {
	char data[sizeof(T) + boost::alignment_of<T>::value];
	T& get() { return *pointer_cast_and_align<T>(data); }
	operator T&() { return get(); }
	const T& get() const { return *pointer_cast_and_align<T>(data); }
	operator const T&() const { return get(); }
	void init() { new (&get()) T; }
	void init(const T& v) { new (&get()) T(v); }
	void uninit() { get().~T(); }
	bool operator==(const T& o) const { return get() == o; }
	bool operator<(const T& o) const { return get() < o; }
};

// Helper wrapper for common var types (used in GUI skinning)
class ScriptVar_t
{
public:
	const ScriptVarType_t type;
	PIVar(bool,false) isUnsigned;  // True for values that are unsigned and where negative values mean infinity

private:
	union {
		bool b;
		int i;
		float f;
		PODForClass<std::string> str;
		PODForClass<Color> col;
		PODForClass<CVec> vec2;
		PODForClass<CustomVar::Ref> custom;
	};

public:
	// No callback here - we cannot assign callbacks to each other
	ScriptVar_t(): type(SVT_BOOL), b(false) {}
	ScriptVar_t( bool v ): type(SVT_BOOL), b(v) {}
	ScriptVar_t( int v ): type(SVT_INT), i(v) {}
	ScriptVar_t( float v ): type(SVT_FLOAT), f(v) {}
	ScriptVar_t( const std::string & v ): type(SVT_STRING) { str.init(v); }
	ScriptVar_t( const char* v ): type(SVT_STRING) { str.init(v); }
	ScriptVar_t( Color v ): type(SVT_COLOR) { col.init(v); }
	ScriptVar_t( CVec v ): type(SVT_VEC2) { vec2.init(v); }
	ScriptVar_t( const CustomVar& c ): type(SVT_CUSTOM) { custom.init(c.copy()); }

	ScriptVar_t( const ScriptVar_t& v ) : type(v.type), isUnsigned(v.isUnsigned) {
		switch(v.type) {
			case SVT_BOOL: b = v.b; break;
			case SVT_INT: i = v.i; break;
			case SVT_FLOAT: f = v.f; break;
			case SVT_STRING: str.init(v.str.get()); break;
			case SVT_COLOR: col.init(v.col.get()); break;
			case SVT_VEC2: vec2.init(v.vec2.get()); break;
			case SVT_CUSTOM: custom.init(v.custom); break;
			case SVT_CALLBACK:
			case SVT_DYNAMIC: assert(false);
		}
	}	

	ScriptVar_t& operator=(const ScriptVar_t& v) {
		this -> ~ScriptVar_t(); // uninit
		new (this) ScriptVar_t(v); // init again
		return *this;
	}

	~ScriptVar_t() {
		switch(type) {
			case SVT_STRING: str.uninit(); break;
			case SVT_COLOR: col.uninit(); break;
			case SVT_VEC2: vec2.uninit(); break;
			case SVT_CUSTOM: custom.uninit(); break;
			default: break;
		}		
	}

	operator bool() const { assert(type == SVT_BOOL); return b; }
	operator int() const { assert(type == SVT_INT); return i; }
	operator float() const { assert(type == SVT_FLOAT); return f; }
	operator std::string() const { assert(type == SVT_STRING); return str.get(); }
	operator Color() const { assert(type == SVT_COLOR); return col.get(); }
	operator CVec() const { assert(type == SVT_VEC2); return vec2.get(); }

	template<typename T>
	ScriptVar_t& operator=(const T& v) { fromScriptVar(ScriptVar_t(v)); return *this; }
	
	template<typename T> T* as() { assert(type == SVT_CUSTOM); return dynamic_cast<T*> (&custom.get().get()); }
	template<typename T> const T* as() const { assert(type == SVT_CUSTOM); return dynamic_cast<const T*> (&custom.get().get()); }

	template<typename T>
	T* ptr() { assert(type == GetType<T>::value); return (T*)&b; }

	template<typename T>
	const T* ptr() const { assert(type == GetType<T>::value); return (T*)&b; }

	CustomVar::Ref& ptrCustom() { assert(type == SVT_CUSTOM); return custom.get(); }
	const CustomVar::Ref& ptrCustom() const { assert(type == SVT_CUSTOM); return custom.get(); }

	bool operator==(const ScriptVar_t& var) const {
		if(var.type != type) return false;
		switch(type) {
			case SVT_BOOL: return b == var.b;
			case SVT_INT: return i == var.i;
			case SVT_FLOAT: return f == var.f;
			case SVT_STRING: return str == var.str;
			case SVT_COLOR: return col == var.col;
			case SVT_VEC2: return vec2 == var.vec2;
			case SVT_CUSTOM: return custom.get().get() == var.custom.get().get();
			case SVT_CALLBACK:
			case SVT_DYNAMIC: assert(false);
		}
		return false;
	}
	bool operator!=(const ScriptVar_t& var) const { return !(*this == var); }
	bool operator<(const ScriptVar_t& var) const {
		if(isNumeric() && var.isNumeric()) return getNumber() < var.getNumber();
		if(var.type != type) return type < var.type;
		switch(type) {
			case SVT_BOOL: return b < var.b;
			case SVT_INT: return i < var.i;
			case SVT_FLOAT: return f < var.f;
			case SVT_STRING: return str < var.str;
			case SVT_COLOR: return col < var.col;
			case SVT_VEC2: return vec2 < var.vec2;
			case SVT_CUSTOM: return custom.get().get() < var.custom.get().get();
			case SVT_CALLBACK:
			case SVT_DYNAMIC: assert(false);
		}
		return false;
	}
	
	bool isNumeric() const { return type == SVT_INT || type == SVT_FLOAT; }
	// TODO: float has the same size as int, so we should convert to double here to avoid data loss with big ints
	float getNumber() const { if(type == SVT_INT) return (float)i; if(type == SVT_FLOAT) return f; return 0.0f; }
	
	std::string toString() const;
	bool fromString( const std::string & str);
	friend std::ostream& operator<< (std::ostream& o, const ScriptVar_t& svt);

	bool toBool() const { return toInt() != 0; }
	int toInt() const {
		switch(type) {
			case SVT_BOOL: return b ? 1 : 0;
			case SVT_INT: return i;
			case SVT_FLOAT: return (int)f;
			case SVT_STRING: return from_string<int>(str.get());
			case SVT_COLOR: return (int)col.get().getDefault();
			case SVT_VEC2: return (int)vec2.get().x; // everything else doesn't make sense
			case SVT_CUSTOM: return from_string<int>(toString());
			case SVT_CALLBACK:
			case SVT_DYNAMIC: assert(false);
		}
		assert(false); return 0;
	}
	float toFloat() const {
		switch(type) {
			case SVT_BOOL: return b ? 1.0f : 0.0f;
			case SVT_INT: return (float)i;
			case SVT_FLOAT: return f;
			case SVT_STRING: return from_string<float>(str.get());
			case SVT_COLOR: return (float)col.get().getDefault();
			case SVT_VEC2: return vec2.get().x; // everything else doesn't make sense
			case SVT_CUSTOM: return from_string<float>(toString());
			case SVT_CALLBACK:
			case SVT_DYNAMIC: assert(false);
		}
		assert(false); return 0.0f;
	}
	Color toColor() const {
		switch(type) {
			case SVT_BOOL: return b ? Color(255,255,255) : Color();
			case SVT_INT: return Color::fromDefault((Uint32)i);
			case SVT_FLOAT: return Color::fromDefault((Uint32)f);
			case SVT_STRING: return StrToCol(str.get());
			case SVT_COLOR: return col.get();
			case SVT_VEC2: return Color::fromDefault((Uint32)toInt());
			case SVT_CUSTOM: return StrToCol(toString());
			case SVT_CALLBACK:
			case SVT_DYNAMIC: assert(false);
		}
		assert(false); return Color();
	}

	// Note: the difference to op= is that we keep the same type here
	void fromScriptVar(const ScriptVar_t& v) {
		switch(type) {
			case SVT_BOOL: assert(v.type == SVT_BOOL); b = v.b; break;
			case SVT_INT: assert(v.type == SVT_INT); i = v.i; break;
			case SVT_FLOAT: assert(v.isNumeric()); f = v.getNumber(); break;
			case SVT_STRING: str.get() = v.toString(); break;
			case SVT_COLOR: assert(v.type == SVT_COLOR); col.get() = v.col.get(); break;
			case SVT_VEC2: assert(v.type == SVT_VEC2); vec2.get() = v.vec2.get(); break;
			case SVT_CUSTOM: assert(v.type == SVT_CUSTOM); custom.get() = v.custom.get(); break;
			case SVT_CALLBACK:
			case SVT_DYNAMIC: assert(false);
		}
	}
};


struct _DynamicVar {
	virtual ~_DynamicVar() {}
	virtual ScriptVarType_t type() = 0;
	virtual ScriptVar_t asScriptVar() = 0;
	virtual void fromScriptVar(const ScriptVar_t&) = 0;
};

template < typename T >
struct DynamicVar : _DynamicVar {
	virtual ~DynamicVar() {}
	virtual ScriptVarType_t type() { return GetType<T>::value; }
	virtual T get() = 0;
	virtual void set(const T&) = 0;
	virtual ScriptVar_t asScriptVar() { return ScriptVar_t(get()); }
	virtual void fromScriptVar(const ScriptVar_t& var) { set(var); }
};


struct __ScriptVarPtrRaw {
	// we use union to save some memory
	union
	{
		bool * b;	// Pointer to static var
		int * i;
		float * f;
		std::string * s;
		Color * cl;
		CVec * vec2;
		CustomVar::Ref* custom;
		ScriptCallback_t cb;
		_DynamicVar* dynVar;
	};
	__ScriptVarPtrRaw() : b(NULL) {}
};

template<typename T> T*& __ScriptVarPtrRaw_ptr(__ScriptVarPtrRaw& v) { return (T*&)v.b; }

// Pointer to any in-game var - var should be global or static
struct ScriptVarPtr_t
{
	ScriptVarType_t type;
	bool isUnsigned;  // True for values that are unsigned and where negative values mean infinity
	__ScriptVarPtrRaw ptr;
	ScriptVar_t defaultValue;
	
	ScriptVarPtr_t() : type(SVT_INVALID), isUnsigned(false) {} // Invalid value initially (all pointer are NULL in union)
	
	template<typename T>
	ScriptVarPtr_t( T * v, const T& def = T() )
	: type(GetType<T>::value), isUnsigned(false), defaultValue(def)
	{ __ScriptVarPtrRaw_ptr<T>(ptr) = v; }
	
	ScriptVarPtr_t( ScriptCallback_t v ) : type(SVT_CALLBACK), isUnsigned(false) { ptr.cb = v; }
	ScriptVarPtr_t( _DynamicVar* v, const ScriptVar_t& def ) : type(SVT_DYNAMIC), isUnsigned(false), defaultValue(def) { ptr.dynVar = v; }
	
	ScriptVarPtr_t( ScriptVar_t * v, const ScriptVar_t& def ) : type(v->type), isUnsigned(v->isUnsigned), defaultValue(def) {
		assert(v->type == def.type);
		switch(type) {
			case SVT_BOOL:
			case SVT_INT:
			case SVT_FLOAT:
			case SVT_STRING:
			case SVT_COLOR:
			case SVT_VEC2: ptr.b = v->ptr<bool>(); break;
			case SVT_CUSTOM: ptr.custom = &v->ptrCustom(); break;
			default: assert(false);
		}
	}

	bool operator==(const ScriptVar_t* var) const {
		if(var == NULL) return ptr.b == NULL;
		if(var->type != type) return false;
		switch(type) {
			case SVT_BOOL:
			case SVT_INT:
			case SVT_FLOAT:
			case SVT_STRING:
			case SVT_COLOR:
			case SVT_VEC2: return ptr.b == var->ptr<bool>();
			case SVT_CUSTOM: return ptr.custom == &var->ptrCustom();
			default: assert(false);
		}
		return false;
	}

	bool operator==(const ScriptVarPtr_t& var) const {
		if(var.type != type) return false;
		return ptr.b == var.ptr.b; // asumes that all ptrs have same sizeof; should work because of union
	}

	bool operator==(const _DynamicVar* var) const {
		if(type != SVT_DYNAMIC) return false;
		return ptr.dynVar == var;
	}
	
	ScriptVarType_t valueType() const {
		if(type != SVT_DYNAMIC) return type;
		else return ptr.dynVar->type();
	}
	
	// These funcs will assert() if you try to call them on Callback varptr
	std::string toString() const;
	bool fromString( const std::string & str) const; // const 'cause we don't change pointer itself, only data it points to
	friend std::ostream& operator<< (std::ostream& o, const ScriptVarPtr_t& svt);
	
	// Note: It must be the same type when you call these.
	void setDefault() const { fromScriptVar(defaultValue); }
	ScriptVar_t asScriptVar() const;
	void fromScriptVar(const ScriptVar_t&) const;
	
};

template<typename T> T* getPointer(ScriptVarPtr_t& varPtr) {
	if(varPtr.type != GetType<T>::value) return NULL;
	return __ScriptVarPtrRaw_ptr<T>(varPtr.ptr);
}



struct RegisteredVar {
	ScriptVarPtr_t var;
	std::string shortDesc;
	std::string longDesc;
	ScriptVar_t min;
	ScriptVar_t max;
	GameInfoGroup group;
	AdvancedLevel advancedLevel;

	RegisteredVar() : group(GIG_Invalid) {}

	bool haveMinMax() const { return min.isNumeric() && max.isNumeric() && min < max; }
	
	template<typename T>
	RegisteredVar( T & v, const std::string & c, const T& def, 
				  const std::string & descr, const std::string & descrLong,
				  GameInfoGroup g, AdvancedLevel l,
				  bool unsig, const T& minval, const T& maxval)
	{
		var = ScriptVarPtr_t( &v, def );
		shortDesc = descr;
		longDesc = descrLong;
		group = g;
		advancedLevel = l;
		var.isUnsigned = unsig;
		min = ScriptVar_t(minval);
		max = ScriptVar_t(maxval);
	}

	RegisteredVar( _DynamicVar* v, const std::string & c, const ScriptVar_t& def, 
				  const std::string & descr, const std::string & descrLong,
				  GameInfoGroup g, AdvancedLevel l,
				  bool unsig, const ScriptVar_t& minval, const ScriptVar_t& maxval)
	{ 
		var = ScriptVarPtr_t( v, def );
		shortDesc = descr;
		longDesc = descrLong;
		group = g;
		advancedLevel = l;
		var.isUnsigned = unsig;
		min = minval;
		max = maxval;
	}
		
	RegisteredVar( ScriptCallback_t v, const std::string & c)
	{
		var = ScriptVarPtr_t( v );
		group = GIG_Invalid;
		advancedLevel = ALT_Basic;
	}


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
		RegisteredVar* var = GetVar(name, GetType<T>::value);
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
		
		template<typename T, typename Str, typename DefT>
		VarRegisterHelper & operator() ( T & v, const Str & c, const DefT& def, 
										const std::string & descr = "", const std::string & descrLong = "", GameInfoGroup group = GIG_Invalid, AdvancedLevel level = ALT_Basic,
										bool unsig = false, const T& minval = T(), const T& maxval = T() )
		{
			m_vars[Name(c)] = RegisteredVar(v, std::string(c), T(def), descr, descrLong, group, level, unsig, minval, maxval);
			return *this; 
		}

		template<typename T, typename Str>
		VarRegisterHelper & operator() ( T & v, const Str & c)
		{
			return (*this)(v, c, T());
		}
		
		VarRegisterHelper& operator() (ScriptCallback_t cb, const std::string& c) {
			m_vars[Name(c)] = RegisteredVar(cb, c);
			return *this;
		}
		
		VarRegisterHelper & operator() ( _DynamicVar* v, const std::string & c, const ScriptVar_t& def,
										const std::string & descr = "", const std::string & descrLong = "", GameInfoGroup group = GIG_Invalid, AdvancedLevel level = ALT_Basic,
										bool unsig = false, const ScriptVar_t& minval = ScriptVar_t(), const ScriptVar_t& maxval = ScriptVar_t())
			{
				m_vars[Name(c)] = RegisteredVar(v, c, def, descr, descrLong, group, level, unsig, minval, maxval);
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
