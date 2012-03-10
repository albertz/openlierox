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

class DynamicList : public CustomVar {
private:
	ScriptVarType_t typeId;
	std::vector<ScriptVar_t> list;

public:
	DynamicList();

	virtual ScriptVar_t defaultValue() const;
	virtual void reset(); // doesn't resize. just resets all to default
	virtual ScriptVarType_t type() const { return typeId; }
	virtual size_t size() const { return list.size(); }
	virtual bool canResize() const { return true; }
	virtual void resize(size_t s) { list.resize(s, ScriptVar_t::FromType(type())); }
	virtual void writeGeneric(size_t i, const ScriptVar_t& v) { assert(i < size()); list[i] = v; }
	virtual ScriptVar_t getGeneric(size_t i) const { assert(i < size()); return list[i]; }

	virtual CustomVar* copy() const;
	virtual void copyFrom(const CustomVar& o);
	virtual bool operator==(const CustomVar& o) const;
	virtual bool operator<(const CustomVar& o) const;
	virtual std::string toString() const;
	virtual bool fromString(const std::string & str);
	virtual Result toBytestream(CBytestream* bs, const CustomVar* diffTo) const;
	virtual Result fromBytestream(CBytestream* bs, bool expectDiffToDefault);
};

template<typename T, typename ImplType = std::vector<T> >
class List : public DynamicList {
public:
	static const ScriptVarType_t typeId = GetType<T>::value;
	typedef T value_type;
	typedef ImplType list_impl_type;

	virtual ScriptVar_t defaultValue() const { return ScriptVar_t(T()); }
	virtual ScriptVarType_t type() const { return typeId; }
	virtual size_t size() const { return list.size(); }
	virtual void resize(size_t s) { list.resize(s); }
	virtual void writeGeneric(size_t i, const ScriptVar_t& v) { assert(i < size()); list[i] = v.castConst<T>(); }
	virtual ScriptVar_t getGeneric(size_t i) const { assert(i < size()); return ScriptVar_t(list[i]); }

	T& write(size_t i) { assert(i < size()); return list[i]; }
	const T& get(size_t i) const { assert(i < size()); return list[i]; }

	T& operator[](size_t i) { return write(i); }
	const T& operator[](size_t i) const { return get(i); }

private:
	ImplType list;

public:
	virtual CustomVar* copy() const { return new List(*this); }

	virtual void copyFrom(const CustomVar& o) {
		const List* ol = dynamic_cast<const List*>(&o);
		assert(ol != NULL);
		*this = *ol;
	}
};

template<typename T, size_t Size>
class Array : public DynamicList {
public:
	static const ScriptVarType_t typeId = GetType<T>::value;
	typedef T value_type;

	virtual ScriptVar_t defaultValue() const { return ScriptVar_t(T()); }
	virtual ScriptVarType_t type() const { return typeId; }
	virtual size_t size() const { return Size; }
	virtual bool canResize() const { return false; }
	virtual void resize(size_t s) { assert(s == size()); /* we cannot resize a static array */ }
	virtual void writeGeneric(size_t i, const ScriptVar_t& v) { assert(i < size()); list[i] = v.castConst<T>(); }
	virtual ScriptVar_t getGeneric(size_t i) const { assert(i < size()); return ScriptVar_t(list[i]); }

	T& write(size_t i) { assert(i < size()); return list[i]; }
	const T& get(size_t i) const { assert(i < size()); return list[i]; }

private:
	T list[Size];

public:
	Array() {
		for(size_t i = 0; i < size(); ++i)
			list[i] = T();
	}

	virtual CustomVar* copy() const { return new Array(*this); }

	virtual void copyFrom(const CustomVar& o) {
		const Array* ol = dynamic_cast<const Array*>(&o);
		assert(ol != NULL);
		*this = *ol;
	}
};

#endif // OLX_LIST_H
