/*
	OpenLieroX
	
	threadsafe variable
	
usage sample:
	define somewhere:
		ThreadVar<CMap> map; // includes a full instance of CMap
	some reader access:
		{	ThreadVar<CMap>::Reader mapR( map );
			mapR.get().something();
		} // mapR will go out of scope here and release the map automatically
	some writer access:
		{	ThreadVar<CMap>::Writer mapW( map );
			mapW.get().something();
		} // mapW will go out of scope here and release the map automatically
		
	code under LGPL
	by Albert Zeyer
*/

#ifndef __THREADVAR_H__
#define __THREADVAR_H__

#include "ReadWriteLock.h"

template< typename _T >
class ThreadVar {

private:
	_T data;
	ReadWriteLock locker;

public:
	ThreadVar() : data() {}
	ThreadVar(const _T& v) : data(v) {}
	
	class Reader {
	private:
		ThreadVar<_T>& tvar;
	public:
		Reader(ThreadVar<_T>& var) : tvar(var) { var.locker.startReadAccess(); }
		~Reader() { tvar.locker.endReadAccess(); }
		const _T& get() const { return tvar.data; }
	};

	class Writer {
	private:
		ThreadVar<_T>& tvar;
	public:
		Writer(ThreadVar<_T>& var) : tvar(var) { var.locker.startWriteAccess(); }
		~Writer() { tvar.locker.endWriteAccess(); }
		_T& get() { return tvar.data; }
		const _T& get() const { return tvar.data; }
	};
	
	struct WriteWrapper {
		ThreadVar& var;
		WriteWrapper(ThreadVar& v) : var(v) {}
		WriteWrapper& operator=(const _T& v) {
			Writer w(var);
			w.get() = v;
			return *this;
		}
	};	
	WriteWrapper write() { return WriteWrapper(*this); }
	
};

#endif
