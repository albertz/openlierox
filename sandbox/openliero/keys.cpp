#include <SDL/SDL.h>

#include "platform.hpp"
#include <cstddef>
#include <cassert>

int SDLToDOSScanCodes[SDLK_LAST] = {};

#if 0 // OLD
int lieroToSDLKeys[177] =
{
	0,
	SDLK_ESCAPE,
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	SDLK_PLUS,
	SDLK_BACKQUOTE, // `
	SDLK_BACKSPACE, // Backspace
	SDLK_TAB, // Tab
	'q', 'w', 'e', 'r', 't', 'y',
	'u', 'i', 'o', 'p',
	0, // TODO: What sym?
	SDLK_CARET, // ^
	SDLK_RETURN, // Enter
	SDLK_LCTRL, // Left Crtl
	'a', 's', 'd', 'f',
	'g', 'h', 'j', 'k', 'l',
	0, 0, // TODO: What syms?
	0, // TODO: What sym?
	SDLK_LSHIFT,
	SDLK_QUOTE, // '  TODO: Is this right?
	'z', 'x', 'c', 'v',
	'b', 'n', 'm',
	SDLK_COMMA, // ,
	SDLK_PERIOD, // .
	SDLK_MINUS, // -
	SDLK_RSHIFT, // Right Shift
	SDLK_KP_MULTIPLY, // * (Pad)
	SDLK_LALT, // Left Alt
	0, //
	SDLK_CAPSLOCK, // Caps Lock
	SDLK_F1,
	SDLK_F2,
	SDLK_F3,
	SDLK_F4,
	SDLK_F5,
	SDLK_F6,
	SDLK_F7,
	SDLK_F8,
	SDLK_F9,
	SDLK_F10,
	SDLK_NUMLOCK, // Num Lock
	SDLK_SCROLLOCK, // Scroll Lock
	SDLK_KP7,
	SDLK_KP8,
	SDLK_KP9,
	SDLK_KP_MINUS, // - (Pad)
	SDLK_KP4,
	SDLK_KP5,
	SDLK_KP6,
	SDLK_KP_PLUS, // + (Pad)
	SDLK_KP1,
	SDLK_KP2,
	SDLK_KP3,
	SDLK_KP0,
	SDLK_KP_PERIOD, // , (Pad)  TODO: I suppose this is correct, but check
	0, //
	0, //
	SDLK_LESS, // <
	SDLK_F11,
	SDLK_F12,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, // 27 zeroes
	SDLK_KP_ENTER, // Enter (Pad)
	SDLK_RCTRL, // Right Ctrl
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, // 12 zeroes
	0, // Print Screen  TODO: Where is print screen?
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 10 zeroes
	SDLK_KP_DIVIDE, // / (Pad)
	0,
	0, // Print Screen  TODO: Where is print screen?
	SDLK_RALT, // Right Alt
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, // 14 zeroes
	SDLK_HOME, // Home
	SDLK_UP, // Up
	SDLK_PAGEUP, // Page Up
	0,
	SDLK_LEFT, // Left
	0,
	SDLK_RIGHT, // Right
	0,
	SDLK_END, // End
	SDLK_DOWN, // Down
	SDLK_PAGEDOWN, // Page Down
	SDLK_INSERT, // Insert
	SDLK_DELETE, // Delete
	0, 0, 0, 0, 0 // 5 zeroes
};
#else
SDLKey const Z = SDLK_UNKNOWN;
SDLKey lieroToSDLKeys[] =
{
	SDLK_UNKNOWN,SDLK_ESCAPE,
	SDLK_1,SDLK_2,SDLK_3,SDLK_4,SDLK_5,SDLK_6,SDLK_7,SDLK_8,SDLK_9,SDLK_0,
	/* 0x0c: */
	SDLK_MINUS,SDLK_EQUALS,SDLK_BACKSPACE,SDLK_TAB,
	SDLK_q,SDLK_w,SDLK_e,SDLK_r,SDLK_t,SDLK_y,SDLK_u,SDLK_i,SDLK_o,SDLK_p,
	SDLK_LEFTBRACKET,SDLK_RIGHTBRACKET,SDLK_RETURN,SDLK_LCTRL,
	SDLK_a,SDLK_s,SDLK_d,SDLK_f,SDLK_g,SDLK_h,SDLK_j,SDLK_k,SDLK_l,
	SDLK_SEMICOLON,SDLK_QUOTE,SDLK_BACKQUOTE,SDLK_LSHIFT,SDLK_BACKSLASH,
	SDLK_z,SDLK_x,SDLK_c,SDLK_v,SDLK_b,SDLK_n,SDLK_m,
	/* 0x33: */
	SDLK_COMMA,SDLK_PERIOD,SDLK_SLASH,SDLK_RSHIFT,SDLK_KP_MULTIPLY,
	SDLK_LALT,SDLK_SPACE,SDLK_CAPSLOCK,
	SDLK_F1,SDLK_F2,SDLK_F3,SDLK_F4,SDLK_F5,SDLK_F6,SDLK_F7,SDLK_F8,SDLK_F9,SDLK_F10,
	/* 0x45: */
	SDLK_NUMLOCK,SDLK_SCROLLOCK,
	SDLK_KP7,SDLK_KP8,SDLK_KP9,SDLK_KP_MINUS,SDLK_KP4,SDLK_KP5,SDLK_KP6,SDLK_KP_PLUS,
	SDLK_KP1,SDLK_KP2,SDLK_KP3,SDLK_KP0,SDLK_KP_PERIOD,
	SDLK_UNKNOWN,SDLK_UNKNOWN,
	SDLK_LESS,SDLK_F11,SDLK_F12,
	/*
	Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,
	Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,
	Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,
	Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z*/
	
	
	Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z,Z, // 27 zeroes
	SDLK_KP_ENTER, // Enter (Pad)
	SDLK_RCTRL, // Right Ctrl
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, // 12 zeroes
	Z, // Print Screen  TODO: Where is print screen?
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, // 10 zeroes
	SDLK_KP_DIVIDE, // / (Pad)
	Z,
	Z, // Print Screen  TODO: Where is print screen?
	SDLK_RALT, // Right Alt
	Z, Z, Z, Z, Z, Z, Z, Z, Z, Z,
	Z, Z, Z, Z, // 14 zeroes
	SDLK_HOME, // Home
	SDLK_UP, // Up
	SDLK_PAGEUP, // Page Up
	Z,
	SDLK_LEFT, // Left
	Z,
	SDLK_RIGHT, // Right
	Z,
	SDLK_END, // End
	SDLK_DOWN, // Down
	SDLK_PAGEDOWN, // Page Down
	SDLK_INSERT, // Insert
	SDLK_DELETE, // Delete
	Z, Z, Z, Z, Z // 5 zeroes
};

Uint32 const maxScanCodes = sizeof(lieroToSDLKeys) / sizeof(*lieroToSDLKeys);
#endif

void initKeys()
{
#if 0
#if defined (LIERO_MACOSX)
		/* nothing */
#elif defined(LIERO_OS2)
		lieroToSDLKeys[0x61]=SDLK_UP;
		lieroToSDLKeys[0x66]=SDLK_DOWN;
		lieroToSDLKeys[0x63]=SDLK_LEFT;
		lieroToSDLKeys[0x64]=SDLK_RIGHT;
		lieroToSDLKeys[0x60]=SDLK_HOME;
		lieroToSDLKeys[0x65]=SDLK_END;
		lieroToSDLKeys[0x62]=SDLK_PAGEUP;
		lieroToSDLKeys[0x67]=SDLK_PAGEDOWN;
		lieroToSDLKeys[0x68]=SDLK_INSERT;
		lieroToSDLKeys[0x69]=SDLK_DELETE;
		lieroToSDLKeys[0x5C]=SDLK_KP_DIVIDE;
		lieroToSDLKeys[0x5A]=SDLK_KP_ENTER;
		lieroToSDLKeys[0x5B]=SDLK_RCTRL;
		lieroToSDLKeys[0x5F]=SDLK_PAUSE;
//		lieroToSDLKeys[0x00]=SDLK_PRINT;
		lieroToSDLKeys[0x5E]=SDLK_RALT;
		lieroToSDLKeys[0x40]=SDLK_KP5;
		lieroToSDLKeys[0x41]=SDLK_KP6;
#elif !defined (LIERO_WIN32) /* => Linux */
		lieroToSDLKeys[0x5a]=SDLK_UP;
		lieroToSDLKeys[0x60]=SDLK_DOWN;
		lieroToSDLKeys[0x5c]=SDLK_LEFT;
		lieroToSDLKeys[0x5e]=SDLK_RIGHT;
		lieroToSDLKeys[0x59]=SDLK_HOME;
		lieroToSDLKeys[0x5f]=SDLK_END;
		lieroToSDLKeys[0x5b]=SDLK_PAGEUP;
		lieroToSDLKeys[0x61]=SDLK_PAGEDOWN;
		lieroToSDLKeys[0x62]=SDLK_INSERT;
		lieroToSDLKeys[0x63]=SDLK_DELETE;
		lieroToSDLKeys[0x68]=SDLK_KP_DIVIDE;
		lieroToSDLKeys[0x64]=SDLK_KP_ENTER;
		lieroToSDLKeys[0x65]=SDLK_RCTRL;
		lieroToSDLKeys[0x66]=SDLK_PAUSE;
		lieroToSDLKeys[0x67]=SDLK_PRINT;
		lieroToSDLKeys[0x69]=SDLK_RALT;
#else
		lieroToSDLKeys[0xc8]=SDLK_UP;
		lieroToSDLKeys[0xd0]=SDLK_DOWN;
		lieroToSDLKeys[0xcb]=SDLK_LEFT;
		lieroToSDLKeys[0xcd]=SDLK_RIGHT;
		lieroToSDLKeys[0xc7]=SDLK_HOME;
		lieroToSDLKeys[0xcf]=SDLK_END;
		lieroToSDLKeys[0xc9]=SDLK_PAGEUP;
		lieroToSDLKeys[0xd1]=SDLK_PAGEDOWN;
		lieroToSDLKeys[0xd2]=SDLK_INSERT;
		lieroToSDLKeys[0xd3]=SDLK_DELETE;
		lieroToSDLKeys[0xb5]=SDLK_KP_DIVIDE;
		lieroToSDLKeys[0x9c]=SDLK_KP_ENTER;
		lieroToSDLKeys[0x9d]=SDLK_RCTRL;
		lieroToSDLKeys[0xc5]=SDLK_PAUSE;
		lieroToSDLKeys[0xb7]=SDLK_PRINT;
		lieroToSDLKeys[0xb8]=SDLK_RALT;
#endif
#endif

	for(std::size_t i = 0; i < sizeof(SDLToDOSScanCodes) / sizeof(*SDLToDOSScanCodes); ++i)
	{
		SDLToDOSScanCodes[i] = 89;
	}
	
	for(std::size_t i = 0; i < maxScanCodes; ++i)
	{
		if(lieroToSDLKeys[i] && lieroToSDLKeys[i] < SDLK_LAST)
		{
			SDLToDOSScanCodes[lieroToSDLKeys[i]] = int(i);
		}
	}
}

// Adapted from DOSBOX

SDLKey DOSToSDLKey(Uint32 scan)
{
	if(scan < maxScanCodes)
		return lieroToSDLKeys[scan];
	else
		return SDLK_UNKNOWN;
}

Uint32 SDLToDOSKey(SDLKey key)
{
	if(key < SDLK_LAST)
		return SDLToDOSScanCodes[Uint32(key)];
	return 0;
}

Uint32 SDLToDOSKey(SDL_keysym const& keysym)
{
	Uint32 key = Uint32(keysym.scancode);
	key = SDLToDOSScanCodes[Uint32(keysym.sym)];
	
#if 0
	if (key==0
#if defined (LIERO_MACOSX)
	    /* On Mac on US keyboards, scancode 0 is actually the 'a'
	     * key.  For good measure exclude all printables from this
	     * condition. */
	    && (keysym.sym < SDLK_SPACE || keysym.sym > SDLK_WORLD_95)
#endif
	)
	{
		/* try to retrieve key from symbolic key as scancode is zero */
		if (keysym.sym < SDLK_LAST) key = SDLToDOSScanCodes[Uint32(keysym.sym)];
	} 
#if !defined (LIERO_WIN32) && !defined (LIERO_MACOSX) && !defined(LIERO_OS2)
	/* Linux adds 8 to all scancodes */
	else key -= 8;
#endif
#if defined (LIERO_WIN32)
	switch (key)
	{
		case 0x1c:	// ENTER
		case 0x35:	// SLASH
		case 0x45:	// PAUSE
		case 0x47:	// HOME
		case 0x48:	// cursor UP
		case 0x49:	// PAGE UP
		case 0x4b:	// cursor LEFT
		case 0x4d:	// cursor RIGHT
		case 0x4f:	// END
		case 0x50:	// cursor DOWN
		case 0x51:	// PAGE DOWN
		case 0x52:	// INSERT
		case 0x53:	// DELETE
#if 0 // TODO: Where is GFX_SDLUsingWinDIB defined in DOSBOX?
			if (GFX_SDLUsingWinDIB())
				key = SDLToDOSScanCodes[Uint32(keysym.sym)];
#endif
			break;
	}
#endif
#endif
	if(key >= 177) // Liero doesn't have keys >= 177
		return 89; // Arbitrarily translate it to 89
	return key;

}
