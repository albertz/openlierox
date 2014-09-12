#include "group.h"

namespace OmfgGUI
{

LuaReference Group::metaTable;

int Group::classID()
{
	return Context::Group;
}

}
