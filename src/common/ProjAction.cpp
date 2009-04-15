/*
 *  ProjAction.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 02.04.09.
 *  code under LGPL
 *
 */

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
	GoThroughSpeed = a.GoThroughSpeed;
	UseOverwriteOwnSpeed = a.UseOverwriteOwnSpeed;
	OverwriteOwnSpeed = a.OverwriteOwnSpeed;
	ChangeOwnSpeed = a.ChangeOwnSpeed;
	UseOverwriteTargetSpeed = a.UseOverwriteTargetSpeed;
	OverwriteTargetSpeed = a.OverwriteTargetSpeed;
	ChangeTargetSpeed = a.ChangeTargetSpeed;
	if(a.additionalAction) additionalAction = new Proj_Action(*a.additionalAction);
	
	return *this;
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
			if(UseSound)
				info->playSound = true;
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
				info->deleteAfter = true;
			}
			push_worm = false;
			if(eventInfo.timerHit) spawnprojs = false;
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
	
	if(UseOverwriteOwnSpeed)
		info->OverwriteOwnSpeed = &OverwriteOwnSpeed;
	
	if(ChangeOwnSpeed != MatrixD2<float>(1.0f))
		info->ChangeOwnSpeed = &ChangeOwnSpeed;
	
	if(UseOverwriteTargetSpeed) {
		if(eventInfo.colType && eventInfo.colType->withWorm) {
			cClient->getWorm(eventInfo.colType->wormId)->velocity() = OverwriteTargetSpeed;
		}
		
		for(std::set<CProjectile*>::const_iterator p = eventInfo.projCols.begin(); p != eventInfo.projCols.end(); ++p) {
			(*p)->setNewVel(OverwriteTargetSpeed);
		}
	}
	
	if(ChangeTargetSpeed != MatrixD2<float>(1.0f)) {
		if(eventInfo.colType && eventInfo.colType->withWorm) {
			CVec& v = cClient->getWorm(eventInfo.colType->wormId)->velocity();
			v = ChangeTargetSpeed * v;
		}
		
		for(std::set<CProjectile*>::const_iterator p = eventInfo.projCols.begin(); p != eventInfo.projCols.end(); ++p) {
			(*p)->setNewVel( ChangeTargetSpeed * (*p)->GetVelocity() );
		}
	}
	
	if(additionalAction) {
		Proj_EventOccurInfo ev(eventInfo);
		ev.timerHit = false; // remove LX56 timer flag
		additionalAction->applyTo(ev, prj, info);
	}
}



bool Proj_LX56Timer::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj) const {
	float f = prj->getTimeVarRandom();
	if(Time > 0 && (Time + TimeVar * f) < prj->getLife()) {
		eventInfo.timerHit = true;
		return true;
	}
	return false;
}

bool Proj_TimerEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj) const {
	float& last = prj->timerInfo[this];
	if(last > 0 && !Repeat) return false;
	
	if(UseGlobalTime) {		
		float cur = eventInfo.serverTime.seconds() * (float)cClient->getGameLobby()->features[FT_GameSpeed];
		if(last == 0) {
			float startTime = cur - prj->getLife();
			float mstart = startTime; FMOD(mstart, Delay);
			float next = startTime - mstart + Delay;
			if(cur >= next) {
				last = cur;
				return true;
			}
		}
		else {
			float mcur = last; FMOD(last, Delay);
			float next = last - mcur + Delay;			
			if(cur >= next) {
				last = cur;
				return true;
			}
		}
	}
	else { // not global time
		if(last + Delay <= prj->getLife()) {
			last = prj->getLife();
			return true;
		}
	}
	
	return false;
}


static bool checkProjHit(const Proj_ProjHitEvent& info, Proj_EventOccurInfo& ev, CProjectile* prj, CProjectile* p) {
	if(p == prj) return true;
	if(info.Target && p->getProjInfo() != info.Target) return true;
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

bool Proj_ProjHitEvent::checkEvent(Proj_EventOccurInfo& ev, CProjectile* prj) const {
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
		if(DiffWormAsProjOwner || DiffTeamAsProjOwner) return false;
		return true;
	}
	
	if(DiffWormAsProjOwner) {
		return !SameTeamAsProjOwner;
	}
	
	if(SameTeamAsProjOwner) return !DiffTeamAsProjOwner;
	
	return true;
}

bool Proj_WormHitEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj) const {
	if(eventInfo.colType == NULL || !eventInfo.colType->withWorm) return false;
	const int worm = eventInfo.colType->wormId;
	const int team = cClient->getWorm(worm)->getTeam();
	
	if(SameWormAsProjOwner && prj->GetOwner() != worm) return false;
	if(DiffWormAsProjOwner && prj->GetOwner() == worm) return false;
	
	if(prj->GetOwner() < 0 || prj->GetOwner() >= MAX_WORMS)
		return SameTeamAsProjOwner;
	
	const int projTeam = cClient->getWorm(prj->GetOwner())->getTeam();
	if(SameTeamAsProjOwner && projTeam != team) return false;
	if(DiffTeamAsProjOwner && projTeam == team) return false;
	
	return true;
}



bool Proj_TerrainHitEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj) const {
	if(eventInfo.colType == NULL || eventInfo.colType->withWorm) return false;
	
	const int colMask = eventInfo.colType->colMask;
	if(MapBound && (colMask & PJC_MAPBORDER) == PJC_NONE) return false;
	if(Dirt && (colMask & PJC_DIRT) == PJC_NONE) return false;
	if(Rock && colMask != PJC_TERRAIN) return false;

	return true;
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
	
	if(ChangeOwnSpeed)
		prj->setNewVel( *ChangeOwnSpeed * prj->GetVelocity() );
	
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
	
	if(playSound) {
		PlaySoundSample(pi->smpSample);
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

