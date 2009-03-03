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
		case SVT_COLOR: c = StrToCol(str).get(); break;
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
		case SVT_COLOR: *cl = StrToCol(str).get(); break;
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
		case SVT_COLOR: *cl = cldef; break;
		case SVT_DYNAMIC: {
			// TODO: this could be simpler if the default values would be just a ScriptVar_t
			switch(dynVar->type()) {
				case SVT_BOOL: ((DynamicVar<bool>*)dynVar)->set(bdef); break;
				case SVT_INT: ((DynamicVar<int>*)dynVar)->set(idef); break;
				case SVT_FLOAT: ((DynamicVar<float>*)dynVar)->set(fdef); break;
				case SVT_STRING: ((DynamicVar<std::string>*)dynVar)->set(sdef); break;
				case SVT_COLOR: ((DynamicVar<Color_t>*)dynVar)->set(cldef); break;
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

ScriptVarPtr_t CScriptableVars::GetVar( const std::string & name )
{
	Init();
	for( std::map< std::string, ScriptVarPtr_t > :: iterator it = m_instance->m_vars.begin();
			it != m_instance->m_vars.end(); it++ )
	{
		if( !stringcasecmp( it->first, name ) )
		{
			return it->second;
		}
	}
	return ScriptVarPtr_t();
}


ScriptVarPtr_t CScriptableVars::GetVar( const std::string & name, ScriptVarType_t type )
{
	Init();
	for( std::map< std::string, ScriptVarPtr_t > :: iterator it = m_instance->m_vars.begin();
			it != m_instance->m_vars.end(); it++ )
	{
		if( !stringcasecmp( it->first, name ) && it->second.type == type )
		{
			return it->second;
		}
	}
	return ScriptVarPtr_t();
}

void CScriptableVars::DeRegisterVars( const std::string & base )
{
	if( ! m_instance )
	{
		errors << "CScriptableVars::DeRegisterVars() - error, deregistering vars \"" << base << "\" after CScriptableVars were destroyed" << endl;
		return;
	}
	Init();
	for( std::map< std::string, ScriptVarPtr_t > :: iterator it = m_instance->m_vars.begin();
			it != m_instance->m_vars.end();  )
	{
		bool remove = false;
		if( !stringcasecmp( it->first, base ) )
			remove = true;
		if( stringcasefind( it->first, base ) == 0 && it->first.size() > base.size() )
			if( it->first[base.size()] == '.' )
				remove = true;
		if( remove )
		{
			m_instance->m_vars.erase( it );
			it = m_instance->m_vars.begin();
		}
		else
		{
			it++;
		}
	}
}

std::string CScriptableVars::DumpVars()
{
	Init();
	std::ostringstream ret;
	for( std::map< std::string, ScriptVarPtr_t > :: iterator i = m_instance->m_vars.begin();
			i != m_instance->m_vars.end(); i++ )
	{
		ret << i->first + ": ";
		switch( i->second.type == SVT_DYNAMIC ? i->second.dynVar->type() : i->second.type )
		{
			case SVT_BOOL: ret << "bool: "; break;
			case SVT_INT: ret << "int: "; break;
			case SVT_FLOAT: ret << "float: "; break;
			case SVT_STRING: ret << "string: "; break;
			case SVT_COLOR: ret << "color: "; break;
			case SVT_CALLBACK: ret << "callback: "; break;
			default: assert(false);
		}
		ret << i->second.toString();
		ret << "\n";
	}
	return ret.str();
}

void CScriptableVars::SetVarByString(const ScriptVarPtr_t& var, const std::string& str) 
{
	var.fromString(str);
}

std::string CScriptableVars::GetDescription( const std::string & name )
{
	Init();
	if( m_instance->m_descriptions.count(name) == 0 )
		return StripClassName(name);
	if( m_instance->m_descriptions.find(name)->second.first == "" )
		return StripClassName(name);
	return m_instance->m_descriptions.find(name)->second.first;
}

std::string CScriptableVars::GetLongDescription( const std::string & name )
{
	Init();
	if( m_instance->m_descriptions.count(name) == 0 )
		return StripClassName(name);
	if( m_instance->m_descriptions.find(name)->second.second == "" )
		return StripClassName(name);
	return m_instance->m_descriptions.find(name)->second.second;
}

bool CScriptableVars::GetMinMaxValues( const std::string & name, int * minVal, int * maxVal )
{
	Init();
	if( m_instance->m_minmax.count(name) == 0 )
		return false;
	if( m_instance->m_vars.find(name)->second.type != SVT_INT || 
		m_instance->m_minmax.find(name)->second.first == m_instance->m_minmax.find(name)->second.second )
		return false;
	*minVal = m_instance->m_minmax.find(name)->second.first.i;
	*maxVal = m_instance->m_minmax.find(name)->second.second.i;
	return true;
}

bool CScriptableVars::GetMinMaxValues( const std::string & name, float * minVal, float * maxVal )
{
	Init();
	if( m_instance->m_minmax.count(name) == 0 )
		return false;
	if( m_instance->m_vars.find(name)->second.type != SVT_FLOAT || 
		m_instance->m_minmax.find(name)->second.first == m_instance->m_minmax.find(name)->second.second )
		return false;
	*minVal = m_instance->m_minmax.find(name)->second.first.f;
	*maxVal = m_instance->m_minmax.find(name)->second.second.f;
	return true;
}

int CScriptableVars::GetGroup( const std::string & name )
{
	Init();
	if( m_instance->m_groups.count(name) == 0 )
		return -1;
	return m_instance->m_groups.find(name)->second;
}
