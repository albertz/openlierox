#ifndef DEDSERV

#include "keyboard.h"
#include "keys.h"
#include "console.h"
#include <algorithm> // For std::find

#include <allegro.h>
#include <list>
#include <iostream>
#include <cctype>

using namespace std;

////////////////////////////PUBLIC//////////////////////////////

//int KeyHandler::keyMap[KEY_MAX];
//int KeyHandler::shiftCharMap[KEY_MAX]; // The shift map
//int KeyHandler::capsCharMap[KEY_MAX]; // The caps lock map
//int KeyHandler::altgrCharMap[KEY_MAX]; // The alt gr map
//int KeyHandler::charMap[KEY_MAX]; // The character map

KeyHandler keyHandler;

//=========================LIFECYCLE==========================//

KeyHandler::KeyHandler()
{
	for(int i = 0; i < KEY_MAX; ++i)
	{
		oldKeys[i] = false;
	}
	
#ifdef X11
	_xwin_keyboard_callback =; //TODO
#endif
}

KeyHandler::~KeyHandler()
{
	
}

//=========================INTERFACE==========================//

void KeyHandler::init()
{
	install_keyboard();
	
/*
	// Map as identity by default
	for(int i = 0; i < KEY_MAX; ++i)
	{
		keyMap[i] = i;
		altgrCharMap[i] = charMap[i] = scancode_to_ascii(i);
		capsCharMap[i] = shiftCharMap[i] = toupper(charMap[i]);
	}
	
	keyboard_ucallback = keyMapCallback; // Install key mapping callback
*/
}

void KeyHandler::shutDown()
{
	remove_keyboard();
}

void KeyHandler::pollKeyboard()
{
	for (int i = 0; i < KEY_MAX; ++i)
	{
		bool state = getKey(i);
		
		if (state != oldKeys[i])
		{
			if(state)
				keyDown(i);
			else
				keyUp(i);
			
			/*
			string s = cast<string>(i);
			
			for(string::const_iterator o = s.begin(); o != s.end(); ++o)
			{
				printableChar(*o);
			}*/

			oldKeys[i] = state;
		}
	}
	
	while(keypressed())
	{
		int key = readkey();
		printableChar(key & 0xFF, (key >> 8) & 0xFF);
	}
}

/*
KeyEvent KeyHandler::getEvent()
{
	KeyEvent event;
	event.type = KEY_EVENT_NONE;
	if (events.size() > 0)
	{
		event=events.back();
		events.pop_back();
	}
	return event;
}
*/

/*
int KeyHandler::keyMapCallback(int, int *scancode)
{
	// Translate scancode
	int newScancode = mapKey(*scancode);

	*scancode = newScancode;

	// Map the key to a character and return it
	if(getKey(KEY_LSHIFT) || getKey(KEY_RSHIFT))
		return shiftCharMap[newScancode]; // Use shift map
	else if(getKey(KEY_ALTGR))
		return altgrCharMap[newScancode];
	else
		return charMap[newScancode]; // Use regular map
		
	//TODO: Add caps-lock and ctrl tables
}*/

bool KeyHandler::getKey(int k)
{
	return key[mapKey(k)] != 0;
}

int KeyHandler::mapKey(int k)
{
	return k; //keyMap[k];
}
/*
int KeyHandler::reverseMapKey(int k)
{	
	return k;
}

void KeyHandler::swapKeyMapping(int keyA, int keyB)
{
	// Find what hardware key maps to the software keys to swap
	int keyAmap = reverseMapKey(keyA);
	int keyBmap = reverseMapKey(keyB);
	
	// Swap them
	keyMap[keyAmap] = keyB;
	keyMap[keyBmap] = keyA;
}

void KeyHandler::setShiftCharacter(int key, int character)
{
	shiftCharMap[key] = character;
}

void KeyHandler::setAltGrCharacter(int key, int character)
{
	altgrCharMap[key] = character;
}

void KeyHandler::setCharacter(int key, int character)
{
	charMap[key] = character;
}
*/

////////////////////////////PRIVATE//////////////////////////////

/*
void KeyHandler::addEvent(int type, char key)
{
	if (events.size() < BUFF_SIZE)
	{
		KeyEvent event;
		event.type = type;
		event.key = key;
		events.push_back(event);
	}
}*/

#endif
