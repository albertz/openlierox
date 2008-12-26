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

#include <map>
#include <vector>
#include <string>
#include <cassert>
#include "FeatureList.h"

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

	SIN_STRAFE,

	SIN_WEAPON1,
	SIN_WEAPON2,
	SIN_WEAPON3,
	SIN_WEAPON4,
	SIN_WEAPON5
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
	SIN_TEAMCHAT
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
	std::string ctrl[14];
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
	std::string sHasBeenBannedReason ;
	std::string sHasBeenMuted ;
	std::string sHasBeenUnmuted ;
	std::string sIsSpectating;
	std::string sIsPlaying;

	std::string sKickedYou ;
	std::string sKickedYouReason ;
	std::string sBannedYou ;
	std::string sBannedYouReason ;
	std::string sYouTimed ;
	std::string sYouQuit ;

	std::string sKilled ;
	std::string sCommitedSuicide ;
	std::string sFirstBlood ;
	std::string sTeamkill ;
	std::string sHasScored;
	std::string sKilledAFK ;

	std::string sPlayerOut ;
	std::string sPlayerHasWon ;
	std::string sTeamOut ;
	std::string sTeamHasWon ;
	std::string sTimeLimit ;

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
	bool	bFullscreen;
	bool	bShowFPS;
	bool	bOpenGL;
	std::string	sResolution;
	std::string sVideoPostProcessor;
	int		iColourDepth;

	// Network
	int		iNetworkPort;
	int		iNetworkSpeed;
	bool	bUseIpToCountry;
	int		iMaxUploadBandwidth;
	std::string	sHttpProxy;
	bool	bAutoSetupHttpProxy;

	bool	bRegServer;
	std::string	sServerName;
	std::string	sWelcomeMessage;
	std::string sServerPassword;			// Password to connect to server - not used anywhere
	bool	bAllowWantsJoinMsg;
	bool	bWantsJoinBanned;
	bool	bAllowRemoteBots;

	// IRC chat
	bool	bEnableChat;

	// Audio
	bool	bSoundOn;
	int		iSoundVolume;
	int		iMusicVolume;

	// Controls
	std::vector<controls_t> sPlayerControls; // sPC[playernr][controlnr]
	controls_t	sGeneralControls;

	// Game
	int		iBloodAmount;
	bool	bShadows;
	bool	bParticles;
	bool	bOldSkoolRope;
	bool	bShowHealth;
	bool	bShowNetRates;
	bool	bColorizeNicks;
	bool	bAutoTyping;
	std::string	sSkinPath;	// Old unfinished skinned GUI
	bool	bNewSkinnedGUI;	// Just for test
	bool	bAntiAliasing;
	bool	bMouseAiming;
	int		iMouseSensity;
	bool	bAntilagMovementPrediction;
	std::string	sLastSelectedPlayer;
	std::string sTheme;
	bool	bTopBarVisible;
	bool	bScreenShaking;


    // Advanced
    int     nMaxFPS;
	int		iJpegQuality;
	int		iMaxCachedEntries;		// Amount of entries to cache, including maps, mods, images and sounds.
	bool	bMatchLogging;			// Save screenshot of every game final score

	// Misc.
	bool    bLogConvos;
	bool	bShowPing;
	int		iScreenshotFormat;

	// Widget states
	int		iInternetList[6];
	int		iLANList[6];
	int		iFavouritesList[6];

	// Last used game details - used as game lobby structure in client
	// Put everything that impacts gameplay here, both server and client-sided
	class GameInfo {
	public:
		int		iLives;
		int		iKillLimit;
		float	fTimeLimit; // Time limit in minutes
		int		iTagLimit;
		int		iLoadingTime;
		bool	bBonusesOn;
		bool	bShowBonusName;
		int		iMaxPlayers;
		std::string	sMapFile;
		std::string	sMapName;	 // Decoded map name from map file
        int     iGameMode;
        std::string sModDir;
        std::string sModName;	// Decoded mod name from script.lgs
		float	fBonusFreq;
		float	fBonusLife;
		float	fRespawnTime;
		bool	bRespawnGroupTeams;	// respawn all team in single spot
		bool	bGroupTeamScore;	// All worms in team will have the same kill count (sum of each one kills)
		bool	bEmptyWeaponsOnRespawn;	// When worm respawns it should wait until all weapons are reloaded
		float	fBonusHealthToWeaponChance;	// if 0.0f only health will be generated, if 1.0f - only weapons
		bool	bForceRandomWeapons; // only for server; implies bServerChoosesWeapons=true
		bool	bSameWeaponsAsHostWorm; // implies bServerChoosesWeapons=true
		bool	bAllowConnectDuringGame; // >=Beta8
		bool	bAllowStrafing;
		bool	bAllowNickChange;
		bool	bCountTeamkills;
		bool	bServerSideHealth;
		int		iWeaponSelectionMaxTime;	// Auto-kick worms who select their weapons too long
		
		FeatureSettings features;
	} tGameInfo;

	// not specified options found in options-file
	std::map< std::string, std::string > additionalOptions;
};


// Option Routines
void	ShutdownOptions(void);


extern	GameOptions		*tLXOptions;
extern  NetworkTexts  *networkTexts;



#endif  //  __OPTIONS_H__
