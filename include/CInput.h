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


#ifndef __CINPUT_H__
#define __CINPUT_H__

#include <string>


// Input variable types
#define		INP_KEYBOARD		0
#define		INP_MOUSE			1
#define		INP_JOYSTICK1		2
#define		INP_JOYSTICK2		3


// Joystick data
#define		JOY_UP				0
#define		JOY_DOWN			1
#define		JOY_LEFT			2
#define		JOY_RIGHT			3
#define		JOY_BUTTON			4


class CInput {
public:
	// Constructor
	CInput() {
		Type = INP_KEYBOARD;
		Data = 0;
		Extra = 0;
		Down = false;

	}


private:
	// Attributes

	int		Type; // keyboard, mouse or joystick
	int		Data;
	int		Extra;
	int		Down;
	std::string m_EventName;


public:
	// Methods

	int		Load(const std::string& name, const std::string& section);
	int		Setup(const std::string& text);
	static void InitJoysticksTemp(); // call this if CInput::Wait shall recognise joystick events
	static void UnInitJoysticksTemp();
	static int Wait(std::string& strText); // TODO: change this name. this function doesn't realy wait, it just checks the event-state
	int		getData() { return Data; }
	int		getType() { return Type; }
	bool	isJoystick() { return Type == INP_JOYSTICK1 || Type == INP_JOYSTICK2; }
	
	int		isUp(void);
	int		isDown(void);
	bool	isDownOnce(void);
	int		wasDown(); // checks if there was such an event in the queue; returns the count of presses (down-events)
	int		wasUp(); // checks if there was an keyup-event; returns the count of up-events
	
	std::string getEventName() { return m_EventName; }

	void	ClearUpState(void); // TODO: why is this needed? and this should be removed as it does not good things (modifies things it should not modify)
};



// Keyboard structure
class keys_t { public:
#ifndef _MSC_VER
	std::string	text;
#else
	// TODO: this is absolute no solution!
	char text[16];
#endif
	int		value;
};


// Joystick structure
class joystick_t { public:
#ifndef _MSC_VER
	std::string	text;
#else
	// TODO: this is absolute no solution!
	char text[16];
#endif
	int		value;
	int		extra;
};




#endif  //  __CINPUT_H__

