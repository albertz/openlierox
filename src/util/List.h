/* OpenLieroX
 * List as a serializeable CustomVar
 * code under LGPL
 * by Albert Zeyer, 2012-03-08
 */

#ifndef OLX_LIST_H
#define OLX_LIST_H

#include <vector>
#include <string>
#include <assert.h>
#include "util/CustomVar.h"
#include "CScriptableVars.h"

template<typename T, typename ImplType = std::vector<T> >
class List : CustomVar {
public:
	static const ScriptVarType_t typeId = GetType<T>::value;
	typedef T type;
	typedef ImplType impl_type;

	size_t size() const { return list.size(); }
	void resize(size_t s) { list.resize(s); }
	T& write(size_t i) { assert(i < size()); return list[i]; }
	const T& get(size_t i) const { assert(i < size()); return list[i]; }

private:
	ImplType list;

public:
	virtual CustomVar* copy() const { return new List(*this); }
	virtual bool operator==(const CustomVar& o) const {
		const List* ol = dynamic_cast<const List*>(&o);
		if(ol == NULL) return false;
		return list == ol->list;
	}
	virtual bool operator<(const CustomVar& o) const {
		const List* ol = dynamic_cast<const List*>(&o);
		if(ol == NULL) return this < &o;
		return list < ol->list;
	}
	virtual std::string toString() const {
		std::string r = "[";
		for(typename ImplType::const_iterator i = list.begin(); i != list.end(); ++i) {
			if(i != list.begin()) r += ", ";
			r += ScriptVar_t(*i).toString();
		}
		r += "]";
		return r;
	}
	virtual bool fromString( const std::string & str) { return false; }

	virtual void copyFrom(const CustomVar& o) {
		const List* ol = dynamic_cast<const List*>(&o);
		assert(ol != NULL);
		*this = *ol;
	}

	virtual Result toBytestream( CBytestream* bs, const CustomVar* diffTo ) const { return true; }
	virtual Result fromBytestream( CBytestream* bs, bool expectDiffToDefault ) { return true; }
};

#endif // OLX_LIST_H
