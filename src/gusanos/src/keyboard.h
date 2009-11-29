#ifndef KEYBOARD_h
#define KEYBOARD_h

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV

#include <allegro.h>
#include <boost/signal.hpp>

/*
#include <list>
#define KEY_EVENT_NONE	0
#define KEY_EVENT_PRESS	1
#define KEY_EVENT_RELEASE 2

#define BUFF_SIZE 64

struct KeyEvent
{
	int	type;
	char key;
};
*/

struct StopEarly
{
	typedef bool result_type;

	template<typename InputIterator>
	bool operator()(InputIterator first, InputIterator last) const
	{
		// Stop at the first slot returning false
		for(; first != last; ++first)
		{
			if(!*first)
				return false;
		}
		
		return true;
	}
};

class KeyHandler
{
public:
	
	KeyHandler(void);
	~KeyHandler(void);
	
	void init();
	void shutDown();
	void pollKeyboard(); //Isn't "poll" a better name?
	//KeyEvent getEvent();

	//static int keyMapCallback(int key, int *scancode);
	static int mapKey(int k);
	static bool getKey(int k);
	//static int reverseMapKey(int k);
	//static void swapKeyMapping(int keyA, int keyB);
	//static void setShiftCharacter(int key, int character);
	//static void setAltGrCharacter(int key, int character);
	//static void setCharacter(int key, int character);
	
	boost::signal<bool (int), StopEarly> keyDown;
	boost::signal<bool (int), StopEarly> keyUp;
	boost::signal<bool (char, int), StopEarly> printableChar;
	
private:
	
	//static int keyMap[KEY_MAX]; // The keymap
	//static int charMap[KEY_MAX]; // The character map
	//static int shiftCharMap[KEY_MAX]; // The shift map
	//static int capsCharMap[KEY_MAX]; // The caps lock map
	//static int altgrCharMap[KEY_MAX]; // The altgr lock map
	
	//std::list<KeyEvent> events;
	
	
	
	bool oldKeys[KEY_MAX]; // KEY_MAX is defined by allegro (usually 119)
	
	//void addEvent(int type, char key);
	
};

extern KeyHandler keyHandler;

#endif  // _KEYBOARD_h_
