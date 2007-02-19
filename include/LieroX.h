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

#ifdef _MSC_VER
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG
#endif // _MSC_VER

#if DEBUG == 1
#define		_AI_DEBUG
#endif

#define		LX_PORT			23400
#define		SPAWN_HOLESIZE	4
#ifndef		LX_VERSION
#	define		LX_VERSION		"0.57_beta2"
#endif
#define		LX_ENDWAIT		9.0f


// Game types
enum {
	GMT_DEATHMATCH,
	GMT_TEAMDEATH,
	GMT_TAG,
    GMT_DEMOLITION
};


const float	D2R(1.745329e-2f); // degrees to radians
const float	R2D(5.729578e+1f); // radians to degrees

#define DEG2RAD(a)  (a * D2R)
#define RAD2DEG(a)  (a * R2D)


// Game includes
#include "ProfileSystem.h"
#include "Networking.h"
#include "CGameScript.h"
#include "CChatBox.h"
#include "Frame.h"
#include "CViewport.h"
#include "CMap.h"
#include "CSimulation.h"
#include "CNinjaRope.h"
#include "Command.h"
#include "CBonus.h"
#include "CWpnRest.h"
#include "CWorm.h"
#include "CProjectile.h"
#include "CShootList.h"
#include "Entity.h"
#include "CWeather.h"
#include "CClient.h"
#include "CServer.h"
#include "Sounds.h"
#include "Graphics.h"
#include "Protocol.h"
#include "Options.h"


// LieroX structure
typedef struct {
	float	fCurTime;
	float	fDeltaTime;
	CFont	cFont;
	CFont	cOutlineFont;
	CFont	cOutlineFontGrey;

	int		iQuitGame;
	int		iQuitEngine;

	int		debug_int;
	float	debug_float;
	CVec	debug_pos;

	// Default Colours
	Uint32			clNormalLabel;
	Uint32			clHeading;
	Uint32			clSubHeading;
	Uint32			clChatText;
	Uint32			clNetworkText;
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

	Uint32			clPink;


	std::string	debug_string;
} lierox_t;


// Game types
enum {
	GME_LOCAL=0,
	GME_HOST,
	GME_JOIN
};



// Game structure
typedef struct {
	int			iGameType;		// Local, remote, etc
	int			iGameMode;		// DM, team DM, etc
	std::string		sModName;
	std::string		sMapname;
    std::string        sPassword;
	std::string		sModDir;
    maprandom_t sMapRandom;
	int			iLoadingTimes;
	std::string		sServername;
	std::string		sWelcomeMessage;
	bool		bRegServer;
	bool		bTournament;

	int			iLives;
	int			iKillLimit;
	int			iTimeLimit;
	int			iTagLimit;
	int			iBonusesOn;
	int			iShowBonusName;
	
	int			iNumPlayers;
	profile_t	*cPlayers[MAX_WORMS];
} game_t;


extern	lierox_t		*tLX;
extern	game_t			tGameInfo;
extern	CVec			vGravity;
extern	CServer			*cServer;
extern	CClient			*cClient;
extern  CInput			cTakeScreenshot;
extern  CInput			cSwitchMode;
extern	CInput			cToggleMediaPlayer;
extern  int				nDisableSound;
extern	int				iSurfaceFormat;
extern	bool			bActivated;

extern	std::string		binary_dir;


// Main Routines
void    ParseArguments(int argc, char *argv[]);
int		InitializeLieroX(void);
void	StartGame(void);
void	ShutdownLieroX(void);
void	GameLoop(void);
void	QuittoMenu(void);
void	GotoLocalGameMenu(void);



// Miscellanous routines
int		CheckCollision(CVec trg, CVec pos, uchar checkflags, CMap *map);
void	ConvertTime(float time, int *hours, int *minutes, int *seconds);
int 	CarveHole(CMap *cMap, CVec pos);
void	StripQuotes(char *dest, char *src);
void    lx_strncpy(char *dest, char *src, int count);
bool    MouseInRect(int x, int y, int w, int h);
char    *StripLine(char *szLine);
char    *TrimSpaces(char *szLine);
void TrimSpaces(std::string& szLine);
bool	replace(char *text, const char *what, const char *with, char *result);
bool	replace(const std::string& text, const std::string& what, const std::string& with, std::string& result);
bool	replace(std::string& text, std::string what, std::string with);
std::string replacemax(const std::string& text, const std::string& what, const std::string& with, std::string& result, int max);
std::string replacemax(const std::string& text, const std::string& what, const std::string& with, int max);
char	*strip(char *buf, int width);
bool	stripdot(char *buf, int width);
char	*ucfirst(char *text);
void	ReadUntil(const char *text, char until_character, char *result, size_t reslen);
Uint32	StrToCol(char *str);
const char* sex(short wraplen = 0);
std::vector<std::string> explode(const std::string& str, const std::string& with);
std::string freadstr(FILE *fp, size_t maxlen);
size_t findLastPathSep(const std::string& path);

short stringcasecmp(const std::string& s1, const std::string& s2);


// Useful XML functions
int		xmlGetInt(xmlNodePtr Node, const std::string& Name);
float	xmlGetFloat(xmlNodePtr Node, const std::string& Name);
Uint32	xmlGetColour(xmlNodePtr Node, const std::string& Name);
void	xmlEntities(std::string& text);

// Thread functions
#ifdef WIN32
void	nameThread(const DWORD threadId, const char *name);
#endif


#endif  //  __LIEROX_H__
