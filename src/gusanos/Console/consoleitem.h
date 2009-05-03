#ifndef CONSOLE_ITEM_H
#define CONSOLE_ITEM_H

#include <string>
#include <list>

class Console;

class ConsoleItem
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
};

#endif  // _CONSOLE_ITEM_H_
