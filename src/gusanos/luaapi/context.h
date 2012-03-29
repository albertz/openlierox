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

class LuaContext
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
		
		int operator()()
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
				m_ref.invalidate();
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
		lua_pushcfunction(*this, v);
		return *this;
	}
	
	LuaContext& push(lua_Number v)
	{
		lua_pushnumber(*this, v);
		return *this;
	}
		
	LuaContext& push(float v)
	{
		lua_pushnumber(*this, static_cast<lua_Number>(v));
		return *this;
	}
	
	LuaContext& push(char const* v)
	{
		lua_pushstring(*this, v);
		return *this;
	}
	
	LuaContext& push(std::string const& v)
	{
		push(v.c_str());
		return *this;
	}
	
	LuaContext& push(std::pair<char const*, char const*> v)
	{
		lua_pushlstring(*this, v.first, v.second - v.first);
		return *this;
	}
	
	LuaContext& push(LuaReference v)
	{
		v.push(*this);
		return *this;
	}
	
	LuaContext& push(LuaReferenceLazy r) {
		push(r.get(*this));
		return *this;
	}

	template<class T>
	LuaContext& push(FullReference<T> const& v)
	{
		T** i = (T **)lua_newuserdata_init (*this, sizeof(T *));
		*i = &v.x;
		push(v.metatable);
		
		lua_setmetatable(*this, -2);
		return *this;
	}
	
	LuaContext& push(bool v)
	{
		lua_pushboolean(*this, v);
		return *this;
	}
	
	LuaContext& push(int v)
	{
		lua_pushinteger(*this, static_cast<lua_Integer>(v));
		return *this;
	}
	
	LuaContext& push(long v)
	{
		lua_pushinteger(*this, static_cast<lua_Integer>(v));
		return *this;
	}
	
	LuaContext& push(unsigned int v)
	{
		lua_pushinteger(*this, static_cast<lua_Integer>(v));
		return *this;
	}
	
	LuaContext& push(unsigned long v)
	{
		lua_pushinteger(*this, static_cast<lua_Integer>(v));
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
		lua_newtable(*this);
		for(size_t n = 0; n < v.size(); ++n)
		{
			push(v[n]);
			lua_rawseti(*this, -2, n + 1);
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
		T** i = (T **)lua_newuserdata_init (*this, sizeof(T *));
		*i = &x;
		push(metatable);
		
		lua_setmetatable(*this, -2);
	}
	
	template<class T>
	void pushLightReference(T* x, LuaReference metatable)
	{
		lua_pushlightuserdata (*this, x);
		push(metatable);
		lua_setmetatable(*this, -2);
	}
		
	void* pushObject(size_t count) // Pops a metatable from the stack
	{
		void* p = lua_newuserdata_init (*this, count); // <metatable> <object>

		lua_insert(*this, -2); // <object> <metatable>
		lua_setmetatable(*this, -2);
		
		return p;
	}
	
	void* pushObject(LuaReference metatable, size_t count)
	{
		void* p = lua_newuserdata_init (*this, count);
		
		push(metatable);
		lua_setmetatable(*this, -2);
		
		return p;
	}
	
	LuaContext& pushvalue(int i)
	{
		lua_pushvalue(*this, i);
		return *this;
	}
	
	LuaContext& pushScriptVar(const ScriptVar_t& var);

	void pushError(const std::string& msg) {
		lua_pushstring(*this, msg.c_str());
		lua_error(*this);
	}

	LuaContext& rawgeti(int table, int key)
	{
		lua_rawgeti(*this, table, key);
		return *this;
	}
	
	LuaContext& rawseti(int table, int key)
	{
		lua_rawseti(*this, table, key);
		return *this;
	}
	
	int getLuaType(int idx) { return lua_type(*this, idx); }
	std::string getLuaTypename(int idx) { return lua_typename(*this, getLuaType(idx)); }

	const char* tostring(int i) { return lua_tostring(*this, i); }
	std::string convert_tostring(int i);
	bool tobool(int i) { return bool(lua_toboolean(*this, i)); }
	Result toScriptVar(int idx, ScriptVar_t& var);

	LuaContext& newtable()
	{
		lua_newtable(*this);
		return *this;
	}
	
	LuaContext& pop(int c = 1)
	{
		lua_settop(*this, (-1)-c);
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
			lua_rawgeti(*this, -1, n + 1);
			if(lua_isnil(*this, -1))
			{
				lua_settop(*this, -3); // Pop nil and table
				return;
			}
			//TODO: v.push_back(get<T>(-1));
			lua_settop(*this, -2); // Pop value
		}
	}
	
	void function(char const* name, lua_CFunction func, int upvalues = 0);
	
	void tableFunction(char const* name, lua_CFunction func);
	
	void tableSetField(int id)
	{
		lua_pushboolean(*this, true);
		lua_rawseti(*this, -2, id);
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
	
	LuaReference::Idx createReference(); // [-1,0]
	
	void assignReference(LuaReference::Idx ref); // [-1,0]
	void destroyReference(LuaReference::Idx ref); // [0,0]
	void pushReference(LuaReference::Idx ref); // [0,1]
	
	void serialize(std::ostream& s, int i);
	void deserialize(std::istream& s);
	void serializeT(std::ostream& s, int i, int indent = 0);
	
	void serialize(BitStream& s, int i);
	bool deserialize(BitStream& s);
	
	void regObject(char const* name)
	{
		lua_setfield(*this, LUA_REGISTRYINDEX, name);
	}
	
	void regObjectKeep(char const* name)
	{
		lua_pushvalue(*this, -1);
		regObject(name);
	}
	
	void pushRegObject(char const* name)
	{
		lua_getfield(*this, LUA_REGISTRYINDEX, name);
	}
	
	operator lua_State*() const {
		assert(weakRef.get());
		return weakRef.get();
	}
	operator bool() const { return weakRef; }

	bool operator==(const LuaContext& other) const { return weakRef == other.weakRef; }
	bool operator!=(const LuaContext& other) const { return !(*this == other); }

	void close();
	
	~LuaContext();
	
	WeakRef<lua_State> weakRef;
};

extern LuaContext luaIngame;
extern LuaContext luaGlobal;

void initLuaGlobal();
void quitLuaGlobal();

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
