#ifndef SPECIAL_COMMAND_H
#define SPECIAL_COMMAND_H

#include "consoleitem.h"

#include <string>

#define VAR_TYPE_INVALID 0
#define VAR_TYPE_INT 1

class SpecialCommand : public ConsoleItem
{
	public:

	SpecialCommand(int index, std::string (*func)(int, const std::list<std::string>&));
	SpecialCommand();
	virtual ~SpecialCommand();
	
	std::string invoke(const std::list<std::string> &args);
	
	private:
	
	std::string (*m_func)(int, const std::list<std::string>&);
	int m_index;
};

#endif  // _SPECIAL_COMMAND_H_
