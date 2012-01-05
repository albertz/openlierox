#include "gconsole.h"
#ifndef DEDICATED_ONLY
#include "font.h"
#include "sprite_set.h"
#include "sprite.h"
#endif
#include "script.h"
#include "gusgame.h"
#include "glua.h"
#include "util/math_func.h"

#include "network.h" //TEMP
#include "Debug.h"
#include "FindFile.h"

#include "gusanos/allegro.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

using namespace std;

#ifndef DEDICATED_ONLY

#define TEST_KEY(k_, keyname_) if(k_ < 0) return "UNKNOWN KEY \"" + keyname_ + '"'

struct IdentityGetText
{
	template<class IteratorT>
	std::string const& operator()(IteratorT i) const
	{
		return *i;
	}
};
#endif

string echoCmd(list<string> const& args)
{
	if(args.size() > 0)
	{
		std::list<string>::const_iterator i = args.begin();
		std::string ret = *i++;
		
		for(; i != args.end(); ++i)
		{
			ret += ' ' + *i;
		}
		
		return ret;
	}
	
	return "ECHO <ARGS> ... : PRINTS OUT ARGS TO THE CONSOLE";
}

string execCmd(const list<string> &args)
{
	if (!args.empty())
	{
		if ( console.executeConfig(*args.begin()) )
		{
			return "DONE";
		}
		return ( "COULDN'T EXEC " + *args.begin() );
	}
	return "EXEC <FILENAME> : EXECUTE A SCRIPT FILE";
}

string aliasCmd(const list<string> &args)
{
	string name;
	string action;
	
	if (!args.empty())
	{
		list<string>::const_iterator argument;
		argument=args.begin();
		
		name = *argument;
		
		argument++;
		
		if ( argument != args.end() )
		{
			action = *argument;
			console.registerAlias(name,action);
		}

		return "";
	}
	return "ALIAS <NAME> <ACTION> : REGISTER ALIAS TO ACTION";
}

/////////////////////////////// Console //////////////////////////////////////

//============================= LIFECYCLE ====================================

GConsole::GConsole()
: Console(256)
#ifndef DEDICATED_ONLY
, background(NULL)
#endif
{
	scrolling = false;
}

//============================= INTERFACE ====================================

#ifndef DEDICATED_ONLY
void GConsole::varCbFont( std::string oldValue )
{
	Font* newFont = fontLocator.load(m_fontName);
	if(!newFont)
	{
		addLogMsg("FONT \"" + m_fontName + "\" NOT FOUND, REVERTING TO OLD FONT");
		m_fontName = oldValue;
		return;
	}
	m_font = newFont;
}
#endif
void GConsole::init()
{
	m_mode = CONSOLE_MODE_BINDINGS;
	//m_mode = CONSOLE_MODE_INPUT;

	console.registerVariables()
		("CON_SPEED", &speed, 4)
		("CON_HEIGHT", &height, 120)
#ifndef DEDICATED_ONLY
		("CON_FONT", &m_fontName, "minifont", boost::bind(&GConsole::varCbFont, this, _1))
#endif
	;

	console.registerCommands()
		(string("EXEC"), execCmd)
		(string("ALIAS"), aliasCmd)
		(string("ECHO"), echoCmd)
	;
	
	currentCommand = commandsLog.end(); //To workaround a crashbug with uninitialized iterator
	logRenderPos = log.rbegin();
	scrolling = false;
}

void GConsole::shutDown()
{
#ifndef DEDICATED_ONLY
	//m_font must be deleted here!!!! hmm not sure now
#endif
}

void GConsole::loadResources()
{
#ifndef DEDICATED_ONLY
	m_font = fontLocator.load(m_fontName);

	if(!m_font)
		cout << "Console font couldn't be loaded" << endl;
	
	background = spriteList.load("con_background");
#endif
}

#ifndef DEDICATED_ONLY
void GConsole::render(ALLEGRO_BITMAP* where, bool fullScreen)
{
	//int textIndex = 0;

	float pos = m_pos;
	if ( fullScreen ) pos = (float)(where->h-1);
		
	if ( pos > 0)
	{
		if (background)
			background->getSprite()->draw(where, 0, static_cast<int>(pos), ALIGN_LEFT | ALIGN_BOTTOM);

		int y = static_cast<int>(pos) - 5;
		
		string tempString = (']' + m_inputBuff + '*');

		std::pair<int, int> dim;
		string::const_reverse_iterator b = tempString.rbegin(), e = tempString.rend();
		// When using reverse iterators, fitString tries to fit the spacing of
		// the last character as well which isn't exactly what is wanted
		e = m_font->fitString(b, e, 320-5, dim);
		y -= dim.second;
		m_font->draw(where, e.base(), b.base(), 5, y);
		
		list<string>::reverse_iterator msgiter;
		if ( scrolling )
		{
			msgiter = logRenderPos;
		}else
		{
			msgiter = log.rbegin();
		}
		
		for(;
		    msgiter != log.rend() && y > 0;
		    ++msgiter)
		{
			string const& msg = *msgiter;
			
			string::const_iterator b = msg.begin(), e = msg.end(), n;
			
			int totalHeight = 0;
			do
			{
				pair<int, int> dim;
				n = m_font->fitString(b, e, 320-5, dim, 0, Font::Formatting);
				if(n == b)
					break;
				b = n;
				totalHeight += dim.second;
			}
			while(b != e);
			
			y -= totalHeight + 1;
			
			b = msg.begin();
			
			int y2 = y;
			
			Font::CharFormatting format(
				Font::CharFormatting::Item(
					Font::Color(255, 255, 255)
					)
				);
			
			do
			{
				pair<int, int> dim;
				n = m_font->fitString(b, e, 320-5, dim, 0, Font::Formatting);
				if(n == b)
					break;
				m_font->draw(where, b, n, 5, y2, format, 0, 255, Font::Formatting);
				y2 += dim.second;
				
				b = n;
			}
			while(b != e);
		}
	}
}

void GConsole::checkInput()
{
}
#endif

void GConsole::think()
{
#ifndef DEDICATED_ONLY
	if ( height > 240 ) height=240;
	if ( m_mode == CONSOLE_MODE_INPUT && m_pos < height )
	{
		m_pos+=speed;
	}else if ( m_mode == CONSOLE_MODE_BINDINGS && m_pos > 0 )
	{
		m_pos-=speed;
	}
	if (m_pos > height) m_pos = (float)height;
	if (m_pos < 0) m_pos = 0;
#endif
	while( !commandsQueue.empty() )
	{
		console.parseLine( *commandsQueue.begin() );
		commandsQueue.erase( commandsQueue.begin() );
	}
}

int GConsole::executeConfig(const std::string& filename)
{
	std::string p(gusGame.getModPath() + "/" + filename);
	if ( gusExists(p) )
		return Console::executeConfig(p);
	else
		return Console::executeConfig(gusGame.getDefaultPath() + "/" + filename);
}

void GConsole::addQueueCommand( std::string const & command )
{
	commandsQueue.push_back( command );
}

#ifdef DEDICATED_ONLY
void GConsole::addLogMsg(const std::string &msg)
{
	cout << "CONSOLE: " << msg << endl;
}
#endif
//============================= PRIVATE ======================================


GConsole console;

