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
#include "StringUtils.h"

#include <iostream>
#include <sstream>

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
	if( ret.find(".") != std::string::npos )	// Leave only last part of name
		ret = ret.substr( ret.find(".") + 1 );
	if( ret.find("->") != std::string::npos )
		ret = ret.substr( ret.find("->") + 2 );
	ret = ret.substr( ret.find_first_not_of(" \t") );	// Strip spaces
	ret = ret.substr( 0, ret.find_last_not_of(" \t") + 1 );
	return ret;
};

CScriptableVars::ScriptVarPtr_t CScriptableVars::GetVar( const std::string & name )
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


CScriptableVars::ScriptVarPtr_t CScriptableVars::GetVar( const std::string & name, CScriptableVars::ScriptVarType_t type )
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
		};
		ret << "\n";
	};
	return ret.str();
};

void CScriptableVars::SetVarByString(const CScriptableVars::ScriptVarPtr_t& var, const std::string& str) 
{
	bool fail = false;
	if( var.b == NULL ) 
		return;
	std::string scopy = str; TrimSpaces(scopy); stringlwr(scopy);
	if( var.type == CScriptableVars::SVT_BOOL )
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
	else if( var.type == CScriptableVars::SVT_INT )
	{
		if( scopy.find_first_of("-0123456789") == 0 )
			*var.i = from_string<int>(scopy, fail);
		else {
			std::cout << "WARNING: " << str << " should be an integer in options.cfg but it isn't" << std::endl;
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
	else if( var.type == CScriptableVars::SVT_FLOAT )
		*var.f = from_string<float>(scopy, fail);
	else if( var.type == CScriptableVars::SVT_STRING )
		*var.s = str;
	else
		std::cout << "WARNING: Invalid var type " << var.type << " of \"" << str << "\" when loading config!" << std::endl;
	
	if(fail)
		std::cout << "WARNING: failed to convert " << str << " into format " << var.type << std::endl;
}


