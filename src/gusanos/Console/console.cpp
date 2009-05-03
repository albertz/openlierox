#include "console.h"
#include "variables.h"
#include "command.h"
#include "special_command.h"
#include "alias.h"
#include "util/text.h"
#include "util/macros.h"
#include "consoleitem.h"

#include "console-grammar.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <stack>
#include <cctype>

#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

/////////////////////////////// Console //////////////////////////////////////

//============================= LIFECYCLE ====================================

Console::Console()
: m_logMaxSize(256)
{
	
	//m_MaxMsgLength=40;
}

Console::Console(int logMaxSize)
: m_logMaxSize(logMaxSize)
{
	
	//m_MaxMsgLength=MaxMsgLength;
}

Console::~Console()
{
	// Delete the registered variables
/*
	map<string, ConsoleItem*>::iterator tempvar = items.begin();
	while (tempvar != items.end())
	{
		delete tempvar->second;
		tempvar++;
	}*/
	
	foreach(i, items)
	{
		delete i->second;
	}
	
	//items.clear();
}

//============================= INTERFACE ====================================

void Console::registerItem(std::string const& name, ConsoleItem* item)
{
	item->m_owner = this;
	items[name] = item;
}

void Console::registerVariable(Variable* var)
{
	string const& name = var->getName();
	if (!name.empty())
	{
		std::map<std::string, ConsoleItem*, IStrCompare>::iterator i = items.find(name);
		if(i != items.end())
		{
			// Replace old variable
			delete i->second;
		}
		
		registerItem(name, var);
	}
	else
	{
		delete var; // Empty name
	}
}

void Console::registerCommand(std::string const& name, Command* command)
{
	if (!name.empty() && items.find(name) == items.end())
	{
		registerItem(name, command);
	}
	else
		delete command;
}

void Console::registerSpecialCommand(const std::string &name, int index, std::string (*func)(int,const std::list<std::string>&))
{
	if (!name.empty())
	{
		ItemMap::iterator tempItem = items.find(name);
		if (tempItem == items.end())
		{
			//items[name] = new SpecialCommand(index,func);
			registerItem(name, new SpecialCommand(index, func));
		}
	}
}

void Console::registerAlias(const std::string &name, const std::string &action)
{
	if (!name.empty())
	{
		ItemMap::iterator tempItem = items.find(name);
		if (tempItem == items.end()
		|| !tempItem->second->isLocked())
		{
			if(tempItem != items.end())
				delete tempItem->second;
			registerItem(name, new Alias(name, action));
		}
	}
}
/*
struct IsTemporary
{
	bool operator()(std::pair<std::string, ConsoleItem*> const& x) const
	{
		return x.second->temp;
	}
};
*/
void Console::clearTemporaries()
{
	//std::remove_if(items.begin(), items.end(), IsTemporary());

	foreach_delete(i, items)
	{	
		if(i->second->temp)
		{
			delete i->second;
			items.erase(i);
		}
	}
}

struct TestHandler : public ConsoleGrammarBase
{
	TestHandler(std::istream& str_, Console& console_, bool parseRelease_ = false)
	: str(str_), console(console_), parseRelease(parseRelease_)
	{
		next();
	}
	
	int cur()
	{
		return c;
	}
	
	void next()
	{
		c = str.get();
		if(c == std::istream::traits_type::eof())
			c = -1;
	}
	
	std::string invoke(std::string const& name, std::list<std::string> const& args)
	{
		return console.invoke(name, args, parseRelease);
	}
	
	int c;
	std::istream& str;
	Console& console;
	bool parseRelease;
};

void Console::parseLine(const string &text, bool parseRelease)
{
	std::istringstream ss(text);
	ConsoleGrammar<TestHandler> handler((TestHandler(ss, *this, parseRelease)));

	try
	{
		addLogMsg(handler.block());
	}
	catch(SyntaxError error)
	{
		addLogMsg(text);
		std::streamoff pos = handler.str.tellg();
		if(pos > 0)
		{
			addLogMsg(std::string(pos - 1, '-') + '^');
			addLogMsg(error.what());
		}
		else
		{
			addLogMsg(error.what() + string(" at end of input"));
		}
	}
}

std::string Console::invoke(string const& name, list<string> const& args, bool parseRelease)
{
	if(!parseRelease || name[0] == '+')
	{
		std::string nameCopy(name);
		if(parseRelease)
			nameCopy[0] = '-';

		map<string, ConsoleItem*>::iterator tempItem = items.find(nameCopy);
		if (tempItem != items.end())
		{
			return tempItem->second->invoke(args);
		} else
			return "UNKNOWN COMMAND: " + name;
	}
	
	return "";
}

/*
void Console::parse(list<string> &args, bool parseRelease)
{
	string itemName;
	string arguments;
	string retString;
	
	if (!args.empty())
	{
		itemName = *args.begin();
		if ( !parseRelease || (itemName[0] == '+') )
		{
			if (parseRelease) itemName[0]='-';
			map<string, ConsoleItem*>::iterator tempItem = items.find(itemName);
			if (tempItem != items.end())
			{
				args.pop_front();
				retString = tempItem->second->invoke(args);
				addLogMsg(retString);
			}else
			{
				addLogMsg("UNKNOWN COMMAND \"" + itemName + "\"" );
			}
		}
	}
}
*/

void Console::addLogMsg(const string &msg)
{
	if (!msg.empty())
	{
		if(log.size() >= m_logMaxSize)
			log.pop_front();
		
		log.push_back(msg);
	}
}

void Console::analizeKeyEvent(bool state, char key)
{
	if (state == true)
		parseLine(bindTable.getBindingAction(key));
	else
		parseLine(bindTable.getBindingAction(key), true);
}

void Console::bind(char key, const string &action)
{
	bindTable.bind(key , action);
}

char Console::getKeyForBinding(std::string const& action)
{
	return bindTable.getKeyForAction(action);
}

std::string Console::getActionForBinding(char key)
{
	return bindTable.getBindingAction(key);
}

int Console::executeConfig(const string &filename)
{
	ifstream file(filename.c_str());

	if (file.is_open() && file.good())
	{
		string text2Parse;
		//...parse the file
		while (portable_getline(file, text2Parse))
		{
			//getline(file,text2Parse);
			//std::transform(text2Parse.begin(), text2Parse.end(), text2Parse.begin(), (int(*)(int)) toupper);
			parseLine(text2Parse);
		}

		return 1;
	}
	
	
	return 0;
};

struct CompletionHandler : public ConsoleGrammarBase
{
	struct State
	{
		State()
		{
		}
		
		State(string::const_iterator b_)
		: argumentIdx(0), commandComplete(false), argumentComplete(false)
		, beginCommand(b_), beginArgument(b_)
		{
		}
		
		bool commandComplete;
		bool argumentComplete;
		std::string command;
		std::string argument;
		string::const_iterator beginArgument;
		string::const_iterator beginCommand;
		int argumentIdx;
	};
		
	CompletionHandler(
		string::const_iterator b_,
		string::const_iterator e_,
		Console& console_
	)
	: b(b_), e(e_), console(console_), current(b_), endPrefix(b_)
	, beginPrefix(b_)
	{
		c = (unsigned char)*b;
	}
	
	int cur()
	{
		return c;
	}
	
	void next()
	{
		++b;
		if(b == e)
		{
			if(!current.commandComplete)
			{
				current.command = std::string(current.beginCommand, b);
			}
			else if(!current.argumentComplete)
			{
				if(current.beginArgument != e)
					current.argument = std::string(current.beginArgument, b);
				else
					endPrefix = b;
			}
			
			throw current;
		}
		c = (unsigned char)*b;
	}
	
	void inCommand()
	{
		states.push(current);
		current.commandComplete = false;
		current.beginCommand = b;
		endPrefix = b;
	}
	
	void inArguments(std::string const& command)
	{
		current.commandComplete = true;
		current.argumentIdx = 0;
		current.argumentComplete = false;
		current.argument = "";
		current.beginArgument = e;
		current.command = command;
	}
	
	void inArgument(int idx)
	{
		current.commandComplete = true;
		current.argumentComplete = false;
		current.argumentIdx = idx;
		current.beginArgument = b;
		endPrefix = b;
	}
	
	void outArgument(int idx, std::string const& argument)
	{
		current.argumentComplete = false;
		current.argumentIdx = idx + 1;
		current.argument = "";
		current.beginArgument = e;
	}
	
	void outCommand()
	{
		if(!states.empty())
		{
			current = states.top();
			states.pop();
		}
	}
	
	std::string prefix()
	{
		return std::string(beginPrefix, endPrefix);
	}

	int c;
	string::const_iterator beginPrefix;
	string::const_iterator b;
	string::const_iterator e;
	string::const_iterator endPrefix;
	
	State current;
	std::stack<State> states;
	Console& console;
};

struct ItemGetText
{
	template<class IteratorT>
	std::string const& operator()(IteratorT i) const
	{
		return i->first;
	}
};

std::string Console::completeCommand(std::string const& b)
{
	return shellComplete(items, b.begin(), b.end()
		, ItemGetText(), ConsoleAddLines(*this));
}

string Console::autoComplete(string const& text)
{
	string returnText = text;
	
	if ( !text.empty() )
	{
		ConsoleGrammar<CompletionHandler> handler((CompletionHandler(text.begin(), text.end(), *this)));
		
		try
		{
			handler.block();
		}
		catch(CompletionHandler::State result)
		{
			if(result.commandComplete)
			{
				ItemMap::const_iterator item = items.find(result.command);
				
				if(item != items.end())
				{
					return handler.prefix() + item->second->completeArgument(result.argumentIdx, result.argument);
				}
				
				return text;
			}
			else
			{
				return handler.prefix() + completeCommand(result.command);
			}
		}
		catch(SyntaxError error)
		{
			return text;
		}
	}
	
	return text;
}

void Console::listItems(const string &text)
{
	if ( !text.empty() )
	{
		// Find the first item that matches that text
		map<string, ConsoleItem*>::iterator item = items.lower_bound( text ); 
		if( item != items.end() && text == item->first.substr(0, text.length()) ) // If found
		{
			// Temp item to check if there is only 1 item matching the given text
			map<string, ConsoleItem*>::iterator tempItem = item; 
			tempItem++;
			
			// If the temp item is equal to the first item found it means that there are more than 1 items that match
			if ( tempItem != items.end() )
			if ( tempItem->first.substr(0, text.length() ) == item->first.substr(0, text.length()) )
			{
				
				addLogMsg("]");
				
				// Add a message with the name of all the matching items
				while ( item != items.end() && text == item->first.substr(0, text.length() ) )
				{
					addLogMsg(item->first);
					item++;
				}
			}
		}
	}
}

//============================= PRIVATE ======================================

