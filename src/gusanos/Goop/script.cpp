#include "script.h"

#include <iostream>
#include "luaapi/context.h"
using std::string;
using std::cerr;
using std::endl;

ResourceLocator<Script> scriptLocator;

bool Script::pushFunction(std::string const& name)
{
	if(!lua)
		return false;
	
	lua_pushstring(*lua, table.c_str());
	lua_rawget(*lua, LUA_GLOBALSINDEX);
	if(lua_isnil(*lua, -1))
	{
		cerr << "Table " << table << " not found!" << endl;
		return false;
	}
	lua_pushstring(*lua, name.c_str());
	lua_rawget(*lua, -2);
	lua_replace(*lua, -2);
	
	return true;
}

LuaReference Script::createFunctionRef(std::string const& name)
{
	if(!lua)
		return LuaReference();
		
	std::map<std::string, LuaReference>::const_iterator i = cachedReferences.find(name);
	
	if(i != cachedReferences.end())
		return i->second; // Reference cached
		
	if(!pushFunction(name))
		return LuaReference();
		
	LuaReference ref = lua->createReference();
	cachedReferences[name] = ref; // Cache reference
	return ref;
}

LuaReference Script::functionFromString(std::string const& name)
{
	std::string::size_type p = name.find('.');
	if(p != std::string::npos)
	{
		Script* s = scriptLocator.load(name.substr(0, p));
		if(s)
		{
			return s->createFunctionRef(name.substr(p + 1, name.size() - p - 1));
		}
	}
	
	return LuaReference();
}

LazyScript::LazyScript()
: type(Code)
{
}

LazyScript::LazyScript(std::string const& data)
: data(data), type(Code)
{
	/* This is more of an optimization
	std::string::size_type dot = data.find('.');
	if(dot != std::string::npos)
	{
		for(size_t i = 0; i < data.size(); ++i)
		{
			if(!isalnum(data[i]) && data[i] != '_')
				return;
		}
		
		type = FunctionName;
	}*/
}

LuaReference LazyScript::get()
{
	if(!cached && !data.empty())
	{
		if(type == FunctionName)
			cached = Script::functionFromString(data);
		else if(type == Code)
		{
			lua.evalExpression("<inlined block>", data);
			cached = lua.createReference();
		}
		
		data.clear();
	}
	
	return cached;
}

void LazyScript::free_()
{
	if(cached)
	{
		lua.destroyReference(cached);
		cached.reset();
	}
}

LazyScript::~LazyScript()
{
	free_();
}

void LazyScript::makeNil()
{
	if(!cached)
		return;
	lua_pushnil(lua);
	lua.assignReference(cached);
}

LazyScript& LazyScript::operator=(std::string const& data)
{
	free_();
	this->data = data;
	return *this;
}

bool LazyScript::empty()
{
	return !cached && data.empty();
}
