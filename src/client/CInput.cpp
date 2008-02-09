/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Input class
// Created 10/12/01
// By Jason Boettcher

#include <assert.h>
#include <iostream>

#include "LieroX.h"
#include "AuxLib.h"  // for GetConfig()
#include "Error.h"
#include "ConfigHandler.h"
#include "InputEvents.h"
#include "StringUtils.h"
#include "CInput.h"


using namespace std;


keys_t Keys[] = {
	{ "", 0 },
	{ "a", SDLK_a },
	{ "b", SDLK_b },
	{ "c", SDLK_c },
	{ "d", SDLK_d },
	{ "e", SDLK_e },
	{ "f", SDLK_f },
	{ "g", SDLK_g },
	{ "h", SDLK_h },
	{ "i", SDLK_i },
	{ "j", SDLK_j },
	{ "k", SDLK_k },
	{ "l", SDLK_l },
	{ "m", SDLK_m },
	{ "n", SDLK_n },
	{ "o", SDLK_o },
	{ "p", SDLK_p },
	{ "q", SDLK_q },
	{ "r", SDLK_r },
	{ "s", SDLK_s },
	{ "t", SDLK_t },
	{ "u", SDLK_u },
	{ "v", SDLK_v },
	{ "w", SDLK_w },
	{ "x", SDLK_x },
	{ "y", SDLK_y },
	{ "z", SDLK_z },
	{ "1", SDLK_1 },
	{ "2", SDLK_2 },
	{ "3", SDLK_3 },
	{ "4", SDLK_4 },
	{ "5", SDLK_5 },
	{ "6", SDLK_6 },
	{ "7", SDLK_7 },
	{ "8", SDLK_8 },
	{ "9", SDLK_9 },
	{ "0", SDLK_0 },
	{ "kp 0", SDLK_KP0},
	{ "kp 1", SDLK_KP1},
	{ "kp 2", SDLK_KP2},
	{ "kp 3", SDLK_KP3},
	{ "kp 4", SDLK_KP4},
	{ "kp 5", SDLK_KP5},
	{ "kp 6", SDLK_KP6},
	{ "kp 7", SDLK_KP7},
	{ "kp 8", SDLK_KP8},
	{ "kp 9", SDLK_KP9},
	{ "kp .",SDLK_KP_PERIOD },
	{ "kp enter",SDLK_KP_ENTER },
	{ "kp /",SDLK_KP_DIVIDE },
	{ "kp *",SDLK_KP_MULTIPLY },
	{ "kp -",SDLK_KP_MINUS },
	{ "kp +",SDLK_KP_PLUS },
	{ "kp =",SDLK_KP_EQUALS },
	{ "[",SDLK_LEFTBRACKET },
	{ "]",SDLK_RIGHTBRACKET },
	{ "\\",SDLK_BACKSLASH },
	{ "/",SDLK_SLASH},
	{ ":", SDLK_COLON},
	{ ";", SDLK_SEMICOLON},
	{ "<", SDLK_LESS},
	{ "=", SDLK_EQUALS},
	{ ">", SDLK_GREATER},
	{ "?", SDLK_QUESTION},
	{ "-", SDLK_MINUS},
	{ "'", SDLK_QUOTE},
	{ ",", SDLK_COMMA},
	{ ".", SDLK_PERIOD},
	{ "enter", SDLK_RETURN},
	{ "tab", SDLK_TAB},
	{ "space",SDLK_SPACE },
	{ "up",SDLK_UP },
	{ "down",SDLK_DOWN },
	{ "right",SDLK_RIGHT },
	{ "left",SDLK_LEFT },
	{ "lctrl",SDLK_LCTRL },
	{ "lshift",SDLK_LSHIFT },
	{ "lalt",SDLK_LALT },
	{ "lmeta",SDLK_LMETA },
	{ "lsuper",SDLK_LSUPER },
	{ "rctrl",SDLK_RCTRL },
	{ "rshift",SDLK_RSHIFT },
	{ "ralt",SDLK_RALT },
	{ "rmeta",SDLK_RMETA },
	{ "rsuper",SDLK_RSUPER },
	{ "insert", SDLK_INSERT},
	{ "home", SDLK_HOME},
	{ "pg up", SDLK_PAGEUP},
	{ "end", SDLK_END},
	{ "pg Dn", SDLK_PAGEDOWN},
	{ "delete", SDLK_DELETE},
	{ "num lk", SDLK_NUMLOCK},
	{ "caps", SDLK_CAPSLOCK},
	{ "scr lk", SDLK_SCROLLOCK},
	{ "F1", SDLK_F1 },
	{ "F2", SDLK_F2 },
	{ "F3", SDLK_F3 },
	{ "F4", SDLK_F4 },
	{ "F5", SDLK_F5 },
	{ "F6", SDLK_F6 },
	{ "F7", SDLK_F7 },
	{ "F8", SDLK_F8 },
	{ "F9", SDLK_F9 },
	{ "F10", SDLK_F10 },
	{ "F11", SDLK_F11 },
	{ "F12", SDLK_F12 }
	};


joystick_t Joysticks[] = {
	{ "joy1_up",JOY_UP,0},
	{ "joy1_down",JOY_DOWN,0},
	{ "joy1_left",JOY_LEFT,0},
	{ "joy1_right",JOY_RIGHT,0},
	{ "joy1_but1",JOY_BUTTON,0},
	{ "joy1_but2",JOY_BUTTON,1},
	{ "joy1_but3",JOY_BUTTON,2},
	{ "joy1_but4",JOY_BUTTON,3},
	{ "joy1_but5",JOY_BUTTON,4},
	{ "joy1_but6",JOY_BUTTON,5},
	{ "joy1_but7",JOY_BUTTON,6},
	{ "joy1_but8",JOY_BUTTON,7},
	{ "joy1_but9",JOY_BUTTON,8},
	{ "joy2_up",JOY_UP,0},
	{ "joy2_down",JOY_DOWN,0},
	{ "joy2_left",JOY_LEFT,0},
	{ "joy2_right",JOY_RIGHT,0},
	{ "joy2_but1",JOY_BUTTON,0},
	{ "joy2_but2",JOY_BUTTON,1},
	{ "joy2_but3",JOY_BUTTON,2},
	{ "joy2_but4",JOY_BUTTON,3},
	{ "joy2_but5",JOY_BUTTON,4},
	{ "joy2_but6",JOY_BUTTON,5},
	{ "joy2_but7",JOY_BUTTON,6},
	{ "joy2_but8",JOY_BUTTON,7},
	{ "joy2_but9",JOY_BUTTON,8},
};

static SDL_Joystick* joys[2] = {NULL, NULL};

bool checkJoystickState(int flag, int extra, SDL_Joystick* joy) {
	if(!bJoystickSupport) return false;
	if(joy == NULL) return false;
	
	int val;
	
	// TODO: atm these limits are hardcoded; make them constants (or perhaps also configurable)
	switch(flag) {
		case JOY_UP:
			val = SDL_JoystickGetAxis(joy,1);
			if(val < -3200)
				return true;
			break;
		case JOY_DOWN:
			val = SDL_JoystickGetAxis(joy,1);
			if(val > 3200)
				return true;
			break;
		case JOY_LEFT:
			val = SDL_JoystickGetAxis(joy,0);
			if(val < -3200)
				return true;
			break;
		case JOY_RIGHT:
			val = SDL_JoystickGetAxis(joy,0);
			if(val > 3200)
				return true;
			break;
		case JOY_BUTTON:
			if(SDL_JoystickGetButton(joy,extra))
				return true;
			break;
			
		default:
			printf("WARNING: checkJoystickState: unknown flag\n");
	}
	
	return false;
}

static bool joysticks_inited_temp[2] = {false, false};

void initJoystick(int i, bool isTemp) {
	assert(i == 0 || i == 1);
	if(!bJoystickSupport) return;
	
	if(joys[i] == NULL && SDL_NumJoysticks() > i && !SDL_JoystickOpened(i)) {
		printf("opening joystick %i", i);
		printf(" (\"%s\")\n", SDL_JoystickName(i));
		joys[i] = SDL_JoystickOpen(0);
		if(joys[i]) {
			printf("  Number of Axes: %d\n", SDL_JoystickNumAxes(joys[i]));
			printf("  Number of Buttons: %d\n", SDL_JoystickNumButtons(joys[i]));
			printf("  Number of Balls: %d\n", SDL_JoystickNumBalls(joys[i]));
			if(isTemp) joysticks_inited_temp[i] = true;
		} else
			printf("WARNING: could not open joystick\n");
	}

	if(!isTemp) joysticks_inited_temp[i] = false;
}

void CInput::InitJoysticksTemp() {
	if(!bJoystickSupport) return;
	printf("initing joysticks temporary...\n");
	printf("amout of available joysticks: %i\n", SDL_NumJoysticks());
	initJoystick(0, true);
	initJoystick(1, true);
}

void uninitTempJoystick(int i) {
	if(joysticks_inited_temp[i] && SDL_JoystickOpened(i)) {
		SDL_JoystickClose(joys[i]);
		joys[i] = NULL;
		joysticks_inited_temp[i] = false;
	}
}

void CInput::UnInitJoysticksTemp() {
	uninitTempJoystick(0);
	uninitTempJoystick(1);
}

///////////////////
// Load the input from a config file
int CInput::Load(const std::string& name, const std::string& section)
{
	std::string string;

	Down = false;

	if(!ReadString(GetConfigFile(),section,name,string,""))
		return false;

	return Setup(string);
}


///////////////////
// Waits for any input (used in a loop)
int CInput::Wait(std::string& strText)
{
	mouse_t *Mouse = GetMouse();
	keyboard_t *kb = GetKeyboard();
	unsigned int n,i;

	// First check the mouse
	for(n = 0; n < MAX_MOUSEBUTTONS; n++) {
		i = n;
		if(Mouse->Up & SDL_BUTTON(n)) {
			// Swap rmb id wih mmb (mouse buttons)
			switch (n)  {
			case 2: i=3; break;
			case 3: i=2; break;
			}
			strText = "ms"+itoa(i);
			return true;
		}
	}

	// Keyboard
	if (kb->queueLength > 0)  {
		for(n = 0; n<sizeof(Keys) / sizeof(keys_t); n++) {
			if(kb->keyQueue[0].sym == Keys[n].value) {
#ifdef WIN32
				// Workaround for right alt key which is reported as LCTRL + RALT on windib driver
				if (kb->queueLength > 1 && kb->keyQueue[0].sym == SDLK_LCTRL)  {
					if (kb->keyQueue[1].sym == SDLK_RALT)  {
						strText = "ralt";
						return true;
					}
				}
#endif

				strText = Keys[n].text;
				return true;
			}
		}

		// Our description is not enough, let's call SDL for help
		// TODO: perhaps use SDL for everything?
		if (kb->keyQueue[0].sym != SDLK_ESCAPE)  {
			strText = SDL_GetKeyName((SDLKey)kb->keyQueue[0].sym);
			return true;
		}
	}

	// joystick
	// TODO: more joysticks
	for(n = 0; n < sizeof(Joysticks) / sizeof(joystick_t); n++) {
		int i = Joysticks[n].text[3] - '1'; // at pos 3, there is the number ("joy1_...")
		if(joys[i] != NULL && checkJoystickState(Joysticks[n].value, Joysticks[n].extra, joys[i])) {
			strText = Joysticks[n].text;
			return true;
		}
	}

	return false;
}


///////////////////
// Setup
int CInput::Setup(const std::string& string)
{
	unsigned int n;

	m_EventName = string;
	Down = false;

	// Check if it's a mouse
	if(string.substr(0,2) == "ms") {
		Type = INP_MOUSE;
		Data = atoi(string.substr(2).c_str());
		if( Data == 3 ) Data = 2;
		else if( Data == 2 ) Data = 3;
		return true;
	}


	// Check if it's a joystick #1
	// TODO: allow more joysticks
	if(string.substr(0,5) == "joy1_") {
		Type = INP_JOYSTICK1;
		Data = 0;

		// Make sure there is a joystick present
		if(SDL_NumJoysticks()<=0) {
			SetError("Could not open joystick1");
			return false;
		}

		// Open the joystick if it hasn't been already opened
		initJoystick(0, false);

		// Go through the joystick list
		for(n=0;n<sizeof(Joysticks) / sizeof(joystick_t);n++) {
			if(Joysticks[n].text == string) {
				Data = Joysticks[n].value;
				Extra = Joysticks[n].extra;
				return true;
			}
		}
	}

	// Check if it's a joystick #2
	if(string.substr(0,5) == "joy2_") {
		Type = INP_JOYSTICK2;
		Data = 0;

		// Make sure there is a joystick present
		if(SDL_NumJoysticks()<=1)
			return false;

		// Open the joystick if it hasn't been already opened
		initJoystick(1, false);
		
		// Go through the joystick list
		for(n=0;n<sizeof(Joysticks) / sizeof(joystick_t);n++) {
			if(Joysticks[n].text == string) {
				Data = Joysticks[n].value;
				Extra = Joysticks[n].extra;
				return true;
			}
		}
	}


	// Must be a keyboard character
	Type = INP_KEYBOARD;
	Data = 0;

	// Go through the key list checking with piece of text it was
	// TODO: allow other unknown keys
	for(n=0;n<sizeof(Keys) / sizeof(keys_t);n++) {
		if(Keys[n].text == string) {
			Data = Keys[n].value;
			return true;
		}
	}

	return false;
}


///////////////////
// Returns if the input has just been released
int CInput::isUp(void)
{
	keyboard_t	*Keyb = GetKeyboard();
	mouse_t		*Mouse = GetMouse();


	switch(Type) {

		// Keyboard
		case INP_KEYBOARD:
#ifdef WIN32
			// Workaround for right alt key which is reported as LCTRL + RALT on windib driver
			if (Keyb->queueLength > 1 && Data == SDLK_RALT)
				if (Keyb->keyQueue[0].sym == SDLK_LCTRL && Keyb->keyQueue[1].sym == SDLK_RALT && !Keyb->keyQueue[1].down)
					return true;
#endif

			if(Keyb->KeyUp[Data])
				return true;
			break;

		// Mouse
		case INP_MOUSE:
			// TODO: Calculate mouse_up for ALL the buttons
			if(Mouse->Up)
				return true;
			break;

		// Joystick
		case INP_JOYSTICK1:
		case INP_JOYSTICK2:
			return !isDown();

	}

	return false;
}


///////////////////
// Returns if the input is down
int CInput::isDown(void)
{
	keyboard_t	*Keyb = GetKeyboard();
	mouse_t		*Mouse = GetMouse();

	switch(Type) {

		// Keyboard
		case INP_KEYBOARD:
			if(wasDown()) return true; // in this case, we want to return true here to get it recognised at least once
#ifdef WIN32
			// TODO: please remove this hack here
			// Workaround for right alt key which is reported as LCTRL + RALT on windib driver
			if (Keyb->queueLength > 1 && Data == SDLK_RALT)
				// TODO: why are only the first and second key-events checked? what if they are later
				// what if first event was in previous frame?
				// what if first event is a keyup-event?
				if (Keyb->keyQueue[0].sym == SDLK_LCTRL && Keyb->keyQueue[1].sym == SDLK_RALT && Keyb->keyQueue[1].down)
					return true;
#endif

			if(Keyb->KeyDown[Data])
				return true;
			break;

		// Mouse
		case INP_MOUSE:
			if(Mouse->Button & SDL_BUTTON(Data))
				return true;
			break;

		// Joystick
		case INP_JOYSTICK1:
			return checkJoystickState(Data, Extra, joys[0]);
		case INP_JOYSTICK2:
			return checkJoystickState(Data, Extra, joys[1]);

	}

	return false;
}


///////////////////
// Returns if the input was pushed down once
bool CInput::isDownOnce(void)
{
	// HINT: It is possible that wasUp() and !Down (a case which is not covered in further code)
	if(wasUp() && !Down)
		return true;
	
	// HINT: It's possible that wasDown() > 0 and !isDown().
	// That is the case when we press a key and release it directly after (in one frame).
	// Though wasDown() > 0 doesn't mean directly isDownOnce because it also counts keypresses.
	// HINT: It's also possible that wasDown() == 0 and isDown().
	// That is the case when we have pressed the key in a previous frame and we still hold it
	// and the keyrepeat-interval is bigger than FPS. (Rare case.)
	if(wasDown() || isDown()) {
		// wasUp() > 0 always means that it was down once (though it is not down anymore).
		if(wasUp()) {
			Down = false;
			return true;
		}
		// !Down means that we haven't recognised yet that it is down.
		if(!Down) {
			Down = isDown();
			return true;
		}
	}
	else
		Down = false;

	return false;
}

// goes through the event-signals and searches for the event
int CInput::wasDown() {
	int counter = 0;
	
	switch(Type) {
	case INP_KEYBOARD:
		for(short i = 0; i < GetKeyboard()->queueLength; i++) {
			if(GetKeyboard()->keyQueue[i].down && GetKeyboard()->keyQueue[i].sym == Data)
				counter++;
		}
		return counter;
	
	case INP_MOUSE:
		// TODO: to make this possible, we need to go extend HandleNextEvent to save the mouse events
		counter = isDown() ? 1 : 0; // no other way at the moment
		
	case INP_JOYSTICK1:
	case INP_JOYSTICK2:
		counter = isDown() ? 1 : 0; // no other way at the moment
	}
		
	return 0;
}

// goes through the event-signals and searches for the event
int CInput::wasUp() {
	int counter = 0;
	
	switch(Type) {
	case INP_KEYBOARD:
		for(short i = 0; i < GetKeyboard()->queueLength; i++) {
			if(!GetKeyboard()->keyQueue[i].down && GetKeyboard()->keyQueue[i].sym == Data)
				counter++;
		}
		return counter;
	
	case INP_MOUSE:
		// TODO: to make this possible, we need to go extend HandleNextEvent to save the mouse events
		counter = isUp() ? 1 : 0; // no other way at the moment
		
	case INP_JOYSTICK1:
	case INP_JOYSTICK2:
		counter = isUp() ? 1 : 0; // no other way at the moment
	}
		
	return 0;
}



///////////////////
// Clear the up state of the input device
void CInput::ClearUpState(void)
{
	keyboard_t	*Keyb = GetKeyboard();
	mouse_t		*Mouse = GetMouse();


	switch(Type) {

		// Keyboard
		case INP_KEYBOARD:
			Keyb->KeyUp[Data] = false;
			break;

		// Mouse
		case INP_MOUSE:
			// TODO: Calculate mouse_up for ALL the buttons
			Mouse->Up = false;
			break;

		// Joystick
		case INP_JOYSTICK1:
		case INP_JOYSTICK2:
			// TODO: is this needed?
			break;
	}
}
