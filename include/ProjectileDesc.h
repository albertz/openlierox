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
#include "ProjAction.h"
#include "Geometry.h"

struct SoundSample;


// Projectile types
enum Proj_GfxType {
	PRJ_PIXEL = 0,
	PRJ_IMAGE = 1,
 
	// new since Beta9
	PRJ_CIRCLE = 2,
	PRJ_POLYGON = 3,
	PRJ_RECT = 4,
	
	__PRJ_LBOUND = INT_MIN,
	__PRJ_UBOUND = INT_MAX
};

static_assert(sizeof(Proj_GfxType) == sizeof(int), Proj_Type__SizeCheck);


// Animation types
enum Proj_AnimType {
	ANI_ONCE = 0,
	ANI_LOOP = 1,
	ANI_PINGPONG = 2,
	
	__ANI_LBOUND = INT_MIN,
	__ANI_UBOUND = INT_MAX,
};

static_assert(sizeof(Proj_AnimType) == sizeof(int), Proj_AnimType__SizeCheck);



// Projectile structure
struct proj_t {
	proj_t() : bmpImage(NULL), smpSample(NULL) {}
	
	std::string	filename;		// Compiler use (was 64b before)
	
	Proj_GfxType Type;
	std::vector<Color>	Colour;
	std::string	ImgFilename; // (was 64b before)
	Polygon2D polygon;
	bool	Rotating;
	int		RotIncrement;
	int		RotSpeed;
	bool	UseAngle;
	bool	UseSpecAngle;
	int		AngleImages;
	bool	Animating;
	float	AnimRate;
	Proj_AnimType AnimType;
	
	// physical behaviour
	bool	UseCustomGravity;
	int		Gravity;
	float   Dampening;
	int		Width, Height; // new since Beta9
	
	// general action
	Proj_Trail Trail;
	
	// Timer (When the timer is finished)
	Proj_LX56Timer Timer;
	
	// Hit (When hitting the terrain)
	Proj_Action	Hit;
	
    // OnExplode (When something near has exploded)
	// WARNING: not used in LX56 and should thus never ever be used to keep compatibility
    Proj_Action	Exp;
		
    // Touch (When another projectile has touched this projectile)
	// WARNING: not used in LX56 and should thus never ever be used to keep compatibility
    Proj_Action	Tch;
	
	// Player hit
	Proj_Action PlyHit;
	
	// new events
	std::vector<Proj_EventAndAction> actions; // new since beta9
	
	// if we have no action from other events
	Proj_Action Fallback; // new since beta9	
	
	// event proj spawning (if no specific spawnInfo is set in Proj_Action)
	Proj_SpawnInfo GeneralSpawnInfo;
	
	
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
