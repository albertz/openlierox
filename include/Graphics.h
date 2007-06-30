/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Graphics header file
// Created 30/6/02
// Jason Boettcher


#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__


// Gui graphics
class gfxgui_t { public:
	SDL_Surface		*bmpScrollbar;
	SDL_Surface		*bmpSliderBut;
};


// Game graphics
class gfxgame_t { public:
	SDL_Surface		*bmpCrosshair;
	SDL_Surface		*bmpMuzzle;
	SDL_Surface		*bmpExplosion;
	SDL_Surface		*bmpSmoke;
	SDL_Surface		*bmpChemSmoke;
	SDL_Surface		*bmpSpawn;
	SDL_Surface		*bmpHook;
	SDL_Surface		*bmpGameover;
	SDL_Surface		*bmpInGame;
	SDL_Surface		*bmpScoreboard;
    SDL_Surface		*bmpViewportMgr;
	SDL_Surface		*bmpSparkle;
	SDL_Surface		*bmpInfinite;
	SDL_Surface		*bmpLag;
	SDL_Surface		*bmpGameNetBackground;
	SDL_Surface		*bmpGameLocalBackground;
	SDL_Surface		*bmpGameTopBar;
	SDL_Surface		*bmpTeamColours[4];

	SDL_Surface		*bmpBonus;
	SDL_Surface		*bmpHealth;
};


extern	gfxgui_t	gfxGUI;
extern	gfxgame_t	gfxGame;


// Routines
void	InitializeColors();
bool	LoadFonts();
bool	LoadGraphics(void);
void	ShutdownGraphics(void);



#endif  //  __GRAPHICS_H__
