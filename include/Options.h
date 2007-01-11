/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Options
// Created 21/7/02
// Jason Boettcher


#ifndef __OPTIONS_H__
#define __OPTIONS_H__


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
	SIN_SWITCHMODE
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


// Options structure
typedef struct {

	// Video
	int		iFullscreen;
	int		iShowFPS;
	int		iOpenGL;
	char	sResolution[9];

	// Network
	int		iNetworkPort;
	int		iNetworkSpeed;
	float	fUpdatePeriod;

	// Audio
	int		iSoundOn;
	int		iSoundVolume;

	// Controls
	char	sPlayer1Controls[32][8];
	char	sPlayer2Controls[32][8];
	char	sGeneralControls[32][8];

	// Game
	int		iBloodAmount;
	int		iShadows;
	int		iParticles;
	int		iOldSkoolRope;
	int		iShowHealth;
	int		iColorizeNicks;
	int		iAutoTyping;
	char	sSkinPath[128];

    // Advanced
    int     nMaxFPS;
	int		iJpegQuality;

	// Misc.
	int     iLogConvos;
	int		iShowPing;
	int		iScreenshotFormat;
	
	// File handling
	filelist_t*	tSearchPaths;

	// Widget states
	int		iInternetList[6];
	int		iLANList[6];
	int		iFavouritesList[6];

	// Last used game details
	struct {
		int		iLives;
		int		iKillLimit;
		int		iTimeLimit;
		int		iTagLimit;
		int		iLoadingTime;
		int		iBonusesOn;
		int		iShowBonusName;
		int		iMaxPlayers;
		bool	bTournament;
		char	sServerName[32];
		char	sWelcomeMessage[256];
		char	sMapName[128];
        int     nGameType;
        char    szModName[128];
        char    szPassword[32];
		bool	bRegServer;
		int		iLastSelectedPlayer;
		bool	bAllowWantsJoinMsg;
		bool	bAllowRemoteBots;
	} tGameinfo;

} options_t;

// Network strings
typedef struct {
	char sHasLeft[64];
	char sHasConnected[64];
	char sHasTimedOut[64];

	char sHasBeenKicked[64];
	char sHasBeenBanned[64];
	char sHasBeenMuted[64];
	char sHasBeenUnmuted[64];

	char sKickedYou[64];
	char sBannedYou[64];
	char sYouTimed[64];
	char sYouQuit[64];

	char sKilled[64];
	char sCommitedSuicide[64];
	char sFirstBlood[64];
	char sTeamkill[64];

	char sPlayerOut[64];
	char sPlayerHasWon[64];
	char sTeamOut[64];
	char sTeamHasWon[64];

	char sWormIsIt[64];

	char sSpree1[64];
	char sSpree2[64];
	char sSpree3[64];
	char sSpree4[64];
	char sSpree5[64];

	char sServerFull[64];
	char sNoEmptySlots[64];
	char sWrongProtocol[64];
	char sBadVerification[64];
	char sNoIpVerification[64];
	char sGameInProgress[64];
	char sYouAreBanned[64];
	char sBotsNotAllowed[64];
	char sWantsJoin[64];
} networktexts_t;



// Option Routines
int		LoadOptions(void);
bool	LoadNetworkStrings(void);
void    SaveOptions(void);
void	ShutdownOptions(void);




#endif  //  __OPTIONS_H__
