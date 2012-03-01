// OpenLieroX ClassInfo.h
// by Albert Zeyer, 17.02.12
// code under LGPL

#ifndef OLX_CLASSINFO_H
#define OLX_CLASSINFO_H

#include <boost/function.hpp>
#include <string>

struct BaseObject;

typedef uint16_t ClassId;

struct ClassInfo {
	ClassInfo() : id(-1), superClassId(-1) {}
	ClassId id;
	ClassId superClassId;
	std::string name;
	size_t memSize;
	boost::function<BaseObject*()> createInstance;

	bool isTypeOf(ClassId id) const;
};

const ClassInfo* getClassInfo(ClassId id);
void registerClass(const ClassInfo& c);

#define REGISTER_CLASS(name_, super_) \
static BaseObject* createInstance_ ## name_ () { \
	return new name_(); \
} \
static bool registerClass_ ## name_ () { \
	ClassInfo info; \
	info.id = LuaID<name_>::value; \
	info.superClassId = super_; \
	info.name = #name_; \
	info.memSize = sizeof(name_); \
	info.createInstance = createInstance_ ## name_; \
	registerClass(info); \
	return true; \
} \
static bool registerClass_ ## name_ ## _init = registerClass_ ## name_ ();


#endif // OLX_CLASSINFO_H
