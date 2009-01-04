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
#include "Event.h"

struct SDL_Surface;
class CWorm;
class Sounds;
class CViewport;


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


// Projectile structure
class proj_t { public:
	std::string	filename;		// Compiler use (was 64b before)
	int		id;					// File ref use

	int		Type;
	int		Trail;
	int		NumColours;
	int		Colour1[3];
	int		Colour2[3];
	std::string	ImgFilename; // (was 64b before)
	int		Rotating;
	int		RotIncrement;
	int		RotSpeed;
	int		UseAngle;
	int		UseSpecAngle;
	int		AngleImages;
	int		UseCustomGravity;
	int		Gravity;
	int		Animating;
	float	AnimRate;
	int		AnimType;
	float   Dampening;

	// Timer (When the timer is finished)
	int		Timer_Type;
	float	Timer_Time;
	float	Timer_TimeVar;
	int		Timer_Damage;
	int		Timer_Projectiles;
	int		Timer_Shake;

	// Hit (When hitting the terrain)
	int		Hit_Type;
	int		Hit_Damage;
	int		Hit_Projectiles;
	int		Hit_UseSound;
	int		Hit_Shake;
	std::string	Hit_SndFilename; // (was 64b before)
	float	Hit_BounceCoeff;
	int		Hit_BounceExplode;

    // OnExplode (When something near has exploded)
    int		Exp_Type;
	int		Exp_Damage;
	int		Exp_Projectiles;
	int		Exp_UseSound;
    std::string	Exp_SndFilename; // (was 64b before)
	int		Exp_Shake;


    // Touch (When another projectile has touched this projectile)
    int		Tch_Type;
	int		Tch_Damage;
	int		Tch_Projectiles;
	int		Tch_UseSound;
    std::string	Tch_SndFilename; // (was 64b before)
	int		Tch_Shake;


	// Player hit
	int		PlyHit_Type;
	int		PlyHit_Damage;
	int		PlyHit_Projectiles;
	float	PlyHit_BounceCoeff;

	int		ProjUseangle;
	int		ProjAngle;
	int		ProjSpeed;
	float	ProjSpeedVar;
	float	ProjSpread;
	int		ProjAmount;
	proj_t	*Projectile;


	// Projectile trail
	int		PrjTrl_UsePrjVelocity;
	float	PrjTrl_Delay;
	int		PrjTrl_Amount;
	int		PrjTrl_Speed;
	float	PrjTrl_SpeedVar;
	float	PrjTrl_Spread;
	proj_t	*PrjTrl_Proj;


	SDL_Surface * bmpImage;	// Read-only var, managed by game script, no need in smartpointer
	SoundSample * smpSample;
};

class CProjectile {
public:
	// Constructor
	CProjectile() {
		bUsed = false;
		fSpeed = 0;
		fLife = 0;
		fSpawnTime = 0;
		tProjInfo = NULL;
		fLastTrailProj = 0;
		iColour = 0;
		iRandom = 0;
        bExplode = false;
        bTouched = false;
        fRotation = 0;
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
	float		fSpawnTime;
	float		fLife;
	float		fExtra;
	int			iOwner;
	float		fSpeed;
	Uint32		iColour;
	float		fIgnoreWormCollBeforeTime;

	// Projectile trail
	float		fLastTrailProj;
    float       fTimeVarRandom;

	proj_t		*tProjInfo;

	CVec		vOldPos;
	CVec		vPosition;
	CVec		vVelocity;
	float		fRotation;

	// Network
	int			iRandom;

	// Collision checking
	int			MAX_CHECKSTEP; // only after a step of this a collision check will be made
	int			MIN_CHECKSTEP; // if step is wider than this, it will be intersected
	int			MAX_CHECKSTEP2; // power of max checkstep
	int			MIN_CHECKSTEP2; // power of min checkstep
	int			AVG_CHECKSTEP; // this is used for the intersection, if the step is to wide
	int			iColSize;  // size of the projectile for the collision (1 for pixel, 2 for image)
	float		fGravity;  // gravity of the projectile (either custom or 100 by default)
	float		fWallshootTime;  // period of time from the spawn when no collision checks are made
	bool		bChangesSpeed;  // true if the projectile changes its speed during its life
	int			iCheckSpeedLen;  // speed for which the check steps have been calculated last time
	int			CollisionSide;


	// Animation
	bool		bFrameDelta;
	float		fFrame;
    int         iFrameX;

	// Beam info
	CVec		vStart, vEnd;
	int			iWidth;

    // Queued events
    bool		bExplode;
    float		fExplodeTime;
    bool		bTouched;

	// Debug info
	bool		firstbounce;

private:
	void	CalculateCheckSteps();


public:
	// Methods


	void	Spawn(proj_t *_proj, CVec _pos, CVec _vel, int _rot, int _owner, int _random, float time, float ignoreWormCollBeforeTime);
	int		Collision(uchar pf);

    void	Draw(SDL_Surface * bmpDest, CViewport *view);
    void	DrawShadow(SDL_Surface * bmpDest, CViewport *view, CMap *map);

	int		CheckWormCollision(CWorm *worms);
	int		ProjWormColl(CVec pos, CWorm *worms);
	ColInfo	TerrainCollision(int px, int py, CMap *map);
	bool	MapBoundsCollision(int px, int py, CMap *map);
	void	HandleCollision(const ColInfo& col, const CVec& oldpos, const CVec& oldvel, float dt);
	
	struct CollisionType {
		bool withWorm : 1;
		union {
			unsigned int wormId : 7;
			unsigned int colMask : 7;
		};
		static CollisionType Worm(int wormid) { CollisionType c; c.withWorm = true; c.wormId = wormid; return c; }
		static CollisionType Terrain(int colmask) { CollisionType c; c.withWorm = false; c.colMask = colmask; return c; }
		static CollisionType None() { return Terrain(0); }
		operator bool() { return withWorm || colMask != 0; }
	};
	
	CollisionType SimulateFrame(float dt, CMap *map, CWorm* worms, float* enddt); // returns collision mask
	static int	CheckCollision(proj_t* tProjInfo, float dt, CMap *map, CVec pos, CVec vel); // returns collision mask

	void	Bounce(float fCoeff);

	bool	isUsed(void)			{ return bUsed; }
	void	setUnused();

	float	getLife(void)			{ return fLife; }
	float&	life()					{ return fLife; }

	float&	extra()					{ return fExtra; }
	bool&	explode()				{ return bExplode; }
	float&	explodeTime()			{ return fExplodeTime; }
	bool&	touched()				{ return bTouched; }
	proj_t* getProjInfo()			{ return tProjInfo; }
	float&	rotation()				{ return fRotation; }
	void	setFrameDelta(bool d)	{ bFrameDelta = d; }
	bool	getFrameDelta()			{ return bFrameDelta; }
	float&	frame()					{ return fFrame; }
	float&	lastTrailProj()			{ return fLastTrailProj; }
	float	getIgnoreWormCollBeforeTime()	{ return fIgnoreWormCollBeforeTime; }

	CVec	GetPosition(void)		{ return vPosition; }
	CVec	GetVelocity(void)		{ return vVelocity; }
	proj_t	*GetProjInfo(void)		{ return tProjInfo; }
	int		GetOwner(void)			{ return iOwner; }

    float   getTimeVarRandom(void)  { return fTimeVarRandom; }

	float	getRandomFloat(void);
	int		getRandomIndex(void)	{ return iRandom; }

    void    setExplode(float t, bool _e)     { fExplodeTime = t; bExplode = _e; }
    void    setTouched(bool _t)      { bTouched = _t; }

	void	setNewPosition( const CVec& newpos ) { vOldPos = vPosition = newpos; }
	void	setNewVel( const CVec& newvel ) { vVelocity = newvel; }

	// HINT: saves the current time of the simulation
	// we need to save this also per projectile as they can have different
	// simulation times (different times of spawning or remote projectiles)
	float	fLastSimulationTime;

	Event<> onInvalidation;
};


#endif  //  __CPROJECTILE_H__
