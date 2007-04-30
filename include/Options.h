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

typedef std::vector<tString> searchpathlist;


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
	tString ctrl[8];
public:
	tString& operator[] (const short i) {
		assert(i >= 0 && i < 8);
		return ctrl[i];
	}
	const tString& operator[] (const short i) const {
		assert(i >= 0 && i < 8);
		return ctrl[i];	
	}

	inline byte ControlCount(void) const  { return sizeof(ctrl)/sizeof(tString); }
	// TODO: add specific functions
};


// Network strings
class NetworkTexts {
public:

	static bool Init();
	bool LoadFromDisc();
	
	tString sHasLeft ;
	tString sHasConnected ;
	tString sHasTimedOut ;

	tString sHasBeenKicked ;
	tString sHasBeenBanned ;
	tString sHasBeenMuted ;
	tString sHasBeenUnmuted ;

	tString sKickedYou ;
	tString sBannedYou ;
	tString sYouTimed ;
	tString sYouQuit ;

	tString sKilled ;
	tString sCommitedSuicide ;
	tString sFirstBlood ;
	tString sTeamkill ;

	tString sPlayerOut ;
	tString sPlayerHasWon ;
	tString sTeamOut ;
	tString sTeamHasWon ;

	tString sWormIsIt ;

	tString sSpree1 ;
	tString sSpree2 ;
	tString sSpree3 ;
	tString sSpree4 ;
	tString sSpree5 ;

	tString sServerFull ;
	tString sNoEmptySlots ;
	tString sWrongProtocol ;
	tString sBadVerification ;
	tString sNoIpVerification ;
	tString sGameInProgress ;
	tString sYouAreBanned ;
	tString sBotsNotAllowed ;
	tString sWantsJoin ;
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
	tString	sResolution;
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
	tString	sSkinPath;

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
		tString	sServerName;
		tString	sWelcomeMessage;
		tString	sMapFilename;
        int     nGameType;
        tString szModName;
        tString szPassword;
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
