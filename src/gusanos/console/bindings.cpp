#include "bindings.h"
#include "util/text.h"
#include "util/log.h"

/////////////////////////////BindTable//////////////////////////////////

BindTable::BindTable()
{	
}

BindTable::~BindTable()
{
}

void BindTable::bind(char key, const std::string &action)
{
	binding[(unsigned char)key].m_action = action;
}

std::string BindTable::getBindingAction(char key)
{
	return binding[(unsigned char)key].m_action;
}

char BindTable::getKeyForAction(std::string const& action)
{
	
	for(size_t i = 0; i < 256; ++i)
	{
		//DLOG(action << " == " << binding[i].m_action);
		if(istrCmp(action, binding[i].m_action))
			return static_cast<char>(i);
	}
	
	return -1;
}
