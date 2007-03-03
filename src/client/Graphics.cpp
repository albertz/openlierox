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


#include "defs.h"
#include "LieroX.h"
#include "GfxPrimitives.h"


gfxgui_t	gfxGUI;
gfxgame_t	gfxGame;

///////////////////
// Load the graphics
int LoadGraphics(void)
{
	// Initialize the colours
	tLX->clChatText = MakeColour(128,255,128);
	tLX->clSubHeading = MakeColour(143,176,207);
	tLX->clHeading = MakeColour(0,138,251);
	tLX->clNetworkText = MakeColour(200,200,200);
	tLX->clNormalLabel = tLX->clWhite;
	tLX->clNotice = MakeColour(200,200,200);
	tLX->clNormalText = tLX->clWhite;
	tLX->clDropDownText = tLX->clWhite;
	tLX->clDisabled = MakeColour(96,96,96);
	tLX->clListView = tLX->clWhite;
	tLX->clTextBox = tLX->clWhite;
	tLX->clMouseOver = tLX->clWhite;
	tLX->clError = MakeColour(200,50,50);
	tLX->clCredits1 = MakeColour(150,150,150);
	tLX->clCredits2 = MakeColour(96,96,96);
	tLX->clPopupMenu = tLX->clWhite;
	tLX->clWaiting = tLX->clWhite;
	tLX->clReady = MakeColour(0,255,0);
	tLX->clPlayerName = 0xffff; // TODO: ??
	tLX->clBoxDark = MakeColour(60,60,60);
	tLX->clBoxLight = MakeColour(130,130,130);
	tLX->clWinBtnBody = MakeColour(128,128,128);
	tLX->clWinBtnDark = MakeColour(64,64,64);
	tLX->clWinBtnLight = MakeColour(192,192,192);
	tLX->clMPlayerSong = 0;
	tLX->clMPlayerTime = 0;

	int i;
	LOAD_IMAGE(gfxGUI.bmpMouse[0], "data/frontend/mouse.png");
	LOAD_IMAGE(gfxGUI.bmpMouse[1], "data/frontend/mouse_hand.png");
	LOAD_IMAGE(gfxGUI.bmpMouse[2], "data/frontend/mouse_text.png");
	LOAD_IMAGE(gfxGUI.bmpMouse[3], "data/frontend/mouse_resize.png");

	LOAD_IMAGE(gfxGUI.bmpScrollbar,"data/frontend/scrollbar.png");
	LOAD_IMAGE(gfxGUI.bmpSliderBut,"data/frontend/sliderbut.png");

	LOAD_IMAGE(gfxGame.bmpCrosshair,"data/gfx/crosshair.bmp");
	LOAD_IMAGE(gfxGame.bmpMuzzle,"data/gfx/muzzle.bmp");
	LOAD_IMAGE(gfxGame.bmpExplosion,"data/gfx/explosion.png");
	LOAD_IMAGE(gfxGame.bmpSmoke,"data/gfx/smoke.png");
	LOAD_IMAGE(gfxGame.bmpChemSmoke,"data/gfx/chemsmoke.png");
	LOAD_IMAGE(gfxGame.bmpSpawn,"data/gfx/spawn.png");
	LOAD_IMAGE(gfxGame.bmpHook,"data/gfx/hook.bmp");
	LOAD_IMAGE(gfxGame.bmpGameover,"data/gfx/gameover.png");
	LOAD_IMAGE(gfxGame.bmpInGame,"data/gfx/ingame.png");
	LOAD_IMAGE(gfxGame.bmpScoreboard,"data/gfx/scoreboard.png");
    LOAD_IMAGE(gfxGame.bmpViewportMgr,"data/gfx/viewportmgr.png");
	LOAD_IMAGE(gfxGame.bmpSparkle, "data/gfx/sparkle.png");
	LOAD_IMAGE(gfxGame.bmpInfinite,"data/gfx/infinite.png");
	LOAD_IMAGE(gfxGame.bmpLag, "data/gfx/lag.png");

	LOAD_IMAGE(gfxGame.bmpBonus, "data/gfx/bonus.png");
	LOAD_IMAGE(gfxGame.bmpHealth, "data/gfx/health.png");


	if(!tLX->cFont.Load("data/gfx/font.png",true,15))
		return false;
	if(!tLX->cOutlineFont.Load("data/gfx/out_font.png",true,15))
		return false;
	if(!tLX->cOutlineFontGrey.Load("data/gfx/out_fontgrey.png",false,15))
		return false;

	tLX->cOutlineFont.SetOutline(true);
	//tLX->cOutlineFontGrey.SetOutline(true);

	// Set the colour keys
	Uint32 pink = tLX->clPink;
	for(i=0;i<4;i++)
		SDL_SetColorKey(gfxGUI.bmpMouse[i], SDL_SRCCOLORKEY, pink);

	SDL_SetColorKey(gfxGame.bmpCrosshair, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpMuzzle, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpExplosion, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpSmoke, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpChemSmoke, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpSpawn, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpHook, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpBonus, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpHealth, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpSparkle, SDL_SRCCOLORKEY, pink);
    SDL_SetColorKey(gfxGame.bmpViewportMgr, SDL_SRCCOLORKEY, pink);

	// Load the colours from a file
	const std::string colorfile = "data/frontend/colours.cfg";
	ReadColour(colorfile,"Colours","ChatText",		 &tLX->clChatText,		tLX->clChatText);
	ReadColour(colorfile,"Colours","Credits1",		 &tLX->clCredits1,		tLX->clCredits1);
	ReadColour(colorfile,"Colours","Credits2",		 &tLX->clCredits2,		tLX->clCredits2);
	ReadColour(colorfile,"Colours","Disabled",		 &tLX->clDisabled,		tLX->clDisabled);
	ReadColour(colorfile,"Colours","DropDownText",	 &tLX->clDropDownText,	tLX->clDropDownText);
	ReadColour(colorfile,"Colours","Error",			 &tLX->clError,			tLX->clError);
	ReadColour(colorfile,"Colours","Heading",		 &tLX->clHeading,		tLX->clHeading);
	ReadColour(colorfile,"Colours","ListView",		 &tLX->clListView,		tLX->clListView);
	ReadColour(colorfile,"Colours","MouseOver",		 &tLX->clMouseOver,		tLX->clMouseOver);
	ReadColour(colorfile,"Colours","NetworkText",	 &tLX->clNetworkText,	tLX->clNetworkText);
	ReadColour(colorfile,"Colours","NormalLabel",	 &tLX->clNormalLabel,	tLX->clNormalLabel);
	ReadColour(colorfile,"Colours","NormalText",	 &tLX->clNormalText,	tLX->clNormalText);
	ReadColour(colorfile,"Colours","Notice",		 &tLX->clNotice,		tLX->clNotice);
	ReadColour(colorfile,"Colours","PopupMenu",		 &tLX->clPopupMenu,		tLX->clPopupMenu);
	ReadColour(colorfile,"Colours","SubHeading",	 &tLX->clSubHeading,	tLX->clSubHeading);
	ReadColour(colorfile,"Colours","TextBox",		 &tLX->clTextBox,		tLX->clTextBox);
	ReadColour(colorfile,"Colours","Waiting",		 &tLX->clWaiting,		tLX->clWaiting);
	ReadColour(colorfile,"Colours","Ready",			 &tLX->clReady,			tLX->clReady);
	ReadColour(colorfile,"Colours","PlayerName",	 &tLX->clPlayerName,	tLX->clPlayerName);
	ReadColour(colorfile,"Colours","BoxDark",		 &tLX->clBoxDark,		tLX->clBoxDark);
	ReadColour(colorfile,"Colours","BoxLight",		 &tLX->clBoxLight,		tLX->clBoxLight);
	ReadColour(colorfile,"Colours","WinButtonBody",	 &tLX->clWinBtnBody,	tLX->clWinBtnBody);
	ReadColour(colorfile,"Colours","WinButtonDark",  &tLX->clWinBtnDark,	tLX->clWinBtnDark);
	ReadColour(colorfile,"Colours","WinButtonLight", &tLX->clWinBtnLight,	tLX->clWinBtnLight);
	ReadColour(colorfile,"Colours","MPlayerSongTime",&tLX->clMPlayerTime,	tLX->clMPlayerTime);
	ReadColour(colorfile,"Colours","MPlayerSongName",&tLX->clMPlayerSong,	tLX->clMPlayerSong);

	return true;
}


///////////////////
// Shutdown the graphics
void ShutdownGraphics(void)
{
	tLX->cFont.Shutdown();
	tLX->cOutlineFont.Shutdown();
	tLX->cOutlineFontGrey.Shutdown();
}
