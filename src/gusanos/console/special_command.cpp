#include "special_command.h"

using namespace std;

///////////////////////////////IntVariable////////////////////////////////

//============================= LIFECYCLE ================================

SpecialCommand::SpecialCommand()
{
	m_func=NULL;
}

SpecialCommand::~SpecialCommand()
{
}

SpecialCommand::SpecialCommand(int index, std::string (*func)( int, const std::list<std::string>&))
{
	m_func=func;
	m_index = index;
}

//============================= INTERFACE ================================

string SpecialCommand::invoke( const std::list<std::string> &args)
{
	return m_func(m_index, args);
}
