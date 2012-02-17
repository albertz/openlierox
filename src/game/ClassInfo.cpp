#include <map>
#include "ClassInfo.h"

static std::map<ClassId, ClassInfo> classes;

const ClassInfo& getClassInfo(ClassId id) {
	return classes[id];
}

void registerClass(const ClassInfo& c) {
	classes[c.id] = c;
}
