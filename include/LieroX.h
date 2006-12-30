/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Game header file
// Created 28/6/02
// Jason Boettcher


#ifndef __LIEROX_H__
#define __LIEROX_H__

#ifdef WIN32
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG
#endif // WIN32

#if DEBUG == 1
#define		_AI_DEBUG
#endif

#define		LX_PORT			23400
#define		SPAWN_HOLESIZE	4
#define		LX_VERSION		0.57
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


	char	debug_string[32];
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
	char		sModName[256];
	char		sMapname[256];
    char        sPassword[32];
	char		sModDir[256];
    maprandom_t sMapRandom;
	int			iLoadingTimes;
	char		sServername[32];
	char		sWelcomeMessage[256];
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
	int			iNumBots;
	profile_t	*cBots[MAX_WORMS];
} game_t;


extern	lierox_t		*tLX;
extern	game_t			tGameInfo;
extern	CVec			vGravity;
extern	options_t		*tLXOptions;
extern  networktexts_t  *NetworkTexts;
extern	CServer			*cServer;
extern	CClient			*cClient;
extern	CClient			*cBots;
extern  CInput			cTakeScreenshot;
extern  CInput			cSwitchMode;
extern  int				nDisableSound;

extern	char*	argv0;


// Main Routines
void    ParseArguments(int argc, char *argv[]);
int		InitializeLieroX(void);
void	StartGame(void);
void	ShutdownLieroX(void);
void	GameLoop(void);
void	QuittoMenu(void);



// Miscellanous routines
int		CheckCollision(CVec pos, CVec vel, int width, int height, CMap *map);
void	ConvertTime(float time, int *hours, int *minutes, int *seconds);
int 	CarveHole(CMap *cMap, CVec pos);
void	StripQuotes(char *dest, char *src);
void    lx_strncpy(char *dest, char *src, int count);
bool    MouseInRect(int x, int y, int w, int h);
char    *StripLine(char *szLine);
char    *TrimSpaces(char *szLine);
bool	replace(char *text, const char *what, const char *with, char *result);
char	*replacemax(char *text, char *what, char *with, char *result, int max);
char	*strip(char *buf, int width);
bool	stripdot(char *buf, int width);
char	*ucfirst(char *text);
void	ReadUntil(const char *text, char until_character, char *result);
Uint32	StrToCol(char *str);
const char* sex(short wraplen = 0);


// Useful XML functions
int		xmlGetInt(xmlNodePtr Node, const char *Name);
float	xmlGetFloat(xmlNodePtr Node, const char *Name);
Uint32	xmlGetColour(xmlNodePtr Node, const char *Name);
void	xmlEntities(char *text);


#endif  //  __LIEROX_H__
