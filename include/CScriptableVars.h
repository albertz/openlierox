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
#include <boost/typeof/typeof.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits.hpp>

#include "Color.h"
#include "StringUtils.h"
#include "Ref.h"
#include "PreInitVar.h"
#include "util/CustomVar.h"
#include "CVec.h"
#include "util/WeakRef.h"
#include "CodeAttributes.h"
#include "StaticAssert.h"
#include "util/PODForClass.h"
#include "util/Result.h"


class BaseObject;
struct AttrDesc;


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
	SVT_INT32 = 1,
	SVT_UINT64 = 6,
	SVT_FLOAT = 2,
	SVT_STRING = 3,
	SVT_COLOR = 4,
	SVT_VEC2 = 5,
	SVT_CUSTOM = 20,
	SVT_CustomWeakRefToStatic = 21,
	SVT_CALLBACK,	// Cannot be referenced from XML files directly, only as string
	SVT_DYNAMIC
};

static const ScriptVarType_t SVT_INVALID = ScriptVarType_t(-1);


/*
`GetType<T>` is supposed to provide some info about `T`.
`GetType<T>::type` == `T`.
`GetType<T>::value` is the `static const ScriptVarType_t`.
`GetType<T>::defaultValue()` returns a new `type`.
`GetType<T>::constRef(const type& v)` returns a const-reference-like type to `v`. XXX: why is this useful.
*/
template<typename T> struct GetType;

template<typename T> struct _GetTypeSimple {
	typedef T type;
	static type defaultValue() { return T(); }
	static const type& constRef(const type& v) { return v; }
};

template<> struct GetType<bool> : _GetTypeSimple<bool> { static const ScriptVarType_t value = SVT_BOOL; };
template<> struct GetType<int32_t> : _GetTypeSimple<int32_t> { static const ScriptVarType_t value = SVT_INT32; };
template<> struct GetType<uint64_t> : _GetTypeSimple<uint64_t> { static const ScriptVarType_t value = SVT_UINT64; };
template<> struct GetType<float> : _GetTypeSimple<float> { static const ScriptVarType_t value = SVT_FLOAT; };
template<> struct GetType<std::string> : _GetTypeSimple<std::string> { static const ScriptVarType_t value = SVT_STRING; };
template<> struct GetType<Color> : _GetTypeSimple<Color> { static const ScriptVarType_t value = SVT_COLOR; };
template<> struct GetType<CVec> : _GetTypeSimple<CVec> { static const ScriptVarType_t value = SVT_VEC2; };
template<> struct GetType<CustomVar::Ref> {
	// This type owns the `CustomVar` itself!
	typedef CustomVar::Ref type;
	static const ScriptVarType_t value = SVT_CUSTOM;
	static type defaultValue() { return NullCustomVar().getRefCopy(); }
	static const type& constRef(const type& v) { return v; }
};


template<typename T>
struct CustomVarWeakRefType {
	typedef CustomVar::WeakRef type;
	static const ScriptVarType_t value = SVT_CustomWeakRefToStatic;
	static CustomVar::Ref defaultValue() { return T().getRefCopy(); }
	static CustomVar::WeakRef constRef(const T& v) { return v.thisRef.obj; }
};

struct StringType : GetType<std::string> {};

template<int N> struct _Num { char array[N]; };
template<int, typename T> struct _SelectType;

template<typename T> struct _SelectType<1,T> : _Num<1> { typedef CustomVarWeakRefType<T> type; };
template<typename T> static _SelectType<1,T> _selectType(T*, const CustomVar&) { return NULL; }

template<typename T> struct _SelectType<2,T> : _Num<2> { typedef StringType type; };
template<typename T> static _SelectType<2,T> _selectType(T*, const char*) { return NULL; }
template<typename T> static _SelectType<2,T> _selectType(T*, char[]) { return NULL; }

template<typename T> struct GetType : _SelectType<sizeof(_selectType((T*)NULL, *(T*)NULL).array), T>::type {};



template<typename T> T* PtrFromScriptVar(ScriptVar_t& v, T* dummy = NULL);
template<typename T> T CastScriptVarConst(const ScriptVar_t& s);

class ScriptVar_t
{
	template<typename T> friend T* PtrFromScriptVar(ScriptVar_t& v, T* dummy);
public:
	const ScriptVarType_t type;
	PIVar(bool,false) isUnsigned;  // True for values that are unsigned and where negative values mean infinity

private:
	union {
		bool b;
		int32_t i;
		uint64_t i_uint64;
		float f;
		PODForClass<std::string> str;
		PODForClass<Color> col;
		PODForClass<CVec> vec2;
		PODForClass<CustomVar::Ref> custom;
		PODForClass<CustomVar::WeakRef> customRef;
	};

public:
	// No callback here - we cannot assign callbacks to each other
	ScriptVar_t(): type(SVT_BOOL), b(false) {}
	ScriptVar_t( bool v ): type(SVT_BOOL), b(v) {}
	ScriptVar_t( int32_t v ): type(SVT_INT32), i(v) {}
	ScriptVar_t( uint64_t v ): type(SVT_UINT64), i_uint64(v) {}
	ScriptVar_t( float v ): type(SVT_FLOAT), f(v) {}
	ScriptVar_t( const std::string & v ): type(SVT_STRING) { str.init(v); }
	ScriptVar_t( const char* v ): type(SVT_STRING) { str.init(v); }
	ScriptVar_t( Color v ): type(SVT_COLOR) { col.init(v); }
	ScriptVar_t( CVec v ): type(SVT_VEC2) { vec2.init(v); }
	ScriptVar_t( const CustomVar::Ref& c ): type(SVT_CUSTOM) { custom.init(c); }
	ScriptVar_t( const CustomVar::WeakRef& c ): type(SVT_CustomWeakRefToStatic) { customRef.init(c); }

	ScriptVar_t( const ScriptVar_t& v ) : type(v.type), isUnsigned(v.isUnsigned) {
		switch(v.type) {
		case SVT_BOOL: b = v.b; break;
		case SVT_INT32: i = v.i; break;
		case SVT_UINT64: i_uint64 = v.i_uint64; break;
		case SVT_FLOAT: f = v.f; break;
		case SVT_STRING: str.init(v.str.get()); break;
		case SVT_COLOR: col.init(v.col.get()); break;
		case SVT_VEC2: vec2.init(v.vec2.get()); break;
		case SVT_CUSTOM: custom.init(v.custom); break;
		case SVT_CustomWeakRefToStatic: customRef.init(v.customRef); break;
		case SVT_CALLBACK:
		case SVT_DYNAMIC: assert(false);
		}
	}

	static ScriptVar_t FromType(ScriptVarType_t t) {
		switch(t) {
		case SVT_BOOL: return ScriptVar_t(false);
		case SVT_INT32: return ScriptVar_t(int32_t(0));
		case SVT_UINT64: return ScriptVar_t(uint64_t(0));
		case SVT_FLOAT: return ScriptVar_t(0.f);
		case SVT_STRING: return ScriptVar_t("");
		case SVT_COLOR: return ScriptVar_t(Color());
		case SVT_VEC2: return ScriptVar_t(CVec());
		case SVT_CUSTOM: return ScriptVar_t(NullCustomVar().getRefCopy());
		case SVT_CustomWeakRefToStatic:
		case SVT_CALLBACK:
		case SVT_DYNAMIC: assert(false);
		}
		assert(false);
		return ScriptVar_t();
	}

	~ScriptVar_t() {
		switch(type) {
		case SVT_STRING: str.uninit(); break;
		case SVT_COLOR: col.uninit(); break;
		case SVT_VEC2: vec2.uninit(); break;
		case SVT_CUSTOM: custom.uninit(); break;
		case SVT_CustomWeakRefToStatic: customRef.uninit(); break;
		default: break;
		}
	}

	operator bool() const { assert(type == SVT_BOOL); return b; }
	operator int32_t() const { assert(type == SVT_INT32); return i; }
	operator uint64_t() const { assert(type == SVT_UINT64); return i; }
	operator float() const { assert(type == SVT_FLOAT); return f; }
	operator std::string() const { assert(type == SVT_STRING); return str.get(); }
	operator Color() const { assert(type == SVT_COLOR); return col.get(); }
	operator CVec() const { assert(type == SVT_VEC2); return vec2.get(); }

	template <typename T> T castConst() const { return CastScriptVarConst<T>(*this); }

	template<typename T> T* as() { assert(isCustomType()); return dynamic_cast<T*> (customVar()); }
	template<typename T> const T* as() const { assert(isCustomType()); return dynamic_cast<const T*> (customVar()); }

	template<typename T> T* ptr();

	template<typename T>
	const T* ptr() const {
		assert(type == GetType<T>::value);
		return ((ScriptVar_t*)this)->ptr<T>();
	}

	CustomVar::WeakRef customVarRef() const {
		assert(type == SVT_CustomWeakRefToStatic);
		return customRef.get();
	}

	CustomVar* customVar() {
		switch(type) {
		case SVT_CUSTOM: return &custom.get().get();
		case SVT_CustomWeakRefToStatic: return dynamic_cast<CustomVar*>(customRef.get().get());
		default: assert(false);
		}
		return NULL;
	}
	const CustomVar* customVar() const {
		switch(type) {
		case SVT_CUSTOM: return &custom.get().get();
		case SVT_CustomWeakRefToStatic: return dynamic_cast<const CustomVar*>(customRef.get().get());
		default: assert(false);
		}
		return NULL;
	}
	bool isCustomType() const {
		return type == SVT_CUSTOM || type == SVT_CustomWeakRefToStatic;
	}

	bool operator==(const ScriptVar_t& var) const {
		if(isNumeric() && var.isNumeric()) return getNumber() == var.getNumber();
		if(isCustomType() && var.isCustomType()) {
			if(customVar() == NULL || var.customVar() == NULL)
				return customVar() == var.customVar();
			return *customVar() == *var.customVar();
		}
		if(var.type != type) return false;
		switch(type) {
		case SVT_BOOL: return b == var.b;
		case SVT_INT32: return i == var.i;
		case SVT_UINT64: return i_uint64 == var.i_uint64;
		case SVT_FLOAT: return f == var.f;
		case SVT_STRING: return str == var.str;
		case SVT_COLOR: return col == var.col;
		case SVT_VEC2: return vec2 == var.vec2;
		case SVT_CUSTOM:
		case SVT_CustomWeakRefToStatic:
		case SVT_CALLBACK:
		case SVT_DYNAMIC: assert(false);
		}
		return false;
	}
	bool operator!=(const ScriptVar_t& var) const { return !(*this == var); }
	bool operator<(const ScriptVar_t& var) const {
		if(isNumeric() && var.isNumeric()) return getNumber() < var.getNumber();
		if(isCustomType() && var.isCustomType()) {
			if(customVar() == NULL || var.customVar() == NULL)
				return customVar() < var.customVar();
			return *customVar() < *var.customVar();
		}
		if(var.type != type) return type < var.type;
		switch(type) {
		case SVT_BOOL: return b < var.b;
		case SVT_INT32: return i < var.i;
		case SVT_UINT64: return i_uint64 < var.i_uint64;
		case SVT_FLOAT: return f < var.f;
		case SVT_STRING: return str < var.str;
		case SVT_COLOR: return col < var.col;
		case SVT_VEC2: return vec2 < var.vec2;
		case SVT_CUSTOM:
		case SVT_CustomWeakRefToStatic:
		case SVT_CALLBACK:
		case SVT_DYNAMIC: assert(false);
		}
		return false;
	}
	
	bool isNumeric() const { return type == SVT_INT32 || type == SVT_UINT64 || type == SVT_FLOAT; }
	// TODO: float has the same size as int, so we should convert to double here to avoid data loss with big ints
	float getNumber() const {
		if(type == SVT_INT32) return (float)i;
		if(type == SVT_UINT64) return (float)i_uint64;
		if(type == SVT_FLOAT) return f;
		return 0.0f;
	}
	
	std::string toString() const;
	bool fromString( const std::string & str);
	friend std::ostream& operator<< (std::ostream& o, const ScriptVar_t& svt);

	bool toBool() const { return toFloat() != 0.f; }
	template<typename T> T toNumericType() const {
		switch(type) {
		case SVT_BOOL: return b ? (T)1 : (T)0;
		case SVT_INT32: return (T)i;
		case SVT_UINT64: return (T)i_uint64;
		case SVT_FLOAT: return (T)f;
		case SVT_STRING: return from_string<T>(str.get());
		case SVT_COLOR: return (T)col.get().getDefault();
		case SVT_VEC2: return (T)vec2.get().x; // everything else doesn't make sense
		case SVT_CUSTOM: return from_string<T>(toString());
		case SVT_CustomWeakRefToStatic: return from_string<T>(toString());
		case SVT_CALLBACK:
		case SVT_DYNAMIC: assert(false);
		}
		assert(false); return 0;
	}
	int toInt() const { return toNumericType<int>(); }
	uint64_t toUint64() const { return toNumericType<uint64_t>(); }
	float toFloat() const { return toNumericType<float>(); }
	Color toColor() const;
	CVec toVec2() const;

	ScriptVar_t& operator=(const ScriptVar_t& v) {
		this -> ~ScriptVar_t(); // uninit
		new (this) ScriptVar_t(v); // init again
		return *this;
	}

	template<typename T>
	ScriptVar_t& operator=(const T& v) {
		fromScriptVar(MaybeRef(v));
		return *this;
	}

	// Note: the difference to op=(ScriptVar) is that we keep the same type here
	Result fromScriptVar(const ScriptVar_t& v, bool tryCast = false, bool assertSuccess = true);

	// This is safe to use as long as `v` stays valid.
	// In most cases, it is still a copy.
	// But if it is a custom type, it will be a reference.
	template<typename T>
	static ScriptVar_t MaybeRef(const T& v) {
		return ScriptVar_t(GetType<T>::constRef(v));
	}
	static ScriptVar_t MaybeRef(const ScriptVar_t& v) { return v; }

	template<typename T>
	bool operator==(const T& v) const { return *this == MaybeRef(v); }
	template<typename T>
	bool operator!=(const T& v) const { return *this != MaybeRef(v); }
	template<typename T>
	bool operator<(const T& v) const { return *this < MaybeRef(v); }

};



template<> inline bool* PtrFromScriptVar(ScriptVar_t& v, bool*) { assert(v.type == SVT_BOOL); return (bool*)&v.b; }
template<> inline int32_t* PtrFromScriptVar(ScriptVar_t& v, int32_t*) { assert(v.type == SVT_INT32); return (int32_t*)&v.i; }
template<> inline uint64_t* PtrFromScriptVar(ScriptVar_t& v, uint64_t*) { assert(v.type == SVT_UINT64); return (uint64_t*)&v.i_uint64; }
template<> inline float* PtrFromScriptVar(ScriptVar_t& v, float*) { assert(v.type == SVT_FLOAT); return (float*)&v.f; }
template<> inline std::string* PtrFromScriptVar(ScriptVar_t& v, std::string*) { assert(v.type == SVT_STRING); return (std::string*)&v.str.get(); }
template<> inline Color* PtrFromScriptVar(ScriptVar_t& v, Color*) { assert(v.type == SVT_COLOR); return (Color*)&v.col.get(); }
template<> inline CVec* PtrFromScriptVar(ScriptVar_t& v, CVec*) { assert(v.type == SVT_VEC2); return (CVec*)&v.vec2.get(); }
template<> inline CustomVar::Ref* PtrFromScriptVar(ScriptVar_t& v, CustomVar::Ref*) { assert(v.type == SVT_CUSTOM); return (CustomVar::Ref*)&v.custom.get(); }

template<typename T> T* ScriptVar_t::ptr() {
	assert(type == GetType<T>::value);
	return PtrFromScriptVar<T>(*this, (T*)NULL);
}


template<typename T>
T _CastScriptVarConst(const ScriptVar_t& s, T*, typename boost::enable_if_c<(GetType<T>::value <= SVT_CUSTOM-1), T>::type*) {
	return (T) s;
}

template<typename T>
T _CastScriptVarConst(const ScriptVar_t& s, T*, typename boost::enable_if_c<boost::is_base_of<CustomVar,T>::value, T>::type*) {
	return *s.as<T>();
}

template<typename T> T CastScriptVarConst(const ScriptVar_t& s) { return _CastScriptVarConst(s, (T*)NULL, (T*)NULL); }


struct _DynamicVar {
	virtual ~_DynamicVar() {}
	virtual ScriptVarType_t type() = 0;
	virtual ScriptVar_t asScriptVar() = 0;
	virtual void fromScriptVar(const ScriptVar_t&) = 0;
	virtual const AttrDesc* getAttrDesc() const { return NULL; }
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
		void* voidPt;
		bool * b;	// Pointer to static var
		int32_t * i;
		uint64_t * i_uint64;
		float * f;
		std::string * s;
		Color * cl;
		CVec * vec2;
		CustomVar::Ref* custom;
		CustomVar* customRef;
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

	explicit ScriptVarPtr_t( ScriptVarType_t t, void * v, const ScriptVar_t& def )
	: type(t), isUnsigned(false), defaultValue(def)
	{ ptr.voidPt = v; }

	ScriptVarPtr_t( ScriptCallback_t v ) : type(SVT_CALLBACK), isUnsigned(false) { ptr.cb = v; }
	ScriptVarPtr_t( _DynamicVar* v, const ScriptVar_t& def ) : type(SVT_DYNAMIC), isUnsigned(false), defaultValue(def) { ptr.dynVar = v; }
	
	ScriptVarPtr_t( ScriptVar_t * v, const ScriptVar_t& def ) : type(v->type), isUnsigned(v->isUnsigned), defaultValue(def) {
		assert(v->type == def.type);
		switch(type) {
		case SVT_BOOL: ptr.b = v->ptr<bool>(); break;
		case SVT_INT32: ptr.i = v->ptr<int32_t>(); break;
		case SVT_UINT64: ptr.i_uint64 = v->ptr<uint64_t>(); break;
		case SVT_FLOAT: ptr.f = v->ptr<float>(); break;
		case SVT_STRING: ptr.s = v->ptr<std::string>(); break;
		case SVT_COLOR: ptr.cl = v->ptr<Color>(); break;
		case SVT_VEC2: ptr.vec2 = v->ptr<CVec>(); break;
		case SVT_CUSTOM: ptr.custom = v->ptr<CustomVar::Ref>(); break;
		case SVT_CustomWeakRefToStatic: ptr.customRef = v->customVar();
		default: assert(false);
		}
	}

	bool operator==(const ScriptVar_t* v) const {
		if(v == NULL) return ptr.b == NULL;
		if(v->type != type) return false;
		switch(type) {
		case SVT_BOOL: return ptr.b == v->ptr<bool>();
		case SVT_INT32: return ptr.i == v->ptr<int32_t>();
		case SVT_UINT64: return ptr.i_uint64 == v->ptr<uint64_t>();
		case SVT_FLOAT: return ptr.f == v->ptr<float>();
		case SVT_STRING: return ptr.s == v->ptr<std::string>();
		case SVT_COLOR: return ptr.cl == v->ptr<Color>();
		case SVT_VEC2: return ptr.vec2 == v->ptr<CVec>();
		case SVT_CUSTOM: return ptr.custom == v->ptr<CustomVar::Ref>();
		case SVT_CustomWeakRefToStatic: return ptr.customRef == v->customVar();
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

	CustomVar* customVar() const {
		switch(type) {
		case SVT_CUSTOM: return &ptr.custom->get();
		case SVT_CustomWeakRefToStatic: return ptr.customRef;
		default: assert(false);
		}
		return NULL;
	}
	
};

template<typename T> T* getPointer(ScriptVarPtr_t& varPtr) {
	if(varPtr.type != GetType<T>::value) return NULL;
	return __ScriptVarPtrRaw_ptr<T>(varPtr.ptr);
}


struct ClientRights;

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

	Result allowedToAccess(bool forWrite, const ClientRights& rights);
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
