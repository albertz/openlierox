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
	CachedDataPointer<SDL_Surface> bmpScrollbar;
	CachedDataPointer<SDL_Surface> bmpSliderBut;
	CachedDataPointer<SDL_Surface> bmpCommandBtn;
};


// Game graphics
class gfxgame_t { public:
	CachedDataPointer<SDL_Surface> bmpCrosshair;
	CachedDataPointer<SDL_Surface> bmpMuzzle;
	CachedDataPointer<SDL_Surface> bmpExplosion;
	CachedDataPointer<SDL_Surface> bmpSmoke;
	CachedDataPointer<SDL_Surface> bmpChemSmoke;
	CachedDataPointer<SDL_Surface> bmpSpawn;
	CachedDataPointer<SDL_Surface> bmpHook;
	CachedDataPointer<SDL_Surface> bmpGameover;
	CachedDataPointer<SDL_Surface> bmpInGame;
	CachedDataPointer<SDL_Surface> bmpScoreboard;
    CachedDataPointer<SDL_Surface> bmpViewportMgr;
	CachedDataPointer<SDL_Surface> bmpSparkle;
	CachedDataPointer<SDL_Surface> bmpInfinite;
	CachedDataPointer<SDL_Surface> bmpLag;
	CachedDataPointer<SDL_Surface> bmpGameNetBackground;
	CachedDataPointer<SDL_Surface> bmpGameLocalBackground;
	CachedDataPointer<SDL_Surface> bmpGameLocalTopBar;
	CachedDataPointer<SDL_Surface> bmpGameNetTopBar;
	CachedDataPointer<SDL_Surface> bmpTeamColours[4];

	CachedDataPointer<SDL_Surface> bmpBonus;
	CachedDataPointer<SDL_Surface> bmpHealth;
};


extern	gfxgui_t	gfxGUI;
extern	gfxgame_t	gfxGame;


// Routines
void	InitializeColors();
bool	LoadFonts();
bool	LoadGraphics(void);
void	ShutdownGraphics(void);



#endif  //  __GRAPHICS_H__
