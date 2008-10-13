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


const float	D2R(1.745329e-2f); // degrees to radians
const float	R2D(5.729578e+1f); // radians to degrees

#define DEG2RAD(a)  (a * D2R)
#define RAD2DEG(a)  (a * R2D)

#include <list>
#include <string>
#include "CFont.h"
#include "CVec.h"
#include "Consts.h"
#include "CInput.h"
#include "types.h"

class profile_t;
class IpToCountryDB;

/*
#include "ProfileSystem.h"
#include "IpToCountryDB.h"
*/




// Screenshot structure
class screenshot_t { public:
	std::string sDir;
	std::string	sData;
};


// LieroX structure
class lierox_t { public:
	float	fCurTime;
	float	fDeltaTime;
	float	fRealDeltaTime; // Delta time used for network synchronization,
							// it is not clamped unlike the above one
	CFont	cFont;
	CFont	cOutlineFont;

	std::list<screenshot_t>	tScreenshotQueue;

	bool	bVideoModeChanged;

	bool	bQuitGame;
	bool	bQuitEngine;

	int		debug_int;
	float	debug_float;
	CVec	debug_pos;

	// Default Colours
	Uint32			clNormalLabel;
	Uint32			clHeading;
	Uint32			clSubHeading;
	Uint32			clChatText;
	Uint32			clNetworkText;
	Uint32			clPrivateText;
	Uint32			clNormalText;
	Uint32			clNotice;
	Uint32			clDropDownText;
	Uint32			clDisabled;
	Uint32			clListView;
	Uint32			clTextBox;
	Uint32			clMouseOver;
	Uint32			clError;
	Uint32			clCredits1;
	Uint32			clCredits2;
	Uint32			clPopupMenu;
	Uint32			clWaiting;
	Uint32			clReady;
	Uint32			clPlayerName;
	Uint32			clBoxLight;
	Uint32			clBoxDark;
	Uint32			clWinBtnBody;
	Uint32			clWinBtnLight;
	Uint32			clWinBtnDark;
	Uint32			clMPlayerTime;
	Uint32			clMPlayerSong;
	Uint32			clChatBoxBackground;
	Uint32			clDialogBackground;
	Uint32			clGameBackground;
	Uint32			clViewportSplit;
	Uint32			clScrollbarBack;
	Uint32			clScrollbarBackLight;
	Uint32			clScrollbarFront;
	Uint32			clScrollbarHighlight;
	Uint32			clScrollbarShadow;
	Uint32			clSliderLight;
	Uint32			clSliderDark;
	Uint32			clCurrentSettingsBg;
	Uint32			clScoreBackground;
	Uint32			clScoreHighlight;
	Uint32			clDialogCaption;
	Uint32			clPlayerDividingLine;
	Uint32			clLine;
	Uint32			clProgress;
	Uint32			clListviewSelected;
	Uint32			clComboboxSelected;
	Uint32			clComboboxShowAllMain;
	Uint32			clComboboxShowAllBorder;
	Uint32			clMenuSelected;
	Uint32			clMenuBackground;
	Uint32			clGameChatter;
	Uint32			clSelection;
	Uint32			clTextboxCursor;
	Uint32			clGameChatCursor;
	Uint32			clConsoleCursor;
	Uint32			clConsoleNormal;
	Uint32			clConsoleNotify;
	Uint32			clConsoleError;
	Uint32			clConsoleWarning;
	Uint32			clConsoleDev;
	Uint32			clConsoleChat;
	Uint32			clReturningToLobby;
	Uint32			clTeamColors[4];
	Uint32			clTagHighlight;
	Uint32			clHealthLabel;
	Uint32			clWeaponLabel;
	Uint32			clLivesLabel;
	Uint32			clKillsLabel;
	Uint32			clFPSLabel;
	Uint32			clPingLabel;
	Uint32			clSpecMsgLabel;
	Uint32			clLoadingLabel;
	Uint32			clIpLoadingLabel;
	Uint32			clWeaponSelectionTitle;
	Uint32			clWeaponSelectionActive;
	Uint32			clWeaponSelectionDefault;
	Uint32			clRopeColors[2];
	Uint32			clLaserSightColors[2];


	Uint32			clPink;
	Uint32			clWhite;
	Uint32			clBlack;


	std::string	debug_string;
};


// Game types
enum {
	GME_LOCAL=0,
	GME_HOST,
	GME_JOIN
};


// Object structure (for maprandom_t)
// HINT: DON'T change the variable types because they are saved directly to the file (CMap.cpp)
class object_t { public:
	Sint32		Type;
	Sint32		Size;
	Sint32     X, Y;
};



// Random map data
class maprandom_t { public:
	maprandom_t()  { psObjects = NULL; bUsed = false; }
    bool        bUsed;
    std::string szTheme;
    int         nNumObjects;
    object_t    *psObjects;
};


// TODO: merge this class with GameOptions::GameInfo (Options.h) and game_lobby_t (this file)
// Game structure
class game_t { public:
	int			iGameType;		// Local, remote, etc
	int			iGameMode;		// DM, team DM, etc
	std::string		sModName;
	std::string		sMapFile;
	std::string		sMapName;
    std::string        sPassword;
	std::string		sModDir;
    maprandom_t sMapRandom;
	int			iLoadingTimes;
	std::string		sServername;
	std::string		sWelcomeMessage;
	bool		bRegServer;

	int			iLives;
	int			iKillLimit;
	float		fTimeLimit;	// Time limit in minutes (sent over net as int)
	int			iTagLimit;
	bool		bBonusesOn;
	bool		bShowBonusName;
	float		fGameSpeed;
	
	int			iNumPlayers;
	profile_t	*cPlayers[MAX_WORMS];
};


// TODO: merge this with game_t (this file) and GameOptions::GameInfo (Options.h)
// TODO: move this somewhere else
// Game lobby structure
class game_lobby_t { public:
	bool	bSet;
	int		nGameMode;
	int		nLastGameMode;
	int		nLives;
	int		nMaxWorms;
	int		nMaxKills;
	int		nLoadingTime;
	float	fGameSpeed;
	bool	bBonuses;
	std::string	szMapFile;
	std::string	szDecodedMapName;
	std::string	szModName;
	std::string	szModDir;
	bool	bHaveMap;
	bool	bHaveMod;
	bool	bForceRandomWeapons;
	bool	bSameWeaponsAsHostWorm;
};



extern	lierox_t		*tLX;
extern	game_t			tGameInfo;
extern  CInput			*cTakeScreenshot;
extern  CInput			*cSwitchMode;
extern  bool			bDisableSound;
extern	bool			bDedicated;
extern  bool			bJoystickSupport;
extern  bool			bRestartGameAfterQuit;


typedef bool (*TStartFunction) (void* data);
extern  TStartFunction	startFunction;
extern	void*			startFunctionData;


// Main Routines
void    ParseArguments(int argc, char *argv[]);
int		InitializeLieroX(void);
void	StartGame(void);
void	ShutdownLieroX(void);
void	GameLoopFrame(void);
void	QuittoMenu(void);
void	GotoLocalMenu(void);
void	GotoNetMenu(void);

void	SetQuitEngineFlag(const std::string& reason);
void	ResetQuitEngineFlag();
bool	Warning_QuitEngineFlagSet(const std::string& preText = "");

// Miscellanous routines
float	GetFixedRandomNum(uchar index);
void	ConvertTime(float time, int *hours, int *minutes, int *seconds);
bool    MouseInRect(int x, int y, int w, int h);

void	printf(const std::string& txt);




// Thread functions
#ifdef WIN32
#include <windows.h>
void	nameThread(const DWORD threadId, const std::string& name);
#endif


#endif  //  __LIEROX_H__
