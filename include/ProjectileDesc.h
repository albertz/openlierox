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
	proj_t() : id(0), bmpImage(NULL), smpSample(NULL) {}
	
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
	Proj_Timer Timer;
	
	// Hit (When hitting the terrain)
	Proj_Action	Hit;
	
    // OnExplode (When something near has exploded)
    Proj_Action	Exp;
		
    // Touch (When another projectile has touched this projectile)
    Proj_Action	Tch;

	
	// Player hit
	Proj_Action PlyHit;
	
	// event proj spawning	
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
