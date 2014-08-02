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
		case SVT_INT: return to_string(i);
		case SVT_FLOAT: return to_string(f);
		case SVT_STRING: return s;
		case SVT_COLOR: return ColToHex(c);
		default: assert(false); return "";
	}
}

bool ScriptVar_t::fromString( const std::string & str )
{
	switch(type) {
		case SVT_BOOL: b = from_string<bool>(str); break;
		case SVT_INT: i = from_string<int>(str); break;
		case SVT_FLOAT: f = from_string<float>(str); break;
		case SVT_STRING: s = str; break;
		case SVT_COLOR: c = StrToCol(str); break;
		default: assert(false); return false;
	}
	return true;
}

std::string ScriptVarPtr_t::toString() const
{
	switch(type) {
		case SVT_BOOL: return to_string(*b);
		case SVT_INT: return to_string(*i);
		case SVT_FLOAT: return to_string(*f);
		case SVT_STRING: return *s;
		case SVT_COLOR: return ColToHex(*cl);
		case SVT_DYNAMIC: return dynVar->asScriptVar().toString();
		case SVT_CALLBACK: return "callback(0x" + hex((long)cb) + ")";
		default: assert(false); return "";
	}
}

bool ScriptVarPtr_t::fromString( const std::string & _str) const {
	std::string str = _str; TrimSpaces(str);
	
	switch(type) {
		case SVT_BOOL: *b = from_string<bool>(str); break;
		case SVT_INT:
			// TODO: why is that here and not in ScriptVar_t::fromString ?
			if (isUnsigned && str.size() == 0)
				*i = -1; // Infinite
			else
				*i = from_string<int>(str); 
		break;
		case SVT_FLOAT: 
			// TODO: why is that here and not in ScriptVar_t::fromString ?
			if (isUnsigned && str.size() == 0)
				*f = -1;
			else
				*f = from_string<float>(str); 
		break;
		case SVT_STRING: *s = str; break;
		case SVT_COLOR: *cl = StrToCol(str); break;
		case SVT_DYNAMIC: {
			ScriptVar_t var = dynVar->asScriptVar();
			if(!var.fromString(str)) return false;
			dynVar->fromScriptVar(var);
			return true;
		}
		default: assert(false); return false;
	}
	return true;
}

void ScriptVarPtr_t::setDefault() const {
	switch(type) {
		case SVT_BOOL: *b = bdef; break;
		case SVT_INT: *i = idef; break;
		case SVT_FLOAT: *f = fdef; break;
		case SVT_STRING: *s = sdef; break;
		case SVT_COLOR: *cl = Color(cldef); break;
		case SVT_DYNAMIC: {
			// TODO: this could be simpler if the default values would be just a ScriptVar_t
			switch(dynVar->type()) {
				case SVT_BOOL: ((DynamicVar<bool>*)dynVar)->set(bdef); break;
				case SVT_INT: ((DynamicVar<int>*)dynVar)->set(idef); break;
				case SVT_FLOAT: ((DynamicVar<float>*)dynVar)->set(fdef); break;
				case SVT_STRING: ((DynamicVar<std::string>*)dynVar)->set(sdef); break;
				case SVT_COLOR: ((DynamicVar<Color>*)dynVar)->set(cldef); break;
				default: assert(false);
			}
			break;
		}
		default: assert(false);
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
		switch( i->second.var.type == SVT_DYNAMIC ? i->second.var.dynVar->type() : i->second.var.type )
		{
			case SVT_BOOL: ret << "bool: "; break;
			case SVT_INT: ret << "int: "; break;
			case SVT_FLOAT: ret << "float: "; break;
			case SVT_STRING: ret << "string: "; break;
			case SVT_COLOR: ret << "color: "; break;
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
