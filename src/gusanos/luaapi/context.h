#ifndef LUA_CONTEXT_H
#define LUA_CONTEXT_H

extern "C"
{
#include "lua.h"
}

#include "gusanos/luaapi/types.h"
#include "CodeAttributes.h"
#include "util/WeakRef.h"
#include "util/Result.h"
#include <iostream>
#include <vector>
#include <string>
#include <istream>
#include <ostream>
#include <utility>
#include <map>
#include <assert.h>
#include <boost/function.hpp>

class BitStream;
struct CmdLineIntf;
class ScriptVar_t;

class LuaContext : DontCopyTag
{
public:
	struct FunctionProxy
	{
		FunctionProxy(LuaContext& context)
		: m_context(context)
		{
		}
		
		FunctionProxy const& operator()(char const* name, lua_CFunction func) const
		{
			m_context.function(name, func);
			return *this;
		}

		LuaContext& m_context;
	};
	
	struct TableFunctionProxy
	{
		TableFunctionProxy(LuaContext& context)
		: m_context(context)
		{
		}
		
		TableFunctionProxy const& operator()(char const* name, lua_CFunction func) const
		{
			m_context.tableFunction(name, func);
			return *this;
		}

		LuaContext& m_context;
	};
	
	struct TableItemProxy
	{
		TableItemProxy(LuaContext& context)
		: m_context(context)
		{
		}
		
		template<class T>
		TableItemProxy const& operator()(char const* name, T const& v) const
		{
			lua_pushstring(m_context, name);
			m_context.push(v);
			lua_rawset(m_context, -3);
			return *this;
		}

		LuaContext& m_context;
	};
	
	struct CallProxy
	{
		CallProxy(LuaContext& context, LuaReference ref, int returns)
		: m_context(context), m_ref(ref), m_params(0), m_returns(returns)
		{
			m_context.push(errorReport);
			m_context.push(ref);
		}
		
		template<class T>
		CallProxy& operator,(T const& p)
		{
			++m_params;
			m_context.push(p);
			return *this;
		}
		
		int operator()() const
		{
			if(lua_isnil(m_context, -m_params-1))
			{
				m_context.pop(m_params + 2);
				return 0;
			}
			
			int r = m_context.call(m_params, m_returns, -m_params-2);
			
			if(r < 0)
			{
				m_context.pop(1); // Pop error function
				//m_context.destroyReference(m_ref);
				lua_pushnil(m_context);
				m_context.assignReference(m_ref);
				return 0;
			}
			lua_remove(m_context, -m_returns-1);
			return m_returns;
		}

		LuaContext& m_context;
		LuaReference m_ref;
		int m_params;
		int m_returns;
	};
	
	template<class T>
	struct FullReference
	{
		FullReference(T& x_, LuaReference metatable_)
		: x(x_), metatable(metatable_)
		{
		}
		
		T& x;
		LuaReference metatable;
	};
	
	static int errorReport(lua_State* L);
	
	bool logOnce(std::ostream& str);
	void log(std::ostream& str);
	
	LuaContext();	
	LuaContext(lua_State* state_);
	
	void init();
	void reset();
	
	static const char * istreamChunkReader(lua_State *L, void *data, size_t *size);

	void load(std::string const& chunk, std::istream& stream);
	
	int evalExpression(std::string const& chunk, std::string const& data);
	int evalExpression(std::string const& chunk, std::istream& stream);

	int execCode(const std::string& data, CmdLineIntf& cli);

	/*
	void load(std::string const& chunk, istream& stream, string const& table);
	*/
	int call(int params = 0, int returns = 0, int errfunc = 0);
	
	LuaContext& push(lua_CFunction v)
	{
		lua_pushcfunction(m_State, v);
		return *this;
	}
	
	LuaContext& push(lua_Number v)
	{
		lua_pushnumber(m_State, v);
		return *this;
	}
		
	LuaContext& push(float v)
	{
		lua_pushnumber(m_State, static_cast<lua_Number>(v));
		return *this;
	}
	
	LuaContext& push(char const* v)
	{
		lua_pushstring(m_State, v);
		return *this;
	}
	
	LuaContext& push(std::string const& v)
	{
		push(v.c_str());
		return *this;
	}
	
	LuaContext& push(std::pair<char const*, char const*> v)
	{
		lua_pushlstring(m_State, v.first, v.second - v.first);
		return *this;
	}
	
	LuaContext& push(LuaReference v)
	{
		pushReference(v);
		return *this;
	}
	
	template<class T>
	LuaContext& push(FullReference<T> const& v)
	{
		T** i = (T **)lua_newuserdata_init (m_State, sizeof(T *));
		*i = &v.x;
		pushReference(v.metatable);
		
		lua_setmetatable(m_State, -2);
		return *this;
	}
	
	LuaContext& push(bool v)
	{
		lua_pushboolean(m_State, v);
		return *this;
	}
	
	LuaContext& push(int v)
	{
		lua_pushinteger(m_State, static_cast<lua_Integer>(v));
		return *this;
	}
	
	LuaContext& push(long v)
	{
		lua_pushinteger(m_State, static_cast<lua_Integer>(v));
		return *this;
	}
	
	LuaContext& push(unsigned int v)
	{
		lua_pushinteger(m_State, static_cast<lua_Integer>(v));
		return *this;
	}
	
	LuaContext& push(unsigned long v)
	{
		lua_pushinteger(m_State, static_cast<lua_Integer>(v));
		return *this;
	}
			
	template<class T>
	LuaContext& push(T* v)
	{
		v->pushLuaReference(*this);
		return *this;
	}
	
	template<class T>
	LuaContext& push(std::vector<T> const& v)
	{
		lua_newtable(m_State);
		for(size_t n = 0; n < v.size(); ++n)
		{
			push(v[n]);
			lua_rawseti(m_State, -2, n + 1);
		}
		return *this;
	}
	
	template<class T>
	static FullReference<T> fullReference(T& x, LuaReference metatable)
	{
		return FullReference<T>(x, metatable); 
	}
	
	template<class T>
	void pushFullReference(T& x, LuaReference metatable)
	{
		T** i = (T **)lua_newuserdata_init (m_State, sizeof(T *));
		*i = &x;
		pushReference(metatable);
		
		lua_setmetatable(m_State, -2);
	}
	
	template<class T>
	void pushLightReference(T* x, LuaReference metatable)
	{
		lua_pushlightuserdata (m_State, x);
		pushReference(metatable);
		lua_setmetatable(m_State, -2);
	}
		
	void* pushObject(size_t count) // Pops a metatable from the stack
	{
		void* p = lua_newuserdata_init (m_State, count); // <metatable> <object>

		lua_insert(m_State, -2); // <object> <metatable>
		lua_setmetatable(m_State, -2);
		
		return p;
	}
	
	void* pushObject(LuaReference metatable, size_t count)
	{
		void* p = lua_newuserdata_init (m_State, count);
		
		pushReference(metatable);
		lua_setmetatable(m_State, -2);
		
		return p;
	}
	
	LuaContext& pushvalue(int i)
	{
		lua_pushvalue(m_State, i);
		return *this;
	}
	
	LuaContext& pushScriptVar(const ScriptVar_t& var);

	void pushError(const std::string& msg) {
		lua_pushstring(*this, msg.c_str());
		lua_error(*this);
	}

	LuaContext& rawgeti(int table, int key)
	{
		lua_rawgeti(m_State, table, key);
		return *this;
	}
	
	LuaContext& rawseti(int table, int key)
	{
		lua_rawseti(m_State, table, key);
		return *this;
	}
	
	int getLuaType(int idx) { return lua_type(m_State, idx); }
	std::string getLuaTypename(int idx) { return lua_typename(m_State, getLuaType(idx)); }

	const char* tostring(int i) { return lua_tostring(m_State, i); }
	std::string convert_tostring(int i);
	bool tobool(int i) { return lua_toboolean(m_State, i); }
	Result toScriptVar(int idx, ScriptVar_t& var);

	LuaContext& newtable()
	{
		lua_newtable(m_State);
		return *this;
	}
	
	LuaContext& pop(int c = 1)
	{
		lua_settop(m_State, (-1)-c);
		return *this;
	}
	
	CallProxy call(LuaReference ref, int returns = 0)
	{
		return CallProxy(*this, ref, returns);
	}
	
	template<class T>
	void tableToVector(std::vector<T> const& v)
	{
		v.clear();
		for(size_t n = 1; ; ++n)
		{
			lua_rawgeti(m_State, -1, n + 1);
			if(lua_isnil(m_State, -1))
			{
				lua_settop(m_State, -3); // Pop nil and table
				return;
			}
			//TODO: v.push_back(get<T>(-1));
			lua_settop(m_State, -2); // Pop value
		}
	}
	
	void function(char const* name, lua_CFunction func, int upvalues = 0);
	
	void tableFunction(char const* name, lua_CFunction func);
	
	void tableSetField(int id)
	{
		lua_pushboolean(m_State, true);
		lua_rawseti(m_State, -2, id);
	}
	
	FunctionProxy functions()
	{
		return FunctionProxy(*this);
	}
	
	TableFunctionProxy tableFunctions()
	{
		return TableFunctionProxy(*this);
	}
	
	TableItemProxy tableItems()
	{
		return TableItemProxy(*this);
	}
	
	LuaReference createReference();
	
	void assignReference(LuaReference ref);
	
	void destroyReference(LuaReference ref);
	
	void pushReference(LuaReference ref);
	
	void serialize(std::ostream& s, int i);
	void deserialize(std::istream& s);
	void serializeT(std::ostream& s, int i, int indent = 0);
	
	void serialize(BitStream& s, int i);
	bool deserialize(BitStream& s);
	
	void regObject(char const* name)
	{
		lua_setfield(m_State, LUA_REGISTRYINDEX, name);
	}
	
	void regObjectKeep(char const* name)
	{
		lua_pushvalue(m_State, -1);
		regObject(name);
	}
	
	void pushRegObject(char const* name)
	{
		lua_getfield(m_State, LUA_REGISTRYINDEX, name);
	}
	
	operator lua_State*() const
	{
		return m_State;
	}
	
	void close();
	
	~LuaContext();
	
	WeakRef<lua_State> weakRef;

private:
	lua_State *m_State;
	//std::map<std::string, LuaReference> metaTables;
};

extern LuaContext lua;

#ifndef NDEBUG

struct AssertStack
{
	AssertStack(LuaContext const& c_)
	: c(c_)
	{
		stack = lua_gettop(c);
	}
	
	~AssertStack()
	{
		int newStack = lua_gettop(c);
		assert(stack == newStack);
	}
	
	LuaContext const& c;
	int stack;
};

#else

struct AssertStack
{
	AssertStack(LuaContext const& c_)
	{
	}
};

#endif

struct LuaCustomPrintScope : DontCopyTag {
	typedef boost::function<int(lua_State*)> Func;

	LuaContext& context;
	Func printFunc;
	LuaReference oldPrintFunc;

	LuaCustomPrintScope(LuaContext& context_, Func printFunc_);
	~LuaCustomPrintScope();
};

LuaCustomPrintScope::Func printFuncFromCLI(CmdLineIntf& cli);

#endif //LUA_CONTEXT_H
