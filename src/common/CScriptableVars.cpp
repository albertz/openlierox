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
};

std::string ScriptVarPtr_t::toString() const
{
	switch(type) {
		case SVT_BOOL: return to_string(*b);
		case SVT_INT: return to_string(*i);
		case SVT_FLOAT: return to_string(*f);
		case SVT_STRING: return *s;
		case SVT_COLOR: return ColToHex(*cl);
		default: assert(false); return "";
	}
};

bool ScriptVarPtr_t::fromString( const std::string & str) const
{
	switch(type) {
		case SVT_BOOL: *b = from_string<bool>(str); break;
		case SVT_INT: *i = from_string<int>(str); break;
		case SVT_FLOAT: *f = from_string<float>(str); break;
		case SVT_STRING: *s = str; break;
		case SVT_COLOR: *cl = StrToCol(str).get(); break;
		default: assert(false); return false;
	}
	return true;
};


CScriptableVars * CScriptableVars::m_instance = NULL;

CScriptableVars & CScriptableVars::Init()
{
	if( m_instance == NULL )
		m_instance = new CScriptableVars;
	return *m_instance;
};

void CScriptableVars::DeInit()
{
	if( CScriptableVars::m_instance != NULL )
	{
		delete CScriptableVars::m_instance;
		CScriptableVars::m_instance = NULL;
	};
};

std::string CScriptableVars::StripClassName( const std::string & c )
{
	std::string ret(c);
	if( ret.rfind(".") != std::string::npos )	// Leave only last part of name
		ret = ret.substr( ret.rfind(".") + 1 );
	return ret;
};

ScriptVarPtr_t CScriptableVars::GetVar( const std::string & name )
{
	Init();
	for( std::map< std::string, ScriptVarPtr_t > :: iterator it = m_instance->m_vars.begin();
			it != m_instance->m_vars.end(); it++ )
	{
		if( !stringcasecmp( it->first, name ) )
		{
			return it->second;
		};
	};
	return ScriptVarPtr_t();
};


ScriptVarPtr_t CScriptableVars::GetVar( const std::string & name, ScriptVarType_t type )
{
	Init();
	for( std::map< std::string, ScriptVarPtr_t > :: iterator it = m_instance->m_vars.begin();
			it != m_instance->m_vars.end(); it++ )
	{
		if( !stringcasecmp( it->first, name ) && it->second.type == type )
		{
			return it->second;
		};
	};
	return ScriptVarPtr_t();
};

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
		};
	};
};

std::string CScriptableVars::DumpVars()
{
	Init();
	std::ostringstream ret;
	for( std::map< std::string, ScriptVarPtr_t > :: iterator i = m_instance->m_vars.begin();
			i != m_instance->m_vars.end(); i++ )
	{
		ret << i->first + ": ";
		switch( i->second.type )
		{
			case SVT_BOOL: ret << "bool: " << *i->second.b; break;
			case SVT_INT: ret << "int: " << *i->second.i; break;
			case SVT_FLOAT: ret << "float: " << *i->second.f; break;
			case SVT_STRING: ret << "string: \"" << *i->second.s << "\""; break;
			case SVT_COLOR: ret << "color: " << itoa(*i->second.cl); break;
			case SVT_CALLBACK: ret << "callback: "; break;
		}
		ret << "\n";
	}
	return ret.str();
};

void CScriptableVars::SetVarByString(const ScriptVarPtr_t& var, const std::string& str) 
{
	bool fail = false;
	if( var.b == NULL ) 
		return;
	std::string scopy = str; TrimSpaces(scopy); stringlwr(scopy);
	if( var.type == SVT_BOOL )
	{
		if( scopy.find_first_of("-0123456789") == 0 )
			*var.b = from_string<int>(scopy, fail) != 0; // Some bools are actually ints in config file
		else {
			if(scopy == "true" || scopy == "yes")
				*var.b = true;
			else if(scopy == "false" || scopy == "no")
				*var.b = false;
			else
				fail = true;
		}
	}
	else if( var.type == SVT_INT )
	{
		if( scopy.find_first_of("-0123456789") == 0 )
			*var.i = from_string<int>(scopy, fail);
		else {
			warnings << str << " should be an integer in options.cfg but it isn't" << endl;
			// HACK: because sometimes there is a bool instead of an int in the config
			// TODO: is this still like this?
			if(scopy == "true" || scopy == "yes")
				*var.i = 1;
			else if(scopy == "false" || scopy == "no")
				*var.i = 0;
			else
				fail = true;
		}
	}
	else if( var.type == SVT_FLOAT )
		*var.f = from_string<float>(scopy, fail);
	else if( var.type == SVT_STRING )
		*var.s = str;
	else
		warnings << "Invalid var type " << var.type << " of \"" << str << "\" when loading config!" << endl;
	
	if(fail)
		warnings << "failed to convert " << str << " into format " << var.type << endl;
}

std::string CScriptableVars::GetDescription( const std::string & name )
{
	Init();
	if( m_instance->m_descriptions.count(name) == 0 )
		return StripClassName(name);
	if( m_instance->m_descriptions.find(name)->second.first == "" )
		return StripClassName(name);
	return m_instance->m_descriptions.find(name)->second.first;
};

std::string CScriptableVars::GetLongDescription( const std::string & name )
{
	Init();
	if( m_instance->m_descriptions.count(name) == 0 )
		return StripClassName(name);
	if( m_instance->m_descriptions.find(name)->second.second == "" )
		return StripClassName(name);
	return m_instance->m_descriptions.find(name)->second.second;
};

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
};

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
};

int CScriptableVars::GetGroup( const std::string & name )
{
	Init();
	if( m_instance->m_groups.count(name) == 0 )
		return -1;
	return m_instance->m_groups.find(name)->second;
};
