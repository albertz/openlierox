/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Client class
// Created 28/6/02
// Jason Boettcher


#ifndef __CCLIENT_H__
#define __CCLIENT_H__

// TODO: remove this after we changed network
#include "CChannel.h"

#include "CViewport.h"

#define		MAX_CLIENTS		32
#define		MAX_PLAYERS		8
#define		MAX_CHATLINES	8

#define		RATE_NUMMSGS	10

#define     NUM_VIEWPORTS   3
#define     GAMEOVER_WAIT   3


// Net status
enum {
	NET_DISCONNECTED=0,
	NET_CONNECTING,			// Server doesn't use this state, only client side
	NET_CONNECTED,
	NET_PLAYING,
    NET_ZOMBIE              // Server side only state
};


// Chatbox line
class chat_line_t { public:
	std::string	sText;
	float	fTime;
	float	fScroll;
	int		iType;
};


// Ping structure
class ping_t { public:
	bool	bInitialized;
	float	fSentTime;
	int		iSequence;
};



class CClient {
public:
	// Constructor
	CClient() {
		//printf("cl:Constructor\n");
		cRemoteWorms = NULL;
		cProjectiles = NULL;
		cMap = NULL;
		cBonuses = NULL;
		iGameType = GMT_DEATHMATCH;
		iGameReady = false;
        nTopProjectile = 0;
		bMapGrabbed = false;
		bmpScoreBuffer = NULL;
		cChatList = NULL;
		bUpdateScore = true;
		bRedrawChat = true;

		szServerName="";

		cNetChan.Clear();
		iNetStatus = NET_DISCONNECTED;
		bsUnreliable.Clear();
		iBadConnection = false;
		iServerError = false;
		iChat_Typing = false;
		fChat_BlinkTime = 0;
		iChat_CursorVisible = true;
        iClientError = false;
		bInServer = false;
		cIConnectedBuf = "";
		iNetSpeed = 3;
		fLastUpdateSent = -9999;

		tPing.bInitialized = false;
		tPing.iSequence = 0;
		tPing.fSentTime = 0;

		iMyPing = 0;
		fMyPingRefreshed = 0;
		fMyPingSent = 0;
		iPing = 0;
		fTimePingUpdated = 0;

		iCanToggle = true;

		//fProjDrawTime = 0;
		//fProjSimulateTime = 0;

		fSendWait = 0;

		iMuted = false;

	}


private:
	// Attributes

	// Local Worms (pointers to the remote worms)
	uint		iNumWorms;
	CWorm		*cLocalWorms[MAX_PLAYERS];
	//int			iDrawingViews[2];
    CViewport   cViewports[NUM_VIEWPORTS];

	profile_t	*tProfiles[MAX_PLAYERS];

	// Remote worms
	CWorm		*cRemoteWorms;

	// Map
	CMap		*cMap;
	bool		bMapGrabbed;

	// Projectiles
	CProjectile	*cProjectiles;
    int         nTopProjectile;

	// Frames
	frame_t		tFrames[NUM_FRAMES];

	// Game
	CGameScript	cGameScript;
	int			iGameType;
	int			iLives;
	int			iMaxKills;
	int			iTimeLimit;
	int			iTagLimit;
	float		fLoadingTime;
	std::string	sModName;
	int			iBonusesOn;
	int			iShowBonusName;
    CWpnRest    cWeaponRestrictions;
	//float		fProjDrawTime;
	//float		fProjSimulateTime;
	bool		bRedrawChat;

	// Ping below FPS
	float		fMyPingSent;
	float		fMyPingRefreshed;
	int			iMyPing;

	int			iScoreboard[MAX_WORMS];
	int			iScorePlayers;
	int			iTeamScores[4];
	int			iTeamList[4];

    CWeather    cWeather;

	// Game menu && score
	SDL_Surface *bmpScoreBuffer;
	bool		bUpdateScore;

	// Bonus's
	CBonus		*cBonuses;

	// Chatbox
	int			iChat_Numlines;
	chat_line_t	tChatLines[MAX_CHATLINES];

	CChatBox	cChatbox;		// Our chatbox
	void		*cChatList;		// Ingame chatlist (this is the type of CListview)

	// Send chat
	int			iChat_Typing;
	int			iChat_Lastchar;
	int			iChat_Holding;
	unsigned int	iChat_Pos;
	float		fChat_TimePushed;
	CInput		cChat_Input;
	std::string	sChat_Text;
	float		fChat_BlinkTime;
	int			iChat_CursorVisible;

    CInput      cShowScore;
	CInput		cShowHealth;
	int			iCanToggle;
	CInput		cShowSettings;
	CInput		cViewportMgr;

	int			iMuted;


    // Viewport manager
    bool        bViewportMgr;



	// Network
	int			iServerFrame;
	float		fServerTime;
	int			iNetSpeed;
	int			iNetStatus;
	std::string	strServerAddr;
	int			iNumConnects;
	float		fConnectTime;
	int			iChallenge;
	float		fLastReceived;
	NetworkSocket	tSocket;
	CChannel	cNetChan;
	CBytestream	bsUnreliable;
	CShootList	cShootList;
	int			nMessageSizes[RATE_NUMMSGS];
    float       fZombieTime;
	float		fSendWait;
	float		fLastUpdateSent;
	std::string	szServerName;

	// Ping
	ping_t		tPing;
	int			iPing;
	float		fTimePingUpdated;

	int			iReadySent;

	int			iGameOver;
	int			iGameMenu;
	int			iMatchWinner;
	float		fGameOverTime;

	int			iLobbyReady;
	int			iGameReady;

	game_lobby_t tGameLobby;

	int			iBadConnection;
	std::string	strBadConnectMsg;

	int			iServerError;
	std::string	strServerErrorMsg;

    int         iClientError;

	// Logging variables
    bool		bInServer;
	std::string	cIConnectedBuf;



public:
	// Methods

	void		Clear(void);
	void		MinorClear(void);
	int			Initialize(void);
	void		Shutdown(void);
	void		FinishGame(void);

	// Server editing of the client
	void		SetupWorms(int numworms, CWorm *worms);
	void		RemoveWorm(int id);

	// Game
	void		Simulation(void);
	void		SetupViewports(void);
	void		SendCarve(CVec pos);
	void		PlayerShoot(CWorm *w);
	void		ShootSpecial(CWorm *w);
	void		DrawBeam(CWorm *w);
	void		ProcessShots(void);
	void		ProcessShot(shoot_t *shot);
	void		ProcessShot_Beam(shoot_t *shot);

	void		BotSelectWeapons(void);

	void		SpawnProjectile(CVec pos, CVec vel, int rot, int owner, proj_t *_proj, int _random, int _remote, float remotetime);
    void        disableProjectile(CProjectile *prj);
	void		SimulateProjectiles(float dt);
	void		Explosion(CVec pos, int damage, int shake, int owner);
	void		InjureWorm(CWorm *w, int damage, int owner);
	void		UpdateScoreboard(void);
	void		LaserSight(CWorm *w);
    void        CheckDemolitionsGame(void);

	void		processChatter(void);
    void        processChatCharacter(int c, bool bDown);


	void		SimulateBonuses(float dt);
	void		DestroyBonus(int id, int local, int wormid);

	CVec		FindNearestSpot(CWorm *w);

	// Main
	void		Frame(void);

	// Drawing
	int			InitializeDrawing(void);
	void		Draw(SDL_Surface *bmpDest);
	void		DrawViewport(SDL_Surface *bmpDest, CViewport *v);
	void		DrawProjectiles(SDL_Surface *bmpDest, CViewport *v);
    void        DrawProjectileShadows(SDL_Surface *bmpDest, CViewport *v);
	void		DrawGameOver(SDL_Surface *bmpDest);
	void		DrawRemoteGameOver(SDL_Surface *bmpDest);
	void		DrawGameMenu(SDL_Surface *bmpDest);
	void		DrawBonuses(SDL_Surface *bmpDest, CViewport *v);
	void		UpdateScoreBuf(SDL_Surface *bmpDest, SDL_Surface *bmpImage);
	void		DrawScore(SDL_Surface *bmpDest, SDL_Surface *bmpImage);
	void		DrawTime(SDL_Surface *bmpDest, int x, int y, float t);
	void		DrawReadyOverlay(SDL_Surface *bmpDest);
	void		DrawText(SDL_Surface *bmpDest, int centre, int x, int y, Uint32 fgcol, char *fmt, ...);
	void		DrawLocalChat(SDL_Surface *bmpDest);
	void		DrawRemoteChat(SDL_Surface *bmpDest);
    void        DrawScoreboard(SDL_Surface *bmpDest);
	void        DrawCurrentSettings(SDL_Surface *bmpDest);
	void		DrawMediaPlayer(SDL_Surface *bmpDest);
    void        InitializeViewportManager(void);
    void        DrawViewportManager(SDL_Surface *bmpDest);
	void		SimulateHud(void);

	// Network
	void		Connect(const std::string& address);
	void		Connecting(void);
	void		ReadPackets(void);
	void		SendPackets(void);
	void		SendDeath(int victim, int killer);
	void		SendText(const std::string& sText);
	void		Disconnect(void);
	int			OwnsWorm(CWorm *w);

	// Sending
	void		SendWormDetails(void);

	// Parsing
	void		ParseConnectionlessPacket(CBytestream *bs);
	void		ParseChallenge(CBytestream *bs);
	void		ParseConnected(CBytestream *bs);
	void		ParsePong(void);

	void		ParsePacket(CBytestream *bs);
	bool		ParsePrepareGame(CBytestream *bs);
	void		ParseStartGame(CBytestream *bs);
	void		ParseSpawnWorm(CBytestream *bs);
	void		ParseWormInfo(CBytestream *bs);
	void		ParseText(CBytestream *bs);
	void		ParseScoreUpdate(CBytestream *bs);
	void		ParseGameOver(CBytestream *bs);
	void		ParseSpawnBonus(CBytestream *bs);
	void		ParseTagUpdate(CBytestream *bs);
	void		ParseCLReady(CBytestream *bs);
	void		ParseUpdateLobby(CBytestream *bs);
	void		ParseClientLeft(CBytestream *bs);
	void		ParseUpdateWorms(CBytestream *bs);
	void		ParseUpdateLobbyGame(CBytestream *bs);
	void		ParseWormDown(CBytestream *bs);
	void		ParseServerLeaving(CBytestream *bs);
	void		ParseSingleShot(CBytestream *bs);
	void		ParseMultiShot(CBytestream *bs);
	void		ParseUpdateStats(CBytestream *bs);
	void		ParseDestroyBonus(CBytestream *bs);
	void		ParseGotoLobby(CBytestream *bs);
    void        ParseDropped(CBytestream *bs);


	// Variables
	CChannel	*getChannel(void)			{ return &cNetChan; }
	int			getStatus(void)				{ return iNetStatus; }
	void		setStatus(int _s)			{ iNetStatus = _s; }
	CBytestream	*getUnreliable(void)		{ return &bsUnreliable; }

	int			getNumWorms(void)			{ return iNumWorms; }
	void		setNumWorms(int _w)			{ iNumWorms = _w; }

	CWorm		*getWorm(int w)				{ return cLocalWorms[w]; }
	void		setWorm(int i, CWorm *w)	{ cLocalWorms[i] = w; }

	CWorm		*getRemoteWorms(void)		{ return cRemoteWorms; }

	int			getGameReady(void)			{ return iGameReady; }
	void		setGameReady(int _g)		{ iGameReady = _g; }

    CChatBox    *getChatbox(void)           { return &cChatbox; }

	game_lobby_t *getGameLobby(void)		{ return &tGameLobby; }

	int			getBadConnection(void)		{ return iBadConnection; }
	inline std::string	getBadConnectionMsg(void)	{ return strBadConnectMsg; }

	int			getServerError(void)		{ return iServerError; }
	inline std::string	getServerErrorMsg(void)	{ return strServerErrorMsg; }

    int         getClientError(void)        { return iClientError; }

	float		getLastReceived(void)		{ return fLastReceived; }
	void		setLastReceived(float _l)	{ fLastReceived = _l; }

	int			getNetSpeed(void)			{ return iNetSpeed; }
	void		setNetSpeed(int _n)			{ iNetSpeed = _n; }

	int			*getMsgSize(void)			{ return nMessageSizes; }

	CShootList	*getShootList(void)			{ return &cShootList; }

    CBonus      *getBonusList(void)         { return cBonuses; }

    void        setZombieTime(float z)      { fZombieTime = z; }
    float       getZombieTime(void)         { return fZombieTime; }

	float		getLastPingTime(void)		{ return fTimePingUpdated; }
	void		setLastPingTime(float Time) { fTimePingUpdated = Time; }

	frame_t		*getFrame(int FrameID)		{ return &tFrames[ FrameID ]; }

	ping_t		*getPingStruct(void)		{ return &tPing; }

	int			getTeamScore(int team)		{ return iTeamScores[team]; }

	int			isTyping(void)				{ return iChat_Typing; }

	int			getMuted(void)				{ return iMuted; }
	void		setMuted(int _m)			{ iMuted = _m; }

	inline int	getPing(void)				{ return iPing; }
	inline void	setPing(int Ping)			{ iPing = Ping; }

	inline void	setServerAddress(const std::string& _a)	{ strServerAddr = _a; }
	inline std::string getServerAddress(void)		{ return strServerAddr; }

	inline void setServerName(const std::string& _n)		{ szServerName = _n; }
	inline std::string getServerName(void)		{ return szServerName; }

};

extern	CClient			*cClient;



#endif  //  __CCLIENT_H__
