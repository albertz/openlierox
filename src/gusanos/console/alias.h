#ifndef ALIAS_H
#define ALIAS_H

#include "console.h"
#include "consoleitem.h"
#include <string>

class Alias : public ConsoleItem
{
public:
	
	Alias(/*Console *parent, */const std::string &name, const std::string &action);
	
	Alias();
	virtual ~Alias();
	
	std::string invoke(const std::list<std::string> &args);
	
private:
	
	std::string m_name;
	std::string m_action;
	//Console *m_parent;
};

#endif  // _ALIAS_H_
