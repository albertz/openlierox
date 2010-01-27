#ifndef CONSOLE_ITEM_H
#define CONSOLE_ITEM_H

#include <string>
#include <list>

#include "CScriptableVars.h"

class Console;

class ConsoleItem : /* OLX wrapper: */ private DynamicVar<std::string>
{
public:
	friend class Console;
	
	ConsoleItem(bool locked = true);

	virtual std::string invoke(const std::list<std::string> &args) = 0;
	
	virtual std::string completeArgument(int idx, std::string const& beginning)
	{
		return beginning;
	}
	
	bool isLocked();
	
	bool temp;
	
	virtual ~ConsoleItem()
	{}

protected:
	Console* m_owner;
	
private:
	bool m_locked;

	
	/* OLX dynamicvar wrapper */
	std::string get() { return invoke(std::list<std::string>()); }
	void set(const std::string& p) { std::list<std::string> l; l.push_back(p); invoke(l); }

};

#endif  // _CONSOLE_ITEM_H_
