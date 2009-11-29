#ifndef COMMAND_H
#define COMMAND_H

#include "consoleitem.h"

#include <string>
#include <boost/function.hpp>

#define VAR_TYPE_INVALID 0
#define VAR_TYPE_INT 1

class Console;

class Command : public ConsoleItem
{
	public:
	
	typedef boost::function<std::string (std::list<std::string> const&)> CallbackT;
	typedef boost::function<std::string (Console*, int, std::string const&)> CompleteCallbackT;
	
	Command(CallbackT const& func, CompleteCallbackT const& completeFunc = CompleteCallbackT());
	Command();
	virtual ~Command();
	
	std::string invoke(std::list<std::string> const& args);
	
	virtual std::string completeArgument(int idx, std::string const& beginning);
	
	private:
	
	CallbackT m_func;
	CompleteCallbackT m_completeFunc;
};

#endif  // _COMMAND_H_
