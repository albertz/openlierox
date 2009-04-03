//
// C++ Interface: ProjAction
//
// Description: actions and events for projectiles
//
//
// Author: Albert Zeyer <ich@az2000.de>, (C) 2009
//
// code under LGPL
//
//


#ifndef __PROJACTION_H__
#define __PROJACTION_H__

#include <string>
#include "StaticAssert.h"
#include "types.h"
#include "CVec.h"

struct proj_t;


class CProjectile;
struct shoot_t;

struct Proj_SpawnParent {
	union {
		shoot_t* shot;
		CProjectile* proj;
	};
	enum {
		PSPT_NOTHING = -1,
		PSPT_SHOT = 0,
		PSPT_PROJ,
	} type;
	
	Proj_SpawnParent() : shot(NULL), type(PSPT_NOTHING) {}
	Proj_SpawnParent(shoot_t* s) : shot(s), type(PSPT_SHOT) {}
	Proj_SpawnParent(CProjectile* p) : proj(p), type(PSPT_PROJ) {}
	
	int ownerWorm() const;
	int fixedRandomIndex() const;
	float fixedRandomFloat() const;
	CVec position() const;
	CVec velocity() const;
	float angle() const;
};


class CGameScript;

struct Proj_SpawnInfo {
	Proj_SpawnInfo() :
	Speed(0), SpeedVar(0), Spread(0), Amount(0), Proj(NULL),
	UseParentVelocityForSpread(false), ParentVelFactor(0.3f),
	Useangle(false), Angle(0),
	UseSpecial11VecForSpeedVar(false), UseRandomRot(false), AddParentVel(false) {}
	
	int		Speed;
	float	SpeedVar;
	float	Spread;
	int		Amount;
	proj_t	*Proj;
	
	bool	UseParentVelocityForSpread; // LX56: only for trail, this could be true
	float	ParentVelFactor;
	
	bool	Useangle; // LX56: only for event
	int		Angle; // LX56: only for event

	bool	UseSpecial11VecForSpeedVar; // LX56: true iff trail
	bool	UseRandomRot; // LX56: true iff shot
	bool	AddParentVel; // LX56: true iff shot
	
	void apply(Proj_SpawnParent parent, AbsTime spawnTime) const;
	bool isSet() const { return Proj != NULL; }
	
	// returns projectile filename (used in CGameScript::compile*)
	std::string readFromIni(const std::string& file, const std::string& section);
};



// Projectile trails
enum Proj_TrailType {
	TRL_NONE = 0,
	TRL_SMOKE = 1,
	TRL_CHEMSMOKE = 2,
	TRL_PROJECTILE = 3,
	TRL_DOOMSDAY = 4,
	TRL_EXPLOSIVE = 5,
	
	__TRL_LBOUND = INT_MIN,
	__TRL_UBOUND = INT_MAX // force enum to be of size int
};

static_assert(sizeof(Proj_TrailType) == sizeof(int), Proj_TrailType__SizeCheck);


struct Proj_Trail {
	Proj_TrailType Type;
	
	float	Delay; // used for spawning
	Proj_SpawnInfo Proj;
	
	Proj_Trail() { Proj.UseSpecial11VecForSpeedVar = true; }
};



// Projectile method types
enum Proj_ActionType {
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

static_assert(sizeof(Proj_ActionType) == sizeof(int), Proj_ActionType__SizeCheck);



struct Proj_Action {
	Proj_Action() : Type(PJ_NOTHING) {}
	
	//  --------- LX56 start ----------
	Proj_ActionType Type;
	int		Damage;
	bool	Projectiles;
	int		Shake; // LX56: ignored for PlyHit
	// ---------- LX56 (timer hit) end -----------
	
	bool	UseSound; // LX56: only used for terrain
	std::string	SndFilename; // LX56: only used for terrain // (was 64b before)
	// ---------- LX56 (Exp/Tch hit) end -----------
	
	float	BounceCoeff;
	int		BounceExplode; // LX56: ignored for PlyHit
	//  --------- LX56 (terrain hit) end ----------
	
	Proj_SpawnInfo Proj;
};

struct Proj_Timer : Proj_Action {
	Proj_Timer() : Time(0) {}
	
	float	Time;
	float	TimeVar;	
};


#endif
