/*
 *  ProjectileDesc.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 29.03.09.
 *  code under LGPL
 *
 */

#ifndef __PROJECTILEDESC_H__
#define __PROJECTILEDESC_H__

#include <string>
#include <vector>
#include "StaticAssert.h"

struct SDL_Surface;
struct SoundSample;


// Projectile types
enum Proj_Type {
	PRJ_PIXEL = 0,
	PRJ_IMAGE = 1,
	
	__PRJ_LBOUND = INT_MIN,
	__PRJ_UBOUND = INT_MAX
};

static_assert(sizeof(Proj_Type) == sizeof(int), Proj_Type__SizeCheck);


// Projectile trails
enum Proj_Trail {
	TRL_NONE = 0,
	TRL_SMOKE = 1,
	TRL_CHEMSMOKE = 2,
	TRL_PROJECTILE = 3,
	TRL_DOOMSDAY = 4,
	TRL_EXPLOSIVE = 5,
	
	__TRL_LBOUND = INT_MIN,
	__TRL_UBOUND = INT_MAX // force enum to be of size int
};

static_assert(sizeof(Proj_Trail) == sizeof(int), Proj_Trail__SizeCheck);


// Projectile method types
enum Proj_Action {
	PJ_BOUNCE = 0,
	PJ_EXPLODE = 1,
	PJ_INJURE = 2,
	PJ_CARVE = 4,
	PJ_DIRT = 5,
	PJ_GREENDIRT = 6,
	PJ_DISAPPEAR = 7,
	PJ_NOTHING = 8,
	
	__PJ_LBOUND = INT_MIN,
	__PJ_UBOUND = INT_MAX // force enum to be of size int
};

static_assert(sizeof(Proj_Action) == sizeof(int), Proj_Action__SizeCheck);




// Animation types
enum Proj_AnimType {
	ANI_ONCE = 0,
	ANI_LOOP = 1,
	ANI_PINGPONG = 2,
	
	__ANI_LBOUND = INT_MIN,
	__ANI_UBOUND = INT_MAX,
};

static_assert(sizeof(Proj_AnimType) == sizeof(int), Proj_AnimType__SizeCheck);


struct Proj_Hit {
	Proj_Hit() : Type(PJ_NOTHING) {}
	
	Proj_Action Type;
	int		Damage;
	bool	Projectiles;
	bool	UseSound;
	int		Shake;
	std::string	SndFilename; // (was 64b before)
	float	BounceCoeff;
	int		BounceExplode;
	
};


// Projectile structure
struct proj_t {
	proj_t() : id(0), PrjTrl_Proj(NULL), bmpImage(NULL), smpSample(NULL) {}
	
	std::string	filename;		// Compiler use (was 64b before)
	int		id;					// File ref use
	
	Proj_Type Type;
	Proj_Trail Trail;
	std::vector<Color>	Colour;
	std::string	ImgFilename; // (was 64b before)
	bool	Rotating;
	int		RotIncrement;
	int		RotSpeed;
	bool	UseAngle;
	bool	UseSpecAngle;
	int		AngleImages;
	bool	UseCustomGravity;
	int		Gravity;
	bool	Animating;
	float	AnimRate;
	Proj_AnimType AnimType;
	float   Dampening;
	
	// Timer (When the timer is finished)
	Proj_Action Timer_Type;
	float	Timer_Time;
	float	Timer_TimeVar;
	int		Timer_Damage;
	bool	Timer_Projectiles;
	int		Timer_Shake;
	
	// Hit (When hitting the terrain)
	Proj_Hit	Hit;
	
    // OnExplode (When something near has exploded)
    int		Exp_Type;
	int		Exp_Damage;
	bool	Exp_Projectiles;
	int		Exp_UseSound;
    std::string	Exp_SndFilename; // (was 64b before)
	int		Exp_Shake;
	
	
    // Touch (When another projectile has touched this projectile)
    int		Tch_Type;
	int		Tch_Damage;
	bool	Tch_Projectiles;
	bool	Tch_UseSound;
    std::string	Tch_SndFilename; // (was 64b before)
	int		Tch_Shake;
	
	
	// Player hit
	Proj_Action PlyHit_Type;
	int		PlyHit_Damage;
	bool	PlyHit_Projectiles;
	float	PlyHit_BounceCoeff;
	
	bool	ProjUseangle;
	int		ProjAngle;
	int		ProjSpeed;
	float	ProjSpeedVar;
	float	ProjSpread;
	int		ProjAmount;
	proj_t	*Projectile;
	
	
	// Projectile trail
	bool	PrjTrl_UsePrjVelocity;
	float	PrjTrl_Delay;
	int		PrjTrl_Amount;
	int		PrjTrl_Speed;
	float	PrjTrl_SpeedVar;
	float	PrjTrl_Spread;
	proj_t	*PrjTrl_Proj;
	
	
	SDL_Surface * bmpImage;	// Read-only var, managed by game script, no need in smartpointer
	SoundSample * smpSample;
	
	
	// new since Beta9
	
	
	
	// not implemented yet:
	bool WormOwnerInjure; // SelfInjure=true in options will overwrite this
	bool WormOwenrHit; // SelfHit=true in options will overwrite this
	bool TeamInjure; // TeamInjure=true in options will overwrite this
	bool TeamHit; // TeamHit=true in options will overwrite this
	
	
};


#endif
