#ifndef COMMAND_H
#define COMMAND_H

#include "consoleitem.h"
#include <string>
#include <boost/function.hpp>

class Console;

class GusCommand : public ConsoleItem
{
	public:
	
	typedef boost::function<std::string (std::list<std::string> const&)> CallbackT;
	typedef boost::function<std::string (Console*, int, std::string const&)> CompleteCallbackT;
	
	GusCommand(CallbackT const& func, CompleteCallbackT const& completeFunc = CompleteCallbackT());
	GusCommand();
	virtual ~GusCommand();
	
	std::string invoke(std::list<std::string> const& args);
	
	virtual std::string completeArgument(int idx, std::string const& beginning);
	
	private:
	
	CallbackT m_func;
	CompleteCallbackT m_completeFunc;
};

#endif  // _COMMAND_H_
