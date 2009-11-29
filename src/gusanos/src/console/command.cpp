#include "command.h"

using namespace std;

///////////////////////////////IntVariable////////////////////////////////

//============================= LIFECYCLE ================================

Command::Command()
{
	m_func=NULL;
}

Command::~Command()
{
}

Command::Command(CallbackT const& func, CompleteCallbackT const& completeFunc)
: m_func(func), m_completeFunc(completeFunc)
{

}

//============================= INTERFACE ================================

string Command::invoke(std::list<std::string> const& args)
{
	return m_func(args);
}

std::string Command::completeArgument(int idx, std::string const& beginning)
{
	if(m_completeFunc)
	{
		return m_completeFunc(m_owner, idx, beginning);
	}
	else
		return beginning;
}
