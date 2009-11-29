#include "gconsole.h"
#ifndef DEDSERV
#include "keyboard.h"
#include "keys.h"
#include "font.h"
#include "sprite_set.h"
#include "sprite.h"
#endif
#include "script.h"
#include "game.h"
#include "glua.h"
#include "util/math_func.h"

#include "network.h" //TEMP

#include <allegro.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

using namespace std;

#ifndef DEDSERV

#define TEST_KEY(k_, keyname_) if(k_ < 0) return "UNKNOWN KEY \"" + keyname_ + '"'

// Bind console command
string bindCmd(const list<string> &args)
{
	// <GLIP> Simplified a little, removed unused vars
	// and made it print out the help text when too
	// few arguments are passed.

	if (args.size() >= 2) 
	{
		std::list<string>::const_iterator arguments = args.begin();
		
		std::string const& keyName = *arguments++;
		int key = kName2Int(keyName);
		TEST_KEY(key, keyName);

		console.bind(key, *arguments++);
	
		return "";
	}
	
	return "BIND <KEY> [COMMAND] : ATTACH A COMMAND TO A KEY";
}

struct IdentityGetText
{
	template<class IteratorT>
	std::string const& operator()(IteratorT i) const
	{
		return *i;
	}
};

string bindCompleter(Console* con, int idx, std::string const& beginning)
{
	if(idx != 0)
		return beginning;
		
	return shellComplete(
		keyNames,
		beginning.begin(),
		beginning.end(),
		IdentityGetText(),
		ConsoleAddLines(*con)
	);
}
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

#ifndef DEDSERV
/*
// Key map swapping command
string swapKeysCmd(const list<string> &args)
{
	if (args.size() >= 2)
	{
		std::list<string>::const_iterator arguments = args.begin();
		std::string const& keyNameA = *arguments++;
		int keyA = kName2Int(keyNameA);
		TEST_KEY(keyA, keyNameA);
		
		std::string const& keyNameB = *arguments++;
		int keyB = kName2Int(keyNameB);
		TEST_KEY(keyB, keyNameB);
		
		KeyHandler::swapKeyMapping(keyA, keyB);
		return "";
	}
	return "SWAPKEYS <KEY A> <KEY B> : SWAPS KEY A AND KEY B WITH EACHOTHER";
}


string setShiftChar(const list<string> &args)
{
	if (args.size() >= 2)
	{
		std::list<string>::const_iterator arguments = args.begin();
		
		std::string const& keyName = *arguments++;
		int key = kName2Int(keyName);
		TEST_KEY(key, keyName);
		
		int shiftCharacter = (*arguments)[0];
		
		KeyHandler::setShiftCharacter(key, shiftCharacter);
		return "";
	}
	return "SETSHIFTCHAR <KEY> <CHARACTER> : SETS THE CHARACTER TO BE USED WITH SHIFT+KEY";
}

string setChar(const list<string> &args)
{
	if (args.size() >= 2)
	{
		std::list<string>::const_iterator arguments = args.begin();
		
		std::string const& keyName = *arguments++;
		int key = kName2Int(keyName);
		TEST_KEY(key, keyName);
		
		int shiftCharacter = (*arguments)[0];
		
		KeyHandler::setCharacter(key, shiftCharacter);
		return "";
	}
	return "SETCHAR <KEY> <CHARACTER> : SETS THE CHARACTER TO BE USED WITH KEY";
}

string setAltGrChar(const list<string> &args)
{
	if (args.size() >= 2)
	{
		std::list<string>::const_iterator arguments = args.begin();
		
		std::string const& keyName = *arguments++;
		int key = kName2Int(keyName);
		TEST_KEY(key, keyName);
		
		int altgrCharacter = (*arguments)[0];
		
		KeyHandler::setAltGrCharacter(key, altgrCharacter);
		return "";
	}
	return "SETALTGRCHAR <KEY> <CHARACTER> : SETS THE CHARACTER TO BE USED WITH ALTGR+KEY";
}
*/
string GConsole::setConsoleKey(list<string> const& args)
{
	if (args.size() >= 1)
	{
		std::list<string>::const_iterator arguments = args.begin();
		
		std::string const& keyName = *arguments++;
		int key = kName2Int(keyName);
		TEST_KEY(key, keyName);

		m_consoleKey = key;
		return "";
	}
	return "SETCONSOLEKEY <KEY> : SETS THE KEY TO SHOW/HIDE THE CONSOLE";
}
#endif


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
	return "BIND <KEY> [COMMAND] : ATTACH A COMMAND TO A KEY";
}

/*
string execScript(list<string> const& args)
{
	if (args.size() >= 2)
	{
		list<string>::const_iterator i = args.begin();
		string const& file = *i++;
		string const& function = *i++;
		Script* s = scriptLocator.load(file);
		if(!s)
			return "SCRIPT FILE \"" + file + "\" COULDN'T BE LOADED";
			
		s->pushFunction(function);
		int params = 0;
		for(; i != args.end(); ++i)
		{
			lua_pushstring(*s->lua, i->c_str());
			++params;
		}
		
		int result = s->lua->call(params);
		
		if(result < 0)
		{
			return ( "COULDN'T EXECUTE " + file + " " + function );
		}
		else
			return "";
	}
	return "EXECSCRIPT <FILE> <FUNCTION> : EXECUTE A SCRIPT FILE";
}
*/

string rndSeedCmd(list<string> const& args)
{
	if(args.size() > 0)
	{
		std::list<string>::const_iterator i = args.begin();

		rndgen.seed(cast<boost::mt19937::result_type>(*i));
		return "";
	}
	
	return "RND_SEED <SEED> : SEEDS THE RANDOM GENERATOR WITH <SEED>";
}

string restCmd(list<string> const& args)
{
	if(args.size() > 0)
	{
		std::list<string>::const_iterator i = args.begin();
		
		int t = cast<int>(*i);
		rest(t);		
		return "DONE";
	}
	
	return "REST <MS> : RESTS FOR A NUMBER OF MILLISECONDS";
}
/////////////////////////////// Console //////////////////////////////////////

//============================= LIFECYCLE ====================================

GConsole::GConsole()
: Console(256)
#ifndef DEDSERV
, m_consoleKey(KEY_TILDE), background(NULL)
#endif
{
#ifndef DEDSERV
	m_lockRefCount.assign(0);
#endif
	scrolling = false;
}

//============================= INTERFACE ====================================

#ifndef DEDSERV
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
#ifndef DEDSERV
	keyHandler.init();
	
	//Connect the handlers as group 0 so they are called first

	keyHandler.printableChar.connect(0, boost::bind(&GConsole::eventPrintableChar, this, _1, _2));
	keyHandler.keyDown.connect(0, boost::bind(&GConsole::eventKeyDown, this, _1));
	keyHandler.keyUp.connect(0, boost::bind(&GConsole::eventKeyUp, this, _1));
#endif

	m_mode = CONSOLE_MODE_BINDINGS;
	//m_mode = CONSOLE_MODE_INPUT;

	console.registerVariables()
		("CON_SPEED", &speed, 4)
		("CON_HEIGHT", &height, 120)
#ifndef DEDSERV
		("CON_FONT", &m_fontName, "minifont", boost::bind(&GConsole::varCbFont, this, _1))
#endif
	;

	console.registerCommands()
#ifndef DEDSERV
		(string("BIND"), bindCmd, bindCompleter)
/*
		(string("SWAPKEYS"), swapKeysCmd)
		(string("SETSHIFTCHAR"), setShiftChar)
		(string("SETALTGRCHAR"), setAltGrChar)
		(string("SETCHAR"), setChar)
*/
		(string("SETCONSOLEKEY"), boost::bind(&GConsole::setConsoleKey, this, _1))
#endif
		(string("EXEC"), execCmd)
		//(string("EXECSCRIPT"), execScript)
		(string("ALIAS"), aliasCmd)
		(string("ECHO"), echoCmd)
		(string("RND_SEED"), rndSeedCmd)
		(string("REST"), restCmd)
	;
	
	currentCommand = commandsLog.end(); //To workaround a crashbug with uninitialized iterator
	logRenderPos = log.rbegin();
	scrolling = false;
}

void GConsole::shutDown()
{
#ifndef DEDSERV
	keyHandler.shutDown();
	
	//m_font must be deleted here!!!! hmm not sure now
#endif
}

void GConsole::loadResources()
{
#ifndef DEDSERV
	m_font = fontLocator.load(m_fontName);

	if(!m_font)
		cout << "Console font couldn't be loaded" << endl;
	
	background = spriteList.load("con_background");
#endif
}

#ifndef DEDSERV
void GConsole::render(BITMAP* where, bool fullScreen)
{
	//int textIndex = 0;

	float pos = m_pos;
	if ( fullScreen ) pos = where->h-1;
		
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
	keyHandler.pollKeyboard();
}

bool GConsole::eventKeyDown(int k)
{
	if(k == m_consoleKey)
	{
		if ( m_mode == CONSOLE_MODE_INPUT )	// If the console is in input mode toogle to Binding mode
		{
			clear_keybuf();
			m_mode = CONSOLE_MODE_BINDINGS;
			return false;
		}
		else											// If not toogle to input
		{
			m_mode = CONSOLE_MODE_INPUT;
			clear_keybuf();						// Clear allegro buffer so that old keys dont bother
			m_inputBuff.clear();
			currentCommand = commandsLog.end();
		}
	}
	else if ( m_mode == CONSOLE_MODE_BINDINGS )		// Only if in bindings mode
	{
		if(m_lockRefCount.at(k) <= 0)
		{
			analizeKeyEvent(true, k);
			clear_keybuf();
			return false;
		}
	}
	else if ( m_mode == CONSOLE_MODE_INPUT )
	{
		if ( k == KEY_UP )
		{
			clear_keybuf();
			if (currentCommand != commandsLog.begin() )
				currentCommand--;
			if ( currentCommand == commandsLog.end() )
			{
				m_inputBuff.clear();
			}else
			{
				m_inputBuff = *currentCommand;
			}
		}
		else if ( k == KEY_DOWN )
		{
			clear_keybuf();
			if (currentCommand != commandsLog.end() )
				currentCommand++;
			if ( currentCommand == commandsLog.end() )
			{
				m_inputBuff.clear();
			}
			else
			{
				m_inputBuff = *currentCommand;
			}
		}
		else if ( k == KEY_PGUP )
		{
			clear_keybuf();
			if ( !scrolling )
			{
				logRenderPos = log.rbegin();
				scrolling = true;
			}
			
			for ( int i = 0; logRenderPos != log.rend() && i < 3; ++i, ++logRenderPos )
			{
				
			}
		}
		else if ( k == KEY_PGDN )
		{
			clear_keybuf();
			if ( scrolling )
			{
				for ( int i = 0; logRenderPos != log.rbegin() && i < 3; ++i, --logRenderPos )
				{
					
				}
				if ( logRenderPos == log.rbegin() )
				{
					scrolling = false;
				}
			}
		}
		else if ( k == KEY_END )
		{
			clear_keybuf();
			scrolling = false;
		}
		
		return false;
	}
	
	return true;
}

bool GConsole::eventKeyUp(int k)
{
	if ( m_mode == CONSOLE_MODE_BINDINGS )		// Only if in bindings mode
	{
		if(m_lockRefCount.at(k) <= 0)
		{
			analizeKeyEvent(false, k);
			clear_keybuf();
			return false;
		}
	}
	else
		return false;
	
	return true;
}

bool GConsole::eventPrintableChar(char c, int k)
{

	if ( m_mode == CONSOLE_MODE_INPUT ) // console is in input read mode so..
	{
		if (c == 8)//Backspace
		{
			if (!m_inputBuff.empty()) //if the string is not already empty...
				m_inputBuff.erase(m_inputBuff.length()-1); //delete last char
		}
		else if (c == 13) //Enter
		{
			addLogMsg(']'+m_inputBuff); //add the text to the console log
			console.parseLine(m_inputBuff); //parse the text
			commandsLog.push_back(m_inputBuff); //add the text to the commands log too
			currentCommand = commandsLog.end(); //reset the command log position
			m_inputBuff.clear(); // and then clear the buffer
		}
		else if (c == '\t') //Tab
		{
			/*
			string autoCompText = autoComplete( m_inputBuff );
			if (m_inputBuff == autoCompText)
			{
				listItems(m_inputBuff);
			}else
			{
				m_inputBuff = autoCompText;
			}
			*/
			
			m_inputBuff = autoComplete( m_inputBuff );
		}
		else // No special keys where detected so the char gets added to the string
		{
			//m_inputBuff += toupper(c);
			m_inputBuff += c;
		}
		return false;
	}
	return true;
}
#endif

void GConsole::think()
{
#ifndef DEDSERV
	if ( height > 240 ) height=240;
	if ( m_mode == CONSOLE_MODE_INPUT && m_pos < height )
	{
		m_pos+=speed;
	}else if ( m_mode == CONSOLE_MODE_BINDINGS && m_pos > 0 )
	{
		m_pos-=speed;
	}
	if (m_pos > height) m_pos = height;
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
	fs::path p(game.getModPath() / filename);
	if ( fs::exists(p) )
		return Console::executeConfig(p.native_file_string());
	else
		return Console::executeConfig((game.getDefaultPath() / filename).native_file_string());
}

void GConsole::addQueueCommand( std::string const & command )
{
	commandsQueue.push_back( command );
}

#ifdef DEDSERV
void GConsole::addLogMsg(const std::string &msg)
{
	cout << "CONSOLE: " << msg << endl;
}
#endif
//============================= PRIVATE ======================================


GConsole console;

