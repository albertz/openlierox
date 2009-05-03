#ifndef BINDINGS_H
#define BINDINGS_H

#include <string>

struct Binding
{
	std::string m_action;
};

class BindTable
{
public:
	
	BindTable(void);
	~BindTable(void);
	
	void bind(char key, const std::string &action);
	void unBind(char key);
	void unBindAll();
	std::string getBindingAction(char key);
	char getKeyForAction(std::string const& action);
	
private:
	
	Binding binding[256];
};

#endif  // _BINDINGS_H_
