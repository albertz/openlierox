/*
 *  ProjAction.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 02.04.09.
 *  code under LGPL
 *
 */

#include <cmath>
#include "ProjAction.h"
#include "CGameScript.h"
#include "CWorm.h"
#include "CProjectile.h"
#include "CClient.h"
#include "ProjectileDesc.h"
#include "Physics.h"
#include "ConfigHandler.h"
#include "EndianSwap.h"

int Proj_SpawnParent::ownerWorm() const {
	switch(type) {
		case PSPT_NOTHING: return -1;
		case PSPT_SHOT: return shot->nWormID;
		case PSPT_PROJ: return proj->GetOwner(); 
	}
	return -1;
}

int Proj_SpawnParent::fixedRandomIndex() const {
	switch(type) {
		case PSPT_NOTHING: return -1;
		case PSPT_SHOT: return shot->nRandom;
		case PSPT_PROJ: return proj->getRandomIndex() + 1; 
	}
	return -1;	
}

float Proj_SpawnParent::fixedRandomFloat() const {
	switch(type) {
		case PSPT_NOTHING: return -1;
		case PSPT_SHOT: return GetFixedRandomNum(shot->nRandom);
		case PSPT_PROJ: return proj->getRandomFloat();
	}
	return -1;	
}

CVec Proj_SpawnParent::position() const {
	switch(type) {
		case PSPT_NOTHING: return CVec(0,0);
		case PSPT_SHOT: {
			CVec dir;
			GetVecsFromAngle(shot->nAngle, &dir, NULL);
			CVec pos = shot->cPos + dir*8;
			return pos;
		}
		case PSPT_PROJ: return proj->GetPosition(); 
	}
	return CVec(0,0);
}

CVec Proj_SpawnParent::velocity() const {
	switch(type) {
		case PSPT_NOTHING: return CVec(0,0);
		case PSPT_SHOT: return shot->cWormVel;
		case PSPT_PROJ: return proj->GetVelocity(); 
	}
	return CVec(0,0);
}

float Proj_SpawnParent::angle() const {
	switch(type) {
		case PSPT_NOTHING: return 0;
		case PSPT_SHOT: return (float)shot->nAngle;
		case PSPT_PROJ: {
			CVec v = velocity();
			NormalizeVector(&v);
			float heading = (float)( -atan2(v.x,v.y) * (180.0f/PI) );
			heading+=90;
			FMOD(heading, 360.0f);
			return heading;
		}
	}
	return 0;
}


void Proj_SpawnInfo::dump() const {
	if(Proj)
		notes << "spawn: " << Amount << " of " << Proj->filename << endl;
}

void Proj_SpawnInfo::apply(Proj_SpawnParent parent, AbsTime spawnTime) const {
	// Calculate the angle of the direction the projectile is heading
	float heading = 0;
	if(Useangle)
		heading = parent.angle();
	
	for(int i=0; i < Amount; i++) {		
		CVec sprd;
		if(UseParentVelocityForSpread)
			sprd = parent.velocity() * ParentVelSpreadFactor;
		else {
			int a = (int)( (float)Angle + heading + parent.fixedRandomFloat() * (float)Spread );
			GetVecsFromAngle(a, &sprd, NULL);
		}
		
		int rot = 0;
		if(UseRandomRot) {
			// Calculate a random starting angle for the projectile rotation (if used)
			if(Proj->Rotating) {
				// Prevent div by zero
				if(Proj->RotIncrement == 0)
					Proj->RotIncrement = 1;
				rot = GetRandomInt( 360 / Proj->RotIncrement ) * Proj->RotIncrement;
			}
		}
		
		if(parent.type == Proj_SpawnParent::PSPT_SHOT) {
			parent.shot->nRandom++;
			parent.shot->nRandom %= 255;		
		}
		
		CVec v = sprd * (float)Speed;
		CVec speedVarVec = sprd;
		if(UseSpecial11VecForSpeedVar) speedVarVec = CVec(1,1);
		v += speedVarVec * (float)SpeedVar * parent.fixedRandomFloat();
		if(AddParentVel)
			v += ParentVelFactor * parent.velocity();
		
		if(parent.type == Proj_SpawnParent::PSPT_SHOT) {
			parent.shot->nRandom *= 5;
			parent.shot->nRandom %= 255;		
		}		
		
		AbsTime ignoreWormCollBeforeTime = spawnTime;
		if(parent.type == Proj_SpawnParent::PSPT_PROJ)
			ignoreWormCollBeforeTime = parent.proj->getIgnoreWormCollBeforeTime();
		else
			// we set the ignoreWormCollBeforeTime to the current time to let the physics engine
			// first emulate the projectiles to the curtime and ignore earlier colls as the worm-pos
			// is probably outdated at this time
			ignoreWormCollBeforeTime = GetPhysicsTime() + 0.1f; // HINT: we add 100ms (it was dt before) because the projectile is spawned -> worms are simulated (pos change) -> projectiles are simulated
		
		int random = parent.fixedRandomIndex();
		
		VectorD2<int> pos = parent.position() + PosDiff;
		if(SnapToGrid.x >= 1 && SnapToGrid.y >= 1) {
			pos.x -= pos.x % SnapToGrid.x; pos.x += SnapToGrid.x / 2;
			pos.y -= pos.y % SnapToGrid.y; pos.y += SnapToGrid.y / 2;
		}
		
		cClient->SpawnProjectile(pos, v, rot, parent.ownerWorm(), Proj, random, spawnTime, ignoreWormCollBeforeTime);

		if(parent.type == Proj_SpawnParent::PSPT_SHOT) {
			parent.shot->nRandom++;
			parent.shot->nRandom %= 255;		
		}		
	}
	
}


Proj_Action& Proj_Action::operator=(const Proj_Action& a) {
	if(additionalAction) delete additionalAction; additionalAction = NULL;
	Type = a.Type;
	Damage = a.Damage;
	Projectiles = a.Projectiles;
	Shake = a.Shake;
	UseSound = a.UseSound;
	SndFilename = a.SndFilename;
	BounceCoeff = a.BounceCoeff;
	BounceExplode = a.BounceExplode;
	Proj = a.Proj;
	Sound = a.Sound;
	GoThroughSpeed = a.GoThroughSpeed;
	ChangeRadius = a.ChangeRadius;
	UseOverwriteOwnSpeed = a.UseOverwriteOwnSpeed;
	OverwriteOwnSpeed = a.OverwriteOwnSpeed;
	ChangeOwnSpeed = a.ChangeOwnSpeed;
	UseOverwriteTargetSpeed = a.UseOverwriteTargetSpeed;
	OverwriteTargetSpeed = a.OverwriteTargetSpeed;
	ChangeTargetSpeed = a.ChangeTargetSpeed;
	DiffOwnSpeed = a.DiffOwnSpeed;
	DiffTargetSpeed = a.DiffTargetSpeed;
	HeadingToNextWormSpeed = a.HeadingToNextWormSpeed;
	HeadingToNextOtherWormSpeed = a.HeadingToNextOtherWormSpeed;
	HeadingToNextEnemyWormSpeed = a.HeadingToNextEnemyWormSpeed;
	HeadingToNextTeamMateSpeed = a.HeadingToNextTeamMateSpeed;
	if(a.additionalAction) additionalAction = new Proj_Action(*a.additionalAction);
	
	return *this;
}


static VectorD2<float> wormAngleDiff(CWorm* w, CProjectile* prj) {
	CVec diff = w->getPos() - prj->GetPosition(); NormalizeVector(&diff);
	return MatrixD2<float>::Rotation(diff.x, -diff.y) * prj->GetVelocity();
}

static CWorm* nearestWorm(CVec pos) {
	CWorm* best = NULL;
	for(int i = 0; i < MAX_WORMS; ++i) {
		CWorm* w = &cClient->getRemoteWorms()[i];
		if(!w->isUsed()) continue;
		if(!w->getAlive()) continue;
		if(best == NULL) { best = w; continue; }
		if((best->getPos() - pos).GetLength2() > (w->getPos() - pos).GetLength2())
			best = w; 
	}
	return best;
}

static CWorm* nearestOtherWorm(CVec pos, int worm) {
	CWorm* best = NULL;
	for(int i = 0; i < MAX_WORMS; ++i) {
		CWorm* w = &cClient->getRemoteWorms()[i];
		if(i == worm) continue;
		if(!w->isUsed()) continue;
		if(!w->getAlive()) continue;
		if(best == NULL) { best = w; continue; }
		if((best->getPos() - pos).GetLength2() > (w->getPos() - pos).GetLength2())
			best = w; 
	}
	return best;	
}

static CWorm* nearestEnemyWorm(CVec pos, int worm) {
	const int team = (worm >= 0 && worm < MAX_WORMS) ? cClient->getRemoteWorms()[worm].getTeam() : -1;
	CWorm* best = NULL;
	for(int i = 0; i < MAX_WORMS; ++i) {
		CWorm* w = &cClient->getRemoteWorms()[i];
		if(i == worm) continue;
		if(!w->isUsed()) continue;
		if(!w->getAlive()) continue;
		if(cClient->isTeamGame() && team == w->getTeam()) continue;
		if(best == NULL) { best = w; continue; }
		if((best->getPos() - pos).GetLength2() > (w->getPos() - pos).GetLength2())
			best = w; 
	}
	return best;
	
}

static CWorm* nearestTeamMate(CVec pos, int worm) {
	if(!cClient->isTeamGame()) return NULL;
	const int team = (worm >= 0 && worm < MAX_WORMS) ? cClient->getRemoteWorms()[worm].getTeam() : -1;
	CWorm* best = NULL;
	for(int i = 0; i < MAX_WORMS; ++i) {
		CWorm* w = &cClient->getRemoteWorms()[i];
		if(i == worm) continue;
		if(!w->isUsed()) continue;
		if(!w->getAlive()) continue;
		if(team != w->getTeam()) continue;
		if(best == NULL) { best = w; continue; }
		if((best->getPos() - pos).GetLength2() > (w->getPos() - pos).GetLength2())
			best = w; 
	}
	return best;	
}

static MatrixD2<float> getVelChangeForProj(CWorm* w, CProjectile* prj, float maxAngle) {
	if(w == NULL) return MatrixD2<float>(1.0f);
	maxAngle *= (float)PI / 180.0f;
	
	VectorD2<float> angleDiffV = wormAngleDiff(w, prj);
	float angleDiff = atan2f(angleDiffV.y, angleDiffV.x);
	if(fabs(angleDiff) > fabs(maxAngle)) angleDiff = maxAngle * SIGN(angleDiff);
	
	return MatrixD2<float>::Rotation(cos(-angleDiff), sin(-angleDiff));
}

void Proj_Action::applyTo(const Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const {
	/*
	 * Well, some behaviour here seems strange, but it's all *100% exact* LX56 behaviour.
	 * Please, before touching anything, be very sure that it stays exactly the same!
	 * If you think something is wrong here, check the revision log from this file and
	 * from PhysicsLX56.cpp *before* changing it here!
	 * See also CGameScript.cpp because we enforce some specific values in some cases.
	 */
	
	bool push_worm = true;
	bool spawnprojs = Projectiles;
	
	switch (Type)  {
			// Explosion
		case PJ_EXPLODE:
			info->explode = true;
			info->damage = Damage;
			
			if(eventInfo.timerHit)
				info->timer = true;
			
			if(Shake > info->shake)
				info->shake = Shake;
			
			// Play the hit sound
			if(UseSound && Sound)
				info->sound = Sound;
			break;
			
			// Bounce
		case PJ_BOUNCE:
			if(eventInfo.timerHit) { spawnprojs = false; break; }
			push_worm = false;
			prj->Bounce(BounceCoeff);
			
			// Do we do a bounce-explosion (bouncy larpa uses this)
			if(BounceExplode > 0)
				cClient->Explosion(prj->GetPosition(), BounceExplode, false, prj->GetOwner());
			break;
			
			// Carve
		case PJ_CARVE: 
			if(eventInfo.timerHit || (eventInfo.colType && !eventInfo.colType->withWorm)) {
				int d = cClient->getMap()->CarveHole(Damage, prj->GetPosition());
				info->deleteAfter = true;
				
				// Increment the dirt count
				if(prj->hasOwner())
					cClient->getRemoteWorms()[prj->GetOwner()].incrementDirtCount( d );
			}
			break;
			
			// Dirt
		case PJ_DIRT:
			info->dirt = true;			
			if(eventInfo.timerHit && Shake > info->shake)
				info->shake = Shake;				
			break;
			
			// Green Dirt
		case PJ_GREENDIRT:
			info->grndirt = true;
			if(eventInfo.timerHit && Shake > info->shake)
				info->shake = Shake;
			break;
			
		case PJ_DISAPPEAR2:
			info->deleteAfter = true;
			break;
			
		case PJ_INJURE:
			if(eventInfo.colType && eventInfo.colType->withWorm) {
				info->deleteAfter = true;
				cClient->InjureWorm(&cClient->getRemoteWorms()[eventInfo.colType->wormId], Damage, prj->GetOwner());
				break;
			}
			
		case PJ_DISAPPEAR:
			// TODO: do something special?
			if(eventInfo.colType && eventInfo.colType->withWorm) break;

		case PJ_GOTHROUGH:
		case PJ_NOTHING:
			// if Hit_Type == PJ_NOTHING, it means that this projectile goes through all walls
			if(eventInfo.colType && !eventInfo.colType->withWorm && eventInfo.colType->colMask & PJC_MAPBORDER) {
				// HINT: This is new since Beta9. I hope it doesn't change any serious behaviour.
				if((prj->GetPosition().x < 0 || prj->GetPosition().x > cClient->getMap()->GetWidth())
				&& (prj->GetPosition().y < 0 || prj->GetPosition().y > cClient->getMap()->GetHeight()))
					info->deleteAfter = true;
			}
			push_worm = false;
			if(eventInfo.timerHit) spawnprojs = false;
			break;
		
		case PJ_INJUREPROJ:
			for(std::set<CProjectile*>::const_iterator p = eventInfo.projCols.begin(); p != eventInfo.projCols.end(); ++p)
				(*p)->injure(Damage);
			break;
		
		case PJ_PLAYSOUND:
			if(UseSound && Sound)
				info->sound = Sound;
			break;
			
		case __PJ_LBOUND: case __PJ_UBOUND: errors << "Proj_Action::applyTo: hit __PJ_BOUND" << endl;
	}
	
	// Push the worm back
	if(push_worm && eventInfo.colType && eventInfo.colType->withWorm) {
		CVec d = prj->GetVelocity();
		NormalizeVector(&d);
		cClient->getRemoteWorms()[eventInfo.colType->wormId].velocity() += (d * 100) * eventInfo.dt.seconds();
	}
	
	if(spawnprojs) {
		if(Proj.isSet())
			info->otherSpawns.push_back(&Proj);
		else
			info->spawnprojectiles = true;
	}
	
	if(UseOverwriteOwnSpeed) info->OverwriteOwnSpeed = &OverwriteOwnSpeed;
	info->ChangeOwnSpeed = ChangeOwnSpeed * info->ChangeOwnSpeed;
	info->DiffOwnSpeed += DiffOwnSpeed;
	
	if(HeadingToNextWormSpeed) {
		info->ChangeOwnSpeed = getVelChangeForProj(nearestWorm(prj->GetPosition()), prj, HeadingToNextWormSpeed) * info->ChangeOwnSpeed;
	}

	if(HeadingToNextOtherWormSpeed) {
		info->ChangeOwnSpeed = getVelChangeForProj(nearestOtherWorm(prj->GetPosition(), prj->GetOwner()), prj, HeadingToNextOtherWormSpeed) * info->ChangeOwnSpeed;
	}

	if(HeadingToNextEnemyWormSpeed) {
		info->ChangeOwnSpeed = getVelChangeForProj(nearestEnemyWorm(prj->GetPosition(), prj->GetOwner()), prj, HeadingToNextEnemyWormSpeed) * info->ChangeOwnSpeed;
	}

	if(HeadingToNextTeamMateSpeed) {
		info->ChangeOwnSpeed = getVelChangeForProj(nearestTeamMate(prj->GetPosition(), prj->GetOwner()), prj, HeadingToNextTeamMateSpeed) * info->ChangeOwnSpeed;
	}
	
	if(UseOverwriteTargetSpeed) {
		if(eventInfo.colType && eventInfo.colType->withWorm) {
			cClient->getRemoteWorms()[eventInfo.colType->wormId].velocity() = OverwriteTargetSpeed;
		}
		
		for(std::set<CProjectile*>::const_iterator p = eventInfo.projCols.begin(); p != eventInfo.projCols.end(); ++p) {
			(*p)->setNewVel(OverwriteTargetSpeed);
		}
	}
	
	if(ChangeTargetSpeed != MatrixD2<float>(1.0f) || DiffTargetSpeed != VectorD2<float>()) {
		if(eventInfo.colType && eventInfo.colType->withWorm) {
			CVec& v = cClient->getRemoteWorms()[eventInfo.colType->wormId].velocity();
			v = ChangeTargetSpeed * v + DiffTargetSpeed;
		}
		
		for(std::set<CProjectile*>::const_iterator p = eventInfo.projCols.begin(); p != eventInfo.projCols.end(); ++p) {
			(*p)->setNewVel( ChangeTargetSpeed * (*p)->GetVelocity() + DiffTargetSpeed );
		}
	}
	
	info->ChangeRadius += ChangeRadius;
	
	if(additionalAction) {
		Proj_EventOccurInfo ev(eventInfo);
		ev.timerHit = false; // remove LX56 timer flag
		additionalAction->applyTo(ev, prj, info);
	}
}



bool Proj_LX56Timer::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo*) const {
	float f = prj->getTimeVarRandom();
	if(Time > 0 && (Time + TimeVar * f) < prj->getLife()) {
		eventInfo.timerHit = true;
		return true;
	}
	return false;
}

bool Proj_TimerEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo*) const {
	ProjTimerState& state = prj->timerInfo[this];
	if(state.c > 0 && !Repeat) return PermanentMode == 1;
	
	if(UseGlobalTime) {
		float cur = eventInfo.serverTime.seconds() * (float)cClient->getGameLobby()->features[FT_GameSpeed];
		if(state.c == 0) {
			float startTime = cur - prj->getLife();
			float mstart = startTime; FMOD(mstart, Delay);
			float next = startTime - mstart + Delay;
			if(cur >= next) {
				state.c++;
				state.last = cur;
				if(PermanentMode == 0) return true;
			}
		}
		else {
			float mcur = state.last; FMOD(mcur, Delay);
			float next = state.last - mcur + Delay;			
			if(cur >= next) {
				state.c++;
				state.last = cur;
				if(PermanentMode == 0) return true;
			}
		}
	}
	else { // not global time
		if(state.last + Delay <= prj->getLife()) {
			state.c++;
			state.last = prj->getLife();
			if(PermanentMode == 0) return true;
		}
	}
	
	if(PermanentMode > 0)
		return state.c % (PermanentMode + 1) != 0;
	else if(PermanentMode < 0)
		return state.c % (-PermanentMode + 1) == 0;
	return false;
}


static bool checkProjHit(const Proj_ProjHitEvent& info, Proj_EventOccurInfo& ev, CProjectile* prj, CProjectile* p) {
	if(p == prj) return true;
	if(info.Target && p->getProjInfo() != info.Target) return true;	
	if(!info.ownerWorm.match(prj->GetOwner(), p)) return true;
	if(info.TargetHealthIsMore && p->getHealth() <= prj->getHealth()) return true;
	if(info.TargetHealthIsLess && p->getHealth() >= prj->getHealth()) return true;
	if(info.TargetTimeIsMore && p->getLife() <= prj->getLife()) return true;
	if(info.TargetTimeIsLess && p->getLife() >= prj->getLife()) return true;
	
	if(info.Width >= 0 && info.Height >= 0) { if(!prj->CollisionWith(p, info.Width/2, info.Height/2)) return true; }
	else { if(!prj->CollisionWith(p)) return true; }

	ev.projCols.insert(p);

	if(info.MaxHitCount < 0 && ev.projCols.size() >= (size_t)info.MinHitCount) return false; // no need to check further
	if(ev.projCols.size() > (size_t)info.MaxHitCount) return false; // no need to check further
	
	return true;
}

template<bool TOP, bool LEFT>
static CClient::MapPosIndex MPI(const VectorD2<int>& p, const VectorD2<int>& r) {
	return CClient::MapPosIndex( p + VectorD2<int>(LEFT ? -r.x : r.x, TOP ? -r.y : r.y) );
}

bool Proj_ProjHitEvent::checkEvent(Proj_EventOccurInfo& ev, CProjectile* prj, Proj_DoActionInfo*) const {
	const VectorD2<int> vPosition = prj->GetPosition();
	const VectorD2<int> radius = prj->getRadius();
	for(int x = MPI<true,true>(vPosition,radius).x; x <= MPI<true,false>(vPosition,radius).x; ++x)
		for(int y = MPI<true,true>(vPosition,radius).y; y <= MPI<false,true>(vPosition,radius).y; ++y) {
			CClient::ProjectileSet* projs = cClient->projPosMap[CClient::MapPosIndex(x,y).index(cClient->getMap())];
			if(projs == NULL) continue;
			for(CClient::ProjectileSet::const_iterator p = projs->begin(); p != projs->end(); ++p)
				if(!checkProjHit(*this, ev, prj, *p)) goto finalChecks;
		}
	
finalChecks:
	if(ev.projCols.size() >= (size_t)MinHitCount && (MaxHitCount < 0 || ev.projCols.size() <= (size_t)MaxHitCount))
		return true;
	return false;
}



bool Proj_WormHitEvent::canMatch() const {
	if(SameWormAsProjOwner) {
		if(DiffWormAsProjOwner || DiffTeamAsProjOwner || EnemyOfProjOwner) return false;
		return true;
	}
		
	if(SameTeamAsProjOwner) {
		if(DiffTeamAsProjOwner) return false;
		if(EnemyOfProjOwner) return false;
		return true;
	}

	if(TeamMateOfProjOwner) return !EnemyOfProjOwner;
	
	return true;
}

bool Proj_WormHitEvent::match(int worm, CProjectile* prj) const {
	if(SameWormAsProjOwner && prj->GetOwner() != worm) return false;
	if(DiffWormAsProjOwner && prj->GetOwner() == worm) return false;
	
	const int team = (worm >= 0 && worm < MAX_WORMS) ?cClient->getRemoteWorms()[worm].getTeam() : -1;
	const int projTeam = (prj->GetOwner() >= 0 && prj->GetOwner() < MAX_WORMS) ? cClient->getRemoteWorms()[prj->GetOwner()].getTeam() : -1;
	if(SameTeamAsProjOwner && projTeam != team) return false;
	if(DiffTeamAsProjOwner && projTeam == team) return false;
	
	if(TeamMateOfProjOwner && !cClient->isTeamGame() && prj->GetOwner() != worm) return false; 
	if(TeamMateOfProjOwner && cClient->isTeamGame() && projTeam != team) return false; 
	if(EnemyOfProjOwner && prj->GetOwner() == worm) return false;
	if(EnemyOfProjOwner && cClient->isTeamGame() && projTeam == team) return false;
	
	return true;
}

bool Proj_WormHitEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo*) const {
	if(eventInfo.colType == NULL || !eventInfo.colType->withWorm) return false;
	return match(eventInfo.colType->wormId, prj);	
}



bool Proj_TerrainHitEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo*) const {
	if(eventInfo.colType == NULL || eventInfo.colType->withWorm) return false;
	
	const int colMask = eventInfo.colType->colMask;
	if(MapBound && (colMask & PJC_MAPBORDER) == PJC_NONE) return false;
	if(Dirt && (colMask & PJC_DIRT) == PJC_NONE) return false;
	if(Rock && colMask != PJC_TERRAIN) return false;

	return true;
}


bool Proj_DeathEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo*) const {
	return prj->getHealth() < 0;
}

bool Proj_FallbackEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, Proj_DoActionInfo* info) const {
	return !info->hasAnyEffect();
}





static void projectile_doExplode(CProjectile* const prj, int damage, int shake) {
	// Explosion
	if(damage != -1) // TODO: why only with -1?
		cClient->Explosion(prj->GetPosition(), damage, shake, prj->GetOwner());
}

static void projectile_doTimerExplode(CProjectile* const prj, int shake) {
	const proj_t *pi = prj->GetProjInfo();
	// Explosion
	int damage = pi->Timer.Damage;
	if(pi->PlyHit.Type == PJ_EXPLODE)
		damage = pi->PlyHit.Damage;

	if(damage != -1) // TODO: why only with -1?
		cClient->Explosion(prj->GetPosition(), damage, shake, prj->GetOwner());
}

static void projectile_doProjSpawn(CProjectile* const prj, const Proj_SpawnInfo* spawnInfo, AbsTime fSpawnTime) {
	//spawnInfo->dump();
	spawnInfo->apply(prj, fSpawnTime);
}

static void projectile_doMakeDirt(CProjectile* const prj) {
	const int damage = 5;
	int d = 0;
	d += cClient->getMap()->PlaceDirt(damage,prj->GetPosition()-CVec(6,6));
	d += cClient->getMap()->PlaceDirt(damage,prj->GetPosition()+CVec(6,-6));
	d += cClient->getMap()->PlaceDirt(damage,prj->GetPosition()+CVec(0,6));

	// Remove the dirt count on the worm
	if(prj->hasOwner())
		cClient->getRemoteWorms()[prj->GetOwner()].incrementDirtCount( -d );
}

static void projectile_doMakeGreenDirt(CProjectile* const prj) {
	int d = cClient->getMap()->PlaceGreenDirt(prj->GetPosition());

	// Remove the dirt count on the worm
	if(prj->hasOwner())
		cClient->getRemoteWorms()[prj->GetOwner()].incrementDirtCount( -d );
}


bool Proj_DoActionInfo::hasAnyEffect() const {
	if(explode) return true;
	if(dirt || grndirt) return true;
	if(trailprojspawn || spawnprojectiles || otherSpawns.size() > 0) return true;
	if(OverwriteOwnSpeed) return true;
	if(ChangeOwnSpeed != MatrixD2<float>(1.0f)) return true;
	if(DiffOwnSpeed != VectorD2<float>()) return true;
	if(ChangeRadius != VectorD2<int>()) return true;
	if(deleteAfter) return true;
	return false;
}

void Proj_DoActionInfo::execute(CProjectile* const prj, const AbsTime currentTime) {
	const proj_t *pi = prj->GetProjInfo();
	
	// Explode?
	if(explode) {
		if(!timer)
			projectile_doExplode(prj, damage, shake);
		else
			projectile_doTimerExplode(prj, shake);
		deleteAfter = true;
	}

	// Dirt
	if(dirt) {
		projectile_doMakeDirt(prj);
		deleteAfter = true;
	}

	// Green dirt
	if(grndirt) {
		projectile_doMakeGreenDirt(prj);
		deleteAfter = true;
	}

	if(OverwriteOwnSpeed)
		prj->setNewVel(*OverwriteOwnSpeed);
	
	prj->setNewVel( ChangeOwnSpeed * prj->GetVelocity() );
	
	prj->setNewVel( prj->GetVelocity() + DiffOwnSpeed );
	
	{
		prj->radius += ChangeRadius;
		if(prj->radius.x < 0) prj->radius.x = 0;
		if(prj->radius.y < 0) prj->radius.y = 0;
	}
	
	if(trailprojspawn) {
		// we use prj->fLastSimulationTime here to simulate the spawing at the current simulation time of this projectile
		projectile_doProjSpawn( prj, &pi->Trail.Proj, prj->fLastSimulationTime );
	}

	// Spawn any projectiles?
	if(spawnprojectiles) {
		// we use currentTime (= the simulation time of the cClient) to simulate the spawing at this time
		// because the spawing is caused probably by conditions of the environment like collision with worm/cClient->getMap()
		projectile_doProjSpawn(prj, &pi->GeneralSpawnInfo, currentTime);
	}

	for(std::list<const Proj_SpawnInfo*>::iterator i = otherSpawns.begin(); i != otherSpawns.end(); ++i) {
		// we use currentTime (= the simulation time of the cClient) to simulate the spawing at this time
		// because the spawing is caused probably by conditions of the environment like collision with worm/cClient->getMap()
		projectile_doProjSpawn(prj, *i, currentTime);
	}
	
	if(sound) {
		PlaySoundSample(sound);
	}
	
	// HINT: delete "junk projectiles" - projectiles that have no action assigned and are therefore never destroyed
	// Some bad-written mods contain those projectiles and they make the game more and more laggy (because new and new
	// projectiles are spawned and never destroyed) and prevent more important projectiles from spawning.
	// These conditions test for those projectiles and remove them
	bool hasAnyAction = pi->Hit.hasAction() || pi->PlyHit.hasAction() || pi->Timer.hasAction();
	for(size_t i = 0; i < pi->actions.size(); ++i) {
		if(hasAnyAction) break;
		hasAnyAction |= pi->actions[i].hasAction();
	}
	if (!hasAnyAction) // Isn't destroyed by any event
		if (!pi->Animating || (pi->Animating && (pi->AnimType != ANI_ONCE || pi->bmpImage == NULL))) // Isn't destroyed after animation ends
			deleteAfter = true;

	if(deleteAfter) {
		prj->setUnused();
	}
	
}

