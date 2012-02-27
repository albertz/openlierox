#include <map>
#include <boost/shared_ptr.hpp>
#include "ClassInfo.h"
#include "Debug.h"
#include "util/macros.h"
#include "util/StaticVar.h"

typedef std::map<ClassId, ClassInfo> Classes;
static StaticVar<Classes> classes;

const ClassInfo* getClassInfo(ClassId id) {
	Classes::iterator it = classes->find(id);
	if(it != classes->end()) return &it->second;
	return NULL;
}

void registerClass(const ClassInfo& c) {
	classes.get()[c.id] = c;
}

