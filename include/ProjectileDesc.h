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

class SoundSample;


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

// MyGravity types
enum Proj_AttractiveObject  {
	ATO_NONE = 0,
	ATO_PLAYERS = 1,  // All worms
	ATO_PROJECTILES = 2,
	ATO_ROPE = 4,
	ATO_BONUSES = 8,
	ATO_ALL = ATO_PLAYERS | ATO_PROJECTILES | ATO_ROPE | ATO_BONUSES
};

// Classes of objects that are attracted to the projectile, used in conjunction with the above
enum Proj_AttractiveClass  {
	ATC_NONE = 0,
	ATC_OWNER = 1,
	ATC_ENEMY = 2,
	ATC_TEAMMATE = 4,
	ATC_ALL = ATC_OWNER | ATC_TEAMMATE | ATC_ENEMY
};

// Behavior of the attractive force
enum Proj_AttractiveType  {
	ATT_GRAVITY = 0,  // Newton gravity
	ATT_CONSTANT,  // The force is the same in the whole radius
	ATT_LINEAR,  // The force fades out linearly
	ATT_QUADRATIC,  // The force fades out using the inverse quadratic function
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
	proj_t() : bmpImage(NULL) {}
	
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
	int		AttractiveForce;  // How much will players/projectiles be attracted to this projectile
	int AttractiveForceObjects;
	int AttractiveForceClasses;
	int		AttractiveForceRadius;  // Radius where the gravity is applied
	bool	AttractiveForceThroughWalls;  // Should the gravity "go through" walls?
	Proj_AttractiveType	AttractiveForceType;  // How the attractive force should behave
	
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
		
	// event proj spawning (if no specific spawnInfo is set in Proj_Action)
	Proj_SpawnInfo GeneralSpawnInfo;
	
	
	SDL_Surface * bmpImage;	// Read-only var, managed by game script, no need in smartpointer
	SmartPointer<SDL_Surface> bmpShadow;  // Pre-generated projectile shadow
	
};





#endif
