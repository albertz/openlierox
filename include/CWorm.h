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

#include "LieroX.h" // for MAX_WORMS, _AI_DEBUG
#include "CProjectile.h"
#include "CGameScript.h"
#include "CNinjaRope.h"
#include "CWpnRest.h"
#include "Options.h" // for control_t
#include "Utils.h"
#include "Frame.h"
#include "CBar.h"

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


// Weapon slot structure
struct wpnslot_t {
	weapon_t	*Weapon;
	int			SlotNum;
	float		Charge;
	int			Reloading;
	float		LastFire;
	bool		Enabled;
};

struct randweapons_t {
	int Weap1, Weap2, Weap3, Weap4, Weap5;
};

// the files have to be included yourself later
// they are used here; but their headers depends on this header
// TODO: remove the usage of these in this header
class CClient;
class CBonus;


// Lobby worm details
#define		LBY_OPEN	0
#define		LBY_CLOSED	1
#define		LBY_USED	2

struct lobbyworm_t {
	int			iType;
    int         iTeam;
	int			iHost;
	int			iReady;
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


// Path finding node
class ai_node_t {
public:

    int     nX, nY;
    int     nCost;
    int     nFound;
    int     nCount;
    //int     *nOpenClose;

    ai_node_t    *psParent;
    ai_node_t    *psPath;
    ai_node_t    *psChildren[8];

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



class CWorm {
public:
	// Constructor
	CWorm() {
		Clear();
	}

	~CWorm() {
		Shutdown();
	}

private:
	// Attributes

	// General
	bool		bUsed;
	int			iID;
	int			iType;
	int			iLocal;
	int			iTeam;
	std::string	sName;
    std::string szSkin;
	Uint32		iColour;
	bool		ProfileGraphics;
	int			iRanking;
	int			iKillsInRow;

	// Client info
	int			iClientID;
	int			iClientWormID;

	// Input
	CInput		cUp, cDown, cLeft, cRight,
				cShoot, cJump, cSelWeapon, cInpRope;
	bool		bUsesMouse;

	// Simulation
	worm_state_t tState;
	CVec		vPos;
	CVec		vOldPos;
	CVec		vVelocity;
	int			iOnGround;

	CVec		vFollowPos;
	bool		bFollowOverride;
	
	// Score
	int			iKills;
	int			iDeaths;
	int			iSuicides;

	// Game
	float		fLoadingTime;
	int			iDrawMuzzle;
	int			iHealth;
	int			iLives;
	int			iAlive;
	float		fTimeofDeath;
	int			iDirection;
	int			iGotTarget;
	float		fAngle;
    float       fAngleSpeed;
	int			iCarving;
	float		fFrame;
	CNinjaRope	cNinjaRope;
	profile_t	*tProfile;
	float		fRopeTime;

	int			iHooked;
	CWorm		*pcHookWorm;

	int			iRopeDown;
	int			iRopeDownOnce;

	int			iTagIT;
	float		fTagTime;
	float		fLastSparkle;

    int         iDirtCount;

	float		fLastBlood;

	CMap*		pcMap;


	// Owner client
	CClient		*cClient;

	// Network
	float		fFrameTimes[NUM_FRAMES];
	lobbyworm_t tLobbyState;



	// Graphics
	SDL_Surface	*bmpWormRight;
	SDL_Surface	*bmpWormLeft;
	SDL_Surface	*bmpGibs;
	SDL_Surface	*bmpPic;
    SDL_Surface *bmpShadowPic;
	CBar		cHealthBar;
	//CViewport	*pcViewport;


	// Arsenal
	int			iWeaponsReady;
	int			iGameReady;
	CGameScript	*cGameScript;
    CWpnRest    *cWeaponRest;
	int			iNumWeaponSlots;
	int			iCurrentWeapon;
	wpnslot_t	tWeapons[MAX_WEAPONSLOTS];
    
    // Force the showing of the current weapon
    bool        bForceWeapon_Name;
    float       fForceWeapon_Time;




    /*
	    Artificial Intelligence
    */
    int         nAIState;
    int         nAITargetType;
    CWorm       *psAITarget;
    CBonus      *psBonusTarget;
    CVec        cPosTarget;
    int         nPathStart[2];
    int         nPathEnd[2];
    float       fLastCarve;
    CVec        cStuckPos;
    float       fStuckTime;
    bool        bStuck;
    float       fStuckPause;
    float       fLastThink;
    CVec        cNinjaShotPos;
	float		fLastFace;
	float		fSpawnTime;
	float		fBadAimTime;
    int			iAiGameType; // AI game type, we have predefined behaviour for mostly played settings
	int			iAiGame;
	int			iAiTeams;
	int			iAiTag;
	int			iAiDiffLevel;
	CProjectile *psHeadingProjectile;
	int			iRandomSpread;

	float		fLastShoot;
	float		fLastJump;
	float		fLastWeaponChange;
	float		fLastCreated;
	float		fLastCompleting;
	float		fLastRandomChange;

	float		fCanShootTime;

	float		fRopeAttachedTime;
	float		fRopeHookFallingTime;


    // Path Finding
    int         nGridCols, nGridRows;
    ai_node_t   *psPath;
    int         *pnOpenCloseGrid;
    ai_node_t   *psCurrentNode;
	float       fLastPathUpdate;
	bool		bPathFinished;
	float		fSearchStartTime;
	
	// its type is searchpath_base*; defined in CWorm_AI.cpp
	void*		pathSearcher;

	NEW_ai_node_t	*NEW_psPath;
	NEW_ai_node_t	*NEW_psCurrentNode;
	NEW_ai_node_t	*NEW_psLastNode;


public:
	// Methods


	//
	// General
	//
	void		Clear(void);
	void		Init(void);
	//void		CopyProfile(plyprofile_t *profile);
	void		Shutdown(void);
	

	//
	// Network
	//
	void		writeInfo(CBytestream *bs);
	void		readInfo(CBytestream *bs);
	static inline bool	skipInfo(CBytestream *bs)  { bs->SkipString(); bs->Skip(2); bs->SkipString(); return bs->Skip(3); }
	void		writeScore(CBytestream *bs);
	void		readScore(CBytestream *bs);
	static inline bool	skipScore(CBytestream *bs)  { return bs->Skip(3); }
	void		writePacket(CBytestream *bs);
	void		readPacket(CBytestream *bs, CWorm *worms);
	static bool	skipPacket(CBytestream *bs);
	void		readPacketState(CBytestream *bs, CWorm *worms);
	static inline bool	skipPacketState(CBytestream *bs)  {return skipPacket(bs); } // For skipping it's the same as skipPacket
	void		writeWeapons(CBytestream *bs);
	void		readWeapons(CBytestream *bs);
	static inline bool	skipWeapons(CBytestream *bs)  { return bs->Skip(5); } // 5 weapons 
	void		writeStatUpdate(CBytestream *bs);
	void		readStatUpdate(CBytestream *bs);
	static inline bool	skipStatUpdate(CBytestream *bs) { return bs->Skip(2); } // Current weapon and charge
	int			GetMyPing(void);

	void		setupLobby(void);


	//
	// Weapon & Input
	//
	void		SetupInputs(const controls_t& Inputs);
	void		InitWeaponSelection(void);
	void		GetRandomWeapons(void);
	void		SelectWeapons(SDL_Surface *bmpDest, CViewport *v);


	//
	// Graphics
	//
	int			LoadGraphics(int gametype);
	void		LoadProfileGraphics();
	void		DeactivateProfileGraphicsOnce() { ProfileGraphics = false; }
	void		FreeGraphics(void);
	SDL_Surface	*ChangeGraphics(const std::string& filename, int team);
	void		Draw(SDL_Surface *bmpDest, CViewport *v);
    void        DrawShadow(SDL_Surface *bmpDest, CViewport *v);


	//
	// Game
	//
	void		Prepare(CMap *pcMap);
	void		Spawn(CVec position);
	int			Injure(int damage);
	int			Kill(void);
	int			CheckBonusCollision(CBonus *b);
	int			GiveBonus(CBonus *b);
	


	//
	// Simulation
	//
	void		getInput(void);
	void		getMouseInput(void);
    void        clearInput(void);
	void		getGamepadInput(void);
	void		Simulate(CWorm *worms, int local, float dt);
	void		SimulateWeapon( float dt );
	bool		CheckWormCollision( float dt, CVec pos, CVec *vel, int jump );
    int			CheckOnGround();


	//
	// Misc.
	//
	int			CanType(void);



    //
    // AI
    //
    bool        AI_Initialize();
    void        AI_Shutdown(void);

	void		AI_GetInput(int gametype, int teamgame, int taggame);
	void		AI_Respawn();
    void        AI_Think(int gametype, int teamgame, int taggame);
    bool        AI_FindHealth();
    bool        AI_SetAim(CVec cPos);
    CVec        AI_GetTargetPos(void);
    
    void        AI_InitMoveToTarget();
    void        AI_MoveToTarget();
    void        AI_SimpleMove(bool bHaveTarget=true);
//    void        AI_PreciseMove();
    
    ai_node_t   *AI_ProcessNode(ai_node_t *psParent, int curX, int curY, int tarX, int tarY);
    void        AI_CleanupPath(ai_node_t *node);
    void		AI_splitUpNodes(NEW_ai_node_t* start, NEW_ai_node_t* end);
    void		AI_storeNodes(NEW_ai_node_t* start, NEW_ai_node_t* end);
    
    int         AI_FindClearingWeapon();
    bool        AI_Shoot();
    int         AI_GetBestWeapon(int nGameType, float fDistance, bool bDirect, float fTraceDist);
    void        AI_ReloadWeapons();
    int         cycleWeapons();
	void		AI_SetGameType(int type)  { iAiGameType = type; }
	int			AI_GetGameType()  { return iAiGameType; }

    void        AI_DEBUG_DrawPath(ai_node_t *node);

    CWorm       *findTarget(int gametype, int teamgame, int taggame);
    int         traceLine(CVec target, float *fDist, int *nType, int divs = 5);
	int			traceWeaponLine(CVec target, float *fDist, int *nType);
	bool		weaponCanHit(int gravity,float speed, CVec cTrgPos);
	bool		IsEmpty(int Cell);
    //void        moveToTarget(CWorm *pcTarget);

	CVec		NEW_AI_GetBestRopeSpot(CVec trg);
	CVec		NEW_AI_FindClosestFreeCell(CVec vPoint);
	bool		NEW_AI_CheckFreeCells(int Num);
	bool		NEW_AI_IsInAir(CVec pos, int area_a=3);
	CVec		NEW_AI_FindClosestFreeSpotDir(CVec vPoint, CVec vDirection, int Direction);
	CVec		NEW_AI_FindBestFreeSpot(CVec vPoint, CVec vStart, CVec vDirection, CVec vTarget, CVec* vEndPoint);
	int			NEW_AI_CreatePath(bool force_break = false);
	void		NEW_AI_MoveToTarget();
	void		NEW_AI_MoveToTargetDC();
	CVec		NEW_AI_GetNearestRopeSpot(CVec trg);
#ifdef _AI_DEBUG
	void		NEW_AI_DrawPath();
#endif



    //int         getBestWeapon(int nGameType, float fDistance, CVec cTarget);
    
    





	//
	// Variables
	//
	inline bool		isUsed(void)				{ return bUsed; }
	inline void		setUsed(bool _u)			{ bUsed = _u; }

	inline std::string getName(void)			{ return sName; }
	inline void		setName(const std::string& val) { sName = val; }
	inline Uint32	getColour(void)				{ return iColour; }
	inline void		setColour(Uint32 c)			{ iColour = c; }
	inline void		setColour(Uint8 r, Uint8 g, Uint8 b) { iColour = MakeColour(r,g,b); }

	inline void		setLocal(int _l)			{ iLocal = _l; }
	inline int			getLocal(void)				{ return iLocal; }

	inline int			getHealth(void)				{ return iHealth; }
	inline void			setHealth(int _h)			{ iHealth = MIN(100, MAX(0, _h)); }
	inline int			getLives(void)				{ return iLives; }
	inline int			getKills(void)				{ return iKills; }
	inline void		setLives(int l)				{ iLives = l; }

	inline void		AddKill(void)				{ iKills++; }
    inline void        setKills(int k)             { iKills = k; }

	inline void		setID(int i)				{ iID = i; }
	inline int			getID(void)					{ return iID; }

	inline int			getType(void)				{ return iType; }
    inline void        setType(int t)              { iType = t; }

	inline int			getAlive(void)				{ return iAlive; }
	inline void		setAlive(int _a)			{ iAlive = _a; }

	inline float		getTimeofDeath(void)		{ return fTimeofDeath; }

	inline void		setHooked(int h, CWorm *w)	{ iHooked=h; pcHookWorm=w; }
	inline void		setClient(CClient *cl)		{ cClient = cl; }
    inline CClient     *getClient(void)            { return cClient; }

	inline CInput		*getShoot(void)				{ return &cShoot; }

	inline CVec		getFollowPos(void)				{ return (bFollowOverride?vFollowPos:vPos); }
	inline void		resetFollow(void)				{ bFollowOverride = false; }
	inline void		doFollow(int x, int y)				{ bFollowOverride = true; vFollowPos.x = (float)x; vFollowPos.y = (float)y; }
	
	inline CVec		getPos(void)				{ return vPos; }
	inline void		setPos(CVec v)				{ vPos = v; }

	inline CVec		*getVelocity(void)			{ return &vVelocity; }

	inline worm_state_t *getWormState(void)		{ return &tState; }

	inline float		getAngle(void)				{ return fAngle; }
	inline int			getDirection(void)			{ return iDirection; }

	inline void		setLoadingTime(float l)		{ fLoadingTime = l; }

	inline void		setGameScript(CGameScript *gs)	{ cGameScript = gs; }
    inline void        setWpnRest(CWpnRest *wr)    { cWeaponRest = wr; }

	inline void		setDrawMuzzle(int _d)		{ iDrawMuzzle = _d; }

	inline int			getWeaponsReady(void)		{ return iWeaponsReady; }
	inline void		setWeaponsReady(int _w)		{ iWeaponsReady = _w; }
	inline wpnslot_t	*getCurWeapon(void)			{ return &tWeapons[MIN(4,iCurrentWeapon)]; }
	inline int			getCurrentWeapon(void)		{ return MIN(4,iCurrentWeapon); }
	inline void		setCurrentWeapon(int _w)	{ iCurrentWeapon = MIN(4,_w); }
	inline wpnslot_t	*getWeapon(int id)			{ return &tWeapons[id]; }

	inline void		setGameReady(int _g)		{ iGameReady = _g; }
	inline int			getGameReady(void)			{ return iGameReady; }

	inline void		setProfile(profile_t *p)	{ tProfile = p; }

	inline void		setTeam(int _t)				{ iTeam = _t; }
	inline int			getTeam(void)				{ return iTeam; }

	inline SDL_Surface	*getGibimg(void)			{ return bmpGibs; }
	inline SDL_Surface	*getPicimg(void)			{ return bmpPic; }

	inline lobbyworm_t	*getLobby(void)				{ return &tLobbyState; }

	inline int			getTagIT(void)				{ return iTagIT; }
	inline void		setTagIT(int _t)			{ iTagIT = _t; }

    inline void        incrementDirtCount(int d)   { iDirtCount += d; }
    inline int         getDirtCount(void)          { return iDirtCount; }

	inline void		setTarget(int _t)			{ iGotTarget = _t; }

	inline float		getTagTime(void)			{ return fTagTime; }
	inline void		setTagTime(float _t)		{ fTagTime = _t; }
	inline void		incrementTagTime(float dt)	{ fTagTime+=dt; }

	inline std::string getSkin(void)				{ return szSkin; }
	inline void		setSkin(const std::string& skin)	{ szSkin = skin; }

	inline void		setKillsInRow(int _k)		{ iKillsInRow = 0; }
	inline int		getKillsInRow(void)			{ return iKillsInRow; }
	inline void		addKillInRow(void)			{ iKillsInRow++; }

	inline void		setHeading(CProjectile *_p) { psHeadingProjectile = _p; }
	inline CProjectile *getHeading(void)			{ return psHeadingProjectile; }


};


int traceWormLine(CVec target, CVec start, CMap *pcMap, CVec* collision = NULL);


#endif  //  __CWORM_H__
