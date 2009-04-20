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
#include "types.h"
#include "Color.h"
#include "Consts.h"

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

class CProjectile {
	friend struct Proj_TimerEvent;
	friend struct Proj_DoActionInfo;
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


private:
	// Types
	struct ColInfo  {
		int left, right, top, bottom;
		bool collided;
		bool onlyDirt;
	};

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
	int			health;
	
	// Projectile trail
	AbsTime		fLastTrailProj;
    float       fTimeVarRandom;

	proj_t		*tProjInfo;

	CVec		vOldPos;
	CVec		vPosition;
	CVec		vVelocity;
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

	
	float		fGravity;  // gravity of the projectile (either custom or 100 by default)
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


public:
	// Methods


	void	Spawn(proj_t *_proj, CVec _pos, CVec _vel, int _rot, int _owner, int _random, AbsTime time, AbsTime ignoreWormCollBeforeTime);
	int		Collision(uchar pf);

    void	Draw(SDL_Surface * bmpDest, CViewport *view);
    void	DrawShadow(SDL_Surface * bmpDest, CViewport *view);

	int		CheckWormCollision(CWorm *worms);
	int		ProjWormColl(CVec pos, CWorm *worms);
	ColInfo	TerrainCollision(int px, int py);
	bool	MapBoundsCollision(int px, int py);
	bool	HandleCollision(const ColInfo& col, const CVec& oldpos, const CVec& oldvel, float dt);
		
	ProjCollisionType SimulateFrame(float dt, CMap *map, CWorm* worms, float* enddt); // returns collision mask
	static int	CheckCollision(proj_t* tProjInfo, float dt, CVec pos, CVec vel); // returns collision mask

	bool	CollisionWith(const CProjectile* prj) const;
	bool	CollisionWith(const CProjectile* prj, int rx, int ry) const;
	
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

	CVec	GetPosition()		{ return vPosition; }
	CVec	GetVelocity()		{ return vVelocity; }
	VectorD2<int> getRadius()	{ return radius; }
	proj_t	*GetProjInfo()		{ return tProjInfo; }
	int		GetOwner() const			{ return iOwner; }
	bool	hasOwner() const	{ return iOwner >= 0 && iOwner < MAX_WORMS; }

    float   getTimeVarRandom()  { return fTimeVarRandom; }

	float	getRandomFloat();
	int		getRandomIndex()	{ return iRandom; }

	void	setNewPosition( const CVec& newpos ) { vOldPos = vPosition = newpos; }
	void	setNewVel( const CVec& newvel ) { vVelocity = newvel; }

	void	injure(int damage) { health -= damage; }
	int		getHealth() const { return health; }
	
	void	updateCollMapInfo(const VectorD2<int>* oldPos = NULL, const VectorD2<int>* oldRadius = NULL);
	
	// HINT: saves the current time of the simulation
	// we need to save this also per projectile as they can have different
	// simulation times (different times of spawning or remote projectiles)
	AbsTime	fLastSimulationTime;

	Event<> onInvalidation;
};


#endif  //  __CPROJECTILE_H__
