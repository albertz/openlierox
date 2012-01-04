#include "alias.h"
#include "util/text.h"

using namespace std;

//============================= LIFECYCLE ================================

Alias::Alias()
{}

Alias::~Alias()
{}

Alias::Alias(const std::string &name, const std::string &action) :
ConsoleItem(false), m_name(name), m_action(action)
{}

//============================= INTERFACE ================================

string Alias::invoke(const std::list<std::string> &args)
{
	if (!m_action.empty())
	{
		m_owner->parseLine(m_action);
	}
	return "";
}

