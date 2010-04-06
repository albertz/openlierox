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
#include "DynDraw.h"
#include "CGameObject.h"

// TODO: remove this after we changed network
#include "CBytestream.h"

// Gusanos related includes
#include "gusanos/luaapi/types.h"
#include "gusanos/netstream.h"
#include "util/vec.h"
#include "util/angle.h"
#include <vector>


class CWormInputHandler;
class NinjaRope;
class Weapon;
class WeaponType;
struct LuaEventDef;
class NetWormInterceptor;
#ifndef DEDICATED_ONLY
class SpriteSet;
class BaseAnimator;
class CViewport;
#endif


#ifdef _MSC_VER
// Warning: this used in member initializer list
#pragma warning(disable:4355)
#endif

enum { MAX_WEAPONSLOTS = 10 };

// Direction
enum DIR_TYPE
{
	DIR_LEFT = 0, 
	DIR_RIGHT = 1 
};

inline DIR_TYPE OppositeDir(DIR_TYPE d) {
	switch(d) {
		case DIR_LEFT: return DIR_RIGHT;
		case DIR_RIGHT: return DIR_LEFT;
	}
	return DIR_LEFT;
}

enum WRM_LIVES
{
	WRM_OUT = -1,
	WRM_UNLIM = -2 
};

// AI Game Type
enum GAM_AI_TYPE
{
	GAM_RIFLES	= 0,
	GAM_100LT	= 1,
	GAM_MORTARS	= 2,
	GAM_OTHER	= 3
};

struct WormType;
class CWpnRest;
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


//    Artificial Intelligence

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

class CWormInputHandler;
struct WormJoinInfo;
class Client;
class Game;

// TODO: split into classes: one for CClient and one for CServerConnection (latter only containing some general information, more like a simple struct)
class CWorm: public CGameObject {
	friend class CWormInputHandler;
	friend class CWormBotInputHandler; // TODO: remove
	friend class CWormHumanInputHandler; // TODO: remove
	friend struct WormJoinInfo;
	friend class Client;
	friend class Game;
	friend class PhysicsLX56;
public:
	CWorm();
	~CWorm();

private:
	// disallow these!
	CWorm(const CWorm&): cSparkles(this) { assert(false); }
	CWorm& operator=(const CWorm&) { assert(false); return *this; }
	
protected:
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

	CVec		vLastPos;
	CVec		vDrawPos;
	bool		bOnGround;
	AbsTime		fLastInputTime;
	// last time we moved left or right
	AbsTime		lastMoveTime;

	
	TimeDiff	fServertime; // only for CServerConnection: when a wormupdate arrives, the servertime of client (>=beta8)
	
	CVec		vFollowPos;
	bool		bFollowOverride;

    AbsTime		fLastCarve;
	

	
	// Score
	int			iKills;
	int			iDeaths;
	int			iSuicides;
	int			iTeamkills;
	float		fDamage;

	int			iTotalWins;
	int			iTotalLosses;
	int			iTotalKills;
	int			iTotalDeaths;
	int			iTotalSuicides;
	Version		cClientVersion;

	// Game
	bool		bDrawMuzzle;
	int			iLives;
	bool		bAlive;
	AbsTime		fTimeofDeath;
	DIR_TYPE	iFaceDirectionSide;
	DIR_TYPE	iMoveDirectionSide;
	bool		bGotTarget;
	float		fAngle;
    float       fAngleSpeed;
    float		fMoveSpeedX;
	float		fSpeedFactor;
	bool		bCanUseNinja;
	float		fDamageFactor;
	float		fShieldFactor;
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
	bool		bLobbyReady; // Lobby Ready state

	// Graphics
	CWormSkin	cSkin;
	struct SkinDynDrawer; SkinDynDrawer* skinPreviewDrawerP;
	SmartPointer<DynDrawIntf> skinPreviewDrawer;
	SmartPointer<SDL_Surface> bmpGibs;
	DeprecatedGUI::CBar		cHealthBar;


	// Arsenal
	bool		bWeaponsReady;
	bool		bIsPrepared;
	bool		bGameReady;
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
		float damage;
		AbsTime lastTime;
		DamageReport(): damage(0.0f), lastTime(AbsTime()) {}
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

	IVec size() { return IVec(4,4); }
	Color renderColorAt(/* relative coordinates */ int x, int y);
	
	//
	// Game
	//
	bool		isPrepared() { return bIsPrepared; }
	void		Prepare(bool serverSide); // weapon selection and so on
	void		Unprepare(); // after a game
	void		StartGame();
	void		Spawn(CVec position);
	void		Kill(bool serverside);
	bool		CheckBonusCollision(CBonus *b);
	bool		GiveBonus(CBonus *b);
	void		Hide(int forworm, bool immediate);
	void		Show(int forworm, bool immediate);


	void		getInput();
	void		clearInput();
	void		initWeaponSelection();
	void		doWeaponSelectionFrame(SDL_Surface * bmpDest, CViewport *v);

	void		setAiDiff(int aiDif);
	
	NewNet::KeyState_t NewNet_GetKeys();
	void		NewNet_SimulateWorm( NewNet::KeyState_t keys, NewNet::KeyState_t keysChanged );

	//
	// Misc.
	//
	bool		CanType();

	bool		isLocalHostWorm();
	bool		isFirstLocalHostWorm();
	bool		shouldDoOwnWeaponSelection();
	
	
    int         traceLine(CVec target, float *fDist, int *nType, int divs = 5);
	int			traceLine(CVec target, CVec start, int *nType, int divs = 5, uchar checkflag = PX_EMPTY);
	
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
	void		setColour(Color c)			{ cSkin.Colorize(c); }
	void		setColour(Uint8 r, Uint8 g, Uint8 b) { cSkin.Colorize(Color(r,g,b)); }

	void		setLocal(bool _l)			{ bLocal = _l; }
	bool		getLocal()				{ return bLocal; }

	void		setSpawnedOnce()			{ bSpawnedOnce = true; }
	bool		haveSpawnedOnce()			{ return bSpawnedOnce; }
	
	int			getLives()				{ return iLives; }
	void		setLives(int l)				{ iLives = l; }

	float		getDamage()				{ return fDamage; }
	void		setDamage(float l)		{ fDamage = l; }
	void		addDamage(float damage, CWorm* victim, bool serverside);

	int			getKills() const		{ return iKills; }
    void        setKills(int k)			{ iKills = k; }
    void        addKill()				{ iKills++; }

	int			getScore() const;		// Not same as getKills, takes into account suicides and deaths

	int			getDeaths() const		{ return iDeaths; }
	void		setDeaths(int d)		{ iDeaths = d; }
	void		addDeath();

	int			getSuicides() const		{ return iSuicides; }
	void		setSuicides(int d)		{ iSuicides = d; }
	void		addSuicide();

	int			getTeamkills() const	{ return iTeamkills; }
	void		setTeamkills(int d)		{ iTeamkills = d; }
	void		addTeamkill();

	void		setID(int i)				{ iID = i; }
	int			getID()	const			{ return iID; }

	WormType*	getType()				{ return m_type; }
    void        setType(WormType* t)        { m_type = t; }

	bool		getAlive()				{ return bAlive; }

	AbsTime		getTimeofDeath()		{ return fTimeofDeath; }

	void		setHooked(bool h, CWorm *w)	{ bHooked=h; pcHookWorm=w; }
	CWorm		*getHookedWorm()			{ return pcHookWorm; }
	void		setClient(CServerConnection *cl)		{ cOwner = cl; }
    CServerConnection     *getClient()            { return cOwner; }

	CVec		getFollowPos()			{ return (bFollowOverride?vFollowPos:vPos); }
	void		resetFollow()			{ bFollowOverride = false; }
	void		doFollow(int x, int y)		{ bFollowOverride = true; vFollowPos.x = (float)x; vFollowPos.y = (float)y; }

	bool		isOnGround()				{ return bOnGround; }
	void		setOnGround(bool g)			{ bOnGround = g; }

	worm_state_t *getWormState()		{ return &tState; }

	bool		hasOwnServerTime();
	TimeDiff	serverTime()				{ return fServertime; }

	bool		isVisibleForWorm(int worm) const;
	void		setVisibleForWorm(int worm, bool visibility);
	bool		isVisibleForEverybody() const;
	bool		isVisible(const CViewport* v) const;
	bool		isVisible(CWorm* viewerWorm) const;
	
	float		getAngle()	const			{ return fAngle; }
	void		setAngle(float a)			{ fAngle = a; }
	void		resetAngleAndDir();
	DIR_TYPE	getFaceDirectionSide() const		{ return iFaceDirectionSide; }
	void		setFaceDirectionSide(DIR_TYPE d)	{ iFaceDirectionSide = d; }
	CVec		getFaceDirection() const {
		return CVec(cosf(getAngle() * ((float)PI/180)) * ((iFaceDirectionSide == DIR_LEFT) ? -1.0f : 1.0f),
					sinf(getAngle() * ((float)PI/180)) ); }
	DIR_TYPE	getMoveDirectionSide()				{ return iMoveDirectionSide; }
	CVec		getMoveDirection() const {
		return CVec(cosf(getAngle() * ((float)PI/180)) * ((iMoveDirectionSide == DIR_LEFT) ? -1.0f : 1.0f),
					sinf(getAngle() * ((float)PI/180)) ); }

	void		setCanUseNinja(bool b) { bCanUseNinja = b; }
	bool		canUseNinja() const { return bCanUseNinja; }
	void		setSpeedFactor(float f) { fSpeedFactor = f; }
	float		speedFactor() const { return fSpeedFactor; }
	void		setDamageFactor(float f) { fDamageFactor = f; }
	float		damageFactor() const { return fDamageFactor; }
	void		setShieldFactor(float f) { fShieldFactor = f; }
	float		shieldFactor() const { return fShieldFactor; } 
	void		setCanAirJump(bool b) { bCanAirJump = b; }
	bool		canAirJump() const { return bCanAirJump; }
	void		setLastAirJumpTime(AbsTime t) { fLastAirJumpTime = t; }
	AbsTime		getLastAirJumpTime() { return fLastAirJumpTime; }
	
	void		setDrawMuzzle(bool _d)		{ bDrawMuzzle = _d; }

	bool		getWeaponsReady()		{ return bWeaponsReady; }
	void		setWeaponsReady(bool _w)	{ bWeaponsReady = _w; }
	wpnslot_t	*getCurWeapon()			{ return &tWeapons[MIN(4, iCurrentWeapon)]; }
	int			getCurrentWeapon()		{ return MIN(4, iCurrentWeapon); }
	void		setCurrentWeapon(int _w)	{ iCurrentWeapon = MIN(4,_w); }
	wpnslot_t	*getWeapon(int id)			{ return &tWeapons[id]; }

	void		setGameReady(bool _g)		{ bGameReady = _g; }
	bool		getGameReady()			{ return bGameReady; }

	void		setLobbyReady(bool _g)		{ bLobbyReady = _g; }
	bool		getLobbyReady() const		{ return bLobbyReady; }

	void		setProfile(profile_t *p)	{ tProfile = p; }
	profile_t	*getProfile()				{ return tProfile; }

	void		setTeam(int _t)				{ iTeam = _t; }
	int			getTeam() const				{ return iTeam; }

	SmartPointer<SDL_Surface> getGibimg()			{ return bmpGibs; }
	SmartPointer<DynDrawIntf> getPicimg()			{ return skinPreviewDrawer; }

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
	
	void NewNet_CopyWormState(const CWorm & w);
	void NewNet_InitWormState( int seed );
	
	// HINT: saves the current time of the simulation
	// TODO: should be moved later to PhysicsEngine
	// but it's not possible in a clean way until we have no simulateWorms()
	// there which simulates all worms together
	AbsTime	fLastSimulationTime;

	NewNet::NetSyncedRandom NewNet_random;
	
	
	// --------------------------------------------------
	// --------------------- Gusanos --------------------
	
	
public:
	
	enum Actions
	{
		MOVELEFT,
		MOVERIGHT,
		//AIMUP,
		//AIMDOWN,
		FIRE,
		FIRE2,
		JUMP,
		DIG,
		NINJAROPE,
		CHANGEWEAPON,
		RESPAWN
	};
	
	enum Direction
	{
		Down = 0,
		Left,
		Up,
		Right,
		
		DirMax
	};
	
	static LuaReference metaTable;
	//static int const luaID = 2;
	
	void gusInit();
	void gusShutdown();
	void deleteThis();
		
	void draw(CViewport* viewport);
	
	void calculateReactionForce(BaseVec<long> origin, Direction dir);
	void calculateAllReactionForces(BaseVec<float>& nextPos, BaseVec<long>& inextPos);
	void processMoveAndDig(void);
	void processPhysics();
	void processJumpingAndNinjaropeControls();
	
	virtual void think();
	void actionStart( Actions action );
	void actionStop( Actions action );
	void addAimSpeed(AngleDiff speed);
	void addRopeLength(float distance);
	
	Vec getWeaponPos();
#ifndef DEDICATED_ONLY
	Vec getRenderPos();
#endif
	
	bool isChanging()
	{
		return changing;
	}
	
	virtual void damage( float amount, CWormInputHandler* damager );
	
	// This are virtual so that NetWorm can know about them and tell others over the network.
	virtual void respawn();
	void respawn(const Vec& newPos);
	
	virtual void dig();
	void dig(const Vec& digPos, Angle angle);
	
	virtual void die();
	void base_die();
	virtual void changeWeaponTo( unsigned int weapIndex );
	
	virtual void setWeapon(size_t index, WeaponType* type );
	void base_setWeapon(size_t index, WeaponType* type );
	virtual void setWeapons( std::vector<WeaponType*> const& weaps);
	virtual void clearWeapons();
	void base_clearWeapons();
	
	Weapon* getCurrentWeaponRef();
	
	// getWeaponIndexOffset can be used to get the currentWeapon index or
	//to get the one to the right or the left or the one 1000 units to the
	//right ( it will wrap the value so that its always inside the worm's weapons size )
	int getWeaponIndexOffset( int offset );
	Angle getPointingAngle();
	void setDir(int d); // Only use this if you are going to sync it over netplay with an event
	int getDir()
	{
		return m_dir;
	}
	bool isCollidingWith( const Vec& point, float radius );
	bool isActive();
	void removeRefsToPlayer( CWormInputHandler* player );
	
#ifndef DEDICATED_ONLY
	void showFirecone( SpriteSet* sprite, int frames, float distance );
#endif
	
	NinjaRope* getNinjaRopeObj();
	
	AngleDiff aimSpeed; // Useless to add setters and getters for this
	Angle aimAngle;
	
	void sendWeaponMessage( int index, BitStream* data, Net_U8 repRules = Net_REPRULE_AUTH_2_ALL );
	
	virtual void makeReference();
	virtual void finalize();
	
	void sendLuaEvent(LuaEventDef* event, eNet_SendMode mode, Net_U8 rules, BitStream* userdata, Net_ConnID connID);
		
protected:
	//LuaReference luaReference;
	
#ifndef DEDICATED_ONLY
	
	Vec renderPos;
#endif
	
	int reacts[DirMax];
	
	float aimRecoilSpeed;
	
#ifndef DEDICATED_ONLY
	
	int m_fireconeTime;
	float m_fireconeDistance;
#endif
	
	int m_timeSinceDeath; // Used for the min and max respawn time sv variables
	
	size_t currentWeapon;
	
	std::vector<Weapon*> m_weapons;
	int m_weaponCount;
	
	CWormInputHandler* m_lastHurt;
	NinjaRope* m_ninjaRope;
	
#ifndef DEDICATED_ONLY
	
	SpriteSet *skin;
	SpriteSet *skinMask;
	SpriteSet *m_currentFirecone;
	BaseAnimator* m_fireconeAnimator;
	
	BaseAnimator* m_animator;
#endif
	// Smaller vars last to improve alignment and/or decrease structure size
	//bool m_isActive; -> alive
	bool movingLeft;
	bool movingRight;
	bool jumping;
	bool animate;
	bool movable; // What do we need this for? // Dunno, did I put this here? :o
	bool changing; // This shouldnt be in the worm class ( its player stuff >:O )
	int m_dir;
	
	
	// ----------------------------------
	// ---------- Gusanos NetWorm -----------------
	// -------------------------------------
	
public:
	friend class NetWormInterceptor;
	
	enum NetEvents
	{
		PosCorrection = 0,
		Respawn,
		Dig,
		Die,
		ChangeWeapon,
		WeaponMessage,
		SetWeapon,
		ClearWeapons,
		SYNC,
		LuaEvent,
		EVENT_COUNT,
	};
	
	enum ReplicationItems
	{
		PlayerID = 0,
		Position,
		AIM
	};
	
	static Net_ClassID  classID;
	static const float MAX_ERROR_RADIUS;
	
	void NetWorm_Init(bool isAuthority);
	void NetWorm_Shutdown();
	
	void NetWorm_think();
	void correctOwnerPosition();
	
	void sendSyncMessage( Net_ConnID id );
	
	eNet_NodeRole getRole()
	{
		if ( m_node )
		{
			return m_node->getRole();
		}else
			return eNet_RoleUndefined;
	}
		
	Net_NodeID getNodeID();
	Net_Node* getNode() { return m_node; }
	
	Vec lastPosUpdate;
	int timeSinceLastUpdate;
	
	bool gusSkinVisble;
	
private:
	
	void addEvent(BitStream* data, NetEvents event);
	
	bool m_isAuthority;
	Net_Node *m_node;
	NetWormInterceptor* m_interceptor;
	
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
	std::string skinFilename;
	Color skinColor;
};

#endif  //  __CWORM_H__
