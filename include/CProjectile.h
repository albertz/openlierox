/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Projectile class
// Created 26/6/02
// Jason Boettcher


#ifndef __CPROJECTILE_H__
#define __CPROJECTILE_H__

#include <string>
#include <vector>
#include <map>
#include "Event.h"
#include "olx-types.h"
#include "Color.h"
#include "Consts.h"
#include "CGameObject.h"

struct SDL_Surface;
class CWorm;
class Sounds;
class CViewport;
struct proj_t;
struct Proj_DoActionInfo;

#define		MAX_PROJECTILES	3000

#define		COL_TOP		    0x01
#define		COL_RIGHT	    0x02
#define		COL_BOTTOM	    0x04
#define		COL_LEFT	    0x08


// Projectile collisions
#define     PJC_NONE        0x00
#define     PJC_TERRAIN     0x01
#define     PJC_WORM        0x02
#define		PJC_MAPBORDER	0x04
#define     PJC_DIRT     	0x08



struct ProjCollisionType {
	bool withWorm : 1;
	union {
		unsigned int wormId : 7;
		unsigned int colMask : 7;
	};
	static ProjCollisionType Worm(int wormid) { ProjCollisionType c; c.withWorm = true; c.wormId = wormid; return c; }
	static ProjCollisionType Terrain(int colmask) { ProjCollisionType c; c.withWorm = false; c.colMask = colmask; return c; }
	static ProjCollisionType NoCol() { return Terrain(0); }
	operator bool() { return withWorm || colMask != 0; }
};

struct Proj_TimerEvent;
struct ProjTimerState {
	ProjTimerState() : last(0), c(0) {}
	float last;
	Uint32 c;
};

typedef std::map<const Proj_TimerEvent*, ProjTimerState> ProjTimerInfo; // saves CProj->fLife of last event hit

class CProjectile: public CGameObject {
	friend struct Proj_TimerEvent;
	friend struct Proj_DoActionInfo;
	friend struct Proj_Action;
	friend ProjCollisionType LX56Projectile_checkCollAndMove(CProjectile* const prj, TimeDiff dt, CMap *map);
	friend ProjCollisionType LX56Projectile_checkCollAndMove_Frame(CProjectile* const prj, TimeDiff dt, CMap *map);
	friend ProjCollisionType FinalWormCollisionCheck(CProjectile* proj, const CVec& vFrameOldPos, const CVec& vFrameOldVel, TimeDiff dt, ProjCollisionType curResult);
	friend void Projectile_HandleAttractiveForceForProjectiles(CProjectile* const prj, TimeDiff dt);
public:
	// Constructor
	CProjectile() {
		bUsed = false;
		fSpeed = 0;
		fLife = 0;
		iOwner = -1;
		fSpawnTime = AbsTime(0);
		tProjInfo = NULL;
		fLastTrailProj = AbsTime(0);
		iRandom = 0;
        fRotation = 0;
		health = 0;
	}

	// Types
	struct ColInfo  {
		int left, right, top, bottom;
		bool collided;
		bool onlyDirt;
	};
	
private:
	// Attributes

	bool		bUsed;
	int			iType;
	AbsTime		fSpawnTime;
	float		fLife;
	float		fExtra;
	int			iOwner;
	float		fSpeed;
	Color		iColour;
	AbsTime		fIgnoreWormCollBeforeTime;
	ProjTimerInfo timerInfo;
	
	// Projectile trail
	AbsTime		fLastTrailProj;
    float       fTimeVarRandom;

	proj_t		*tProjInfo;

	CVec		vOldPos;
	float		fRotation;
	VectorD2<int> radius;

	// Network
	int			iRandom;

	// Collision checking
	int			MAX_CHECKSTEP; // only after a step of this a collision check will be made
	int			MIN_CHECKSTEP; // if step is wider than this, it will be intersected
	int			MAX_CHECKSTEP2; // power of max checkstep
	int			MIN_CHECKSTEP2; // power of min checkstep
	int			AVG_CHECKSTEP; // this is used for the intersection, if the step is to wide

	
	float		fWallshootTime;  // period of time from the spawn when no collision checks are made
	bool		bChangesSpeed;  // true if the projectile changes its speed during its life
	int			iCheckSpeedLen;  // speed for which the check steps have been calculated last time
	int			CollisionSide;


	// Animation
	bool		bFrameDelta;
	float		fFrame;
    int         iFrameX;

	// Debug info
	bool		firstbounce;

private:
	void	CalculateCheckSteps();
	
	
	// used in LX56 physics
	int ProjWormColl(CVec pos);
	bool MapBoundsCollision(int px, int py);
	ColInfo TerrainCollision(int px, int py);
	bool HandleCollision(const CProjectile::ColInfo &c, const CVec& oldpos, const CVec& oldvel, TimeDiff dt);
	
public:
	// Methods


	IVec	size() { return IVec(radius.x,radius.y); }
	Color renderColorAt(/* relative coordinates */ int x, int y) { return iColour; }
	
	void	Spawn(proj_t *_proj, CVec _pos, CVec _vel, int _rot, int _owner, int _random, AbsTime time, AbsTime ignoreWormCollBeforeTime);

    void	Draw(SDL_Surface * bmpDest, CViewport *view);
    void	DrawShadow(SDL_Surface * bmpDest, CViewport *view);

	// Note: This is only used in AI and not in physics and it also should not be used in physics.
	static int	CheckCollision(proj_t* tProjInfo, float dt, CVec pos, CVec vel); // returns collision mask
	
	void	Bounce(float fCoeff);

	bool	isUsed() const		{ return bUsed; }
	void	setUnused();

	float	getLife()			{ return fLife; }
	float&	life()					{ return fLife; }

	float&	extra()					{ return fExtra; }
	proj_t* getProjInfo() const 	{ return tProjInfo; }
	float&	rotation()				{ return fRotation; }
	void	setFrameDelta(bool d)	{ bFrameDelta = d; }
	bool	getFrameDelta()			{ return bFrameDelta; }
	float&	frame()					{ return fFrame; }
	AbsTime&	lastTrailProj()			{ return fLastTrailProj; }
	AbsTime	getIgnoreWormCollBeforeTime()	{ return fIgnoreWormCollBeforeTime; }

	VectorD2<int> getRadius() const	{ return radius; }
	proj_t	*GetProjInfo()		{ return tProjInfo; }
	int		GetOwner() const			{ return iOwner; }
	bool	hasOwner() const	{ return iOwner >= 0 && iOwner < MAX_WORMS; }

    float   getTimeVarRandom()  { return fTimeVarRandom; }

	float	getRandomFloat();
	int		getRandomIndex()	{ return iRandom; }
	
	void	updateCollMapInfo(const VectorD2<int>* oldPos = NULL, const VectorD2<int>* oldRadius = NULL);
	
	// HINT: saves the current time of the simulation
	// we need to save this also per projectile as they can have different
	// simulation times (different times of spawning or remote projectiles)
	AbsTime	fLastSimulationTime;
	
	Event<> onInvalidation;
};


#endif  //  __CPROJECTILE_H__
