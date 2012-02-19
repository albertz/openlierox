#include <map>
#include <boost/shared_ptr.hpp>
#include "ClassInfo.h"

typedef std::map<ClassId, ClassInfo> Classes;
static boost::shared_ptr< Classes > classes;

static void initClassInfo() {
	if(!classes)
		classes.reset(new Classes());
}

const ClassInfo& getClassInfo(ClassId id) {
	initClassInfo();
	return (*classes)[id];
}

void registerClass(const ClassInfo& c) {
	initClassInfo();
	(*classes)[c.id] = c;
}
