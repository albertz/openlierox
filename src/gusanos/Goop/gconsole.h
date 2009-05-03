#ifndef GUSANOS_GCONSOLE_H
#define GUSANOS_GCONSOLE_H

#include <console.h>
//#include "font.h"

#ifndef DEDSERV
#include <allegro.h>
#endif

#include <list>
#include <string>
#include <list>
#include <map>
#include <set>
#include <boost/array.hpp>
using boost::array;

#ifndef DEDSERV
class SpriteSet;
class Font;
#endif

class GConsole : public Console
{
public:
#ifndef DEDSERV
	struct BindingLock
	{
		BindingLock()
		{
			enable.assign(true);
		}
		
		array<bool, 256> enable;
	};
#endif

	GConsole();
	
	void init();
	void shutDown();
	void loadResources();
	void checkInput();
#ifndef DEDSERV
	void render(BITMAP *where, bool fullScreen = false);
#endif
	void think();
	int executeConfig(const std::string &filename);
#ifdef DEDSERV
	virtual void addLogMsg(const std::string &msg);
#endif
	
#ifndef DEDSERV
	bool eventPrintableChar(char c, int k);
	bool eventKeyDown(int k);
	bool eventKeyUp(int k);
	
	std::string setConsoleKey(std::list<std::string> const& args);
	
	void lockBindings(BindingLock const& lock)
	{
		if(m_locks.insert(&lock).second)
		{
			for(size_t i = 0; i < m_lockRefCount.size(); ++i)
			{
				if(lock.enable[i])
					++m_lockRefCount[i];
			}
		}
	}
	
	void releaseBindings(BindingLock const& lock)
	{
		std::set<BindingLock const*>::iterator l = m_locks.find(&lock);
		if(l != m_locks.end())
		{
			m_locks.erase(l);
			
			for(size_t i = 0; i < m_lockRefCount.size(); ++i)
			{
				if(lock.enable[i])
					--m_lockRefCount[i];
			}
		}
	}
	
	void varCbFont( std::string oldValue );
#endif

	void addQueueCommand( std::string const & command );

private:
	
	float m_pos;
	float speed;
	int height;
	int m_mode;
	
	//KeyHandler keyHandler;

#ifndef DEDSERV
	Font* m_font;
	std::string m_fontName;
	int m_consoleKey;
	
	std::string m_inputBuff;
	SpriteSet *background;
	std::set<BindingLock const*> m_locks;
	array<int, 256> m_lockRefCount;
#endif

	std::list< std::string >::reverse_iterator logRenderPos; //For scrolling
	bool scrolling;
	
	std::list< std::string > commandsLog;
	std::list< std::string >::iterator currentCommand;
	
	std::list< std::string > commandsQueue;
	
	enum
	{
		CONSOLE_MODE_INPUT,
		CONSOLE_MODE_BINDINGS
	};
};

std::string bindCmd(const std::list<std::string> &args);

extern GConsole console;

#endif  // GUSANOS_GCONSOLE_H
