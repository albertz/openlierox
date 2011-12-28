#ifndef CONSOLE_H
#define CONSOLE_H

// SYSTEM INCLUDES
//

// PROJECT INCLUDES
//
#include "util/text.h"
#include "variables.h"
#include "command.h"

// LOCAL INCLUDES
//
#include <string>
#include <list>
#include <map>
#include <set>

// FORWARD REFERENCES
//
class ConsoleItem;

class Console;

class Console
{
public:

	typedef std::map<std::string, ConsoleItem*, IStrCompare> ItemMap;
	
	struct RegisterVariableProxy
	{
		RegisterVariableProxy(Console& console)
		: m_console(console)
		{
		}
		
		template<class T, class IT, class FT>
		RegisterVariableProxy const& operator()(std::string const& name, T* src, IT defaultValue, FT callback) const
		{
			m_console.registerVariable(new TVariable<T>(name, src, defaultValue, callback));
			return *this;
		}
		
		template<class T, class IT>
		RegisterVariableProxy const& operator()(std::string const& name, T* src, IT defaultValue) const
		{
			m_console.registerVariable(new TVariable<T>(name, src, (T)defaultValue));
			return *this;
		}
		
		template<class VarT>
		RegisterVariableProxy const& operator()(VarT* var) const
		{
			m_console.registerVariable(var);
			return *this;
		}
		
		Console& m_console;
	};
	
	struct RegisterCommandProxy
	{
		RegisterCommandProxy(Console& console)
		: m_console(console)
		{
		}
		
		template<class FT>
		RegisterCommandProxy const& operator()(std::string const& name, FT func) const
		{
			m_console.registerCommand(name, new GusCommand(func));
			return *this;
		}
		
		template<class FT, class CFT>
		RegisterCommandProxy const& operator()(std::string const& name, FT func, CFT completeFunc) const
		{
			m_console.registerCommand(name, new GusCommand(func, completeFunc));
			return *this;
		}
		
		template<class FT>
		RegisterCommandProxy const& operator()(std::string const& name, FT func, bool temp) const
		{
			GusCommand *c = new GusCommand(func);
			m_console.registerCommand(name, c);
			c->temp = temp;
			return *this;
		}
		
		Console& m_console;
	};

	Console(void);
	Console(int logMaxSize);
	virtual ~Console(void);

	void registerVariable(Variable* var);
	RegisterVariableProxy registerVariables()
	{
		return RegisterVariableProxy(*this);
	}
	
	RegisterCommandProxy registerCommands()
	{
		return RegisterCommandProxy(*this);
	}
	
	void registerItem(std::string const& name, ConsoleItem* item);
	void registerAlias(const std::string &name, const std::string &action);
	void registerCommand(std::string const& name, GusCommand* command);
	void registerSpecialCommand(const std::string &name, int index, std::string (*func)(int,const std::list<std::string>&));
	void clearTemporaries();
	
	void parseLine(const std::string &text, bool parseRelease = false);
	std::string invoke(std::string const& name, std::list<std::string> const& args, bool parseRelease);
	virtual void addLogMsg(const std::string &msg);
	virtual int executeConfig(const std::string &filename);
	std::string autoComplete(std::string const& text);
	void listItems(const std::string &text);
	
	std::string completeCommand(std::string const& b);
	
protected:
	
	ItemMap items;
	std::list<std::string> log;
	
	int m_variableCount;
	unsigned int m_logMaxSize;
	//unsigned int m_MaxMsgLength;
	int m_mode;
};

struct ConsoleAddLines
{
	ConsoleAddLines(Console& console_)
	: console(console_)
	{
	}
	
	template<class IteratorT>
	void operator()(IteratorT b, IteratorT e) const
	{
		console.addLogMsg("]");
		
		for(; b != e; ++b)
		{
			console.addLogMsg(*b);
		}
	}
	
	Console& console;
};


#endif  // _CONSOLE_H_
