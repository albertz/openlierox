/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Game header file
// Created 28/6/02
// Jason Boettcher

#ifndef __LIEROX_H__
#define __LIEROX_H__


// HINT: for AI debug define _AI_DEBUG in your IDE/compiler


#include <list>
#include <string>
#include <setjmp.h>
#include "Options.h"
#include "CFont.h"
#include "CVec.h"
#include "Consts.h"
#include "CInput.h"
#include "types.h"

class profile_t;
class IpToCountryDB;



// Game types
enum GameType_t {
	GME_LOCAL=0,
	GME_HOST,
	GME_JOIN
};

// LieroX structure
struct lierox_t {
	lierox_t() :
			bVideoModeChanged(false), bQuitGame(false), bQuitEngine(false), bQuitCtrlC(false)
			, iGameType(GME_LOCAL), bHosted(false) {}
	AbsTime	currentTime;
	TimeDiff	fDeltaTime;
	TimeDiff	fRealDeltaTime; // Delta time used for network synchronization,
							// it is not clamped unlike the above one
	CFont	cFont;
	CFont	cOutlineFont;

	bool	bVideoModeChanged;

	bool	bQuitGame;
	bool	bQuitEngine;
	bool	bQuitCtrlC;

	int		debug_int;
	float	debug_float;
	CVec	debug_pos;
	
	GameType_t iGameType;
	bool	bHosted;  // True if the user has run the server at least once this session

	// Default Colours
	Color			clNormalLabel;
	Color			clHeading;
	Color			clSubHeading;
	Color			clChatText;
	Color			clNetworkText;
	Color			clPrivateText;
	Color			clNormalText;
	Color			clNotice;
	Color			clDropDownText;
	Color			clDisabled;
	Color			clListView;
	Color			clTextBox;
	Color			clMouseOver;
	Color			clError;
	Color			clCredits1;
	Color			clCredits2;
	Color			clPopupMenu;
	Color			clWaiting;
	Color			clReady;
	Color			clPlayerName;
	Color			clBoxLight;
	Color			clBoxDark;
	Color			clWinBtnBody;
	Color			clWinBtnLight;
	Color			clWinBtnDark;
	Color			clMPlayerTime;
	Color			clMPlayerSong;
	Color			clChatBoxBackground;
	Color			clDialogBackground;
	Color			clGameBackground;
	Color			clViewportSplit;
	Color			clScrollbarBack;
	Color			clScrollbarBackLight;
	Color			clScrollbarFront;
	Color			clScrollbarHighlight;
	Color			clScrollbarShadow;
	Color			clSliderLight;
	Color			clSliderDark;
	Color			clCurrentSettingsBg;
	Color			clScoreBackground;
	Color			clScoreHighlight;
	Color			clDialogCaption;
	Color			clPlayerDividingLine;
	Color			clLine;
	Color			clProgress;
	Color			clListviewSelected;
	Color			clComboboxSelected;
	Color			clComboboxShowAllMain;
	Color			clComboboxShowAllBorder;
	Color			clMenuSelected;
	Color			clMenuBackground;
	Color			clGameChatter;
	Color			clSelection;
	Color			clTextboxCursor;
	Color			clGameChatCursor;
	Color			clConsoleCursor;
	Color			clConsoleNormal;
	Color			clConsoleNotify;
	Color			clConsoleError;
	Color			clConsoleWarning;
	Color			clConsoleDev;
	Color			clConsoleChat;
	Color			clReturningToLobby;
	Color			clTeamColors[4];
	Color			clTagHighlight;
	Color			clHealthLabel;
	Color			clWeaponLabel;
	Color			clLivesLabel;
	Color			clKillsLabel;
	Color			clFPSLabel;
	Color			clPingLabel;
	Color			clSpecMsgLabel;
	Color			clLoadingLabel;
	Color			clIpLoadingLabel;
	Color			clWeaponSelectionTitle;
	Color			clWeaponSelectionActive;
	Color			clWeaponSelectionDefault;
	Color			clRopeColors[2];
	Color			clLaserSightColors[2];
	Color			clTimeLeftLabel;
	Color			clTimeLeftWarnLabel;


	Color			clPink;
	Color			clWhite;
	Color			clBlack;


	std::string	debug_string;


	CInput		cTakeScreenshot;
	CInput		cSwitchMode;
	CInput		cIrcChat;
	CInput		cConsoleToggle;
	void setupInputs();
	bool isAnyControlKeyDown() const;
};


// Object structure (for maprandom_t)
// HINT: DON'T change the variable types because they are saved directly to the file (CMap.cpp)
class object_t { public:
	Sint32		Type;
	Sint32		Size;
	Sint32     X, Y;
};


/*
// Random map data
// TODO: Move in lierox_t if needed, actually random map layouts not used anymore, so it should be removed
class maprandom_t { public:
	maprandom_t()  { psObjects = NULL; bUsed = false; }
    bool        bUsed;
    std::string szTheme;
    int         nNumObjects;
    object_t    *psObjects;
};
*/



extern	lierox_t		*tLX;
extern  bool			bDisableSound; // only true in dedicated mode or if soundinit failed; it's false even if you did not activate sound
extern	bool			bDedicated;
extern  bool			bJoystickSupport;
extern  bool			bRestartGameAfterQuit;

#ifndef WIN32
extern	sigjmp_buf longJumpBuffer;
#endif


typedef bool (*TStartFunction) (void* data);
extern  TStartFunction	startFunction;
extern	void*			startFunctionData;

class FileListCacheIntf;
extern FileListCacheIntf* mapList;
extern FileListCacheIntf* modList;
extern FileListCacheIntf* skinList;
extern FileListCacheIntf* settingsPresetList;
void updateFileListCaches();

// Main Routines
void    ParseArguments(int argc, char *argv[]);
int		InitializeLieroX();
void	ShutdownLieroX();
void	GameLoopFrame();
void	QuittoMenu();
void	GotoLocalMenu();
void	GotoNetMenu();

void	SetQuitEngineFlag(const std::string& reason);
void	ResetQuitEngineFlag();
bool	Warning_QuitEngineFlagSet(const std::string& preText = "");

// Miscellanous routines
float	GetFixedRandomNum(uchar index);
void	ConvertTime(TimeDiff time, int *hours, int *minutes, int *seconds);
bool    MouseInRect(int x, int y, int w, int h);

void	printf(const std::string& txt);



enum GameState {
	S_INACTIVE, // server was not started
	S_SVRLOBBY, // in lobby
	S_SVRWEAPONS, // in game: in weapon selection
	S_SVRPLAYING, // in game: playing
	S_CLICONNECTING, // client game: connecting right now
	S_CLILOBBY,
	S_CLIWEAPONS,
	S_CLIPLAYING
};

inline std::string GameStateAsString(GameState s) {
	switch(s) {
		case S_INACTIVE: return "S_INACTIVE";
		case S_SVRLOBBY: return "S_SVRLOBBY";
		case S_SVRWEAPONS: return "S_SVRWEAPONS";
		case S_SVRPLAYING: return "S_SVRPLAYING";
		case S_CLICONNECTING: return "S_CLICONNECTING";
		case S_CLILOBBY: return "S_CLILOBBY";
		case S_CLIWEAPONS: return "S_CLIWEAPONS";
		case S_CLIPLAYING: return "S_CLIPLAYING";
	}
	return "INVALID GAMESTATE";
}

GameState currentGameState();

#endif  //  __LIEROX_H__
