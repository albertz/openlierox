/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#include "CScriptableVars.h"
#include "Debug.h"
#include "StringUtils.h"
#include "StaticAssert.h"
#include "CVec.h"

#include <sstream>



const char * GameInfoGroupDescriptions[][2] =
{
{"General", "General game options"},
{"Advanced", "Advanced game options"},
{"Scores", "Scoreboard related game options"},
{"Weapons", "Weapons and game physics related game options"},
{"Bonuses", "Bonuses related game options"},
{"Other", "Other game options"},
{"", ""}, // GIG_GameModeSpecific_Start - dummy value
{"Tag", "Tag gamemode settings"},
{"Hide and Seek", "Hide and Seek gamemode settings"},
{"Capture The Flag", "Capture The Flag gamemode settings"},
{"Race", "Race gamemode settings"},
};

static_assert( sizeof(GameInfoGroupDescriptions) / (sizeof(char*) * 2) == GIG_Size, GIG_desc__sizecheck );


std::string AdvancedLevelDescription(AdvancedLevel l) {
	switch(l) {
		case ALT_Basic: return "Basic settings.";
		case ALT_Advanced: return "Advanced settings. For more professional players.";
		case ALT_VeryAdvanced: return "Very advanced settings. For people who like to try out special unusual settings.";
		case ALT_Dev: return "Development features. Some of them can be unstable or are incomplete yet. Please report errors if you see any.";
		case ALT_DevKnownUnstable: return "Unstable development features. These features are known to be unstable. You have been warned.";
		case ALT_OnlyViaConfig: return "ONLY VIA CONFIG.";
		case __AdvancedLevelType_Count: return "INVALID BOTTOM ADVANCED LEVEL MARKER";
	}
	
	return "INVALID ADVANCED LEVEL VAR";
}

std::string AdvancedLevelShortDescription(AdvancedLevel l) {
	switch(l) {
		case ALT_Basic: return "Basic";
		case ALT_Advanced: return "Advanced";
		case ALT_VeryAdvanced: return "Very advanced";
		case ALT_Dev: return "Development";
		case ALT_DevKnownUnstable: return "Unstable";
		case ALT_OnlyViaConfig: return "CONFIG.";
		case __AdvancedLevelType_Count: return "INVALID";
	}
	
	return "INVALID";
}



std::string ScriptVar_t::toString() const {
	switch(type) {
	case SVT_BOOL: return to_string(b);
	case SVT_INT32: return to_string(i);
	case SVT_UINT64: return to_string(i_uint64);
	case SVT_FLOAT: return to_string(f);
	case SVT_STRING: return str.get();
	case SVT_COLOR: return ColToHex(col.get());
	case SVT_VEC2: return to_string(vec2.get());
	case SVT_CUSTOM: return custom.get().get().toString();
	case SVT_CustomWeakRefToStatic:
		assert(customVar() != NULL);
		return customVar()->toString();
	default: assert(false); return "";
	}
}

bool ScriptVar_t::fromString( const std::string & s )
{
	switch(type) {
	case SVT_BOOL: b = from_string<bool>(s); break;
	case SVT_INT32: i = from_string<int>(s); break;
	case SVT_FLOAT: f = from_string<float>(s); break;
	case SVT_STRING: str.get() = s; break;
	case SVT_COLOR: col.get() = StrToCol(s); break;
	case SVT_VEC2: vec2.get() = from_string<CVec>(s); break;
	case SVT_CUSTOM: custom.get().get().fromString(s); break;
	case SVT_CustomWeakRefToStatic:
		assert(customVar() != NULL);
		customVar()->fromString(s);
		break;
	default: assert(false); return false;
	}
	return true;
}

std::string ScriptVarPtr_t::toString() const
{
	switch(type) {
	case SVT_BOOL: return to_string(*ptr.b);
	case SVT_INT32: return to_string(*ptr.i);
	case SVT_UINT64: return to_string(*ptr.i_uint64);
	case SVT_FLOAT: return to_string(*ptr.f);
	case SVT_STRING: return *ptr.s;
	case SVT_COLOR: return ColToHex(*ptr.cl);
	case SVT_VEC2: return to_string(*ptr.vec2);
	case SVT_CUSTOM: return ptr.custom->get().toString();
	case SVT_CustomWeakRefToStatic: return ptr.customRef->toString();
	case SVT_DYNAMIC: return ptr.dynVar->asScriptVar().toString();
	case SVT_CALLBACK: return "callback(0x" + hex((long)ptr.cb) + ")";
	default: assert(false); return "";
	}
}

bool ScriptVarPtr_t::fromString( const std::string & _str) const {
	std::string str = _str; TrimSpaces(str);
	
	switch(type) {
	case SVT_BOOL: *ptr.b = from_string<bool>(str); break;
	case SVT_INT32:
		// TODO: why is that here and not in ScriptVar_t::fromString ?
		if (isUnsigned && str.size() == 0)
			*ptr.i = -1; // Infinite
		else
			*ptr.i = from_string<int32_t>(str);
		break;
	case SVT_UINT64:
		// TODO: why is that here and not in ScriptVar_t::fromString ?
		*ptr.i_uint64 = from_string<uint64_t>(str);
		break;
	case SVT_FLOAT:
		// TODO: why is that here and not in ScriptVar_t::fromString ?
		if (isUnsigned && str.size() == 0)
			*ptr.f = -1;
		else
			*ptr.f = from_string<float>(str);
		break;
	case SVT_STRING: *ptr.s = str; break;
	case SVT_COLOR: *ptr.cl = StrToCol(str); break;
	case SVT_VEC2: *ptr.vec2 = from_string<CVec>(str); break;
	case SVT_CUSTOM: return ptr.custom->get().fromString(_str);
	case SVT_CustomWeakRefToStatic: return ptr.customRef->fromString(_str);
	case SVT_DYNAMIC: {
		ScriptVar_t var = ptr.dynVar->asScriptVar();
		if(!var.fromString(str)) return false;
		ptr.dynVar->fromScriptVar(var);
		return true;
	}
	default: assert(false); return false;
	}
	return true;
}

ScriptVar_t ScriptVarPtr_t::asScriptVar() const {
	switch(type) {
	case SVT_BOOL: return ScriptVar_t(*ptr.b);
	case SVT_INT32: return ScriptVar_t(*ptr.i);
	case SVT_UINT64: return ScriptVar_t(*ptr.i_uint64);
	case SVT_FLOAT: return ScriptVar_t(*ptr.f);
	case SVT_STRING: return ScriptVar_t(*ptr.s);
	case SVT_COLOR: return ScriptVar_t(*ptr.cl);
	case SVT_VEC2: return ScriptVar_t(*ptr.vec2);
	case SVT_CUSTOM: return ScriptVar_t(ptr.custom->get());
	case SVT_CustomWeakRefToStatic: return ScriptVar_t(*ptr.customRef);
	case SVT_DYNAMIC: return ptr.dynVar->asScriptVar();
	case SVT_CALLBACK: assert(false);
	}
	return ScriptVar_t();
}

void ScriptVarPtr_t::fromScriptVar(const ScriptVar_t& v) const {
	switch(type) {
	case SVT_BOOL: *ptr.b = v; break;
	case SVT_INT32: *ptr.i = v; break;
	case SVT_UINT64: *ptr.i_uint64 = v; break;
	case SVT_FLOAT: *ptr.f = v; break;
	case SVT_STRING: *ptr.s = v.toString(); break;
	case SVT_COLOR: *ptr.cl = v; break;
	case SVT_VEC2: *ptr.vec2 = v; break;
	case SVT_CUSTOM:
	case SVT_CustomWeakRefToStatic: customVar()->fromScriptVar(v); break;
	case SVT_DYNAMIC: ptr.dynVar->fromScriptVar(v); break;
	case SVT_CALLBACK: assert(false);
	}
}


CScriptableVars * CScriptableVars::m_instance = NULL;

CScriptableVars & CScriptableVars::Init()
{
	if( m_instance == NULL )
		m_instance = new CScriptableVars;
	return *m_instance;
}

void CScriptableVars::DeInit()
{
	if( CScriptableVars::m_instance != NULL )
	{
		delete CScriptableVars::m_instance;
		CScriptableVars::m_instance = NULL;
	}
}

std::string CScriptableVars::StripClassName( const std::string & c )
{
	std::string ret(c);
	if( ret.rfind(".") != std::string::npos )	// Leave only last part of name
		ret = ret.substr( ret.rfind(".") + 1 );
	return ret;
}

RegisteredVar* CScriptableVars::GetVar( const std::string & name )
{
	Init();
	VarMap::iterator it = m_instance->m_vars.find(name);
	if(it != m_instance->m_vars.end()) return &it->second;
	return NULL;
}


RegisteredVar* CScriptableVars::GetVar( const std::string & name, ScriptVarType_t type )
{
	RegisteredVar* var = GetVar(name);
	if(!var) return NULL;
	if(var->var.type != type) return NULL;
	return var;
}

void CScriptableVars::DeRegisterVars( const std::string & base )
{
	if( ! m_instance )
	{
		errors << "CScriptableVars::DeRegisterVars() - error, deregistering vars \"" << base << "\" after CScriptableVars were destroyed" << endl;
		return;
	}
	Init();
	VarMap::iterator upper = m_instance->m_vars.upper_bound(base + ".\177\177\177");
	for( VarMap::iterator it = m_instance->m_vars.lower_bound(base + "."); it != upper;  )
	{
		VarMap::iterator cp = it; ++it;
		m_instance->m_vars.erase( cp );
	}
}

std::string CScriptableVars::DumpVars()
{
	Init();
	std::ostringstream ret;
	for( VarMap :: iterator i = m_instance->m_vars.begin();
			i != m_instance->m_vars.end(); i++ )
	{
		ret << i->first + ": ";
		switch( i->second.var.type == SVT_DYNAMIC ? i->second.var.ptr.dynVar->type() : i->second.var.type )
		{
		case SVT_BOOL: ret << "bool: "; break;
		case SVT_INT32: ret << "int32: "; break;
		case SVT_UINT64: ret << "uint64: "; break;
		case SVT_FLOAT: ret << "float: "; break;
		case SVT_STRING: ret << "string: "; break;
		case SVT_COLOR: ret << "color: "; break;
		case SVT_CUSTOM: ret << "custom: "; break;
		case SVT_CustomWeakRefToStatic: ret << "static custom: "; break;
		case SVT_CALLBACK: ret << "callback: "; break;
		default: assert(false);
		}
		ret << i->second.var.toString();
		ret << "\n";
	}
	return ret.str();
}

void CScriptableVars::SetVarByString(const ScriptVarPtr_t& var, const std::string& str) 
{
	var.fromString(str);
}

std::ostream& operator<< (std::ostream& o, const ScriptVar_t& svt)
{
	o << svt.toString();
	return o;
}

std::ostream& operator<< (std::ostream& o, const ScriptVarPtr_t& svt)
{
	o << svt.toString();
	return o;
}
