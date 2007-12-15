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
#include <string>
#include <cassert>

#include "FindFile.h" // for searchpathlist; TODO: extract this to an own file

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
	SIN_ROPE,

	SIN_STRAFE
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
	SIN_TOGGLETOPBAR,
	SIN_TEAMCHAT,
#ifdef WITH_MEDIAPLAYER
	SIN_MEDIAPLAYER
#endif
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
	std::string ctrl[10];
public:
	std::string& operator[] (const short i) { assert(i >= 0 && (unsigned)i < sizeof(ctrl)/sizeof(std::string) ); return ctrl[i]; }
	const std::string& operator[] (const short i) const { assert(i >= 0 && (unsigned)i < sizeof(ctrl)/sizeof(std::string) ); return ctrl[i]; }

	unsigned short ControlCount(void) const  { return sizeof(ctrl)/sizeof(std::string); }
	// TODO: add specific functions
};


// Network strings
class NetworkTexts {
public:

	static bool Init();
	bool LoadFromDisc();
	
	std::string sHasLeft ;
	std::string sHasConnected ;
	std::string sHasTimedOut ;

	std::string sHasBeenKicked ;
	std::string sHasBeenKickedReason ;
	std::string sHasBeenBanned ;
	std::string sHasBeenMuted ;
	std::string sHasBeenUnmuted ;

	std::string sKickedYou ;
	std::string sKickedYouReason ;
	std::string sBannedYou ;
	std::string sYouTimed ;
	std::string sYouQuit ;

	std::string sKilled ;
	std::string sCommitedSuicide ;
	std::string sFirstBlood ;
	std::string sTeamkill ;
	std::string sHasScored;

	std::string sPlayerOut ;
	std::string sPlayerHasWon ;
	std::string sTeamOut ;
	std::string sTeamHasWon ;

	std::string sWormIsIt ;

	std::string sSpree1 ;
	std::string sSpree2 ;
	std::string sSpree3 ;
	std::string sSpree4 ;
	std::string sSpree5 ;

	std::string sDSpree1 ;
	std::string sDSpree2 ;
	std::string sDSpree3 ;
	std::string sDSpree4 ;
	std::string sDSpree5 ;

	std::string sServerFull ;
	std::string sNoEmptySlots ;
	std::string sWrongProtocol ;
	std::string sBadVerification ;
	std::string sNoIpVerification ;
	std::string sGameInProgress ;
	std::string sYouAreBanned ;
	std::string sBotsNotAllowed ;
	std::string sWantsJoin ;

	std::string sKnownAs;
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
	bool	bOpenGL;
	std::string	sResolution;
	int		iColourDepth;

	// Network
	int		iNetworkPort;
	int		iNetworkSpeed;
	float	fUpdatePeriod;
	bool	bUseIpToCountry;
	bool	bLoadDbAtStartup;
	std::string	sSTUNServer;

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
	std::string	sSkinPath;
	bool	bAntiAliasing;
	bool	bMouseAiming;
	bool	bAllowMouseAiming;
	bool	bUseNumericKeysToSwitchWeapons;
	bool	bAntilagMovementPrediction;

    // Advanced
    int     nMaxFPS;
	int		iJpegQuality;
	bool	bCountTeamkills;
	bool	bServerSideHealth;

	// Misc.
	int     iLogConvos;
	int		iShowPing;
	int		iScreenshotFormat;
	
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
		bool	bMatchLogging;
		std::string	sServerName;
		std::string	sWelcomeMessage;
		std::string	sMapFilename;
        int     nGameType;
        std::string szModName;
        std::string szPassword;
		bool	bRegServer;
		int		iLastSelectedPlayer;
		bool	bAllowWantsJoinMsg;
		bool	bWantsJoinBanned;
		bool	bAllowRemoteBots;
		bool	bTopBarVisible;
		bool	bAllowNickChange;
		float	fBonusFreq;
		float	fBonusLife;
		bool	bAllowConnectDuringGame;
		int		iAllowConnectDuringGameLives;
		int		iAllowConnectDuringGameLivesMin;
	} tGameinfo;

};


// Option Routines
void	ShutdownOptions(void);


extern	GameOptions		*tLXOptions;
extern  NetworkTexts  *networkTexts;



#endif  //  __OPTIONS_H__
