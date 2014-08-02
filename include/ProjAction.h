//
// C++ Interface: ProjAction
//
// Description: actions and events for projectiles
//
/*
 This file contains the structure which describe the LX56 projectiles,
 like what they are doing, their actions for specific events, and so on.
 
 The parsing of this data is done in CGameScript.cpp.
 
 The actual simulation and the implementation of all actions/events is
 done in PhysicsLX56_Projectiles.cpp.
 */

// Author: Albert Zeyer <ich@az2000.de>, (C) 2009
//
// code under LGPL
//


#ifndef __PROJACTION_H__
#define __PROJACTION_H__

#include <string>
#include <set>
#include <list>
#include <vector>
#include <limits.h>
#include <cassert>
#include "types.h"
#include "CVec.h"

struct proj_t;
class CProjectile;
struct shoot_t;
struct SoundSample;
class IniReader;

/*
 If you spawn a projectile, this is the spawner object itself.
 Depending on this object, the speed/position of the spawned projectile
 will be set. (See Proj_SpawnInfo::apply())
 */
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

/*
 This contains specific attributes about how to spawn specific projectile (Proj_SpawnInfo::Proj).
 The Proj_SpawnInfo::apply() does the actual spawning.
 */
struct Proj_SpawnInfo {
	Proj_SpawnInfo() :
	Speed(0), SpeedVar(0), Spread(0), Amount(0), Proj(NULL),
	UseParentVelocityForSpread(false), ParentVelSpreadFactor(0.3f),
	Useangle(false), Angle(0),
	UseSpecial11VecForSpeedVar(false), UseRandomRot(false),
	AddParentVel(false), ParentVelFactor(1.0f) {}
	
	int		Speed;
	float	SpeedVar;
	float	Spread;
	int		Amount;
	proj_t	*Proj;
	
	bool	UseParentVelocityForSpread; // LX56: only for trail, this could be true
	float	ParentVelSpreadFactor;
	
	bool	Useangle;
	int		Angle; // LX56: only for event

	bool	UseSpecial11VecForSpeedVar; // LX56: true iff trail
	bool	UseRandomRot; // LX56: true iff shot
	bool	AddParentVel; // LX56: true iff shot
	MatrixD2<float>	ParentVelFactor; // new since Beta9	
	VectorD2<int> PosDiff; // new since Beta9
	VectorD2<int> SnapToGrid; // new since Beta9
	
	void apply(Proj_SpawnParent parent, AbsTime spawnTime) const;
	
	bool isSet() const { return Proj != NULL; }
	void dump() const;
	
	bool readFromIni(CGameScript* gs, const std::string& dir, const IniReader& ini, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};



// Projectile trails (Type of Proj_Trail)
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

static_assert(sizeof(Proj_TrailType) == sizeof(int), "Proj_TrailType__SizeCheck");


struct Proj_Trail {
	Proj_TrailType Type;
	
	float	Delay; // used for spawning
	Proj_SpawnInfo Proj;
	
	Proj_Trail() : Type(TRL_NONE), Delay(0.1f) { Proj.UseSpecial11VecForSpeedVar = true; }
};



// Projectile method types
/*
 Defines the general action type of a Proj_Action.
 Note that some of these (PJ_NOTHING, PJ_GOTHROUGH), when set in proj_t::Hit, define also their physical behaviour on collision.
 */
enum Proj_ActionType {
	PJ_BOUNCE = 0,
	PJ_EXPLODE = 1,
	PJ_INJURE = 2,
	PJ_CARVE = 4,
	PJ_DIRT = 5,
	PJ_GREENDIRT = 6,
	PJ_DISAPPEAR = 7, // mostly ignored (LX56)
	PJ_NOTHING = 8,
	PJ_DISAPPEAR2 = 9,
	PJ_GOTHROUGH = 10,
	PJ_INJUREPROJ = 11,
	PJ_PLAYSOUND = 12,
	PJ_INJUREWORM = 13,
	PJ_ChangeRadius = 14,
	PJ_OverwriteOwnSpeed = 15,
	PJ_MultiplyOwnSpeed = 16,
	PJ_DiffOwnSpeed = 17,
	PJ_OverwriteTargetSpeed = 18,
	PJ_MultiplyTargetSpeed = 19,
	PJ_DiffTargetSpeed = 20,
	PJ_HeadingToNextWorm = 21,
	PJ_HeadingToOwner = 22,
	PJ_HeadingToNextOtherWorm = 23,
	PJ_HeadingToNextEnemyWorm = 24,
	PJ_HeadingToNextTeamMate = 25,
	PJ_HeadTargetToUs = 26,
	
	__PJ_LBOUND = INT_MIN,
	__PJ_UBOUND = INT_MAX // force enum to be of size int
};

static_assert(sizeof(Proj_ActionType) == sizeof(int), "Proj_ActionType__SizeCheck");


struct ProjCollisionType;
struct Proj_DoActionInfo;
class CGameObject;

/*
 This occurs the information when an event occurs. It's like the EventData() in the OLX event system.
 */
struct Proj_EventOccurInfo {
	const ProjCollisionType* colType;
	typedef std::set<CGameObject*> Targets;
	Targets targets;
	bool timerHit;
	TimeDiff serverTime; 
	TimeDiff dt;
	
	Proj_EventOccurInfo(TimeDiff t, TimeDiff _dt) : colType(NULL), timerHit(false), serverTime(t), dt(_dt) {}
	static Proj_EventOccurInfo Unspec(TimeDiff t, TimeDiff _dt) { return Proj_EventOccurInfo(t, _dt); }
	static Proj_EventOccurInfo Timer(TimeDiff t, TimeDiff _dt) { Proj_EventOccurInfo e(t, _dt); e.timerHit = true; return e; }
	static Proj_EventOccurInfo Col(TimeDiff t, TimeDiff _dt, const ProjCollisionType* col) { Proj_EventOccurInfo e(t, _dt); e.colType = col; return e; }	
};

/*
 This defines a specific action which does something (applies damage to an object, modifies some velocity, or whatever).
 The action is done in applyTo(). Note that some of the actions are applied directly and some others
 are saved in the Proj_DoActionInfo structure for later execution. Look at applyTo() for the specification.
 In LX56, we have the default events like terrain hit, timer or playerhit and all these have an Proj_Action structure.
 In addition to that (since Beta9), a proj_t has a vector of Proj_EventAndAction (basically a pair of _Proj_Event + Proj_Action).
 */
struct Proj_Action {
	Proj_Action() :
	Type(PJ_EXPLODE), Damage(0), Projectiles(false), Shake(0),
	UseSound(false), BounceCoeff(0.5), BounceExplode(0),
	Sound(NULL), GoThroughSpeed(1.0f), SpeedMult(1.0f),
	additionalAction(NULL) { Proj.Amount = 1; }
	~Proj_Action() { if(additionalAction) delete additionalAction; additionalAction = NULL; }
	Proj_Action(const Proj_Action& a) : Type(PJ_NOTHING), additionalAction(NULL) { operator=(a); }
	
	Proj_Action& operator=(const Proj_Action& a); // IMPORTANT: don't forget to update this after adding new attributes
	
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
	
	// new since Beta9:
	
	SoundSample* Sound;
	float	GoThroughSpeed;
	VectorD2<int>	ChangeRadius;
	
	VectorD2<float> Speed;
	MatrixD2<float> SpeedMult;
	
	Proj_Action* additionalAction;
	
	bool hasAction() const {
		if(Type != PJ_NOTHING) return true;
		if(Projectiles) return true;
		return (additionalAction && additionalAction->hasAction());
	}
	bool needGeneralSpawnInfo() const { return Projectiles && !Proj.isSet(); }
	void applyTo(const Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const;

	bool readFromIni(CGameScript* gs, const std::string& dir, const IniReader& ini, const std::string& section, int deepCounter = 0);
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};

/*
 General base interface of an event. The checkEvent() function checks the event and returns true if it matches.
 */
struct _Proj_Event {
	virtual ~_Proj_Event() {}
	virtual bool canMatch() const { return true; } // if it is possible at all with current event attributes to match
	virtual bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const = 0;
	virtual bool readFromIni(CGameScript* gs, const std::string& dir, const IniReader& ini, const std::string& section) { return true; }
	virtual bool read(CGameScript* gs, FILE* fp) { return true; }
	virtual bool write(CGameScript* gs, FILE* fp) { return true; }
};

/*
 This is an own timer event. You can have as many different events as you want from these and all timers will run indepenently.
 Don't confuse this with proj_t::Timer (Proj_LX56Timer), it's a different implementation.
 */
struct Proj_TimerEvent : _Proj_Event {
	Proj_TimerEvent() : Delay(1), Repeat(true), UseGlobalTime(false), PermanentMode(0) {}
	
	float	Delay;
	bool	Repeat;
	bool	UseGlobalTime;
	int		PermanentMode;
	
	bool canMatch() const { return Delay >= 0; }
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const;

	bool readFromIni(CGameScript* gs, const std::string& dir, const IniReader& ini, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};

/*
 This is currently based on the collision info by LX56Projectile_checkCollAndMove.
 */
struct Proj_WormHitEvent : _Proj_Event {
	Proj_WormHitEvent() : SameWormAsProjOwner(false), SameTeamAsProjOwner(false), DiffWormAsProjOwner(false), DiffTeamAsProjOwner(false), TeamMateOfProjOwner(false), EnemyOfProjOwner(false) {}
	
	bool SameWormAsProjOwner;
	bool SameTeamAsProjOwner;
	bool DiffWormAsProjOwner;
	bool DiffTeamAsProjOwner;
	bool TeamMateOfProjOwner;
	bool EnemyOfProjOwner;
	
	bool canMatch() const;
	bool match(int worm, CProjectile* prj) const;
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const;
	
	bool readFromIni(CGameScript* gs, const std::string& dir, const IniReader& ini, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};

struct Proj_ProjHitEvent : _Proj_Event {
	Proj_ProjHitEvent() :
	Target(NULL), MinHitCount(1), MaxHitCount(-1), Width(-1), Height(-1),
	TargetHealthIsMore(false), TargetHealthIsLess(false), TargetTimeIsMore(false), TargetTimeIsLess(false) {}
	
	proj_t* Target; // NULL -> any target
	int		MinHitCount, MaxHitCount;
	int		Width, Height; // custom w/h for collision check area
	bool	TargetHealthIsMore, TargetHealthIsLess;
	bool	TargetTimeIsMore, TargetTimeIsLess;
	Proj_WormHitEvent ownerWorm;
	
	bool canMatch() const {
		if(MaxHitCount >= 0 && MaxHitCount < MinHitCount) return false;
		if(TargetHealthIsMore && TargetHealthIsLess) return false;
		if(TargetTimeIsMore && TargetTimeIsLess) return false;
		return ownerWorm.canMatch();
	}
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const;
	
	bool readFromIni(CGameScript* gs, const std::string& dir, const IniReader& ini, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};


struct Proj_TerrainHitEvent : _Proj_Event {
	Proj_TerrainHitEvent() : MapBound(false), Dirt(false), Rock(false) {}
	
	bool MapBound;
	bool Dirt;
	bool Rock;
	
	bool canMatch() const { return (MapBound ? 1 : 0) + (Dirt ? 1 : 0) + (Rock ? 1 : 0) <= 1; }
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const;

	bool readFromIni(CGameScript* gs, const std::string& dir, const IniReader& ini, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};

// If the projectile is out of health, this event will match.
struct Proj_DeathEvent : _Proj_Event {
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const;
};

// If info doesn't contain any action (hasAnyEffect() = false), this will match.
struct Proj_FallbackEvent : _Proj_Event {
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const;
};


// Event struct that contains all possible events.
// This is for some simplification and speed up.
struct Proj_Event {
	enum Type {
		PET_UNSET = 0,
		PET_TIMER = 1,
		PET_PROJHIT = 2,
		PET_WORMHIT = 3,
		PET_TERRAINHIT = 4,
		PET_DEATH = 5,
		PET_FALLBACK = 6,
		__PET_LBOUND = INT_MIN,
		__PET_UBOUND = INT_MAX
	} type;
	static_assert(sizeof(Type) == sizeof(int), "Proj_Event_Type__SizeCheck");
	
	Proj_Event(Type t = PET_UNSET) : type(t) {}
	Proj_TimerEvent timer;
	Proj_ProjHitEvent projHit;
	Proj_WormHitEvent wormHit;
	Proj_TerrainHitEvent terrainHit;
	Proj_DeathEvent death;
	Proj_FallbackEvent fallback;
	
	_Proj_Event* get() {
		switch(type) {
			case PET_UNSET: return NULL;
			case PET_TIMER: return &timer;
			case PET_PROJHIT: return &projHit;
			case PET_WORMHIT: return &wormHit;
			case PET_TERRAINHIT: return &terrainHit;
			case PET_DEATH: return &death;
			case PET_FALLBACK: return &fallback;
			case __PET_LBOUND: case __PET_UBOUND: return NULL;
		}
		return NULL;
	}

	const _Proj_Event* get() const { // copy of above function
		switch(type) {
			case PET_UNSET: return NULL;
			case PET_TIMER: return &timer;
			case PET_PROJHIT: return &projHit;
			case PET_WORMHIT: return &wormHit;
			case PET_TERRAINHIT: return &terrainHit;
			case PET_DEATH: return &death;
			case PET_FALLBACK: return &fallback;
			case __PET_LBOUND: case __PET_UBOUND: return NULL;
		}
		return NULL;
	}
	
	bool canMatch() const { return get()->canMatch(); }
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const { return get()->checkEvent(eventInfo, prj, info); }
	bool readFromIni(CGameScript* gs, const std::string& dir, const IniReader& ini, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};

/*
 A vector of these are saved in proj_t. In simulation, we will go through this vector and check each of this Proj_EventAndAction.
 We have a list of events here (Proj_Event) and if all of them matches, we apply the underlying Proj_Action.
 This is done by the checkAndApply() function.
 */
struct Proj_EventAndAction : Proj_Action {
	typedef std::vector<Proj_Event> Events;
	Events events;
	Proj_EventAndAction() { Type = PJ_NOTHING; }
	bool hasAction() const {
		if(!Proj_Action::hasAction()) return false;
		for(Events::const_iterator i = events.begin(); i != events.end(); ++i) if(!i->canMatch()) return false;
		return true;
	}
	bool checkAndApply(Proj_EventOccurInfo eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const {
		for(Events::const_iterator i = events.begin(); i != events.end(); ++i) if(!i->checkEvent(eventInfo, prj, info)) return false;
		applyTo(eventInfo, prj, info);
		return true;
	}	

	bool readFromIni(CGameScript* gs, const std::string& dir, const IniReader& ini, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};



struct Proj_LX56Timer : Proj_Action {
	Proj_LX56Timer() : Time(0), TimeVar(0) {}
	
	float	Time;
	float	TimeVar;
	
	bool canMatch() const { return Time > 0; }
	bool hasAction() const { return Proj_Action::hasAction() && canMatch(); }
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const;
	bool checkAndApply(Proj_EventOccurInfo eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const {
		if(!hasAction()) return false;
		if(!checkEvent(eventInfo, prj, info)) return false;
		applyTo(eventInfo, prj, info);
		return true;
	}
};


/*
 We collect some specific operations in this structure while doing the simulation. When calling Proj_Action::applyTo(),
 the Proj_Action will not execute all actions immediatly but it will also fill in some operations in Proj_DoActionInfo.
 The operations here in Proj_DoActionInfo are executed at the end of a simulation frame of a projectile.
 */
struct Proj_DoActionInfo {
	Proj_DoActionInfo() :
	explode(false), damage(-1), timer(false), shake(0),
	dirt(false), grndirt(false), deleteAfter(false),
	trailprojspawn(false), spawnprojectiles(false),
	sound(NULL) {}
	
	bool	explode;
	int		damage;
	bool	timer;
	int		shake;
	bool	dirt;
	bool	grndirt;
	bool	deleteAfter;
	bool	trailprojspawn;
	
	bool	spawnprojectiles;
	std::list<const Proj_SpawnInfo*> otherSpawns;
		
	SoundSample*	sound;
	
	bool hasAnyEffect() const; // NOTE: sound doesn't count
	void execute(CProjectile* const prj, const AbsTime currentTime);
};


#endif
