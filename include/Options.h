/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Options
// Created 21/7/02
// Jason Boettcher


#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <vector>
#include "UCString.h"

typedef std::vector<UCString> searchpathlist;


// Setup input id's
enum {
	// Movement
	SIN_UP=0,
	SIN_DOWN,
	SIN_LEFT,
	SIN_RIGHT,

	SIN_SHOOT,
	SIN_JUMP,
	SIN_SELWEAP,
	SIN_ROPE
};

// General controls
enum {
	SIN_CHAT,
    SIN_SCORE,
	SIN_HEALTH,
	SIN_SETTINGS,
	SIN_SCREENSHOTS,
	SIN_VIEWPORTS,
	SIN_SWITCHMODE,
	SIN_MEDIAPLAYER
};


// Network speed types
enum {
	NST_MODEM=0,
	NST_ISDN,
	NST_LAN,
	NST_LOCAL			// Hidden speed, only for local games
};

// Screenshot formats
enum {
	FMT_BMP,
	FMT_PNG,
	FMT_JPG,
	FMT_GIF
};

// input controls structure (for local players)
class controls_t {
private:
	UCString ctrl[8];
public:
	UCString& operator[] (const short i) { return (i >= 0 && i < 8) ? ctrl[i] : ctrl[-9999]; }
	const UCString& operator[] (const short i) const { return (i >= 0 && i < 8) ? ctrl[i] : ctrl[-9999]; }

	inline unsigned short ControlCount(void) const  { return sizeof(ctrl)/sizeof(UCString); }
	// TODO: add specific functions
};


// Network strings
class NetworkTexts {
public:

	static bool Init();
	bool LoadFromDisc();
	
	UCString sHasLeft ;
	UCString sHasConnected ;
	UCString sHasTimedOut ;

	UCString sHasBeenKicked ;
	UCString sHasBeenBanned ;
	UCString sHasBeenMuted ;
	UCString sHasBeenUnmuted ;

	UCString sKickedYou ;
	UCString sBannedYou ;
	UCString sYouTimed ;
	UCString sYouQuit ;

	UCString sKilled ;
	UCString sCommitedSuicide ;
	UCString sFirstBlood ;
	UCString sTeamkill ;

	UCString sPlayerOut ;
	UCString sPlayerHasWon ;
	UCString sTeamOut ;
	UCString sTeamHasWon ;

	UCString sWormIsIt ;

	UCString sSpree1 ;
	UCString sSpree2 ;
	UCString sSpree3 ;
	UCString sSpree4 ;
	UCString sSpree5 ;

	UCString sServerFull ;
	UCString sNoEmptySlots ;
	UCString sWrongProtocol ;
	UCString sBadVerification ;
	UCString sNoIpVerification ;
	UCString sGameInProgress ;
	UCString sYouAreBanned ;
	UCString sBotsNotAllowed ;
	UCString sWantsJoin ;
};

// Options structure
class GameOptions {
public:

	static bool Init();
	bool LoadFromDisc();
	void SaveToDisc();
	
	// Video
	int		iFullscreen;
	int		iShowFPS;
	int		iOpenGL;
	UCString	sResolution;
	int		iColourDepth;

	// Network
	int		iNetworkPort;
	int		iNetworkSpeed;
	float	fUpdatePeriod;

	// Audio
	int		iSoundOn;
	int		iSoundVolume;

	// Controls
	std::vector<controls_t> sPlayerControls; // sPC[playernr][controlnr]
	controls_t	sGeneralControls;

	// Game
	int		iBloodAmount;
	int		iShadows;
	int		iParticles;
	int		iOldSkoolRope;
	int		iShowHealth;
	int		iColorizeNicks;
	int		iAutoTyping;
	UCString	sSkinPath;

    // Advanced
    int     nMaxFPS;
	int		iJpegQuality;

	// Misc.
	int     iLogConvos;
	int		iShowPing;
	int		iScreenshotFormat;
	
	// File handling
	searchpathlist	tSearchPaths;

	// Widget states
	int		iInternetList[6];
	int		iLANList[6];
	int		iFavouritesList[6];

	// Media player
	bool	bRepeatPlaylist;
	bool	bShufflePlaylist;
	int		iMPlayerLeft;
	int		iMPlayerTop;
	int		iMusicVolume;

	// Last used game details
	class GameInfo {
	public:
		int		iLives;
		int		iKillLimit;
		int		iTimeLimit;
		int		iTagLimit;
		int		iLoadingTime;
		int		iBonusesOn;
		int		iShowBonusName;
		int		iMaxPlayers;
		bool	bTournament;
		UCString	sServerName;
		UCString	sWelcomeMessage;
		UCString	sMapFilename;
        int     nGameType;
        UCString szModName;
        UCString szPassword;
		bool	bRegServer;
		int		iLastSelectedPlayer;
		bool	bAllowWantsJoinMsg;
		bool	bAllowRemoteBots;
	} tGameinfo;

};


// Option Routines
void	ShutdownOptions(void);


extern	GameOptions		*tLXOptions;
extern  NetworkTexts  *networkTexts;



#endif  //  __OPTIONS_H__
