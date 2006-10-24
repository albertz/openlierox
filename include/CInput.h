/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Input class
// Created 10/12/01
// By Jason Boettcher


#ifndef __CINPUT_H__
#define __CINPUT_H__


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

	int		Type;
	int		Data;
	int		Extra;
	int		Down;


public:
	// Methods

	int		Load(char *name, char *section);
	int		Setup(char *text);
	int		Wait(char *strText);

	int		isUp(void);
	int		isDown(void);
	int		isDownOnce(void);

	void	ClearUpState(void);
};



// Keyboard structure
typedef struct {
	char	text[32];
	int		value;
} keys_t;


// Joystick structure
typedef struct {
	char	text[32];
	int		value;
	int		extra;
} joystick_t;




#endif  //  __CINPUT_H__

