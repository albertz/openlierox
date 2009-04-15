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
#include <set>
#include <list>
#include <vector>
#include <limits.h>
#include <cassert>
#include "StaticAssert.h"
#include "types.h"
#include "CVec.h"

struct proj_t;


class CProjectile;
class shoot_t;

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
	
	bool	Useangle; // LX56: only for event
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
	
	bool readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
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
	PJ_DISAPPEAR = 7, // mostly ignored (LX56)
	PJ_NOTHING = 8,
	PJ_DISAPPEAR2 = 9,
	PJ_GOTHROUGH = 10,
	
	__PJ_LBOUND = INT_MIN,
	__PJ_UBOUND = INT_MAX // force enum to be of size int
};

static_assert(sizeof(Proj_ActionType) == sizeof(int), Proj_ActionType__SizeCheck);


struct ProjCollisionType;
struct Proj_DoActionInfo;

struct Proj_EventOccurInfo {
	const ProjCollisionType* colType;
	std::set<CProjectile*> projCols;
	bool timerHit;
	TimeDiff serverTime; 
	TimeDiff dt;
	
	Proj_EventOccurInfo(TimeDiff t, TimeDiff _dt) : colType(NULL), timerHit(false), serverTime(t), dt(_dt) {}
	static Proj_EventOccurInfo Unspec(TimeDiff t, TimeDiff _dt) { return Proj_EventOccurInfo(t, _dt); }
	static Proj_EventOccurInfo Timer(TimeDiff t, TimeDiff _dt) { Proj_EventOccurInfo e(t, _dt); e.timerHit = true; return e; }
	static Proj_EventOccurInfo Col(TimeDiff t, TimeDiff _dt, const ProjCollisionType* col) { Proj_EventOccurInfo e(t, _dt); e.colType = col; return e; }	
};

struct Proj_Action {
	Proj_Action() :
	Type(PJ_EXPLODE), Damage(0), Projectiles(false), Shake(0),
	UseSound(false), BounceCoeff(0.5), BounceExplode(0),
	GoThroughSpeed(1.0f),
	UseOverwriteOwnSpeed(false), ChangeOwnSpeed(1.0f),
	UseOverwriteTargetSpeed(false), ChangeTargetSpeed(1.0f),
	additionalAction(NULL) { Proj.Amount = 1; }
	~Proj_Action() { if(additionalAction) delete additionalAction; additionalAction = NULL; }
	Proj_Action(const Proj_Action& a) : Type(PJ_NOTHING), additionalAction(NULL) { operator=(a); }
	Proj_Action& operator=(const Proj_Action& a);
	
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
	
	float	GoThroughSpeed;
	bool	UseOverwriteOwnSpeed;
	VectorD2<float> OverwriteOwnSpeed;
	MatrixD2<float> ChangeOwnSpeed;
	bool	UseOverwriteTargetSpeed;
	VectorD2<float> OverwriteTargetSpeed;
	MatrixD2<float> ChangeTargetSpeed;
	Proj_Action* additionalAction;
	
	bool hasAction() const { return Type != PJ_NOTHING || Projectiles || (additionalAction && additionalAction->hasAction()); }
	bool needGeneralSpawnInfo() const { return Projectiles && !Proj.isSet(); }
	void applyTo(const Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const;

	bool readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section, int deepCounter = 0);
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};

struct _Proj_Event {
	virtual ~_Proj_Event() {}
	virtual bool canMatch() const { return true; }
	virtual bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj) const = 0;
	virtual bool readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section) { return true; }
	virtual bool read(CGameScript* gs, FILE* fp) { return true; }
	virtual bool write(CGameScript* gs, FILE* fp) { return true; }
};

struct Proj_TimerEvent : _Proj_Event {
	Proj_TimerEvent() : Delay(1), Repeat(true), UseGlobalTime(false) {}
	
	float	Delay;
	bool	Repeat;
	bool	UseGlobalTime;
	
	bool canMatch() const { return Delay >= 0; }
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj) const;

	bool readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};

struct Proj_ProjHitEvent : _Proj_Event {
	Proj_ProjHitEvent() : Target(NULL), MinHitCount(1), MaxHitCount(-1), Width(-1), Height(-1) {}
	
	proj_t* Target; // NULL -> any target
	int		MinHitCount, MaxHitCount;
	int		Width, Height; // custom w/h for collision check area
	
	bool canMatch() const { return MaxHitCount < 0 || MaxHitCount >= MinHitCount; }
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj) const;
	
	bool readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};

// Event struct that contains all possible events.
// This is for some simplification and speed up.
struct Proj_Event {
	enum Type {
		PET_UNSET = 0,
		PET_TIMER = 1,
		PET_PROJHIT = 2,
		__PET_LBOUND = INT_MIN,
		__PET_UBOUND = INT_MAX
	} type;
	static_assert(sizeof(Type) == sizeof(int), Proj_Event_Type__SizeCheck);
	
	Proj_Event(Type t = PET_UNSET) : type(t) {}
	Proj_TimerEvent timer;
	Proj_ProjHitEvent projHit;
	
	_Proj_Event* get() {
		switch(type) {
			case PET_UNSET: return NULL;
			case PET_TIMER: return &timer;
			case PET_PROJHIT: return &projHit;
			case __PET_LBOUND: case __PET_UBOUND: return NULL;
		}
		return NULL;
	}

	const _Proj_Event* get() const { // copy of above function
		switch(type) {
			case PET_UNSET: return NULL;
			case PET_TIMER: return &timer;
			case PET_PROJHIT: return &projHit;
			case __PET_LBOUND: case __PET_UBOUND: return NULL;
		}
		return NULL;
	}
	
	bool canMatch() const { return get()->canMatch(); }
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj) const { return get()->checkEvent(eventInfo, prj); }
	bool readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};

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
		if(!hasAction()) return false;
		for(Events::const_iterator i = events.begin(); i != events.end(); ++i) if(!i->checkEvent(eventInfo, prj)) return false;
		applyTo(eventInfo, prj, info);
		return true;
	}	

	bool readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section);	
	bool read(CGameScript* gs, FILE* fp);
	bool write(CGameScript* gs, FILE* fp);
};



struct Proj_LX56Timer : Proj_Action {
	Proj_LX56Timer() : Time(0) {}
	
	float	Time;
	float	TimeVar;
	
	bool canMatch() const { return Time > 0; }
	bool hasAction() const { return Proj_Action::hasAction() && canMatch(); }
	bool checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj) const;
	bool checkAndApply(Proj_EventOccurInfo eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const {
		if(!hasAction()) return false;
		if(!checkEvent(eventInfo, prj)) return false;
		applyTo(eventInfo, prj, info);
		return true;
	}
};



struct Proj_DoActionInfo {
	Proj_DoActionInfo() :
	explode(false), damage(-1), timer(false), shake(0),
	dirt(false), grndirt(false), deleteAfter(false),
	trailprojspawn(false), spawnprojectiles(false),
	OverwriteOwnSpeed(NULL), ChangeOwnSpeed(NULL),
	playSound(false) {}
	
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
	
	const VectorD2<float>* OverwriteOwnSpeed;
	const MatrixD2<float>* ChangeOwnSpeed;

	bool	playSound;
	
	bool hasAnyEffect() const; // NOTE: sound doesn't count
	void execute(CProjectile* const prj, const AbsTime currentTime);
};


#endif
