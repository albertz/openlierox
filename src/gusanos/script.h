#ifndef SCRIPT_H
#define SCRIPT_H

#include "resource_locator.h"
#include "luaapi/types.h"

#include <string>
#include <map>


class LuaContext;

class Script
{
public:
	Script()
	: lua(0)
	{
	}
	
	bool pushFunction(std::string const& name);
	
	LuaReference createFunctionRef(std::string const& name);
	
	static LuaReference functionFromString(std::string const& name);

	LuaContext* lua;
	std::string table;
	
private:
	std::map<std::string, LuaReference> cachedReferences;
};

struct LazyScript
{
	enum Type
	{
		FunctionName,
		Code
	};
	
	LazyScript();
	
	LazyScript(std::string const& data);
	
	~LazyScript();
	
	LazyScript& operator=(std::string const& data);
	
	LuaReference get();
	
	bool empty();
	
	void makeNil();

private:
	void free_();
	
	Type type;
	LuaReference cached;
	std::string data;
};

extern ResourceLocator<Script> scriptLocator;

#endif //SCRIPT_H
