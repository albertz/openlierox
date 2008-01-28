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

#include "FileDownload.h"
#include "CGameScript.h"
#include "CWpnRest.h"
#include "CChatBox.h"
#include "CWeather.h"
#include "CViewport.h"
#include "Frame.h"
#include "CBonus.h"
#include "CShootList.h"
#include "CBar.h"
#include "CGuiLayout.h"
#include "CListview.h"
#include "InputEvents.h"
#include "FileDownload.h"


#define		MAX_CLIENTS		32
#define		MAX_PLAYERS		32
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

// Structure for logging worms
class log_worm_t { public:
	std::string	sName;
	std::string	sSkin;
	int			iLives;
	int			iKills;
	int			iID;
	int			iSuicides;
	int			iTeamKills;
	int			iTeamDeaths;
	int			iTeam;
	bool		bTagIT;
	float		fTagTime;
	bool		bLeft;
	float		fTimeLeft;
	int			iType;
};

// Game log structure
class game_log_t { public:
	log_worm_t	*tWorms;
	int			iNumWorms;
	int			iWinner;
	float		fGameStart;
	std::string	sGameStart;
	std::string sServerName;
	std::string sServerIP;
};


class interface_sett { public:
	int		ChatterX;
	int		ChatterY;
	int		ChatBoxX;
	int		ChatBoxY;
	int		ChatBoxW;
	int		ChatBoxH;
	int		MiniMapX;
	int		MiniMapY;
	int		MiniMapW;
	int		MiniMapH;
	int		FpsX;
	int		FpsY;
	int		FpsW;
	int		PingX;
	int		PingY;
	int		PingW;
	int		LocalChatX;
	int		LocalChatY;
	int		CurrentSettingsX;
	int		CurrentSettingsY;
	int		ScoreboardX;
	int		ScoreboardY;
	int		CurrentSettingsTwoPlayersX;
	int		CurrentSettingsTwoPlayersY;
	int		ScoreboardOtherPosX;  // In local it's for two players, in net when selecting weapons
	int		ScoreboardOtherPosY;
	int		ChatboxScrollbarX;
	int		ChatboxScrollbarY;
	int		ChatboxScrollbarH;
	bool	ChatboxScrollbarAlwaysVisible;

	// Player 1
	int		Lives1X;
	int		Lives1Y;
	int		Lives1W;
	int		Kills1X;
	int		Kills1Y;
	int		Kills1W;
	int		Team1X;
	int		Team1Y;
	int		Team1W;
	int		SpecMsg1X;
	int		SpecMsg1Y;
	int		SpecMsg1W;
	int		HealthLabel1X;
	int		HealthLabel1Y;
	int		WeaponLabel1X;
	int		WeaponLabel1Y;

	// Player 2
	int		Lives2X;
	int		Lives2Y;
	int		Lives2W;
	int		Kills2X;
	int		Kills2Y;
	int		Kills2W;
	int		Team2X;
	int		Team2Y;
	int		Team2W;
	int		SpecMsg2X;
	int		SpecMsg2Y;
	int		SpecMsg2W;
	int		HealthLabel2X;
	int		HealthLabel2Y;
	int		WeaponLabel2X;
	int		WeaponLabel2Y;
	// NOTE: bars are handled in CBar class
};

// Client rights on a server
class ClientRights { public:
	ClientRights(): NameChange(false), Kick(false), Ban(false), Mute(false), ChooseLevel(false), ChooseMod(false), StartGame(false), Authorize(false), Override(false) {}
	void Everything ()  { NameChange = Kick = Ban = Mute = ChooseLevel = ChooseMod = StartGame = Authorize = true; }
	void Nothing ()  { NameChange = Kick = Ban = Mute = ChooseLevel = ChooseMod = StartGame = Authorize = Override = false; }

	bool NameChange;
	bool Kick;
	bool Ban;
	bool Mute;
	bool ChooseLevel;
	bool ChooseMod;
	bool StartGame;
	bool Authorize;
	bool Override;
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
		bmpBoxBuffer = NULL;
		bmpBoxLeft = NULL;
		bmpBoxRight = NULL;
		cHealthBar1 = NULL;
		cHealthBar2 = NULL;
		cWeaponBar1 = NULL;
		cWeaponBar2 = NULL;
		iGameType = GMT_DEATHMATCH;
		bGameReady = false;
        nTopProjectile = 0;
		bMapGrabbed = false;
		cChatList = NULL;
		bUpdateScore = true;
		bShouldRepaintInfo = true;
		bCurrentSettings = false;

		tGameLog = NULL;
		iLastVictim = -1;
		iLastKiller = -1;

		szServerName="";

		cNetChan.Clear();
		iNetStatus = NET_DISCONNECTED;
		bsUnreliable.Clear();
		bBadConnection = false;
		bServerError = false;
		bChat_Typing = false;
		bTeamChat = false;
		fChat_BlinkTime = 0;
		bChat_CursorVisible = true;
        bClientError = false;
		bInServer = false;
		cIConnectedBuf = "";
		iNetSpeed = 3;
		fLastUpdateSent = -9999;
		bForceWeaponsReady = false;

		iMyPing = 0;
		fMyPingRefreshed = 0;
		fMyPingSent = 0;

		//fProjDrawTime = 0;
		//fProjSimulateTime = 0;

		fSendWait = 0;

		bMuted = false;
		bRepaintChatbox = true;

		for(ushort i=0; i<4; i++)
			iTeamScores[i] = 0;

		iHostOLXVer = 0;
		bHostAllowsMouse = false;
		fLastDirtUpdate = fLastFileRequest = tLX->fCurTime;

		bDownloadingMap = false;
		cFileDownloader = NULL;
		sMapDownloadName = "";
		bMapDlError = false;
		sMapDlError = "";
		iMapDlProgress = 0;
	}

	~CClient()  {
		Shutdown();
		Clear();
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

	// Logging
	game_log_t	*tGameLog;
	int			iLastVictim;
	int			iLastKiller;

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
	bool		bBonusesOn;
	bool		bShowBonusName;
    CWpnRest    cWeaponRestrictions;
	bool		bForceWeaponsReady;

	// Ping below FPS
	float		fMyPingSent;
	float		fMyPingRefreshed;
	int			iMyPing;

	int			iScoreboard[MAX_WORMS];
	int			iScorePlayers;
	int			iTeamScores[4];
	int			iTeamList[4];

	// Interface
	interface_sett tInterfaceSettings;
	CBar		*cHealthBar1;
	CBar		*cHealthBar2;
	CBar		*cWeaponBar1;
	CBar		*cWeaponBar2;
	SDL_Surface *bmpBoxBuffer;
	SDL_Surface *bmpBoxLeft;
	SDL_Surface *bmpBoxRight;
	CGuiLayout  cGameMenuLayout;
	bool		bShouldRepaintInfo;
	bool		bCurrentSettings;

    CWeather    cWeather;

	// Game menu && score
	bool		bUpdateScore;

	// Ingame scoreboard
	SDL_Surface *bmpIngameScoreBg;
	CGuiLayout	cScoreLayout;

	// Bonus's
	CBonus		*cBonuses;

	// Chatbox
	int			iChat_Numlines;
	chat_line_t	tChatLines[MAX_CHATLINES];

	CChatBox	cChatbox;		// Our chatbox
	void		*cChatList;		// Ingame chatlist (this is the type of CListview)
	bool		bRepaintChatbox;

	// Send chat
	bool		bChat_Typing;
	UnicodeChar	iChat_Lastchar;
	bool		bChat_Holding;
	unsigned int	iChat_Pos;
	float		fChat_TimePushed;
	CInput		cChat_Input;
	CInput		cTeamChat_Input;
	bool		bTeamChat;
	std::string	sChat_Text;
	float		fChat_BlinkTime;
	bool		bChat_CursorVisible;

    CInput      cShowScore;
	CInput		cShowHealth;
	CInput		cShowSettings;
	CInput		cViewportMgr;
	CInput		cToggleTopBar;

	bool		bMuted;


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
	ClientRights tRights;
	int			iHostOLXVer;
	bool		bHostAllowsMouse;
	std::string	sClientVersion;
	std::string	sServerVersion;
	int			iClientOLXVer;

	// Map downloading
	bool		bDownloadingMap;
	CFileDownloader *cFileDownloader;
	std::string	sMapDownloadName;
	bool		bMapDlError;
	std::string	sMapDlError;
	byte		iMapDlProgress;

	bool		bReadySent;

	bool		bGameOver;
	bool		bGameMenu;
	int			iMatchWinner;
	float		fGameOverTime;

	bool		bLobbyReady;
	bool		bGameReady;

	game_lobby_t tGameLobby;

	bool		bBadConnection;
	std::string	strBadConnectMsg;

	bool		bServerError;
	std::string	strServerErrorMsg;

    bool		bClientError;

	// Logging variables
    bool		bInServer;
	std::string	cIConnectedBuf;
	bool		bConnectingDuringGame;
	
	CFileDownloaderInGame	cFileDownloaderInGame;
	float		fLastDirtUpdate;
	float		fLastFileRequest;
	float		fLastFileRequestPacketReceived;
	struct		cSpectatorViewportKeys_t { 
				CInput Up, Down, Left, Right, V1Type, V2Type, V2Toggle;
	} cSpectatorViewportKeys;
	std::string	sSpectatorViewportMsg;
	float		fSpectatorViewportMsgTimeout;
	bool		bSpectate;	// Spectate only, suicide local worm when it spawns
	
private:
	void		SendTextInternal(const std::string& sText, const std::string& sWormName);

public:
	// Methods

	void		Clear(void);
	void		MinorClear(void);
	int			Initialize(void);
	void		Shutdown(void);
	void		FinishGame(void);

	// Logging
	void		StartLogging(int num_players);
	void		ShutdownLog();
	log_worm_t	*GetLogWorm(int id);
	void		GetLogData(std::string& data);

	// Server editing of the client
	void		SetupWorms(int numworms, CWorm *worms);
	void		RemoveWorm(int id);

	// Game
	void		Simulation(void);
	void		SetupViewports(void);
	void		SetupViewports(CWorm *w1, CWorm *w2, int type1, int type2);
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
    void        processChatCharacter(const KeyboardEvent& input);


	void		SimulateBonuses(float dt);
	void		DestroyBonus(int id, bool local, int wormid);

	CVec		FindNearestSpot(CWorm *w);

	// Main
	void		Frame(void);

	// Drawing
	bool		InitializeDrawing(void);
	bool		InitializeBar(byte number);
	void		DrawBox(SDL_Surface *dst, int x, int y, int w);
	void		Draw(SDL_Surface *bmpDest);
	void		DrawViewport(SDL_Surface *bmpDest, byte viewport_index);
	void		DrawProjectiles(SDL_Surface *bmpDest, CViewport *v);
    void        DrawProjectileShadows(SDL_Surface *bmpDest, CViewport *v);
	void		InitializeGameMenu();
	void		DrawGameMenu(SDL_Surface *bmpDest);
	void		DrawBonuses(SDL_Surface *bmpDest, CViewport *v);
	void		UpdateScore(CListview *Left, CListview *Right);
	void		UpdateIngameScore(CListview *Left, CListview *Right, bool WaitForPlayers);
	void		InitializeIngameScore(bool WaitForPlayers);
	void		DrawTime(SDL_Surface *bmpDest, int x, int y, float t);
	void		DrawReadyOverlay(SDL_Surface *bmpDest);
	void		DrawText(SDL_Surface *bmpDest, bool centre, int x, int y, Uint32 fgcol, const std::string& buf);
	void		DrawLocalChat(SDL_Surface *bmpDest);
	void		DrawRemoteChat(SDL_Surface *bmpDest);
    void        DrawScoreboard(SDL_Surface *bmpDest);
	void        DrawCurrentSettings(SDL_Surface *bmpDest);
#ifdef WITH_MEDIAPLAYER
	void		DrawMediaPlayer(SDL_Surface *bmpDest);
#endif
    void        InitializeViewportManager(void);
    void        DrawViewportManager(SDL_Surface *bmpDest);
	void		InitializeSpectatorViewportKeys();
	void		ProcessSpectatorViewportKeys();	// Fast camera mode switching when local worm is dead
	void		SimulateHud(void);
	int			getTopBarBottom();
	int			getBottomBarTop();
	void		DrawChatter(SDL_Surface *bmpDest);

	// Network
	void		Connect(const std::string& address);
	void		Connecting(void);
	void		ReadPackets(void);
	void		SendPackets(void);
	void		SendDeath(int victim, int killer);
	void		SendText(const std::string& sText, std::string sWormName);
	void		Disconnect(void);
	int			OwnsWorm(CWorm *w);

	// Sending
	void		SendWormDetails(void);
#ifdef DEBUG
	void		SendRandomPacket();
#endif

	// Parsing & network
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
    void        ParseSendFile(CBytestream *bs);

	void		InitializeDownloads();
	void		DownloadMap(const std::string& mapname);
	void		ProcessMapDownloads();
	void		ShutdownDownloads();
	void		processFileRequests();


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
	bool		getGameReady(void)			{ return bGameReady; }
	void		setGameReady(bool _g)		{ bGameReady = _g; }

    CChatBox    *getChatbox(void)           { return &cChatbox; }
	void		setRepaintChatbox(bool _r)  { bRepaintChatbox = true; }

	game_lobby_t *getGameLobby(void)		{ return &tGameLobby; }

	bool		getBadConnection(void)		{ return bBadConnection; }
	std::string	getBadConnectionMsg(void)	{ return strBadConnectMsg; }

	bool		getServerError(void)		{ return bServerError; }
	std::string	getServerErrorMsg(void)		{ return strServerErrorMsg; }

    bool		getClientError(void)        { return bClientError; }

	float		getLastReceived(void)		{ return fLastReceived; }
	void		setLastReceived(float _l)	{ fLastReceived = _l; }

	int			getNetSpeed(void)			{ return iNetSpeed; }
	void		setNetSpeed(int _n)			{ iNetSpeed = _n; }

	int			*getMsgSize(void)			{ return nMessageSizes; }

	CShootList	*getShootList(void)			{ return &cShootList; }

    CBonus      *getBonusList(void)         { return cBonuses; }

    void        setZombieTime(float z)      { fZombieTime = z; }
    float       getZombieTime(void)         { return fZombieTime; }

	frame_t		*getFrame(int FrameID)		{ return &tFrames[ FrameID ]; }

	int			getTeamScore(int team)		{ return iTeamScores[team]; }

	bool		isTyping(void)				{ return bChat_Typing; }
	const std::string& getChatterText()		{ return sChat_Text; }

	bool		getMuted(void)				{ return bMuted; }
	void		setMuted(bool _m)			{ bMuted = _m; }

	ClientRights *getRights()				{ return &tRights; }

	int	getPing(void)						{ return cNetChan.getPing(); }
	void setPing(int _p)					{ cNetChan.setPing(_p); }

	void	setServerAddress(const std::string& _a)	{ strServerAddr = _a; }
	const std::string& getServerAddress(void)		{ return strServerAddr; }

	void setServerName(const std::string& _n)		{ szServerName = _n; }
	const std::string& getServerName(void)			{ return szServerName; }

	int getHostVer(void)				{ return iHostOLXVer; }
	void setHostVer(int _v)				{ iHostOLXVer = _v; }
	const std::string & getClientVersion()				{ return sClientVersion; }
	void setClientVersion(const std::string & _s);
	const std::string & getServerVersion()				{ return sServerVersion; }
	void setServerVersion(const std::string & _s);
	int getClientOLXVer()	{ return iClientOLXVer; }

	bool getHostAllowsMouse(void)				{ return bHostAllowsMouse; }
	void setHostAllowsMouse(bool _b)			{ bHostAllowsMouse = _b; }

	bool		getGamePaused()					{ return (bViewportMgr || bGameMenu) && tGameInfo.iGameType == GME_LOCAL; }

	byte		getMapDlProgress()				{ return iMapDlProgress; }
	bool		getDownloadingMap()				{ return bDownloadingMap; }

	CViewport * getViewports()					{ return cViewports; }
	
	bool		getConnectingDuringGame()		{ return bConnectingDuringGame; };
	void		setConnectingDuringGame(bool b)	{ bConnectingDuringGame = b; };

	CFileDownloaderInGame * getFileDownloaderInGame()	{ return &cFileDownloaderInGame; };
	float		getLastDirtUpdate()						{ return fLastDirtUpdate; };
	void		setLastDirtUpdate( float _f )			{ fLastDirtUpdate = _f; };
	float		getLastFileRequestPacketReceived()		{ return fLastFileRequestPacketReceived; };
	void		setLastFileRequestPacketReceived( float _f ) { fLastFileRequestPacketReceived = _f; };
	
	bool		getSpectate()							{ return bSpectate; };
	void		setSpectate( bool _b )					{ bSpectate = _b; };

	bool		getForceWeaponsReady()					{ return bForceWeaponsReady; }
	void		setForceWeaponsReady(bool _r)			{ bForceWeaponsReady = _r; }

};

extern	CClient			*cClient;



#endif  //  __CCLIENT_H__
