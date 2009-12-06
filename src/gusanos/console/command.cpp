#include "command.h"

using namespace std;

///////////////////////////////IntVariable////////////////////////////////

//============================= LIFECYCLE ================================

GusCommand::GusCommand()
{
	m_func=NULL;
}

GusCommand::~GusCommand()
{
}

GusCommand::GusCommand(CallbackT const& func, CompleteCallbackT const& completeFunc)
: m_func(func), m_completeFunc(completeFunc)
{

}

//============================= INTERFACE ================================

string GusCommand::invoke(std::list<std::string> const& args)
{
	return m_func(args);
}

std::string GusCommand::completeArgument(int idx, std::string const& beginning)
{
	if(m_completeFunc)
	{
		return m_completeFunc(m_owner, idx, beginning);
	}
	else
		return beginning;
}
