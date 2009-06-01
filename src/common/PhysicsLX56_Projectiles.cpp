/*
	LX56 with extensions - Projectile simulation

	merged code of ProjAction and CProjectile

	code under LGPL
	created by Albert Zeyer on 22-05-2009
*/


/*
 
 This file includes the main code of LX56 projectile simulation - much cleaned up and extended.
 
 For a reference, look here:
 http://openlierox.svn.sourceforge.net/viewvc/openlierox/src/common/CProjectile.cpp?revision=1&view=markup&pathrev=1
 SimulateProjectiles in:
 http://openlierox.svn.sourceforge.net/viewvc/openlierox/src/client/CClient_Game.cpp?revision=1&view=markup&pathrev=1
 
 */

#include <cmath>
#include "ProjAction.h"
#include "CGameScript.h"
#include "CWorm.h"
#include "CProjectile.h"
#include "CClient.h"
#include "ProjectileDesc.h"
#include "Physics.h"
#include "PhysicsLX56.h"
#include "ConfigHandler.h"
#include "EndianSwap.h"
#include "Geometry.h"
#include "ThreadPool.h" // for struct Action


/*
	This small struct is intended to give some performance improvement for the simulation.
	It is expected that this struct is in most cases very similar (or parts of it).
	In LX56ProjectileHandler and most subfunctions, we use the information in this struct
	rather than the direct dynamic information in the CProjectile.

	The idea is to have some special LX56ProjectileHandler implementations where parts
	of these attributes are constant, so the compiler can optimise out a lot (for these
	special LX56ProjectileHandler implementations only). That only works of course if
	everything is inlined (everything from LX56ProjectileHandler::doFrame down to the
	specific function, which would benefit from these attributes being constant).
*/

struct LX56ProjAttribs {
	VectorD2<int> radius;
	const proj_t* projInfo;
};



///////////////////
// Lower level projectile-worm collision test
inline int CProjectile::ProjWormColl(const LX56ProjAttribs& attribs, CVec pos, CWorm *worms)
{
	Shape<int> s; s.pos = pos; s.radius = attribs.radius;
	if(attribs.projInfo->Type == PRJ_CIRCLE)
		s.type = Shape<int>::ST_CIRCLE;
	else {
		// that's LX56 behaviour...
		if(s.radius.x <= 2) s.radius.x = 0;
		if(s.radius.y <= 2) s.radius.y = 0;
	}
	
	CWorm* ownerWorm = NULL;
	if(hasOwner()) {
		ownerWorm = &worms[GetOwner()];
		if(!ownerWorm->isUsed())
			ownerWorm = NULL;
	}
	
	CWorm *w = worms;
	for(short i=0;i<MAX_WORMS;i++,w++) {
		if(!w->isUsed() || !w->getAlive())
			continue;
		
		if(ownerWorm && cClient->isTeamGame() && !cClient->getGameLobby()->features[FT_TeamHit] && w != ownerWorm && w->getTeam() == ownerWorm->getTeam())
			continue;
		
		if(ownerWorm && !cClient->getGameLobby()->features[FT_SelfHit] && w == ownerWorm)
			continue;
		
		const static int wsize = 4;
		Shape<int> worm; worm.pos = w->getPos(); worm.radius = VectorD2<int>(wsize, wsize);
		
		if(s.CollisionWith(worm)) {
			
			CollisionSide = 0;
			
			// Calculate the side of the collision (obsolete??)
			if(s.pos.x < worm.pos.x-2)
				CollisionSide |= COL_LEFT;
			else if(s.pos.x > worm.pos.x+2)
				CollisionSide |= COL_RIGHT;
			if(s.pos.y < worm.pos.y-2)
				CollisionSide |= COL_TOP;
			else if(s.pos.y > worm.pos.y+2)
				CollisionSide |= COL_BOTTOM;
			
			return i;
		}
	}
	
	// No worm was hit
	return -1;
}


static inline bool CProjectile_CollisionWith(const CProjectile* src, const CProjectile* target, const LX56ProjAttribs& src_attribs, int src_rx, int src_ry) {
	Shape<int> s1; s1.pos = src->GetPosition(); s1.radius.x = src_rx; s1.radius.y = src_ry;
	Shape<int> s2; s2.pos = target->GetPosition(); s2.radius = target->getRadius();
	if(src_attribs.projInfo->Type == PRJ_CIRCLE) s1.type = Shape<int>::ST_CIRCLE;
	if(target->getProjInfo()->Type == PRJ_CIRCLE) s2.type = Shape<int>::ST_CIRCLE;
	
	return s1.CollisionWith(s2);
}

static inline bool CProjectile_CollisionWith(const CProjectile* src, const CProjectile* target, const LX56ProjAttribs& src_attribs) {
	return CProjectile_CollisionWith(src, target, src_attribs, src_attribs.radius.x, src_attribs.radius.y);
}



inline ProjCollisionType FinalWormCollisionCheck(CProjectile* proj, const LX56ProjAttribs& attribs, const CVec& vFrameOldPos, const CVec& vFrameOldVel, CWorm* worms, TimeDiff dt, ProjCollisionType curResult) {
	// do we get any worm?
	if(proj->GetProjInfo()->PlyHit.Type != PJ_NOTHING) {
		CVec dif = proj->GetPosition() - vFrameOldPos;
		float len = NormalizeVector( &dif );
		
		// the worm has a size of 4*4 in ProjWormColl, so it's save to check every second pixel here
		for (float p = 0.0f; p <= len; p += 2.0f) {
			CVec curpos = vFrameOldPos + dif * p;
			
			int ret = proj->ProjWormColl(attribs, curpos, worms);
			if (ret >= 0)  {
				if(proj->GetProjInfo()->PlyHit.Type != PJ_GOTHROUGH) {
					proj->setNewPosition( curpos ); // save the new position at the first collision
					proj->setNewVel( vFrameOldVel ); // don't get faster
				}
				return ProjCollisionType::Worm(ret);
			}
		}
	}
	
	return curResult;
}


///////////////////////
// Checks for collision with the level border
inline bool CProjectile::MapBoundsCollision(const LX56ProjAttribs& attribs, int px, int py)
{
	CMap* map = cClient->getMap();
	CollisionSide = 0;
	
	if (px - attribs.radius.x < 0)
		CollisionSide |= COL_LEFT;
	
	if (px + attribs.radius.x >= (int)map->GetWidth())
		CollisionSide |= COL_RIGHT;
	
	if (py - attribs.radius.y < 0)
		CollisionSide |= COL_TOP;
	
	if (py + attribs.radius.y >= (int)map->GetHeight())
		CollisionSide |= COL_BOTTOM;
	
	return CollisionSide != 0;
}

////////////////////////////
// Checks for collision with the terrain
// WARNING: assumed to be called only from SimulateFrame
inline CProjectile::ColInfo CProjectile::TerrainCollision(const LX56ProjAttribs& attribs, int px, int py)
{
	CMap* map = cClient->getMap();
	
	ColInfo res = { 0, 0, 0, 0, false, true };
	
	// if we are small, we can make a fast check
	if(radius.x*2 < map->getGridWidth() && radius.y*2 < map->getGridHeight()) {
		// If the current cells are empty, don't check for the collision
		const int gf1 = (py - attribs.radius.y) / map->getGridHeight() * map->getGridCols() + (px - attribs.radius.x) / map->getGridWidth();
		const int gf2 = (py - attribs.radius.y) / map->getGridHeight() * map->getGridCols() + (px + attribs.radius.x) / map->getGridWidth();
		const int gf3 = (py + attribs.radius.y) / map->getGridHeight() * map->getGridCols() + (px - attribs.radius.x) / map->getGridWidth();
		const int gf4 = (py + attribs.radius.y) / map->getGridHeight() * map->getGridCols() + (px + attribs.radius.x) / map->getGridWidth();
		const uchar *pf = map->getAbsoluteGridFlags();
		if ((pf[gf1] | pf[gf2] | pf[gf3] | pf[gf4]) == PX_EMPTY)
			return res;
	}
	
	// Check for the collision
	for(int y = py - attribs.radius.y; y <= py + attribs.radius.y; ++y) {
		// this is safe because in SimulateFrame, we do map bound checks
		uchar *pf = map->GetPixelFlags() + y * map->GetWidth() + px - attribs.radius.x;
		
		for(int x = px - attribs.radius.x; x <= px + attribs.radius.x; ++x, ++pf) {
			
			if(tProjInfo->Type == PRJ_CIRCLE && (VectorD2<int>(x,y) - VectorD2<int>(px,py)).GetLength2() > attribs.radius.GetLength2())
				// outside the range, skip this
				continue;
			
			// Solid pixel
			if(*pf & (PX_DIRT|PX_ROCK)) {
				if (y < py)
					++res.top;
				else if (y > py)
					++res.bottom;
				if (x < px)
					++res.left;
				else if (x > px)
					++res.right;
				
				if (*pf & PX_ROCK)
					res.onlyDirt = false;
				res.collided = true;
			}
		}
	}
	
	return res;
}

////////////////////////
// Handle the terrain collsion (helper function)
// returns false if collision should be ignored
bool CProjectile::HandleCollision(const LX56ProjAttribs& attribs, const CProjectile::ColInfo &c, const CVec& oldpos, const CVec& oldvel, TimeDiff dt)
{
	
	if(tProjInfo->Hit.Type == PJ_EXPLODE && c.onlyDirt) {
		// HINT: don't reset vPosition here, because we want
		//		the explosion near (inside) the object
		//		this behavior is the same as in original LX
		return true;
	}
	
	bool bounce = false;
	
	// Bit of a hack
	switch (tProjInfo->Hit.Type)  {
	case PJ_BOUNCE:
			// HINT: don't reset vPosition here; it will be reset,
			//		depending on the collisionside
		bounce = true;
		break;
	case PJ_NOTHING:  // PJ_NOTHING projectiles go through walls (but a bit slower)
		vPosition = oldpos + (vVelocity * dt.seconds()) * 0.5f;
		vOldPos = vPosition; // TODO: this is a hack; we do it to not go back to real old position because of collision
			// HINT: The above velocity reduction is not exact. SimulateFrame is also executed only for one checkstep because of the collision.
		break;
	case PJ_GOTHROUGH:
		vPosition = oldpos + (vVelocity * dt.seconds()) * tProjInfo->Hit.GoThroughSpeed;
		return false; // ignore collision
	default:
		vPosition = vOldPos;
		vVelocity = oldvel;
		return true;
	}
	
	int vx = (int)vVelocity.x;
	int vy = (int)vVelocity.y;
	
	// Find the collision side
	if ((c.left > c.right || c.left > 2) && c.left > 1 && vx <= 0) {
		if(bounce)
			vPosition.x = oldpos.x;
		if (vx)
			CollisionSide |= COL_LEFT;
	}
	
	if ((c.right > c.left || c.right > 2) && c.right > 1 && vx >= 0) {
		if(bounce)
			vPosition.x = oldpos.x;
		if (vx)
			CollisionSide |= COL_RIGHT;
	}
	
	if (c.top > 1 && vy <= 0) {
		if(bounce)
			vPosition.y = oldpos.y;
		if (vy)
			CollisionSide |= COL_TOP;
	}
	
	if (c.bottom > 1 && vy >= 0) {
		if(bounce)
			vPosition.y = oldpos.y;
		if (vy)
			CollisionSide |= COL_BOTTOM;
	}
	
	// If the velocity is too low, just stop me
	if (abs(vx) < 2)
		vVelocity.x = 0;
	if (abs(vy) < 2)
		vVelocity.y = 0;
	
	return true;
}

///////////////////
// Check for a collision, updates velocity and position
// TODO: move to physicsengine
// TODO: we need one single CheckCollision which is used everywhere in the code
// atm we have two CProj::CC, Map:CC and ProjWormColl and fastTraceLine
// we should complete the function in CMap.cpp in a general way by using fastTraceLine
// also dt shouldn't be a parameter, you should specify a start- and an endpoint
// (for example CWorm_AI also uses this to check some possible cases)
inline ProjCollisionType LX56Projectile_checkCollAndMove_Frame(CProjectile* const prj, const LX56ProjAttribs& attribs, TimeDiff dt, CMap *map, CWorm* worms)
{
	// Gravity
	float fGravity = 100.0f; // Default
	if (prj->getProjInfo()->UseCustomGravity)
		fGravity = (float)prj->getProjInfo()->Gravity;
	prj->vVelocity.y += fGravity * dt.seconds();

	CVec vOldVel = prj->GetVelocity();

	CVec vFrameOldPos = prj->vPosition;
	prj->vPosition += prj->vVelocity * dt.seconds();
	
	// if distance is to short to last check, just return here without a check
	if ((int)(prj->vOldPos - prj->vPosition).GetLength2() < prj->MIN_CHECKSTEP2) {
/*		printf("pos dif = %f , ", (vOldPos - vPosition).GetLength());
		printf("len = %f , ", sqrt(len));
		printf("vel = %f , ", vVelocity.GetLength());
		printf("mincheckstep = %i\n", MIN_CHECKSTEP);	*/
		return FinalWormCollisionCheck(prj, attribs, vFrameOldPos, vOldVel, worms, dt, ProjCollisionType::NoCol());
	}
	
	int px = (int)(prj->vPosition.x);
	int py = (int)(prj->vPosition.y);
	
	// Hit edges
	if (prj->MapBoundsCollision(attribs, px, py))  {
		prj->vPosition = prj->vOldPos;
		prj->vVelocity = vOldVel;
		
		return FinalWormCollisionCheck(prj, attribs, vFrameOldPos, vOldVel, worms, dt, ProjCollisionType::Terrain(PJC_TERRAIN|PJC_MAPBORDER));
	}
	
	// Make wallshooting possible
	// NOTE: wallshooting is a bug in old LX physics that many players got used to
	if (prj->fLastSimulationTime <= prj->fSpawnTime + TimeDiff(prj->fWallshootTime))
		return FinalWormCollisionCheck(prj, attribs, vFrameOldPos, vOldVel, worms, dt, ProjCollisionType::NoCol());
	
	// Check collision with the terrain
	CProjectile::ColInfo c = prj->TerrainCollision(attribs, px, py);
	
	// Check for a collision
	if(c.collided && prj->HandleCollision(attribs, c, vFrameOldPos, vOldVel, dt)) {
		int colmask = PJC_TERRAIN;
		if(c.onlyDirt) colmask |= PJC_DIRT;
		return FinalWormCollisionCheck(prj, attribs, vFrameOldPos, vOldVel, worms, dt, ProjCollisionType::Terrain(colmask));
	}
	
	// the move was safe, save the position
	prj->vOldPos = prj->vPosition;
	
	return FinalWormCollisionCheck(prj, attribs, vFrameOldPos, vOldVel, worms, dt, ProjCollisionType::NoCol());
}

template <typename VectorType>
static void Projectile_HandleGravityForObject(CProjectile* const prj, TimeDiff dt, VectorType objpos, VectorType& objvel)
{
	const proj_t *info = prj->getProjInfo();

	// Handler for fastTraceLine

	// Check that it is visible if the gravity does not go through walls
	if (!info->MyGravityThroughWalls)  {
		if(!fastTraceLine_hasAnyCollision(objpos, prj->GetPosition(), PX_DIRT | PX_ROCK))
			return;
	}

	// Drag the object to us
	CVec force = prj->GetPosition() - objpos;
	float drag = (float)info->MyGravityForce;
	if (info->MyGravityFadeOut)  {  // Lower force if the object is far away
		static const int maxGravityRadius = 5800;  // sqrt(4096^2 + 4096^2)
		float r = (info->MyGravityRadius <= 0) ? maxGravityRadius : info->MyGravityRadius;
		drag *= 1.0f - force.GetLength() / r;
	}

	objvel += force.Normalize() * drag * dt.seconds();

	// Clamp to prevent weird bugs caused by high velocities
	objvel.x = CLAMP(objvel.x, -500.0f, 500.0f);
	objvel.y = CLAMP(objvel.y, -500.0f, 500.0f);
}

static void Projectile_HandleMyGravityForWorms(CProjectile* const prj, TimeDiff dt, CWorm *worms)
{
	// Check
	const proj_t *info = prj->getProjInfo();
	if (info->MyGravityType != GRV_PLAYER && info->MyGravityType != GRV_BOTH)
		return;

	// Go through the worms and check if there are some in the radius
	for (int i = 0; i < MAX_WORMS; i++)  {
		CWorm *w = &worms[i];

		// Only playing worms
		if (!w->isUsed() || !w->getAlive() || w->getLives() == WRM_OUT)
			continue;

		// In radius?
		if (info->MyGravityRadius > 0 && 
			(int)((w->getPos() - prj->GetPosition()).GetLength()) > info->MyGravityRadius)
			continue;

		Projectile_HandleGravityForObject(prj, dt, w->getPos(), w->velocity());
	}
}

void Projectile_HandleMyGravityForProjectiles(CProjectile* const prj, TimeDiff dt)
{
	// Check
	const proj_t *info = prj->getProjInfo();
	if (info->MyGravityType != GRV_PROJECTILE && info->MyGravityType != GRV_BOTH)
		return;

	// Applies to all projectiles
	if (info->MyGravityRadius <= 0)  {
		for(Iterator<CProjectile*>::Ref i = cClient->getProjectiles().begin(); i->isValid(); i->next()) {
			if (i->get() != prj)
				Projectile_HandleGravityForObject(prj, dt, i->get()->GetPosition(), i->get()->vVelocity);
		}

	// Only projectiles in the given range
	} else {
		float r2 = info->MyGravityRadius * info->MyGravityRadius;

		for(Iterator<CProjectile*>::Ref i = cClient->getProjectiles().begin(); i->isValid(); i->next()) {
			if (i->get() != prj && (prj->GetPosition() - i->get()->GetPosition()).GetLength2() <= r2)
				Projectile_HandleGravityForObject(prj, dt, i->get()->GetPosition(), i->get()->vVelocity);
		}
	}
}

inline ProjCollisionType LX56Projectile_checkCollAndMove(CProjectile* const prj, const LX56ProjAttribs& attribs, TimeDiff dt, CMap *map, CWorm* worms) {
	// Check if we need to recalculate the checksteps (projectile changed its velocity too much)
	if (prj->bChangesSpeed)  {
		int len = (int)prj->vVelocity.GetLength2();
		if (abs(len - prj->iCheckSpeedLen) > 50000)
			prj->CalculateCheckSteps();
	}

	// Check for worms that we should attract with gravity
	if (prj->tProjInfo->MyGravityForce != 0 && prj->tProjInfo->MyGravityType != GRV_NONE)  {
		Projectile_HandleMyGravityForProjectiles(prj, dt);
		Projectile_HandleMyGravityForWorms(prj, dt, worms);
	}
	
	/*
	 In LX56, we skipped the simulation into half if dt >= 0.015f.
	 The dt we have here is usually lower (at least LX56PhysicsDT is), so it's like
	 LX56 to do the damping just once here.
	 */
	
	{
		// Dampening
		float dmp = prj->getProjInfo()->Dampening;
		if(dmp != 1.0f) {
			// dt is not fixed (because of possible gamespeed factor)
			if(dt == LX56PhysicsDT)
				prj->vVelocity *= dmp;
			else if(dt > 0)
				prj->vVelocity *= powf(dmp, dt.seconds() / LX56PhysicsDT.seconds());
			else if(dt < 0)
				// doesn't make sense to do negative dampening...
				prj->vVelocity *= powf(dmp, -dt.seconds() / LX56PhysicsDT.seconds());
		}
	}

	TimeDiff cstep;
	{
		float checkstep = prj->vVelocity.GetLength2(); // |v|^2
		
		if((int)(checkstep * dt.seconds() * dt.seconds()) <= prj->MAX_CHECKSTEP2) // |dp|^2=|v*dt|^2
			return LX56Projectile_checkCollAndMove_Frame(prj, attribs, dt, map, worms);		
		
		// calc new dt, so that we have |v*dt|=AVG_CHECKSTEP
		// checkstep is new dt
		checkstep = (float)prj->AVG_CHECKSTEP / sqrt(checkstep);
		checkstep = MAX(checkstep, 0.001f);
		cstep = TimeDiff(checkstep);
	}
	
	// In some bad cases (float accurance problems mainly),
	// it's possible that checkstep >= dt .
	// If we would not check this case, we get in an infinie
	// recursive loop.
	// Therefore if this is the case, we don't do multiple checksteps.
	if(cstep < dt) {
		for(TimeDiff time; time < dt; time += cstep) {
			ProjCollisionType ret = LX56Projectile_checkCollAndMove_Frame(prj, attribs, (time + cstep > dt) ? dt - time : cstep, map, worms);
			if(ret) return ret;
		}
		
		return ProjCollisionType::NoCol();
	}
	
	return LX56Projectile_checkCollAndMove_Frame(prj, attribs, dt, map, worms);		
}



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
	DestroyAfter = a.DestroyAfter;
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
	HeadingToOwnerSpeed = a.HeadingToOwnerSpeed;
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
			info->deleteAfter = DestroyAfter;
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
	
	if(HeadingToOwnerSpeed && prj->GetOwner() >= 0 && prj->GetOwner() < MAX_WORMS) {
		info->ChangeOwnSpeed = getVelChangeForProj(&cClient->getRemoteWorms()[prj->GetOwner()], prj, HeadingToOwnerSpeed) * info->ChangeOwnSpeed;
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



bool Proj_LX56Timer::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, const LX56ProjAttribs& attribs, Proj_DoActionInfo*) const {
	float f = prj->getTimeVarRandom();
	if(Time > 0 && (Time + TimeVar * f) < prj->getLife()) {
		eventInfo.timerHit = true;
		return true;
	}
	return false;
}

bool Proj_TimerEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, const LX56ProjAttribs& attribs, Proj_DoActionInfo*) const {
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


static inline bool checkProjHit(const Proj_ProjHitEvent& info, Proj_EventOccurInfo& ev, CProjectile* prj, const LX56ProjAttribs& attribs, CProjectile* p) {
	if(p == prj) return true;
	if(info.Target && p->getProjInfo() != info.Target) return true;
	if(!info.ownerWorm.match(prj->GetOwner(), p)) return true;
	if(info.TargetHealthIsMore && p->getHealth() <= prj->getHealth()) return true;
	if(info.TargetHealthIsLess && p->getHealth() >= prj->getHealth()) return true;
	if(info.TargetTimeIsMore && p->getLife() <= prj->getLife()) return true;
	if(info.TargetTimeIsLess && p->getLife() >= prj->getLife()) return true;
	
	if(info.Width >= 0 && info.Height >= 0) { if(!CProjectile_CollisionWith(prj, p, attribs, info.Width/2, info.Height/2)) return true; }
	else { if(!CProjectile_CollisionWith(prj, p, attribs)) return true; }
	
	ev.projCols.insert(p);
	
	if(info.MaxHitCount < 0 && ev.projCols.size() >= (size_t)info.MinHitCount) return false; // no need to check further
	if(ev.projCols.size() > (size_t)info.MaxHitCount) return false; // no need to check further
	
	return true;
}

template<bool TOP, bool LEFT>
static CClient::MapPosIndex MPI(const VectorD2<int>& p, const VectorD2<int>& r) {
	return CClient::MapPosIndex( p + VectorD2<int>(LEFT ? -r.x : r.x, TOP ? -r.y : r.y) );
}

bool Proj_ProjHitEvent::checkEvent(Proj_EventOccurInfo& ev, CProjectile* prj, const LX56ProjAttribs& attribs, Proj_DoActionInfo*) const {
	const VectorD2<int> vPosition = prj->GetPosition();
	const VectorD2<int> radius = prj->getRadius();
	for(int x = MPI<true,true>(vPosition,radius).x; x <= MPI<true,false>(vPosition,radius).x; ++x)
		for(int y = MPI<true,true>(vPosition,radius).y; y <= MPI<false,true>(vPosition,radius).y; ++y) {
			CClient::ProjectileSet* projs = cClient->projPosMap[CClient::MapPosIndex(x,y).index(cClient->getMap())];
			if(projs == NULL) continue;
			for(CClient::ProjectileSet::const_iterator p = projs->begin(); p != projs->end(); ++p)
				if(!checkProjHit(*this, ev, prj, attribs, *p)) goto finalChecks;
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

bool Proj_WormHitEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, const LX56ProjAttribs& attribs, Proj_DoActionInfo*) const {
	if(eventInfo.colType == NULL || !eventInfo.colType->withWorm) return false;
	return match(eventInfo.colType->wormId, prj);
}



bool Proj_TerrainHitEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, const LX56ProjAttribs&, Proj_DoActionInfo*) const {
	if(eventInfo.colType == NULL || eventInfo.colType->withWorm) return false;
	
	const int colMask = eventInfo.colType->colMask;
	if(MapBound && (colMask & PJC_MAPBORDER) == PJC_NONE) return false;
	if(Dirt && (colMask & PJC_DIRT) == PJC_NONE) return false;
	if(Rock && colMask != PJC_TERRAIN) return false;
	
	return true;
}


bool Proj_DeathEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, const LX56ProjAttribs&, Proj_DoActionInfo*) const {
	return prj->getHealth() < 0;
}

bool Proj_FallbackEvent::checkEvent(Proj_EventOccurInfo& eventInfo, CProjectile* prj, const LX56ProjAttribs&, Proj_DoActionInfo* info) const {
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
	
	if(ChangeRadius.x || ChangeRadius.y) {
		prj->radius += ChangeRadius;
		if(prj->radius.x < 0) prj->radius.x = 0;
		if(prj->radius.y < 0) prj->radius.y = 0;
		prj->setBestLX56Handler(); // because LX56Handler could depend on radius
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



static inline ProjCollisionType LX56_simulateProjectile_LowLevel(AbsTime currentTime, TimeDiff dt, CProjectile* proj, const LX56ProjAttribs& attribs, CWorm *worms, bool* projspawn, bool* deleteAfter) {
	// If this is a remote projectile, we have already set the correct fLastSimulationTime
	//proj->setRemote( false );

	// Check for collisions
	// ATENTION: dt will manipulated directly here!
	// TODO: use a more general CheckCollision here
	ProjCollisionType res = LX56Projectile_checkCollAndMove(proj, attribs, dt, cClient->getMap(), worms);
	
	proj->life() += dt.seconds();
	proj->extra() += dt.seconds();
	
	
	
	/*
		vOldPos = vPosition;
	*/
	
	const proj_t *pi = proj->GetProjInfo();
	
	if(pi->Rotating)  {
		proj->rotation() += (float)pi->RotSpeed * dt.seconds();
		FMOD(proj->rotation(), 360.0f);
	}
	
		// Animation
	if(pi->Animating) {
		if(proj->getFrameDelta())
			proj->frame() += (float)pi->AnimRate * dt.seconds();
		else
			proj->frame() -= (float)pi->AnimRate * dt.seconds();
		
		if(pi->bmpImage) {
			int NumFrames = pi->bmpImage->w / pi->bmpImage->h;
			if(proj->frame() >= NumFrames) {
				switch(pi->AnimType) {
				case ANI_ONCE:
					*deleteAfter = true;
					break;
				case ANI_LOOP:
					proj->frame() = 0;
					break;
				case ANI_PINGPONG:
					proj->setFrameDelta( ! proj->getFrameDelta() );
					proj->frame() = (float)NumFrames - 1;
					break;
				case __ANI_LBOUND: case __ANI_UBOUND: errors << "simulateProjectile_LowLevel: hit __ANI_BOUND" << endl;
				}
			}
			else if(proj->frame() < 0) {
				if(proj->getProjInfo()->AnimType == ANI_PINGPONG) {
					proj->setFrameDelta( ! proj->getFrameDelta() );
					proj->frame() = 0.0f;
				}
			}
		}
	}
	
		// Trails
	switch(pi->Trail.Type) {
	case TRL_NONE: break;
	case TRL_SMOKE:
		if(proj->extra() >= 0.075f) {
			proj->extra() = 0.0f;
			SpawnEntity(ENT_SMOKE,0,proj->GetPosition(),CVec(0,0),0,NULL);
		}
		break;
	case TRL_CHEMSMOKE:
		if(proj->extra() >= 0.075f) {
			proj->extra() = 0.0;
			SpawnEntity(ENT_CHEMSMOKE,0,proj->GetPosition(),CVec(0,0),0,NULL);
		}
		break;
	case TRL_DOOMSDAY:
		if(proj->extra() >= 0.05f) {
			proj->extra() = 0.0;
			SpawnEntity(ENT_DOOMSDAY,0,proj->GetPosition(),proj->GetVelocity(),0,NULL);
		}
		break;
	case TRL_EXPLOSIVE:
		if(proj->extra() >= 0.05f) {
			proj->extra() = 0.0;
			SpawnEntity(ENT_EXPLOSION,10,proj->GetPosition(),CVec(0,0),0,NULL);
		}
		break;
	case TRL_PROJECTILE: // Projectile trail
		if(currentTime > proj->lastTrailProj()) {
			proj->lastTrailProj() = currentTime + pi->Trail.Delay / (float)cClient->getGameLobby()->features[FT_GameSpeed];
			
			*projspawn = true;
		}
		break;
	case __TRL_LBOUND: case __TRL_UBOUND: errors << "simulateProjectile_LowLevel: hit __TRL_BOUND" << endl;
	}
	return res;
}


struct LX56ProjectileHandler {
	virtual ~LX56ProjectileHandler() {}

	static inline bool doFrame1(const AbsTime currentTime, TimeDiff dt, const proj_t& projInfo, CProjectile* const prj, const LX56ProjAttribs& attribs) {
		Proj_DoActionInfo doActionInfo;
		TimeDiff serverTime = cClient->serverTime();
		{
			TimeDiff timeDiff = currentTime - prj->fLastSimulationTime;
			if(timeDiff >= serverTime)
				serverTime = TimeDiff(0); // strange case
			else
				serverTime -= timeDiff;
		}
		
		// Check if the timer is up
		projInfo.Timer.checkAndApply(Proj_EventOccurInfo::Unspec(serverTime, dt), prj, attribs, &doActionInfo);
		
		// Simulate the projectile
		ProjCollisionType result = LX56_simulateProjectile_LowLevel( prj->fLastSimulationTime, dt, prj, attribs, cClient->getRemoteWorms(), &doActionInfo.trailprojspawn, &doActionInfo.deleteAfter );
		
		Proj_EventOccurInfo eventInfo = Proj_EventOccurInfo::Col(serverTime, dt, &result);
		
		/*
		===================
		Terrain Collision
		===================
		*/
		if( !result.withWorm && (result.colMask & PJC_TERRAIN) ) {
			projInfo.Hit.applyTo(eventInfo, prj, &doActionInfo);
		}
		
		/*
		===================
		Worm Collision
		===================
		*/
		if( result.withWorm && !doActionInfo.explode) {
			bool preventSelfShooting = ((int)result.wormId == prj->GetOwner());
			preventSelfShooting &= (prj->getIgnoreWormCollBeforeTime() > prj->fLastSimulationTime); // if the simulation is too early, ignore this worm col
			if( !preventSelfShooting || NewNet::Active() ) {
				projInfo.PlyHit.applyTo(eventInfo, prj, &doActionInfo);
			}
		}
		
		if(!result) eventInfo = Proj_EventOccurInfo::Unspec(serverTime, dt);
		
		for(size_t i = 0; i < projInfo.actions.size(); ++i) {
			projInfo.actions[i].checkAndApply(eventInfo, prj, attribs, &doActionInfo);
		}
		
		doActionInfo.execute(prj, currentTime);
		return !doActionInfo.deleteAfter;
	}

	static inline bool doFrame(const AbsTime currentTime, TimeDiff dt, const proj_t& projInfo, CProjectile* const prj, const LX56ProjAttribs& attribs) {
		// This case (for dt) is the most often case, so provide an optimised version for it.
		if(dt == LX56PhysicsDT)
			return doFrame1(currentTime, LX56PhysicsDT, projInfo, prj, attribs);
		else
			return doFrame1(currentTime, dt, projInfo, prj, attribs);			
	}
	
	// returns true if we should continue
	virtual bool frame(const AbsTime currentTime, TimeDiff dt, CProjectile* const prj) = 0;
};

struct LX56ProjHandler_Generic : LX56ProjectileHandler {
	bool frame(const AbsTime currentTime, TimeDiff dt, CProjectile* const prj) {
		LX56ProjAttribs attribs = { prj->getRadius(), prj->getProjInfo() };
		return doFrame(currentTime, dt, *prj->GetProjInfo(), prj, attribs);
	}
} lx56default;

struct LX56ProjHandler_Radius1 : LX56ProjectileHandler {
	bool frame(const AbsTime currentTime, TimeDiff dt, CProjectile* const prj) {
		LX56ProjAttribs attribs = { VectorD2<int>(1,1), prj->getProjInfo() };
		return doFrame(currentTime, dt, *prj->GetProjInfo(), prj, attribs);
	}
} lx56radius1;

struct LX56ProjHandler_Radius2 : LX56ProjectileHandler {
	bool frame(const AbsTime currentTime, TimeDiff dt, CProjectile* const prj) {
		LX56ProjAttribs attribs = { VectorD2<int>(2,2), prj->getProjInfo() };
		return doFrame(currentTime, dt, *prj->GetProjInfo(), prj, attribs);
	}
} lx56radius2;




static void LX56_simulateProjectile(const AbsTime currentTime, CProjectile* const prj) {
	static const TimeDiff orig_dt = LX56PhysicsDT;
	const TimeDiff dt = orig_dt * (float)cClient->getGameLobby()->features[FT_GameSpeed];
	
	VectorD2<int> oldPos(prj->GetPosition());
	VectorD2<int> oldRadius(prj->getRadius());
	
simulateProjectileStart:
	if(prj->fLastSimulationTime + orig_dt > currentTime) goto finalMapPosIndexUpdate;
	prj->fLastSimulationTime += orig_dt;
	// It is ensured that prj->lx56handler != NULL, because we cannot get here otherwise.
	if(prj->lx56handler->frame(currentTime, dt, prj))
		goto simulateProjectileStart;

finalMapPosIndexUpdate:
	prj->updateCollMapInfo(&oldPos, &oldRadius);
}


void LX56_simulateProjectiles(Iterator<CProjectile*>::Ref projs) {
	AbsTime currentTime = GetPhysicsTime();
	TimeDiff warpTime = tLX->fRealDeltaTime - tLX->fDeltaTime;
	cClient->fLastSimulationTime += warpTime;
	static const TimeDiff orig_dt = LX56PhysicsDT;
	
simulateProjectilesStart:
	if(cClient->fLastSimulationTime + orig_dt > currentTime) return;
	
	for(Iterator<CProjectile*>::Ref i = projs; i->isValid(); i->next()) {
		CProjectile* p = i->get();
		p->fLastSimulationTime += warpTime;
		LX56_simulateProjectile( cClient->fLastSimulationTime, p );
	}
	
	warpTime = TimeDiff(0);
	cClient->fLastSimulationTime += orig_dt;
	goto simulateProjectilesStart;
}



void CProjectile::setBestLX56Handler() {
	if(radius.x == 1 && radius.y == 1)
		lx56handler = &lx56radius1;
	else if(radius.x == 2 && radius.y == 2)
		lx56handler = &lx56radius2;
	else
		lx56handler = &lx56default;
}

