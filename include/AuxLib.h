/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Auxiliary Library
// Created 12/11/01
// By Jason Boettcher


#ifndef __AUXLIB_H__
#define __AUXLIB_H__


// Initialization sub-systems
#define		AUX_INIT_SDL		0
#define		AUX_INIT_2DVIDEO	1
#define		AUX_INIT_3DVIDEO	2
#define		AUX_INIT_AUDIO		3
#define		AUX_INIT_NETWORK	4
#define		AUX_INIT_TIMER		5
#define		AUX_INIT_GUI		6


#define     MAX_KEYQUEUE        32


// Keyboard structure
typedef struct {
	Uint8	*keys;
	Uint8	KeyUp[SDLK_LAST];
	Uint8	KeyDown[SDLK_LAST];
    int     queueLength;
    int     keyQueue[MAX_KEYQUEUE];
} keyboard_t;


// Mouse structure
typedef struct {
	int		X,Y;
	int		Button;

	int		Up;
	int		Down;
    int     FirstDown;

	int		WheelUp;
	int		WheelDown;
	int		WheelScrollUp;
	int		WheelScrollDown;
} mouse_t;

// Super global variables
//int iRecordingVideo;



// Routines
int			InitializeAuxLib(char *gname, char *config, int bpp, int vidflags);
void		ShutdownAuxLib(void);
int			SetVideoMode(void);

void		ProcessEvents(void);
void        FlipScreen(SDL_Surface *psScreen);
keyboard_t	*GetKeyboard(void);
mouse_t		*GetMouse(void);
SDL_Event	*GetEvent(void);

char		*GetGameName(void);

char		*GetConfigFile(void);

int         GetClipboardText(char *szText, int nMaxLength);
int         SetClipboardText(char *szText);
void        TakeScreenshot(void);


#endif  //  __AUXLIB_H__