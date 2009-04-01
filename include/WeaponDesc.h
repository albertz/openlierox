//
// C++ Interface: WeaponDesc
//
// Description: weapon description
//
//
// Author: Albert Zeyer <ich@az2000.de>, (C) 2009
//
// code under LGPL
//
//

#ifndef __WEAPONDESC_H__
#define __WEAPONDESC_H__

#include <string>
#include <SDL.h>
#include "StaticAssert.h"
#include "ProjAction.h"
#include "Color.h"




// Weapon classes
enum Wpn_Class {
	WCL_AUTOMATIC = 0,
	WCL_POWERGUN = 1,
	WCL_GRENADE = 3,
	WCL_MISSILE = 4,
	WCL_CLOSERANGE = 5,
		
	__WCL_LBOUND = INT_MIN,
	__WCL_UBOUND = INT_MAX,
};

static_assert(sizeof(Wpn_Class) == sizeof(int), Wpn_Class__SizeCheck);

// Weapon types
enum Wpn_Type {
	WPN_PROJECTILE = 0,
	WPN_SPECIAL = 1,
	WPN_BEAM = 2,
		
	__WPN_LBOUND = INT_MIN,
	__WPN_UBOUND = INT_MAX
};

static_assert(sizeof(Wpn_Type) == sizeof(int), Wpn_Type__SizeCheck);


// Special Weapons
enum Wpn_Special {
	SPC_NONE = 0,
	SPC_JETPACK = 1,
		
	__SPC_LBOUND = INT_MIN,
	__SPC_UBOUND = INT_MAX
};

static_assert(sizeof(Wpn_Special) == sizeof(int), Wpn_Special__SizeCheck);



// Special structure
struct gs_special_t {
	Uint32	Thrust;
};


struct SoundSample;

// Weapon structure
struct weapon_t {
	weapon_t() : ID(0), smpSample(NULL) {}
	
	int		ID;
	std::string	Name; // (was 64b before)
	Wpn_Type Type;
	Wpn_Special Special;
	Wpn_Class Class;
	int		Recoil;
	float	Recharge;
	float	Drain;
	float	ROF;
	bool	UseSound;
	std::string	SndFilename; // (was 64b before)
	bool	LaserSight;

	// Projectile
	Proj_SpawnInfo Proj;

	// Beam
	Color	Bm_Colour;
	int		Bm_Damage;
	int		Bm_PlyDamage;
	int		Bm_Length;


	// Special
	gs_special_t tSpecial;

	SoundSample * smpSample;	// Read-only var, managed by game script, no need in smartpointer

};



#endif

