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

typedef std::vector<std::string> searchpathlist;


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
typedef std::string controls_t[8];

// Options structure
typedef class { public:

	// Video
	int		iFullscreen;
	int		iShowFPS;
	int		iOpenGL;
	std::string	sResolution;

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
	std::string	sSkinPath;

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
	class gameinfo_s { public:
		int		iLives;
		int		iKillLimit;
		int		iTimeLimit;
		int		iTagLimit;
		int		iLoadingTime;
		int		iBonusesOn;
		int		iShowBonusName;
		int		iMaxPlayers;
		bool	bTournament;
		std::string	sServerName;
		std::string	sWelcomeMessage;
		std::string	sMapName;
        int     nGameType;
        std::string szModName;
        std::string szPassword;
		bool	bRegServer;
		int		iLastSelectedPlayer;
		bool	bAllowWantsJoinMsg;
		bool	bAllowRemoteBots;
	} tGameinfo;

} options_t;

// Network strings
typedef class { public:
	std::string sHasLeft ;
	std::string sHasConnected ;
	std::string sHasTimedOut ;

	std::string sHasBeenKicked ;
	std::string sHasBeenBanned ;
	std::string sHasBeenMuted ;
	std::string sHasBeenUnmuted ;

	std::string sKickedYou ;
	std::string sBannedYou ;
	std::string sYouTimed ;
	std::string sYouQuit ;

	std::string sKilled ;
	std::string sCommitedSuicide ;
	std::string sFirstBlood ;
	std::string sTeamkill ;

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

	std::string sServerFull ;
	std::string sNoEmptySlots ;
	std::string sWrongProtocol ;
	std::string sBadVerification ;
	std::string sNoIpVerification ;
	std::string sGameInProgress ;
	std::string sYouAreBanned ;
	std::string sBotsNotAllowed ;
	std::string sWantsJoin ;
} networktexts_t;



// Option Routines
int		LoadOptions(void);
bool	LoadNetworkStrings(void);
void    SaveOptions(void);
void	ShutdownOptions(void);


extern	options_t		*tLXOptions;
extern  networktexts_t  *NetworkTexts;



#endif  //  __OPTIONS_H__
