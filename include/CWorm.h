/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Worm class
// Created 28/6/02
// Jason Boettcher


#ifndef __CWORM_H__
#define __CWORM_H__

#include <cassert>
#include "LieroX.h" // for MAX_WORMS, _AI_DEBUG
#include "CProjectile.h"
#include "CNinjaRope.h"
#include "CWpnRest.h"
#include "Options.h" // for control_t
#include "Utils.h"
#include "Frame.h"
#include "DeprecatedGUI/CBar.h"
#include "CMap.h"
#include "CWormSkin.h"
#include "Version.h"


// TODO: remove this after we changed network
#include "CBytestream.h"


#define		MAX_WEAPONSLOTS		10

// Direction
#define		DIR_LEFT			0
#define		DIR_RIGHT			1

#define		WRM_OUT				-1
#define		WRM_UNLIM			-2

// AI Game Type
#define		GAM_RIFLES			0
#define		GAM_100LT			1
#define		GAM_MORTARS			2
#define		GAM_OTHER			3

// Cells
#define		CELL_CURRENT		0
#define		CELL_LEFT			1
#define		CELL_DOWN			2
#define		CELL_RIGHT			3
#define		CELL_UP				4
#define		CELL_LEFTDOWN		5
#define		CELL_RIGHTDOWN		6
#define		CELL_LEFTUP			7
#define		CELL_RIGHTUP		8

struct WormType;
class CWpnRest;
class CGameScript;
class weapon_t;

// Weapon slot structure
struct wpnslot_t {
	const weapon_t	*Weapon;
	int			SlotNum;
	float		Charge;
	bool		Reloading;
	float		LastFire;
	bool		Enabled;
};

struct randweapons_t {
	int Weap1, Weap2, Weap3, Weap4, Weap5;
};

// the files have to be included yourself later
// they are used here; but their headers depends on this header
// TODO: remove the usage of these in this header
class CServerConnection;
class CBonus;

// TODO: why do we need that lobbyworm_t struct? Merge it with CWorm

// Lobby worm details
#define		LBY_OPEN	0
#define		LBY_CLOSED	1
#define		LBY_USED	2

struct lobbyworm_t {
	int			iType;
    int         iTeam;
	bool		bHost;
	bool		bReady;
};



/*

    Artificial Intelligence


*/

// AI states
enum {
    AI_THINK,
    //AI_FINDTARGET,
    AI_MOVINGTOTARGET,
    AI_AIMING,
    AI_SHOOTING
};

// Target types
enum {
    AIT_NONE,
    AIT_WORM,
    AIT_BONUS,
    AIT_POSITION
};


class NEW_ai_node_t {
public:
	float fX, fY;
	NEW_ai_node_t *psPrev, *psNext;
};

NEW_ai_node_t* get_last_ai_node(NEW_ai_node_t* n);
void delete_ai_nodes(NEW_ai_node_t* start);
void delete_ai_nodes(NEW_ai_node_t* start, NEW_ai_node_t* end);
float get_ai_nodes_length(NEW_ai_node_t* start);
// this do the same as the fct above except that it don't do the sqrt
float get_ai_nodes_length2(NEW_ai_node_t* start);

class CWorm;

class CWormInputHandler {
protected:
	CWorm* m_worm;
public: 
	CWormInputHandler(CWorm* w) : m_worm(w) {}
	virtual ~CWormInputHandler() {}
	
	virtual std::string name() = 0;
	
	virtual void initWeaponSelection() = 0; // should reset at least bWeaponsReady
	virtual void doWeaponSelectionFrame(SDL_Surface * bmpDest, CViewport *v) = 0;

	// simulation
	virtual void startGame() {}
	virtual void getInput() = 0;
    virtual void clearInput() {}
	
	virtual void onRespawn() {}
};


// TODO: split into classes: one for CClient and one for CServerConnection (latter only containing some general information, more like a simple struct)
class CWorm { friend class CWormInputHandler; friend class CWormBotInputHandler; friend class CWormHumanInputHandler;
public:
	// Constructor
	CWorm() {
		m_inputHandler = NULL;
		cOwner = NULL;
		pcMap = NULL;
		tProfile = NULL;
		pcHookWorm = NULL;
		cGameScript = NULL;
		cWeaponRest = NULL;
		Clear();
	}

	~CWorm() {
		Shutdown();
	}

private:
	// disallow these!
	CWorm(const CWorm&) { assert(false); }
	CWorm& operator=(const CWorm&) { assert(false); return *this; }
	
private:
	// Attributes

	// General
	bool		bUsed;
	int			iID;
	WormType*	m_type;
	bool		bLocal;
	int			iTeam;
	std::string	sName;
	int			iRanking;
	bool		bAlreadyKilled;
	bool		bSpectating;

	// Client info
	int			iClientID;
	int			iClientWormID;


	// Simulation
	worm_state_t tState;
	CVec		vPos;
	CVec		vVelocity;
	CVec		vLastPos;
	CVec		vDrawPos;
	bool		bOnGround;
	float		fCollisionTime;
	CVec		vCollisionVelocity;
	float		fLastInputTime;
	bool		bCollidedLastFrame;
	// last time we moved left or right
	float		lastMoveTime;

	
	float		fServertime; // only for CServerConnection: when a wormupdate arrives, the servertime of client (>=beta8)
	
	CVec		vFollowPos;
	bool		bFollowOverride;

    float       fLastCarve;
	

	
	// Score
	int			iKills;
	int			iDeaths;
	int			iSuicides;
	int			iDamage;

	int			iTotalWins;
	int			iTotalLosses;
	int			iTotalKills;
	int			iTotalDeaths;
	int			iTotalSuicides;
	Version		cClientVersion;

	// Game
	float		fLoadingTime;
	bool		bDrawMuzzle;
	int			iHealth;
	int			iLives;
	bool		bAlive;
	float		fTimeofDeath;
	int			iDirection;
	int			iMoveDirection;
	bool		bGotTarget;
	float		fAngle;
    float       fAngleSpeed;
    float		fMoveSpeedX;
	float		fFrame;
	CNinjaRope	cNinjaRope;
	profile_t	*tProfile;
	float		fRopeTime;
	bool		bVisible;
	float		fVisibilityChangeTime;  // Time when the worm was hidden/shown

	bool		bHooked;
	CWorm		*pcHookWorm;

	bool		bRopeDown;
	bool		bRopeDownOnce;

	bool		bTagIT;
	float		fTagTime;
	float		fLastSparkle;

    int         iDirtCount;

	float		fLastBlood;

	CMap*		pcMap;


	// Owner client
	CServerConnection *cOwner;

	// Network
	float		fFrameTimes[NUM_FRAMES];
	lobbyworm_t tLobbyState;
		// server
	worm_state_t tLastState; // Used for checking if we need to send the packet
	float		fLastAngle;
	float		fLastUpdateWritten;
	CVec		vLastUpdatedPos; // last pos we have send to client
		// client
	float		fLastPosUpdate;  // Used for velocity calculations (client does not send velocity)
	CVec		vOldPosOfLastPaket;
	CVec		vPreOldPosOfLastPaket;
	float		fPreLastPosUpdate;
	CVec		vLastEstimatedVel;
	CVec		vPreLastEstimatedVel;
	int			iLastCharge;
	int			iLastCurWeapon;

	
	float		fSpawnTime;

	// Graphics
	CWormSkin	cSkin;
	SmartPointer<SDL_Surface> bmpGibs;
	DeprecatedGUI::CBar		cHealthBar;
	//CViewport	*pcViewport;


	// Arsenal
	bool		bWeaponsReady;
	bool		bIsPrepared;
	bool		bGameReady;
	CGameScript	*cGameScript;
    CWpnRest    *cWeaponRest;
	int			iNumWeaponSlots;
	int			iCurrentWeapon;
	wpnslot_t	tWeapons[MAX_WEAPONSLOTS];
	bool		bNoShooting;
    bool		bFlag;
	AFK_TYPE	iAFK;
	std::string	sAFKMessage;

    // Force the showing of the current weapon
    bool        bForceWeapon_Name;
    float       fForceWeapon_Time;

	CWormInputHandler* m_inputHandler;

	// Used to print damage numbers over the worm head
	struct DamageReport_t {
		int damage;
		float lastTime;
	};
	DamageReport_t cDamageReport[MAX_WORMS];

public:
	// Methods


	//
	// General
	//
	void		Clear(void);
	void		Init(void);
	//void		CopyProfile(plyprofile_t *profile);
	void		Shutdown(void);


	// TODO: move this out here (to network engine)
	//
	// Network
	//
	void		writeInfo(CBytestream *bs);
	void		readInfo(CBytestream *bs);
	static bool	skipInfo(CBytestream *bs)  { bs->SkipString(); bs->Skip(2); bs->SkipString(); return bs->Skip(3); }
	void		updateCheckVariables();
	bool		checkPacketNeeded();
	void		writePacket(CBytestream *bs, bool fromServer, CServerConnection* receiver);
	void		readPacket(CBytestream *bs, CWorm *worms);
	void		net_updatePos(const CVec& newpos);
	bool		skipPacket(CBytestream *bs);
	void		readPacketState(CBytestream *bs, CWorm *worms);
	static bool	skipPacketState(CBytestream *bs);
	void		writeWeapons(CBytestream *bs);
	void		readWeapons(CBytestream *bs);
	static bool	skipWeapons(CBytestream *bs)  { return bs->Skip(5); } // 5 weapons
	void		writeStatUpdate(CBytestream *bs);
	void		updateStatCheckVariables();
	bool		checkStatePacketNeeded();
	void		readStatUpdate(CBytestream *bs);
	static bool	skipStatUpdate(CBytestream *bs) { return bs->Skip(2); } // Current weapon and charge
	int			GetMyPing(void);

	
	void		setupLobby(void);
	void		loadDetails(void);
	void		saveDetails(void);



	// Weapon
	void		GetRandomWeapons();
	void		CloneWeaponsFrom(CWorm* w);


	//
	// Graphics
	//
	bool		ChangeGraphics(int generalgametype);
	void		FreeGraphics(void);
	SmartPointer<SDL_Surface> ChangeGraphics(const std::string& filename, bool team);
	void		Draw(SDL_Surface * bmpDest, CViewport *v);
    void        DrawShadow(SDL_Surface * bmpDest, CViewport *v);
	void		UpdateDrawPos();

	//
	// Game
	//
	bool		isPrepared() { return bIsPrepared; }
	void		Prepare(CMap *pcMap); // weapon selection and so on
	void		Unprepare(); // after a game
	void		StartGame();
	void		Spawn(CVec position);
	bool		Injure(int damage);
	bool		Kill(void);
	bool		CheckBonusCollision(CBonus *b);
	bool		GiveBonus(CBonus *b);
	void		Hide(bool immediate);
	void		Show(bool immediate);


	void		getInput();
	void		clearInput();
	void		initWeaponSelection();
	void		doWeaponSelectionFrame(SDL_Surface * bmpDest, CViewport *v);




	//
	// Misc.
	//
	bool		CanType(void);

	bool		isHostWorm();
	bool		shouldDoOwnWeaponSelection();
	
	
    int         traceLine(CVec target, float *fDist, int *nType, int divs = 5);
	int			traceLine(CVec target, CVec start, int *nType, int divs = 5, uchar checkflag = PX_EMPTY);
	
	bool		IsEmpty(int Cell);
    bool		CheckOnGround();
	

	//
	// Variables
	//
	bool		isUsed(void)				{ return bUsed; }
	void		setUsed(bool _u);

	CMap*		getMap()					{ return pcMap; }
	void		setMap(CMap *map)			{ pcMap = map; }
	CNinjaRope*	getNinjaRope()				{ return &cNinjaRope; }

	std::string getName(void)			{ return sName; }
	void		setName(const std::string& val) { sName = val; }
	Uint32		getGameColour(void);
	void		setColour(Uint32 c)			{ cSkin.Colorize(c); }
	void		setColour(Uint8 r, Uint8 g, Uint8 b) {cSkin.Colorize(MakeColour(r,g,b)); }

	void		setLocal(bool _l)			{ bLocal = _l; }
	bool		getLocal(void)				{ return bLocal; }

	int			getHealth(void)				{ return iHealth; }
	void		setHealth(int _h)			{ iHealth = CLAMP(_h, 0, 100); }
	int			getLives(void)				{ return iLives; }
	int			getKills(void)				{ return iKills; }
	void		setLives(int l)				{ iLives = l; }
	int			getDamage(void)				{ return iDamage; }
	void		setDamage(int l)			{ iDamage = l; }
	void		addDamage(int damage, CWorm* victim, const GameOptions::GameInfo & settings);

	void		AddKill(void)				{ iKills++; }
    void        setKills(int k)             { iKills = k; }

	void		setID(int i)				{ iID = i; }
	int			getID(void)	const			{ return iID; }

	WormType*	getType(void)				{ return m_type; }
    void        setType(WormType* t)        { m_type = t; }

	bool		getAlive(void)				{ return bAlive; }
	void		setAlive(bool _a)			{ bAlive = _a; }

	float		getTimeofDeath(void)		{ return fTimeofDeath; }

	void		setHooked(bool h, CWorm *w)	{ bHooked=h; pcHookWorm=w; }
	void		setClient(CServerConnection *cl)		{ cOwner = cl; }
    CServerConnection     *getClient(void)            { return cOwner; }

	CVec		getFollowPos(void)			{ return (bFollowOverride?vFollowPos:vPos); }
	void		resetFollow(void)			{ bFollowOverride = false; }
	void		doFollow(int x, int y)		{ bFollowOverride = true; vFollowPos.x = (float)x; vFollowPos.y = (float)y; }

	CVec		getPos(void)				{ return vPos; }
	void		setPos(CVec v)				{ vPos = v; }

	CVec&		pos()						{ return vPos; }
	bool		isOnGround()				{ return bOnGround; }
	void		setOnGround(bool g)			{ bOnGround = g; }

	CVec		*getVelocity(void)			{ return &vVelocity; }
	CVec&		velocity()					{ return vVelocity; }

	worm_state_t *getWormState(void)		{ return &tState; }

	bool		hasOwnServerTime();
	float		serverTime()				{ return fServertime; }

	bool		isVisible()	const			{ return bVisible; }
	bool		isVisible(CViewport* v) const;
	bool		isVisible(CWorm* viewerWorm) const;
	
	float		getAngle(void)				{ return fAngle; }
	void		setAngle(float a)			{ fAngle = a; }
	int			getDirection(void)			{ return iDirection; }
	void		setDirection(int d)			{ iDirection = d; }
	int			getMoveDirection()		{ return iMoveDirection; }

	void		setLoadingTime(float l)		{ fLoadingTime = l; }
	float		getLoadingTime()			{ return fLoadingTime; }

	CGameScript* getGameScript()			{ return cGameScript; }
	void		setGameScript(CGameScript *gs)	{ cGameScript = gs; }
    void        setWpnRest(CWpnRest *wr)    { cWeaponRest = wr; }

	void		setDrawMuzzle(bool _d)		{ bDrawMuzzle = _d; }

	bool		getWeaponsReady(void)		{ return bWeaponsReady; }
	void		setWeaponsReady(bool _w)	{ bWeaponsReady = _w; }
	wpnslot_t	*getCurWeapon(void)			{ return &tWeapons[MIN(4, iCurrentWeapon)]; }
	int			getCurrentWeapon(void)		{ return MIN(4, iCurrentWeapon); }
	void		setCurrentWeapon(int _w)	{ iCurrentWeapon = MIN(4,_w); }
	wpnslot_t	*getWeapon(int id)			{ return &tWeapons[id]; }

	void		setGameReady(bool _g)		{ bGameReady = _g; }
	bool		getGameReady(void)			{ return bGameReady; }

	void		setProfile(profile_t *p)	{ tProfile = p; }
	profile_t	*getProfile()				{ return tProfile; }

	void		setTeam(int _t)				{ iTeam = _t; if(getLobby()) getLobby()->iTeam = _t; }
	int			getTeam() const				{ return iTeam; }

	SmartPointer<SDL_Surface> getGibimg(void)			{ return bmpGibs; }
	SmartPointer<SDL_Surface> getPicimg(void)			{ return cSkin.getPreview(); }

	lobbyworm_t	*getLobby(void)				{ return &tLobbyState; }

	bool		getTagIT(void)				{ return bTagIT; }
	void		setTagIT(bool _t)			{ bTagIT = _t; }

	float		getLastSparkle()			{ return fLastSparkle; }
	void		setLastSparkle(float s)		{ fLastSparkle = s; }
	float		getLastBlood()				{ return fLastBlood; }
	void		setLastBlood(float b)		{ fLastBlood = b; }

    void        incrementDirtCount(int d);
    int         getDirtCount(void)          { return iDirtCount; }

	void		setTarget(bool _t)			{ bGotTarget = _t; }

	float		getTagTime(void)			{ return fTagTime; }
	void		setTagTime(float _t)		{ fTagTime = _t; }
	void		incrementTagTime(float dt)	{ fTagTime+=dt; }

	CWormSkin&	getSkin(void)				{ return cSkin; }
	void		setSkin(const CWormSkin& skin)	{ cSkin = skin; }
	void		setSkin(const std::string& skin)	{ cSkin.Change(skin); }

	bool		getAlreadyKilled()			{ return bAlreadyKilled; }
	void		setAlreadyKilled(bool _k)	{ bAlreadyKilled = _k; }

	bool		isShooting()				{ return tState.bShoot; }
	bool		isWeaponReloading()			{ return getCurWeapon()->Reloading; }

	bool		getVIP(void)				{ return bNoShooting; }
	void		setVIP(bool _s)				{ bNoShooting = _s; }

	bool		isSpectating()				{ return bSpectating; }
	void		setSpectating(bool _s)		{ bSpectating = _s; }

	// TODO: the sense of this isn't clear; so make it clear
	bool		getFlag(void)				{ return bFlag; }
	void		setFlag(bool _f)			{ bFlag = _f; bNoShooting = _f; }

	AFK_TYPE	getAFK(void)				{ return iAFK; }
	const std::string & getAFKMessage()		{ return sAFKMessage; }
	void		setAFK(AFK_TYPE _f, const std::string & msg);

	void	addTotalWins(int _w = 1)		{ iTotalWins += _w; }
	int		getTotalWins(void)				{ return iTotalWins; }
	void	addTotalLosses(int _l = 1)		{ iTotalLosses += _l; }
	int		getTotalLosses(void)			{ return iTotalLosses; }
	void	addTotalKills(int _k = 1)		{ iTotalKills += _k; }
	int		getTotalKills(void)				{ return iTotalKills; }
	void	addTotalDeaths(int _d = 1)		{ iTotalDeaths += _d; }
	int		getTotalDeaths(void)			{ return iTotalDeaths; }
	void	addTotalSuicides(int _d = 1)	{ iTotalSuicides += _d; }
	int		getTotalSuicides(void)			{ return iTotalSuicides; }
	
	const Version & getClientVersion()				{ return cClientVersion; }
	void	setClientVersion(const Version & v)		{ cClientVersion = v; }


	float&		frame()						{ return fFrame; }
	
	CWormInputHandler* inputHandler() { return m_inputHandler; }
	
	DamageReport_t* getDamageReport() { return cDamageReport; }

	void setCollisionTime(float _t)			{ fCollisionTime = _t; }
	float getCollisionTime() const			{ return fCollisionTime; }
	void setCollisionVel(CVec _v)			{ vCollisionVelocity = _v; }
	CVec getCollisionVel() const			{ return vCollisionVelocity; }
	void setCollidedLastFrame(bool _c)		{ bCollidedLastFrame = _c; }
	bool hasCollidedLastFrame() const		{ return bCollidedLastFrame; }
	
	
	// HINT: saves the current time of the simulation
	// TODO: should be moved later to PhysicsEngine
	// but it's not possible in a clean way until we have no simulateWorms()
	// there which simulates all worms together
	float	fLastSimulationTime;
};


int traceWormLine(CVec target, CVec start, CMap *pcMap, CVec* collision = NULL);


#endif  //  __CWORM_H__
