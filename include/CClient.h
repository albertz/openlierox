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

#include <string>
#include <vector>
#include <map>
#include <set>
#include "FastVector.h"
#include "CChatBox.h"
#include "Networking.h"
#include "CBytestream.h"
#include "CShootList.h"
#include "Version.h"
#include "FileDownload.h"
#include "Frame.h"
#include "CProjectile.h"
#include "CWpnRest.h"
#include "Consts.h"
#include "LieroX.h"
#include "CViewport.h"
#include "SafeVector.h"


namespace DeprecatedGUI {
	class CBrowser;
	class CListview;
	class CBar;
}

class CHttpDownloadManager;
class CChannel;
class CClientNetEngine;
class CBonus;
class CMap;
class profile_t;
class FlagInfo;

// TODO: this is just a small helper for now; some of these parts should just move into CClient::Connect
bool JoinServer(const std::string& addr, const std::string& name, const std::string& player);


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
	TimeDiff	fTagTime;
	bool		bLeft;
	TimeDiff	fTimeLeft;
	int			iType;
};

// Game log structure
class game_log_t { public:
	log_worm_t	*tWorms;
	int			iNumWorms;
	int			iWinner;
	AbsTime		fGameStart;
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
	int		TimeLeftX;
	int		TimeLeftY;
	int		TimeLeftW;

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

enum {
	DL_HTTP = 0,
	DL_UDP
};

enum  {
	NAT_RESOLVING_DNS = 0,
	NAT_WAIT_TRAVERSE_REPLY,
	NAT_SEND_CHALLENGE,
	NAT_WAIT_CHALLENGE_REPLY
};


class CClient {
public:
	CClient();
	~CClient();
	
	friend class CClientNetEngine;
	friend class CClientNetEngineBeta7;
	friend class CClientNetEngineBeta9;

	typedef void (*DownloadFinishedCB) ();

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
	typedef FastVector<CProjectile,MAX_PROJECTILES> Projectiles;
	Projectiles	cProjectiles;
	
public:
	struct MapPosIndex {
		static const int GRIDW = 20, GRIDH = 20;
		int x, y;
		MapPosIndex(int _x = 0, int _y = 0) : x(_x), y(_y) {}
		MapPosIndex(const VectorD2<int>& v) { x = v.x / GRIDW; y = v.y / GRIDH; }
		
		bool operator==(const MapPosIndex& i) const { return x == i.x && y == i.y; }
		bool operator!=(const MapPosIndex& i) const { return !(*this == i); }
		bool operator<(const MapPosIndex& i) const { return y < i.y || (y == i.y && x < i.x); }
		
		long index(const CMap* m) const;
	};
	typedef std::set<CProjectile*> ProjectileSet;
	typedef SafeVector<ProjectileSet> ProjectilePosMap;
	ProjectilePosMap projPosMap;
	
private:	
	// Frames
	frame_t		tFrames[NUM_FRAMES];

	// Game
	SmartPointer<CGameScript> cGameScript;
    CWpnRest    cWeaponRestrictions;
	GameOptions::GameInfo tGameInfo;	// Also game lobby
	FeatureCompatibleSettingList otherGameInfo;	
	bool	bServerChoosesWeapons; // the clients will not get the weapon selection screen and the server sets it; if true, only >=Beta7 is supported
	FlagInfo*	m_flagInfo;

	// Ping below FPS
	AbsTime		fMyPingSent;
	AbsTime		fMyPingRefreshed;
	int			iMyPing;
	TimeDiff	fServertime; // only in >=Beta8 correctly synchronised
	
	int			iScoreboard[MAX_WORMS];
	int			iScorePlayers;
	int			iTeamScores[4];
	int			iTeamList[4];

	// Interface
	interface_sett tInterfaceSettings;
	DeprecatedGUI::CBar		*cHealthBar1;
	DeprecatedGUI::CBar		*cHealthBar2;
	DeprecatedGUI::CBar		*cWeaponBar1;
	DeprecatedGUI::CBar		*cWeaponBar2;
	DeprecatedGUI::CBar		*cDownloadBar;
	SmartPointer<SDL_Surface> bmpBoxBuffer;
	SmartPointer<SDL_Surface> bmpBoxLeft;
	SmartPointer<SDL_Surface> bmpBoxRight;
	bool		bShouldRepaintInfo;
	bool		bCurrentSettings;


	// Game menu && score
	bool		bUpdateScore;
	AbsTime		fLastScoreUpdate;

	// Ingame scoreboard
	SmartPointer<SDL_Surface> bmpIngameScoreBg;

	// Bonus's
	CBonus		*cBonuses;

	// Chatbox
	int			iChat_Numlines;
	chat_line_t	tChatLines[MAX_CHATLINES];

	CChatBox	cChatbox;		// Our chatbox
	DeprecatedGUI::CBrowser	*cChatList;		// Ingame chatlist
	bool		bRepaintChatbox;

	// Send chat
	bool		bChat_Typing;
	UnicodeChar	iChat_Lastchar;
	bool		bChat_Holding;
	size_t		iChat_Pos;
	AbsTime		fChat_TimePushed;
	CInput		cChat_Input;
	CInput		cTeamChat_Input;
	bool		bTeamChat;
	std::string	sChat_Text;
	TimeDiff	fChat_BlinkTime;
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
	CClientNetEngine * cNetEngine;	// Should never be NULL, to skip some checks
	int			iNetSpeed;
	int			iNetStatus;
	int			reconnectingAmount;
	std::string	strServerAddr;
	std::string	strServerAddr_HumanReadable;
	NetworkAddr	cServerAddr;
	int			iNumConnects;
	AbsTime		fConnectTime;
	int			iChallenge;
	AbsTime		fLastReceived;
	SmartPointer<NetworkSocket>	tSocket;
	CChannel	* cNetChan;
	CBytestream	bsUnreliable;
	CShootList	cShootList;
    AbsTime      fZombieTime;
	TimeDiff	fSendWait;
	AbsTime		fLastUpdateSent;
	std::string	szServerName;
	bool		bHostAllowsStrafing;
	Version		cClientVersion;
	Version		cServerVersion;
	bool		bLocalClient;

	// Map downloading
	bool		bDownloadingMap;
	CHttpDownloadManager *cHttpDownloader;
	std::string	sMapDownloadName;
	bool		bDlError;
	std::string	sDlError;
	int			iDlProgress;
	int			iDownloadMethod;  // HTTP or UDP
	bool		bWaitingForMap;  // In game and waiting for the map to finish downloading
	DownloadFinishedCB tMapDlCallback;

	// Mod downloading
	bool		bDownloadingMod;
	std::string	sModDownloadName;
	bool		bWaitingForMod;
	DownloadFinishedCB tModDlCallback;

	CUdpFileDownloader	cUdpFileDownloader;
	AbsTime		fLastFileRequest;
	AbsTime		fLastFileRequestPacketReceived;
	size_t		iModDownloadingSize;	// For progress bar, UDP only

	bool		bReadySent;

	bool		bGameOver;
	bool		bGameMenu;
	int			iMatchWinner;
	int			iMatchWinnerTeam;
	AbsTime		fGameOverTime;

	bool		bLobbyReady;
	bool		bGameReady; // bGameReady says if the game (including cMap) has been initialized
	bool		bGameRunning; // game was started and is running (implies bGameReady, but our own worm could be in weapon selection)
	bool		bHaveMap;
	bool		bHaveMod;

	bool		bBadConnection;
	std::string	strBadConnectMsg;

	bool		bServerError;
	std::string	strServerErrorMsg;

    bool		bClientError;

	std::string	strDebug;	// This is only a debug string which is reset after each frame and which is printed on the screen.
	
	struct		cSpectatorViewportKeys_t {
				CInput Up, Down, Left, Right, V1Type, V2Type, V2Toggle;
	} cSpectatorViewportKeys;
	std::string	sSpectatorViewportMsg;
	AbsTime		fSpectatorViewportMsgTimeout;
	int			iNatTraverseState;
	bool		bConnectingBehindNat;
	AbsTime		fLastChallengeSent;
	AbsTime		fLastTraverseSent;
	int			iNatTryPort;
	std::string sUdpMasterserverAddress;

public:
	// HINT: saves the current time of the simulation
	// TODO: should be moved later to PhysicsEngine
	// but it's not possible in a clean way until we have no simulateWorld()
	// there which simulates everything together
	// HINT: this is currently used for simulating the projectiles
	// if you are going to use this also for something else,
	// then be sure that is is run together with simulateProjectiles() !
	AbsTime	fLastSimulationTime;

// IRC callbacks
private:
	static void IRC_OnNewMessage(const std::string& msg, int type);
	static void IRC_OnDisconnect();

public:
	// Methods

	void		Clear();
	void		MinorClear();
	int			Initialize();
	void		Shutdown();
	void		FinishGame();
	void		ReinitLocalWorms();
	
	// Logging
	void		StartLogging(int num_players);
	void		ShutdownLog();
	log_worm_t	*GetLogWorm(int id);
	void		GetLogData(std::string& data);

	std::list<int> AddRandomBots(int amount = 1, bool outOfGame = false); // returns list of added worms
	int			AddWorm(profile_t* p, bool outOfGame = false); // returns -1 if failed, worm id otherwise
	void		RemoveWorm(int id);

	// Game
	void		Simulation();
	void		SetupViewports();
	void		SetupViewports(CWorm *w1, CWorm *w2, int type1, int type2);
	void		SendCarve(CVec pos);
	void		PlayerShoot(CWorm *w);
	void		ShootSpecial(CWorm *w);
	void		DrawBeam(CWorm *w);
	void		ProcessServerShotList();
	void		DoLocalShot( float fTime, float fSpeed, int nAngle, CWorm *pcWorm );
	void		ProcessShot(shoot_t *shot, AbsTime fSpawnTime);
	void		ProcessShot_Beam(shoot_t *shot);

	void		BotSelectWeapons();

	void		SpawnProjectile(CVec pos, CVec vel, int rot, int owner, proj_t *_proj, int _random, AbsTime time, AbsTime ignoreWormCollBeforeTime);
    void        disableProjectile(CProjectile *prj);
	void		Explosion(CVec pos, float damage, int shake, int owner);
	void		InjureWorm(CWorm *w, float damage, int owner);
	void		UpdateScoreboard();
	void		LaserSight(CWorm *w, float Angle, bool highlightCrosshair = true);

	void		processChatter();
    void        processChatCharacter(const KeyboardEvent& input);

	void		DestroyBonus(int id, bool local, int wormid);

	CVec		FindNearestSpot(CWorm *w);

	// Main
	void		Frame();

	// Drawing
	bool		InitializeDrawing();
	bool		InitializeBar(int number);
	void		DrawPlayerWaitingColumn(SDL_Surface * bmpDest, int x, int y, std::list<CWorm *>::iterator& it, const std::list<CWorm *>::iterator& last, int num);
	void		DrawPlayerWaiting(SDL_Surface * bmpDest);
	void		DrawBox(SDL_Surface * dst, int x, int y, int w);
	void		Draw(SDL_Surface * bmpDest);
	void		DrawViewport(SDL_Surface * bmpDest, int viewport_index);
	void		DrawViewport_Game(SDL_Surface* bmpDest, CViewport* v);
	void		DrawProjectiles(SDL_Surface * bmpDest, CViewport *v);
    void        DrawProjectileShadows(SDL_Surface * bmpDest, CViewport *v);
	void		InitializeGameMenu();
	void		DrawGameMenu(SDL_Surface * bmpDest);
	void		DrawBonuses(SDL_Surface * bmpDest, CViewport *v);
	void		UpdateScore(DeprecatedGUI::CListview *Left, DeprecatedGUI::CListview *Right);
	void		UpdateIngameScore(DeprecatedGUI::CListview *Left, DeprecatedGUI::CListview *Right, bool WaitForPlayers);
	void		InitializeIngameScore(bool WaitForPlayers);
	void		DrawTime(SDL_Surface * bmpDest, int x, int y, float t);
	void		DrawReadyOverlay(SDL_Surface * bmpDest);
	void		DrawText(SDL_Surface * bmpDest, bool centre, int x, int y, Color fgcol, const std::string& buf);
	void		DrawLocalChat(SDL_Surface * bmpDest);
	void		DrawRemoteChat(SDL_Surface * bmpDest);
    void        DrawScoreboard(SDL_Surface * bmpDest);
	void        DrawCurrentSettings(SDL_Surface * bmpDest);

    void        InitializeViewportManager();
    void        DrawViewportManager(SDL_Surface * bmpDest);
	void		InitializeSpectatorViewportKeys();
	void		ProcessSpectatorViewportKeys();	// Fast camera mode switching when local worm is dead
	void		SimulateHud();
	int			getTopBarBottom();
	int			getBottomBarTop();
	void		DrawChatter(SDL_Surface * bmpDest);
	void		SetupGameInputs(); // Re-setup inputs for worms, viewports and all game actions
	
	CClientNetEngine * getNetEngine() { return cNetEngine; };
	void		setNetEngineFromServerVersion();

	void		Connect(const std::string& address);
	void		Reconnect();
	void		Connecting(bool force = false);
	void		ConnectingBehindNAT();
	void		SetSocketWithEvents(bool v);
	void		Disconnect();

	bool		ReadPackets();
	void		SendPackets(bool sendPendingOnly = false);

	void		InitializeDownloads();
	void		DownloadMap(const std::string& mapname);
	void		ProcessMapDownloads();
	void		AbortDownloads();
	void		ShutdownDownloads();
	void		FinishMapDownloads();
	void		DownloadMod(const std::string& modname);
	void		ProcessModDownloads();
	void		FinishModDownloads();
	void		ProcessUdpUploads();

	// Variables
	CChannel	*getChannel()			{ return cNetChan; }
	CChannel	*createChannel(const Version& v);
	int			getStatus()				{ return iNetStatus; }
	void		setStatus(int _s)			{ iNetStatus = _s; }
	CBytestream	*getUnreliable()		{ return &bsUnreliable; }
	bool		RebindSocket();	// If client has taken the port on which server should start - free it

	CMap*		getMap() const				{ return cMap; }
	bool		isMapReady() const;
	void		resetMap()					{ cMap = NULL; }

	bool		canSimulate() const			{ return bGameReady && !bGameOver && isMapReady(); }
	
	int			OwnsWorm(int id);
	int			getNumWorms()			{ return iNumWorms; }
	void		setNumWorms(int _w)			{ iNumWorms = _w; }
	bool		canAddWorm(std::string* noReason = NULL);
	
	CWorm		*getWorm(int w)				{ return cLocalWorms[w]; }
	void		setWorm(int i, CWorm *w)	{ cLocalWorms[i] = w; }

	bool		serverChoosesWeapons()		{ return bServerChoosesWeapons; }
	
	void		clearHumanWormInputs();
	void		clearLocalWormInputs();

	CWorm		*getRemoteWorms()		{ return cRemoteWorms; }
	int			getTeamWormCount(int t) const;
	bool		getGameReady()			{ return bGameReady; }
	void		setGameReady(bool _g)		{ bGameReady = _g; }

    CChatBox    *getChatbox()           { return &cChatbox; }
	void		setRepaintChatbox(bool _r)  { bRepaintChatbox = true; }

	GameOptions::GameInfo *getGameLobby()		{ return &tGameInfo; }

	bool		getBadConnection()		{ return bBadConnection; }
	std::string	getBadConnectionMsg()	{ return strBadConnectMsg; }

	bool		getServerError()		{ return bServerError; }
	std::string	getServerErrorMsg()		{ return strServerErrorMsg; }

    bool		getClientError()        { return bClientError; }

	AbsTime		getLastReceived()		{ return fLastReceived; }
	void		setLastReceived(const AbsTime& _l)	{ fLastReceived = _l; }

	int			getNetSpeed()			{ return iNetSpeed; }
	void		setNetSpeed(int _n)			{ iNetSpeed = _n; }

	CShootList	*getShootList()			{ return &cShootList; }
    CBonus      *getBonusList()         { return cBonuses; }
	CWpnRest*	getWeaponRestrictions()		{ return &cWeaponRestrictions; }
	SmartPointer<CGameScript>& getGameScript() { return cGameScript; }
	
    void        setZombieTime(const AbsTime& z)      { fZombieTime = z; }
    AbsTime       getZombieTime()         { return fZombieTime; }

	frame_t		*getFrame(int FrameID)		{ return &tFrames[ FrameID ]; }

	int			getTeamScore(int team)		{ if(team >= 0 && team < 4) return iTeamScores[team]; else return 0; }

	bool		isTyping()				{ return bChat_Typing; }
	void		setChatPos(size_t v)		{ iChat_Pos = v; }
	std::string& chatterText()		{ return sChat_Text; }
	std::string	getChatterCommand();

	bool		getMuted()				{ return bMuted; }
	void		setMuted(bool _m)			{ bMuted = _m; }

	int	getPing();
	void setPing(int _p);

	int getMyPing()							{ return iMyPing; } // Use only when iGameType == GME_JOIN

	TimeDiff serverTime()						{ return fServertime; }
	
	const std::string& getServerAddress()		{ return strServerAddr; }
	std::string getServerAddr_HumanReadable()		{ return strServerAddr_HumanReadable; }

	void setServerName(const std::string& _n)		{ szServerName = _n; }
	const std::string& getServerName()			{ return szServerName; }

	int getGeneralGameType() const				{ return tGameInfo.iGeneralGameType; }
	bool isTeamGame() const						{ return tGameInfo.iGeneralGameType == GMT_TEAMS; }

	const Version& getClientVersion()				{ return cClientVersion; }
	void setClientVersion(const Version& v);
	const Version& getServerVersion()				{ return cServerVersion; }
	void setServerVersion(const std::string & _s);

	FeatureCompatibleSettingList& getUnknownFeatures() { return otherGameInfo; } // Both unknown and server-side features here
	
	bool isHostAllowingStrafing()				{ return bHostAllowsStrafing; }

	bool		getGamePaused()					{ return (bViewportMgr || bGameMenu) && tLX->iGameType == GME_LOCAL; }
	
	int			getDlProgress()					{ return iDlProgress; }
	bool		getDownloadingMap()				{ return bDownloadingMap; }
	bool		getDownloadingMod()				{ return bDownloadingMod; }
	int			getDownloadMethod()				{ return iDownloadMethod; }
	bool		getDownloadingError()			{ return bDlError; }
	void		clearDownloadingError()			{ bDlError = false; }
	std::string	getDownloadingErrorMessage()	{ return sDlError; }

	CViewport * getViewports()					{ return cViewports; }
	bool		isWormVisibleOnAnyViewport(int worm) const; 

	CUdpFileDownloader * getUdpFileDownloader()	{ return &cUdpFileDownloader; };
	AbsTime		getLastFileRequest()					{ return fLastFileRequest; };
	void		setLastFileRequest( const AbsTime& _f ) 			{ fLastFileRequest = _f; };
	AbsTime		getLastFileRequestPacketReceived()		{ return fLastFileRequestPacketReceived; };
	void		setLastFileRequestPacketReceived( const AbsTime& _f ) { fLastFileRequestPacketReceived = _f; };

	bool		isGameMenu()			{ return bGameMenu; }
	bool		isChatTyping()			{ return bChat_Typing; }
	bool		isGameOver()			{ return bGameOver; }
	bool&		shouldRepaintInfo()		{ return bShouldRepaintInfo; }

	void		setReadySent(bool b)	{ bReadySent = b; }
	bool		shouldDoProjectileSimulation();
	
	bool		isLocalClient()			{ return bLocalClient; }
	void		setLocalClient(bool _l)	{ bLocalClient = _l; }

	std::string	debugName();
	
	bool		getHaveMap()			{ return bHaveMap; }
	bool		getHaveMod()			{ return bHaveMod; }
	void		setHaveMap( bool _b )	{ bHaveMap = _b; }
	void		setHaveMod( bool _b )	{ bHaveMod = _b; }
	int			getNumRemoteWorms();
	profile_t	**getLocalWormProfiles()	{ return tProfiles; }
	FlagInfo*	flagInfo()			{ return m_flagInfo; }

	bool		isTeamGame() { return getGameLobby()->iGeneralGameType == GMT_TEAMS; }
	bool		isTagGame() { return getGameLobby()->iGeneralGameType == GMT_TIME; }
	
	void		setOnMapDlFinished(DownloadFinishedCB f)  { tMapDlCallback = f; }
	void		setOnModDlFinished(DownloadFinishedCB f)  { tModDlCallback = f; }
	
	Projectiles & getProjectiles()		{ return cProjectiles; }
	
	void		DumpGameState(CmdLineIntf* caller);
	
	void		addDebugStr(const std::string& str) { if(strDebug != "") strDebug += "\n" + str; else strDebug = str; }
	void		resetDebugStr() { strDebug = ""; }
};

extern	CClient			*cClient;



#endif  //  __CCLIENT_H__
