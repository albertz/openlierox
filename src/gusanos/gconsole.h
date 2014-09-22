#ifndef VERMES_GCONSOLE_H
#define VERMES_GCONSOLE_H

#include "console/console.h"
//#include "font.h"

#ifndef DEDICATED_ONLY
#include "gusanos/allegro.h"
#endif

#include <list>
#include <string>
#include <list>
#include <map>
#include <set>
#include <boost/array.hpp>
using boost::array;

#ifndef DEDICATED_ONLY
class SpriteSet;
class Font;
#endif

class GConsole : public Console
{
public:
#ifndef DEDICATED_ONLY
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
	int executeConfig(const std::string &filename);
#ifdef DEDICATED_ONLY
	virtual void addLogMsg(const std::string &msg);
#endif
	
#ifndef DEDICATED_ONLY
	void varCbFont( std::string oldValue );
#endif

private:
	
	float m_pos;
	float speed;
	int height;
	int m_mode;
	
	//KeyHandler keyHandler;

#ifndef DEDICATED_ONLY
	Font* m_font;
	std::string m_fontName;
	
	std::string m_inputBuff;
	SpriteSet *background;
#endif

	std::list< std::string >::reverse_iterator logRenderPos; //For scrolling
	bool scrolling;
	
	std::list< std::string > commandsLog;
	std::list< std::string >::iterator currentCommand;
		
	enum
	{
		CONSOLE_MODE_INPUT,
		CONSOLE_MODE_BINDINGS
	};
};

extern GConsole console;

#endif  // VERMES_GCONSOLE_H
