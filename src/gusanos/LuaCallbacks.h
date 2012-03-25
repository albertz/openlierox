// Lua callbacks
// code under LGPL
// created 2012-03-25

#ifndef OLX_LUACALLBACKS_H
#define OLX_LUACALLBACKS_H

#include <string>
#include <vector>
#include <boost/function.hpp>
#include "gusanos/luaapi/types.h"
#include "gusanos/luaapi/context.h"
#include "util/macros.h"

#define C_LocalPlayer_ActionCount 8

struct LuaCallbackRef {
	LuaContext context;
	LuaReference ref;
	LuaCallbackRef(const LuaContext& ctx_, const LuaReference& ref_) : context(ctx_), ref(ref_) {}
	operator bool() const { return context && ref; }
};
typedef std::vector<LuaCallbackRef> LuaCallbackList;

struct LuaCallbackProxy {
	LuaCallbackList& callbacks;
	int nreturns;
	typedef boost::function<void(LuaContext&,int)> PostHandler;
	PostHandler postHandler;
	LuaCallbackProxy(LuaCallbackList& callbacks_, int nreturns_, PostHandler postHandler_)
		: callbacks(callbacks_), nreturns(nreturns_), postHandler(postHandler_) {}

	LuaCallbackProxy& root() { return *this; }

	template<typename T>
	static void _Exec(T& base) {
		foreach(f, base.root().callbacks) {
			if(!*f) continue;
			base.root()._pushFunc(*f);
			base._pushParams(f->context);
			int r = base._exec(*f, 0);
			if(base.root().postHandler)
				base.root().postHandler(f->context, r);
			f->context.pop(r);
		}
	}

	template<typename T, typename BaseT>
	struct WithParam {
		BaseT& base;
		const T& param;
		WithParam(BaseT& base_, const T& p_) : base(base_), param(p_) {}

		LuaCallbackProxy& root() { return base.root(); }
		int _exec(LuaCallbackRef& ref, int nparams) { return base._exec(ref, nparams + 1); }
		void _pushParams(LuaContext& context) {
			base._pushParams(context);
			context.push(param);
		}

		void operator()() { _Exec(*this); }

		template<typename T2>
		WithParam<T2,WithParam> operator()(const T2& p) {
			return WithParam<T2,WithParam>(*this, p);
		}
	};

	void _pushParams(LuaContext&) {}
	void _pushFunc(LuaCallbackRef& c);
	int _exec(LuaCallbackRef& ref, int nparams);

	void operator()() { _Exec(*this); }

	template<typename T>
	WithParam<T,LuaCallbackProxy> operator()(const T& p) {
		return WithParam<T,LuaCallbackProxy>(*this, p);
	}
};

struct LuaCallbacks
{
	enum Type
	{
		startup,
		exit,
		serverStart,
		serverStop,
		gamePrepare,
		gameBegin,
		gameEnd,
		gotoLobby,
		afterRender,
		afterUpdate,
		wormRender,
		viewportRender,
		wormDeath,
		wormRemoved,
		playerUpdate,
		playerInit,
		playerRemoved,
		playerNetworkInit,
		gameNetworkInit,
		gameEnded,
		localplayerEventAny,
		localplayerInit,
		localplayerEvent,
		networkStateChange = localplayerEvent+C_LocalPlayer_ActionCount,
		gameError,
		max
	};
	void bind(const LuaContext& ctx, std::string callback, LuaReference ref);
	LuaCallbackList callbacks[max];
	void cleanup();
};

extern LuaCallbacks luaCallbacks;

struct LuaCallbackProxyEnv {
	LuaCallbacks::Type t;
	LuaCallbackProxyEnv(LuaCallbacks::Type t_) : t(t_) {}
	LuaCallbackProxy call(int nreturns = 0, LuaCallbackProxy::PostHandler postHandler = NULL) {
		return LuaCallbackProxy(luaCallbacks.callbacks[t], nreturns, postHandler);
	}
};

#define LUACALLBACK(t_) LuaCallbackProxyEnv(LuaCallbacks::Type(LuaCallbacks::t_))

#endif // OLX_LUACALLBACKS_H
