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
#include "Frame.h"
#include "DeprecatedGUI/CBar.h"
#include "CMap.h"
#include "CGameSkin.h"
#include "Entity.h"
#include "NewNetEngine.h" // For NetSyncedRandom
#include "Version.h"
#include "NewNetEngine.h"

// TODO: remove this after we changed network
#include "CBytestream.h"

#ifdef _MSC_VER
// Warning: this used in member initializer list
#pragma warning(disable:4355)
#endif

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
struct weapon_t;

// Weapon slot structure
struct wpnslot_t {
	const weapon_t	*Weapon;
	int			SlotNum;
	float		Charge;
	bool		Reloading;
	float		LastFire;
	bool		Enabled;
	wpnslot_t() : Weapon(NULL), SlotNum(0), Charge(0), Reloading(false), LastFire(0), Enabled(false) {}
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


struct WormJoinInfo;

// TODO: split into classes: one for CClient and one for CServerConnection (latter only containing some general information, more like a simple struct)
class CWorm {
	friend class CWormInputHandler;
	friend class CWormBotInputHandler;
	friend class CWormHumanInputHandler;
	friend struct WormJoinInfo;
public:
	// Constructor
	CWorm(): cSparkles(this)
	{
		// set all pointers to NULL
		m_inputHandler = NULL;
		cOwner = NULL;
		tProfile = NULL;
		pcHookWorm = NULL;
		cGameScript = NULL;
		cWeaponRest = NULL;
		m_type = NULL;
		Clear();
	}

	~CWorm() {
		Shutdown();
	}

private:
	// disallow these!
	CWorm(const CWorm&): cSparkles(this) { assert(false); }
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
	bool		bSpawnedOnce;
	
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
	AbsTime		fCollisionTime;
	CVec		vCollisionVelocity;
	AbsTime		fLastInputTime;
	bool		bCollidedLastFrame;
	// last time we moved left or right
	AbsTime		lastMoveTime;

	
	TimeDiff	fServertime; // only for CServerConnection: when a wormupdate arrives, the servertime of client (>=beta8)
	
	CVec		vFollowPos;
	bool		bFollowOverride;

    AbsTime       fLastCarve;
	

	
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
	AbsTime		fTimeofDeath;
	int			iDirection;
	int			iMoveDirection;
	bool		bGotTarget;
	float		fAngle;
    float       fAngleSpeed;
    float		fMoveSpeedX;
	float		fSpeedFactor;
	bool		bCanUseNinja;
	float		fDamageFactor;
	bool		bCanAirJump; // For instant air jump
	AbsTime		fLastAirJumpTime; // For relative air-jump
	float		fFrame;
	CNinjaRope	cNinjaRope;
	profile_t	*tProfile;
	AbsTime		fRopeTime;
	std::vector<bool>	bVisibleForWorm;
	AbsTime		fVisibilityChangeTime;  // AbsTime when the worm was hidden/shown

	bool		bHooked;
	CWorm		*pcHookWorm;

	bool		bRopeDown;
	bool		bRopeDownOnce;

	bool		bTagIT;
	TimeDiff	fTagTime;
	EntityEffect cSparkles;

    int         iDirtCount;

	AbsTime		fLastBlood;


	// Owner client
	CServerConnection *cOwner;

	// Network
	AbsTime		fFrameTimes[NUM_FRAMES];
	lobbyworm_t tLobbyState;
		// server
	worm_state_t tLastState; // Used for checking if we need to send the packet
	float		fLastAngle;
	AbsTime		fLastUpdateWritten;
	CVec		vLastUpdatedPos; // last pos we have send to client
		// client
	AbsTime		fLastPosUpdate;  // Used for velocity calculations (client does not send velocity)
	CVec		vOldPosOfLastPaket;
	CVec		vPreOldPosOfLastPaket;
	AbsTime		fPreLastPosUpdate;
	CVec		vLastEstimatedVel;
	CVec		vPreLastEstimatedVel;
	int			iLastCharge;
	int			iLastCurWeapon;

	
	AbsTime		fSpawnTime;

	// Graphics
	CWormSkin	cSkin;
	SmartPointer<SDL_Surface> bmpGibs;
	DeprecatedGUI::CBar		cHealthBar;


	// Arsenal
	bool		bWeaponsReady;
	bool		bIsPrepared;
	bool		bGameReady;
	CGameScript	*cGameScript;
    CWpnRest    *cWeaponRest;
	int			iNumWeaponSlots;
	int			iCurrentWeapon;
	wpnslot_t	tWeapons[MAX_WEAPONSLOTS];
	AFK_TYPE	iAFK;
	std::string	sAFKMessage;

    // Force the showing of the current weapon
    bool        bForceWeapon_Name;
    AbsTime       fForceWeapon_Time;

	CWormInputHandler* m_inputHandler;

public:
	// Used to print damage numbers over the worm head
	struct DamageReport {
		DamageReport(): damage(0), lastTime(AbsTime()) {};
		
		int damage;
		AbsTime lastTime;
	};

private:
	std::map< int, DamageReport> cDamageReport; // WormID can be -1, then this is projectile generated by gamemode or level script

public:
	// Methods


	//
	// General
	//
	void		Clear();
	void		Init();
	//void		CopyProfile(plyprofile_t *profile);
	void		Shutdown();


	// TODO: move this out here (to network engine)
	//
	// Network
	//
	void		writeInfo(CBytestream *bs);
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
	int			GetMyPing();

	
	void		setupLobby();
	void		loadDetails();
	void		saveDetails();



	// Weapon
	void		GetRandomWeapons();
	void		CloneWeaponsFrom(CWorm* w);


	//
	// Graphics
	//
	bool		ChangeGraphics(int generalgametype);
	void		FreeGraphics();
	SmartPointer<SDL_Surface> ChangeGraphics(const std::string& filename, bool team);
	void		Draw(SDL_Surface * bmpDest, CViewport *v);
    void        DrawShadow(SDL_Surface * bmpDest, CViewport *v);
	void		UpdateDrawPos();

	//
	// Game
	//
	bool		isPrepared() { return bIsPrepared; }
	void		Prepare(bool serverSide); // weapon selection and so on
	void		Unprepare(); // after a game
	void		StartGame();
	void		Spawn(CVec position);
	bool		Injure(int damage);
	bool		Kill();
	bool		CheckBonusCollision(CBonus *b);
	bool		GiveBonus(CBonus *b);
	void		Hide(int forworm, bool immediate);
	void		Show(int forworm, bool immediate);


	void		getInput();
	void		clearInput();
	void		initWeaponSelection();
	void		doWeaponSelectionFrame(SDL_Surface * bmpDest, CViewport *v);

	void		NewNet_SimulateWorm( NewNet::KeyState_t keys, NewNet::KeyState_t keysChanged ); // Synthetic input from new net engine - Ignores inputHandler

	//
	// Misc.
	//
	bool		CanType();

	bool		isHostWorm();
	bool		shouldDoOwnWeaponSelection();
	
	
    int         traceLine(CVec target, float *fDist, int *nType, int divs = 5);
	int			traceLine(CVec target, CVec start, int *nType, int divs = 5, uchar checkflag = PX_EMPTY);
	
	bool		IsEmpty(int Cell);
    bool		CheckOnGround();
	

	//
	// Variables
	//
	bool		isUsed()				{ return bUsed; }
	void		setUsed(bool _u);

	CNinjaRope*	getNinjaRope()				{ return &cNinjaRope; }

	std::string getName()			{ return sName; }
	void		setName(const std::string& val) { sName = val; }
	Color		getGameColour();
	void		setColour(Uint32 c)			{ cSkin.Colorize(c); }
	void		setColour(Uint8 r, Uint8 g, Uint8 b) {cSkin.Colorize(MakeColour(r,g,b)); }

	void		setLocal(bool _l)			{ bLocal = _l; }
	bool		getLocal()				{ return bLocal; }

	void		setSpawnedOnce()			{ bSpawnedOnce = true; }
	bool		haveSpawnedOnce()			{ return bSpawnedOnce; }
	
	int			getHealth()				{ return iHealth; }
	void		setHealth(int _h)			{ iHealth = CLAMP(_h, 0, 100); }
	int			getLives()				{ return iLives; }
	int			getKills()				{ return iKills; }
	void		setLives(int l)				{ iLives = l; }
	int			getDamage()				{ return iDamage; }
	void		setDamage(int l)			{ iDamage = l; }
	void		addDamage(int damage, CWorm* victim, const GameOptions::GameInfo & settings);

	void		AddKill()				{ iKills++; }
    void        setKills(int k)             { iKills = k; }

	void		setID(int i)				{ iID = i; }
	int			getID()	const			{ return iID; }

	WormType*	getType()				{ return m_type; }
    void        setType(WormType* t)        { m_type = t; }

	bool		getAlive()				{ return bAlive; }
	void		setAlive(bool _a)			{ bAlive = _a; }

	AbsTime		getTimeofDeath()		{ return fTimeofDeath; }

	void		setHooked(bool h, CWorm *w)	{ bHooked=h; pcHookWorm=w; }
	void		setClient(CServerConnection *cl)		{ cOwner = cl; }
    CServerConnection     *getClient()            { return cOwner; }

	CVec		getFollowPos()			{ return (bFollowOverride?vFollowPos:vPos); }
	void		resetFollow()			{ bFollowOverride = false; }
	void		doFollow(int x, int y)		{ bFollowOverride = true; vFollowPos.x = (float)x; vFollowPos.y = (float)y; }

	CVec		getPos()				{ return vPos; }
	void		setPos(CVec v)				{ vPos = v; }

	CVec&		pos()						{ return vPos; }
	bool		isOnGround()				{ return bOnGround; }
	void		setOnGround(bool g)			{ bOnGround = g; }

	CVec		*getVelocity()			{ return &vVelocity; }
	CVec&		velocity()					{ return vVelocity; }

	worm_state_t *getWormState()		{ return &tState; }

	bool		hasOwnServerTime();
	TimeDiff	serverTime()				{ return fServertime; }

	bool		isVisibleForWorm(int worm) const;
	void		setVisibleForWorm(int worm, bool visibility);
	bool		isVisibleForEverybody() const;
	bool		isVisible(const CViewport* v) const;
	bool		isVisible(CWorm* viewerWorm) const;
	
	float		getAngle()				{ return fAngle; }
	void		setAngle(float a)			{ fAngle = a; }
	int			getDirection()			{ return iDirection; }
	void		setDirection(int d)			{ iDirection = d; }
	int			getMoveDirection()		{ return iMoveDirection; }

	void		setLoadingTime(float l)		{ fLoadingTime = l; }
	float		getLoadingTime()			{ return fLoadingTime; }

	void		setCanUseNinja(bool b) { bCanUseNinja = b; }
	bool		canUseNinja() const { return bCanUseNinja; }
	void		setSpeedFactor(float f) { fSpeedFactor = f; }
	float		speedFactor() const { return fSpeedFactor; }
	void		setDamageFactor(float f) { fDamageFactor = f; }
	float		damageFactor() const { return fDamageFactor; }
	void		setCanAirJump(bool b) { bCanAirJump = b; }
	bool		canAirJump() const { return bCanAirJump; }
	void		setLastAirJumpTime(AbsTime t) { fLastAirJumpTime = t; }
	AbsTime		getLastAirJumpTime() { return fLastAirJumpTime; }
	
	CGameScript* getGameScript()			{ return cGameScript; }
	void		setGameScript(CGameScript *gs)	{ cGameScript = gs; }
    void        setWpnRest(CWpnRest *wr)    { cWeaponRest = wr; }

	void		setDrawMuzzle(bool _d)		{ bDrawMuzzle = _d; }

	bool		getWeaponsReady()		{ return bWeaponsReady; }
	void		setWeaponsReady(bool _w)	{ bWeaponsReady = _w; }
	wpnslot_t	*getCurWeapon()			{ return &tWeapons[MIN(4, iCurrentWeapon)]; }
	int			getCurrentWeapon()		{ return MIN(4, iCurrentWeapon); }
	void		setCurrentWeapon(int _w)	{ iCurrentWeapon = MIN(4,_w); }
	wpnslot_t	*getWeapon(int id)			{ return &tWeapons[id]; }

	void		setGameReady(bool _g)		{ bGameReady = _g; }
	bool		getGameReady()			{ return bGameReady; }

	void		setProfile(profile_t *p)	{ tProfile = p; }
	profile_t	*getProfile()				{ return tProfile; }

	void		setTeam(int _t)				{ iTeam = _t; if(getLobby()) getLobby()->iTeam = _t; }
	int			getTeam() const				{ return iTeam; }

	SmartPointer<SDL_Surface> getGibimg()			{ return bmpGibs; }
	SmartPointer<SDL_Surface> getPicimg()			{ return cSkin.getPreview(); }

	lobbyworm_t	*getLobby()				{ return &tLobbyState; }

	bool		getTagIT()				{ return bTagIT; }
	void		setTagIT(bool _t);

	AbsTime		getLastBlood()				{ return fLastBlood; }
	void		setLastBlood(const AbsTime& b)		{ fLastBlood = b; }
	EntityEffect * getSparklesEffect()		{ return &cSparkles; }

    void        incrementDirtCount(int d);
    int         getDirtCount()          { return iDirtCount; }

	void		setTarget(bool _t)			{ bGotTarget = _t; }

	TimeDiff	getTagTime()			{ return fTagTime; }
	void		setTagTime(const TimeDiff& _t)		{ fTagTime = _t; }
	void		incrementTagTime(const TimeDiff& dt)	{ fTagTime+=dt; }

	CWormSkin&	getSkin()				{ return cSkin; }
	void		setSkin(const CWormSkin& skin)	{ cSkin = skin; }
	void		setSkin(const std::string& skin)	{ cSkin.Change(skin); }

	bool		getAlreadyKilled()			{ return bAlreadyKilled; }
	void		setAlreadyKilled(bool _k)	{ bAlreadyKilled = _k; }

	bool		isShooting()				{ return tState.bShoot; }
	bool		isWeaponReloading()			{ return getCurWeapon()->Reloading; }

	bool		isSpectating()				{ return bSpectating; }
	void		setSpectating(bool _s)		{ bSpectating = _s; }

	AFK_TYPE	getAFK()				{ return iAFK; }
	const std::string & getAFKMessage()		{ return sAFKMessage; }
	void		setAFK(AFK_TYPE _f, const std::string & msg);

	void	addTotalWins(int _w = 1)		{ iTotalWins += _w; }
	int		getTotalWins()				{ return iTotalWins; }
	void	addTotalLosses(int _l = 1)		{ iTotalLosses += _l; }
	int		getTotalLosses()			{ return iTotalLosses; }
	void	addTotalKills(int _k = 1)		{ iTotalKills += _k; }
	int		getTotalKills()				{ return iTotalKills; }
	void	addTotalDeaths(int _d = 1)		{ iTotalDeaths += _d; }
	int		getTotalDeaths()			{ return iTotalDeaths; }
	void	addTotalSuicides(int _d = 1)	{ iTotalSuicides += _d; }
	int		getTotalSuicides()			{ return iTotalSuicides; }
	
	const Version & getClientVersion()				{ return cClientVersion; }
	void	setClientVersion(const Version & v)		{ cClientVersion = v; }


	float&		frame()						{ return fFrame; }
	
	CWormInputHandler* inputHandler() { return m_inputHandler; }
	void reinitInputHandler();
	
	std::map< int, DamageReport> & getDamageReport() { return cDamageReport; }

	void setCollisionTime(const AbsTime& _t)			{ fCollisionTime = _t; }
	AbsTime getCollisionTime() const			{ return fCollisionTime; }
	void setCollisionVel(CVec _v)			{ vCollisionVelocity = _v; }
	CVec getCollisionVel() const			{ return vCollisionVelocity; }
	void setCollidedLastFrame(bool _c)		{ bCollidedLastFrame = _c; }
	bool hasCollidedLastFrame() const		{ return bCollidedLastFrame; }
	
	void NewNet_SaveWormState(CWorm * w);
	void NewNet_RestoreWormState(CWorm * w);
	void NewNet_InitWormState( int seed );
	
	// HINT: saves the current time of the simulation
	// TODO: should be moved later to PhysicsEngine
	// but it's not possible in a clean way until we have no simulateWorms()
	// there which simulates all worms together
	AbsTime	fLastSimulationTime;

	NewNet::NetSyncedRandom NewNet_random;
};


int traceWormLine(CVec target, CVec start, CVec* collision = NULL);

struct WormJoinInfo {
	WormJoinInfo() : iTeam(0), m_type(NULL) {}
	void loadFromProfile(profile_t* p);	
	void readInfo(CBytestream *bs);
	static bool	skipInfo(CBytestream *bs)  { bs->SkipString(); bs->Skip(2); bs->SkipString(); return bs->Skip(3); }
	void applyTo(CWorm* worm) const;
	
	std::string sName;
	int iTeam;
	WormType* m_type;
	CWormSkin cSkin;
	
};

#endif  //  __CWORM_H__
