#include <map>
#include <boost/shared_ptr.hpp>
#include "ClassInfo.h"
#include "Debug.h"
#include "util/macros.h"
#include "util/StaticVar.h"

bool ClassInfo::isTypeOf(ClassId id) const {
	if(id == this->id) return true;
	if(this->superClassId != ClassId(-1)) {
		const ClassInfo* superClassInfo = getClassInfo(this->superClassId);
		assert(superClassInfo != NULL); // if there is a superClassId, we always should have the ClassInfo
		return superClassInfo->isTypeOf(id);
	}
	return false;
}

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
