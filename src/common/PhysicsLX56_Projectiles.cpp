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
#include <typeinfo>

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
#include "Timer.h"


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
	bool pureLX56;
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
	
	bool preventSelfShooting =
		!NewNet::Active() &&
		(this->getIgnoreWormCollBeforeTime() > this->fLastSimulationTime); // if the simulation is too early, ignore this worm col

	CWorm *w = worms;
	for(short i=0;i<MAX_WORMS;i++,w++) {
		if(!w->isUsed() || !w->getAlive())
			continue;
		
		if(preventSelfShooting && w == ownerWorm)
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
	Shape<int> s1; s1.pos = src->getPos(); s1.radius.x = src_rx; s1.radius.y = src_ry;
	Shape<int> s2; s2.pos = target->getPos(); s2.radius = target->getRadius();
	if(src_attribs.projInfo->Type == PRJ_CIRCLE) s1.type = Shape<int>::ST_CIRCLE;
	if(target->getProjInfo()->Type == PRJ_CIRCLE) s2.type = Shape<int>::ST_CIRCLE;
	
	return s1.CollisionWith(s2);
}

static inline bool CProjectile_CollisionWith(const CProjectile* src, const CProjectile* target, const LX56ProjAttribs& src_attribs) {
	return CProjectile_CollisionWith(src, target, src_attribs, src_attribs.radius.x, src_attribs.radius.y);
}




inline ProjCollisionType FinalWormCollisionCheck(CProjectile* proj, const LX56ProjAttribs& attribs, const CVec& vFrameOldPos, const CVec& vFrameOldVel, CWorm* worms, TimeDiff dt, ProjCollisionType curResult) {
	CMap* map = cClient->getMap();
	
	// do we get any worm?
	if(proj->GetProjInfo()->PlyHit.Type != PJ_NOTHING) {
		CVec dif = proj->getPos() - vFrameOldPos;
		float len = NormalizeVector( &dif );
		len = MIN(len, float(cClient->getMap()->GetWidth()) + float(cClient->getMap()->GetHeight()));
		
		// the worm has a size of 4*4 in ProjWormColl, so it's save to check every second pixel here
		for (float p = 0.0f; p <= len; p += 2.0f) {
			CVec curpos = vFrameOldPos + dif * p;
			
			int ret = proj->ProjWormColl(attribs, curpos, worms);
			if (ret >= 0)  {
				if(proj->GetProjInfo()->PlyHit.Type != PJ_GOTHROUGH) {
					proj->setPos( curpos ); // save the new position at the first collision
					proj->vOldPos = curpos;
					proj->setVelocity( vFrameOldVel ); // don't get faster
				}

				if(cClient->getGameLobby()->features[FT_InfiniteMap]) {
					FMOD(proj->vPos.x, (float)map->GetWidth());
					FMOD(proj->vPos.y, (float)map->GetHeight());		
					FMOD(proj->vOldPos.x, (float)map->GetWidth());
					FMOD(proj->vOldPos.y, (float)map->GetWidth());		
				}
				
				return ProjCollisionType::Worm(ret);
			}
		}
	}
	
	if(cClient->getGameLobby()->features[FT_InfiniteMap]) {
		FMOD(proj->vPos.x, (float)map->GetWidth());
		FMOD(proj->vPos.y, (float)map->GetHeight());		
		FMOD(proj->vOldPos.x, (float)map->GetWidth());
		FMOD(proj->vOldPos.y, (float)map->GetHeight());		
	}

	return curResult;
}


///////////////////////
// Checks for collision with the level border
inline bool CProjectile::MapBoundsCollision(const LX56ProjAttribs& attribs, int px, int py)
{
	CollisionSide = 0;
	if(cClient->getGameLobby()->features[FT_InfiniteMap]) return false;
	
	CMap* map = cClient->getMap();
	
	if (px < 0 || px - attribs.radius.x < 0)
		CollisionSide |= COL_LEFT;
	
	if (px >= (int)map->GetWidth() || px + attribs.radius.x >= (int)map->GetWidth())
		CollisionSide |= COL_RIGHT;
	
	if (py < 0 || py - attribs.radius.y < 0)
		CollisionSide |= COL_TOP;
	
	if (py >= (int)map->GetHeight() || py + attribs.radius.y >= (int)map->GetHeight())
		CollisionSide |= COL_BOTTOM;
	
	return CollisionSide != 0;
}


inline static void handlePixelFlag(CProjectile::ColInfo& res, uchar* pf, int x, int y, int cx, int cy) {
	// Solid pixel
	if(*pf & (PX_DIRT|PX_ROCK)) {
		if (y < cy)
			++res.top;
		else if (y > cy)
			++res.bottom;
		if (x < cx)
			++res.left;
		else if (x > cx)
			++res.right;
		
		if (*pf & PX_ROCK)
			res.onlyDirt = false;
		res.collided = true;
	}	
}

////////////////////////////
// Checks for collision with the terrain
// WARNING: assumed to be called only from SimulateFrame
inline CProjectile::ColInfo CProjectile::TerrainCollision(const LX56ProjAttribs& attribs, int px, int py)
{
	CMap* map = cClient->getMap();
	
	ColInfo res = { 0, 0, 0, 0, false, true };

	const bool wrapAround = cClient->getGameLobby()->features[FT_InfiniteMap];

	// A fast check for tiny projectiles
	if (attribs.radius.x <= map->getCollGridCellW() / 2 && attribs.radius.y <= map->getCollGridCellH() / 2)
		if (map->GetCollisionFlag(px, py, wrapAround) == 0)
			return res;
	
	// if we are small, we can make a fast check
	if(attribs.radius.x*2 < map->getGridWidth() && attribs.radius.y*2 < map->getGridHeight()) {
		// If the current cells are empty, don't check for the collision
		const int gf1 = (py - attribs.radius.y) / map->getGridHeight() * map->getGridCols() + (px - attribs.radius.x) / map->getGridWidth();
		const int gf2 = (py - attribs.radius.y) / map->getGridHeight() * map->getGridCols() + (px + attribs.radius.x) / map->getGridWidth();
		const int gf3 = (py + attribs.radius.y) / map->getGridHeight() * map->getGridCols() + (px - attribs.radius.x) / map->getGridWidth();
		const int gf4 = (py + attribs.radius.y) / map->getGridHeight() * map->getGridCols() + (px + attribs.radius.x) / map->getGridWidth();
		const uchar *pf = map->getAbsoluteGridFlags();
		if ((pf[gf1] | pf[gf2] | pf[gf3] | pf[gf4]) == PX_EMPTY)
			return res;
	}

	// check for most common case - we do this because compiler can probably optimise this case very good
	if(!wrapAround && tProjInfo->Type != PRJ_CIRCLE) {	
		// Check for the collision
		for(int y = py - attribs.radius.y; y <= py + attribs.radius.y; ++y) {
			// this is safe because in SimulateFrame, we do map bound checks
			uchar *pf = map->GetPixelFlags() + y * map->GetWidth() + px - attribs.radius.x;
			
			for(int x = px - attribs.radius.x; x <= px + attribs.radius.x; ++x, ++pf) {				
				handlePixelFlag(res, pf, x, y, px, py);
			}
		}
	}
	else if(wrapAround) {
		px %= map->GetWidth();
		py %= map->GetHeight();
		if(px < 0) px += map->GetWidth();
		if(py < 0) py += map->GetHeight();
		
		// Check for the collision
		for(int _y = - attribs.radius.y; _y <= attribs.radius.y; ++_y) {
			int y = (map->GetHeight() + _y + py) % (long)map->GetHeight();
			uchar *_pf = map->GetPixelFlags() + y * map->GetWidth();
			
			for(int _x = - attribs.radius.x; _x <= attribs.radius.x; ++_x) {
				if(tProjInfo->Type == PRJ_CIRCLE && VectorD2<int>(_x,_y).GetLength2() > attribs.radius.GetLength2())
					// outside the range, skip this
					continue;
				
				int x = (map->GetWidth() + _x + px) % (long)map->GetWidth();
				uchar* pf = _pf + x;
				
				handlePixelFlag(res, pf, x, y, px, py);
			}
		}
	}
	else { // circle
		// Check for the collision
		for(int y = py - attribs.radius.y; y <= py + attribs.radius.y; ++y) {
			// this is safe because in SimulateFrame, we do map bound checks
			uchar *pf = map->GetPixelFlags() + y * map->GetWidth() + px - attribs.radius.x;
			
			for(int x = px - attribs.radius.x; x <= px + attribs.radius.x; ++x, ++pf) {				
				if(VectorD2<int>(x - px,y - py).GetLength2() > attribs.radius.GetLength2())
					// outside the range, skip this
					continue;

				handlePixelFlag(res, pf, x, y, px, py);
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
		// HINT: don't reset vPos here, because we want
		//		the explosion near (inside) the object
		//		this behavior is the same as in original LX
		return true;
	}
	
	bool bounce = false;
	
	// Bit of a hack
	switch (tProjInfo->Hit.Type)  {
	case PJ_BOUNCE:
			// HINT: don't reset vPos here; it will be reset,
			//		depending on the collisionside
		bounce = true;
		break;
	case PJ_NOTHING:  // PJ_NOTHING projectiles go through walls (but a bit slower)
		vPos = oldpos + (vVelocity * dt.seconds()) * 0.5f;
		vOldPos = vPos; // TODO: this is a hack; we do it to not go back to real old position because of collision
			// HINT: The above velocity reduction is not exact. SimulateFrame is also executed only for one checkstep because of the collision.
		break;
	case PJ_GOTHROUGH:
		vPos = oldpos + (vVelocity * dt.seconds()) * tProjInfo->Hit.GoThroughSpeed;
		return false; // ignore collision
	default:
		vPos = vOldPos;
		vVelocity = oldvel;
		return true;
	}
	
	int vx = (int)vVelocity.x;
	int vy = (int)vVelocity.y;
	
	// Find the collision side
	if ((c.left > c.right || c.left > 2) && c.left > 1 && vx <= 0) {
		if(bounce)
			vPos.x = oldpos.x;
		if (vx)
			CollisionSide |= COL_LEFT;
	}
	
	if ((c.right > c.left || c.right > 2) && c.right > 1 && vx >= 0) {
		if(bounce)
			vPos.x = oldpos.x;
		if (vx)
			CollisionSide |= COL_RIGHT;
	}
	
	if (c.top > 1 && vy <= 0) {
		if(bounce)
			vPos.y = oldpos.y;
		if (vy)
			CollisionSide |= COL_TOP;
	}
	
	if (c.bottom > 1 && vy >= 0) {
		if(bounce)
			vPos.y = oldpos.y;
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

	{
		const float friction = cClient->getGameLobby()->features[FT_ProjFriction];
		if(friction > 0) {
			const float projSize = (attribs.radius.x + attribs.radius.y) * 0.5f;
			const float projMass = attribs.radius.GetLength();
			// A bit lower drag coefficient as for worms because normally, projectiles have better shape for less dragging.
			static const float projDragCoeff = 0.02f; // Note: Never ever change this! (Or we have to make this configureable)
			applyFriction(prj->vVelocity, dt.seconds(), projSize, projMass, projDragCoeff, friction);
		}
	}
	
	CVec vOldVel = prj->getVelocity();

	CVec vFrameOldPos = prj->vPos;
	prj->vPos += prj->vVelocity * dt.seconds();
	
	// if distance is to short to last check, just return here without a check
	if ((int)(prj->vOldPos - prj->vPos).GetLength2() < prj->MIN_CHECKSTEP2) {
/*		printf("pos dif = %f , ", (vOldPos - vPos).GetLength());
		printf("len = %f , ", sqrt(len));
		printf("vel = %f , ", vVelocity.GetLength());
		printf("mincheckstep = %i\n", MIN_CHECKSTEP);	*/
		return FinalWormCollisionCheck(prj, attribs, vFrameOldPos, vOldVel, worms, dt, ProjCollisionType::NoCol());
	}
	
	int px = (int)(prj->vPos.x);
	int py = (int)(prj->vPos.y);
	
	// Hit edges
	if (prj->MapBoundsCollision(attribs, px, py))  {
		prj->vPos = prj->vOldPos;
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
	prj->vOldPos = prj->vPos;
	
	return FinalWormCollisionCheck(prj, attribs, vFrameOldPos, vOldVel, worms, dt, ProjCollisionType::NoCol());
}


inline ProjCollisionType LX56Projectile_checkCollAndMove(CProjectile* const prj, const LX56ProjAttribs& attribs, TimeDiff dt, CMap *map, CWorm* worms) {
	// Check if we need to recalculate the checksteps (projectile changed its velocity too much)
	if (prj->bChangesSpeed)  {
		const int len = (int)prj->vVelocity.GetLength2();
		if (abs(len - prj->iCheckSpeedLen) > 50000)
			prj->CalculateCheckSteps();
	}
	
	/*
	 In LX56, we skipped the simulation into half if dt >= 0.015f.
	 The dt we have here is usually lower (at least LX56PhysicsDT is), so it's like
	 LX56 to do the damping just once here.
	 */
	
	{
		// Dampening
		const float dmp = prj->getProjInfo()->Dampening;
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
		case PSPT_SHOT: return shot->cPos + GetVecFromAngle((float)shot->nAngle) * 8;
		case PSPT_PROJ: return proj->getPos();
	}
	return CVec(0,0);
}

CVec Proj_SpawnParent::velocity() const {
	switch(type) {
		case PSPT_NOTHING: return CVec(0,0);
		case PSPT_SHOT: return shot->cWormVel;
		case PSPT_PROJ: return proj->getVelocity();
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

void Proj_SpawnInfo::apply(Proj_SpawnParent parent, AbsTime spawnTime, bool pureLX56, bool optimIsTrail, bool optimIsNoTrail, bool optimIsShot, bool optimIsNoShot) const {
	// Calculate the angle of the direction the projectile is heading
	const float heading = Useangle ? parent.angle() : 0;
	
	for(int i = 0; i < Amount; i++) {
		const CVec sprd =
		((pureLX56 && optimIsNoTrail) ? false : UseParentVelocityForSpread) ?
			(parent.velocity() * ParentVelSpreadFactor)
		:
			// NOTE: It was a float -> int -> float conversion before (in LX56). this changed now, we just keep float!
			GetVecFromAngle(
							( (pureLX56 && (optimIsTrail || optimIsShot)) ? 0.0f : (float)Angle ) +
							heading +
							parent.fixedRandomFloat() * (float)Spread );
		
		int rot = 0;
		if((pureLX56 && optimIsShot) ? true : (pureLX56 && optimIsNoShot) ? false : UseRandomRot) {
			// Calculate a random starting angle for the projectile rotation (if used)
			if(Proj->Rotating) {
				// Prevent div by zero
				if(Proj->RotIncrement == 0)
					Proj->RotIncrement = 1;
				rot = GetRandomInt( abs( 360 / Proj->RotIncrement ) ) * Proj->RotIncrement;
			}
		}
		
		if(parent.type == Proj_SpawnParent::PSPT_SHOT) {
			parent.shot->nRandom++;
			parent.shot->nRandom %= 255;
		}
		
		const CVec& speedVarVec =
			( (pureLX56 && optimIsTrail) ? true : (pureLX56 && optimIsNoTrail) ? false : UseSpecial11VecForSpeedVar)
			? CVec(1,1) : sprd;
		const CVec v =
			sprd * (float)Speed +
			speedVarVec * (float)SpeedVar * parent.fixedRandomFloat() +
			( ((pureLX56 && optimIsShot) ? true : (pureLX56 && optimIsNoShot) ? false : AddParentVel)
			  ? (pureLX56 ? 1.0f : ParentVelFactor) * parent.velocity() : CVec(0,0));
		
		if(parent.type == Proj_SpawnParent::PSPT_SHOT) {
			parent.shot->nRandom *= 5;
			parent.shot->nRandom %= 255;
		}
		
		const AbsTime ignoreWormCollBeforeTime =
			(parent.type == Proj_SpawnParent::PSPT_PROJ) ?
				parent.proj->getIgnoreWormCollBeforeTime()
			:
				// we set the ignoreWormCollBeforeTime to the current time to let the physics engine
				// first emulate the projectiles to the curtime and ignore earlier colls as the worm-pos
				// is probably outdated at this time
				(GetPhysicsTime() + 0.1f); // HINT: we add 100ms (it was dt before) because the projectile is spawned -> worms are simulated (pos change) -> projectiles are simulated
		
		const int random = parent.fixedRandomIndex();
		
		CVec pos = parent.position() + (pureLX56 ? VectorD2<int>(0,0) : PosDiff);
		if(!pureLX56 && SnapToGrid.x >= 1 && SnapToGrid.y >= 1) {
			pos.x -= (int)pos.x % SnapToGrid.x; pos.x += SnapToGrid.x / 2;
			pos.y -= (int)pos.y % SnapToGrid.y; pos.y += SnapToGrid.y / 2;
		}
		
		cClient->SpawnProjectile(pos, v, rot, parent.ownerWorm(), Proj, random, spawnTime, ignoreWormCollBeforeTime);
		
		if(parent.type == Proj_SpawnParent::PSPT_SHOT) {
			parent.shot->nRandom++;
			parent.shot->nRandom %= 255;
		}
	}
	
}

void Proj_SpawnInfo::apply_Shot(Proj_SpawnParent parent, AbsTime spawnTime, bool pureLX56Optimisation) const {
#define SPAWNOPTIMMASK false, true, true, false
	if(pureLX56Optimisation)
		apply(parent, spawnTime, true, SPAWNOPTIMMASK);
	else
		apply(parent, spawnTime, false, SPAWNOPTIMMASK);		
#undef SPAWNOPTIMMASK
}

void Proj_SpawnInfo::apply_Trail(Proj_SpawnParent parent, AbsTime spawnTime, bool pureLX56Optimisation) const {
#define SPAWNOPTIMMASK true, false, false, true
	if(pureLX56Optimisation)
		apply(parent, spawnTime, true, SPAWNOPTIMMASK);
	else
		apply(parent, spawnTime, false, SPAWNOPTIMMASK);
#undef SPAWNOPTIMMASK
}

void Proj_SpawnInfo::apply_Event(Proj_SpawnParent parent, AbsTime spawnTime, bool pureLX56Optimisation) const {
#define SPAWNOPTIMMASK false, true, false, true
	if(pureLX56Optimisation)
		apply(parent, spawnTime, true, SPAWNOPTIMMASK);
	else
		apply(parent, spawnTime, false, SPAWNOPTIMMASK);
#undef SPAWNOPTIMMASK
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
	Speed = a.Speed;
	SpeedMult = a.SpeedMult;
	if(a.additionalAction) additionalAction = new Proj_Action(*a.additionalAction);
	
	return *this;
}


static VectorD2<float> objectAngleDiff(CGameObject* w, CGameObject* prj) {
	CVec diff = w->getPos() - prj->getPos(); NormalizeVector(&diff);
	return MatrixD2<float>::Rotation(diff.x, -diff.y) * prj->getVelocity();
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

static VectorD2<float> getHeadingVelTo(CGameObject* target, CGameObject* src) {
	if(target == NULL) return VectorD2<float>();

	VectorD2<float> ret = src->getVelocity().orthogonal();
	NormalizeVector(&ret);
	
	VectorD2<float> angleDiffV = objectAngleDiff(target, src);
	float angleDiff = atan2f(angleDiffV.y, angleDiffV.x);
	if(angleDiff >= float(PI)) angleDiff -= float(PI) * 2;
	ret *= angleDiff / float(PI);
	
	return ret;
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
				cClient->Explosion(prj->getPos(), (float)BounceExplode, false, prj->GetOwner());
			break;
			
		// Carve
		case PJ_CARVE:
			if(eventInfo.timerHit || (eventInfo.colType && !eventInfo.colType->withWorm)) {
				int d = cClient->getMap()->CarveHole(Damage, prj->getPos(), cClient->getGameLobby()->features[FT_InfiniteMap]);
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
		
		case PJ_INJUREWORM:
			if(eventInfo.colType && eventInfo.colType->withWorm) {
				cClient->InjureWorm(&cClient->getRemoteWorms()[eventInfo.colType->wormId], (float)Damage, prj->GetOwner());
			}			
			break;
				
		case PJ_INJURE:
			if(eventInfo.colType && eventInfo.colType->withWorm) {
				info->deleteAfter = true;
				cClient->InjureWorm(&cClient->getRemoteWorms()[eventInfo.colType->wormId], (float)Damage, prj->GetOwner());
				break;
			}
			
		case PJ_DISAPPEAR:
			// TODO: do something special?
			if(eventInfo.colType && eventInfo.colType->withWorm) break;
			
		case PJ_GOTHROUGH:
		case PJ_NOTHING:
			// if Hit_Type == PJ_NOTHING, it means that this projectile goes through all walls
			if(eventInfo.colType && !eventInfo.colType->withWorm && (eventInfo.colType->colMask & PJC_MAPBORDER)) {
				// HINT: This is new since Beta9. I hope it doesn't change any serious behaviour.
				// It means, if such a projectile which goes through everything hits the mapborder, it will
				// get deleted - otherwise it would be stuck there at the border.
				info->deleteAfter = true;
			}
			push_worm = false;
			if(eventInfo.timerHit) spawnprojs = false;
			break;
			
		case PJ_INJUREPROJ:
			for(Proj_EventOccurInfo::Targets::const_iterator p = eventInfo.targets.begin(); p != eventInfo.targets.end(); ++p) {
				if(typeid(**p) == typeid(CProjectile))
					(*p)->injure((float)Damage);
			}
			break;
			
		case PJ_PLAYSOUND:
			if(UseSound && Sound)
				info->sound = Sound;
			break;
		
		case PJ_ChangeRadius:
			prj->radius += ChangeRadius;
			if(prj->radius.x < 0) prj->radius.x = 0;
			if(prj->radius.y < 0) prj->radius.y = 0;
			prj->setBestLX56Handler(); // because LX56Handler could depend on radius
			break;
			
		case PJ_OverwriteOwnSpeed:
			prj->setVelocity(Speed);
			break;
			
		case PJ_MultiplyOwnSpeed:
			prj->setVelocity( SpeedMult * prj->getVelocity() );
			break;
			
		case PJ_DiffOwnSpeed:
			prj->setVelocity( prj->getVelocity() + Speed );
			break;
			
		case PJ_OverwriteTargetSpeed:
			for(Proj_EventOccurInfo::Targets::const_iterator p = eventInfo.targets.begin(); p != eventInfo.targets.end(); ++p) {
				(*p)->setVelocity(Speed);
			}
			break;
			
		case PJ_MultiplyTargetSpeed:
			for(Proj_EventOccurInfo::Targets::const_iterator p = eventInfo.targets.begin(); p != eventInfo.targets.end(); ++p) {
				(*p)->setVelocity( SpeedMult * (*p)->getVelocity() );
			}
			break;
			
		case PJ_DiffTargetSpeed:
			for(Proj_EventOccurInfo::Targets::const_iterator p = eventInfo.targets.begin(); p != eventInfo.targets.end(); ++p) {
				(*p)->setVelocity( (*p)->getVelocity() + Speed );
			}
			break;
			
		case PJ_HeadingToNextWorm:
			prj->velocity() += SpeedMult * getHeadingVelTo(nearestWorm(prj->getPos()), prj);
			break;
			
		case PJ_HeadingToOwner:
			if(prj->GetOwner() >= 0 && prj->GetOwner() < MAX_WORMS) {
				prj->velocity() += SpeedMult * getHeadingVelTo(&cClient->getRemoteWorms()[prj->GetOwner()], prj);
			}
			break;
			
		case PJ_HeadingToNextOtherWorm:
			prj->velocity() += SpeedMult * getHeadingVelTo(nearestOtherWorm(prj->getPos(), prj->GetOwner()), prj);
			break;
			
		case PJ_HeadingToNextEnemyWorm:
			prj->velocity() += SpeedMult * getHeadingVelTo(nearestEnemyWorm(prj->getPos(), prj->GetOwner()), prj);
			break;
			
		case PJ_HeadingToNextTeamMate:
			prj->velocity() += SpeedMult * getHeadingVelTo(nearestTeamMate(prj->getPos(), prj->GetOwner()), prj);
			break;
			
		case PJ_HeadTargetToUs:
			for(Proj_EventOccurInfo::Targets::const_iterator obj = eventInfo.targets.begin(); obj != eventInfo.targets.end(); ++obj) {
				(*obj)->velocity() += SpeedMult * getHeadingVelTo(prj, *obj);
			}
			break;
			
		case __PJ_LBOUND: case __PJ_UBOUND:
			errors << "Proj_Action::applyTo: hit __PJ_BOUND" << endl;
			break;
	}
	
	// Push the worm back
	if(push_worm && eventInfo.colType && eventInfo.colType->withWorm) {
		CVec d = prj->getVelocity();
		NormalizeVector(&d);
		cClient->getRemoteWorms()[eventInfo.colType->wormId].velocity() += (d * 100) * eventInfo.dt.seconds();
	}
	
	if(spawnprojs) {
		if(Proj.isSet())
			info->otherSpawns.push_back(&Proj);
		else
			info->spawnprojectiles = true;
	}
	
	if(additionalAction)
		additionalAction->applyTo(eventInfo, prj, info);
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


static inline bool checkProjHit(const Proj_ProjHitEvent& info, std::set<CGameObject*>& projs, CProjectile* prj, const LX56ProjAttribs& attribs, CProjectile* p) {
	if(p == prj) return true;
	if(info.Target && p->getProjInfo() != info.Target) return true;
	if(!info.ownerWorm.match(prj->GetOwner(), p)) return true;
	if(info.TargetHealthIsMore && p->getHealth() <= prj->getHealth()) return true;
	if(info.TargetHealthIsLess && p->getHealth() >= prj->getHealth()) return true;
	if(info.TargetTimeIsMore && p->getLife() <= prj->getLife()) return true;
	if(info.TargetTimeIsLess && p->getLife() >= prj->getLife()) return true;
	
	if(info.Width >= 0 && info.Height >= 0) { if(!CProjectile_CollisionWith(prj, p, attribs, info.Width/2, info.Height/2)) return true; }
	else { if(!CProjectile_CollisionWith(prj, p, attribs)) return true; }
	
	projs.insert(p);
	
	if(info.MaxHitCount < 0 && projs.size() >= (size_t)info.MinHitCount) return false; // no need to check further
	if(projs.size() > (size_t)info.MaxHitCount) return false; // no need to check further
	
	return true;
}

template<bool TOP, bool LEFT>
static CClient::MapPosIndex MPI(const VectorD2<int>& p, const VectorD2<int>& r) {
	return CClient::MapPosIndex( p + VectorD2<int>(LEFT ? -r.x : r.x, TOP ? -r.y : r.y) );
}

bool Proj_ProjHitEvent::checkEvent(Proj_EventOccurInfo& ev, CProjectile* prj, const LX56ProjAttribs& attribs, Proj_DoActionInfo*) const {
	const VectorD2<int> vPos = prj->getPos();
	const VectorD2<int> radius = prj->getRadius();
	for(int x = MPI<true,true>(vPos,radius).x; x <= MPI<true,false>(vPos,radius).x; ++x)
		for(int y = MPI<true,true>(vPos,radius).y; y <= MPI<false,true>(vPos,radius).y; ++y) {
			CClient::ProjectileSet* projs = cClient->projPosMap[CClient::MapPosIndex(x,y).index(cClient->getMap())];
			if(projs == NULL) continue;
			for(CClient::ProjectileSet::const_iterator p = projs->begin(); p != projs->end(); ++p)
				if(!checkProjHit(*this, ev.targets, prj, attribs, *p)) goto finalChecks;
		}
	
finalChecks:
	if(ev.targets.size() >= (size_t)MinHitCount && (MaxHitCount < 0 || ev.targets.size() <= (size_t)MaxHitCount))
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
	
	const int team = (worm >= 0 && worm < MAX_WORMS) ? cClient->getRemoteWorms()[worm].getTeam() : -1;
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
	if(match(eventInfo.colType->wormId, prj)) {
		eventInfo.targets.insert(&cClient->getRemoteWorms()[eventInfo.colType->wormId]);
		return true;
	}
	return false;
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
		cClient->Explosion(prj->getPos(), (float)damage, shake, prj->GetOwner());
}

static void projectile_doTimerExplode(CProjectile* const prj, int shake) {
	const proj_t *pi = prj->GetProjInfo();
	// Explosion
	int damage = pi->Timer.Damage;
	if(pi->PlyHit.Type == PJ_EXPLODE)
		damage = pi->PlyHit.Damage;
	
	if(damage != -1) // TODO: why only with -1?
		cClient->Explosion(prj->getPos(), (float)damage, shake, prj->GetOwner());
}

static void projectile_doMakeDirt(CProjectile* const prj) {
	const int damage = 5;
	int d = 0;
	d += cClient->getMap()->PlaceDirt(damage,prj->getPos()-CVec(6,6));
	d += cClient->getMap()->PlaceDirt(damage,prj->getPos()+CVec(6,-6));
	d += cClient->getMap()->PlaceDirt(damage,prj->getPos()+CVec(0,6));
	
	// Remove the dirt count on the worm
	if(prj->hasOwner())
		cClient->getRemoteWorms()[prj->GetOwner()].incrementDirtCount( -d );
}

static void projectile_doMakeGreenDirt(CProjectile* const prj) {
	int d = cClient->getMap()->PlaceGreenDirt(prj->getPos());
	
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

void Proj_DoActionInfo::execute(CProjectile* const prj, const AbsTime currentTime, const LX56ProjAttribs& attribs) {
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
		
	if(trailprojspawn) {
		// we use prj->fLastSimulationTime here to simulate the spawing at the current simulation time of this projectile
		pi->Trail.Proj.apply_Trail(prj, prj->fLastSimulationTime, attribs.pureLX56);
	}
	
	// Spawn any projectiles?
	if(spawnprojectiles) {
		// we use currentTime (= the simulation time of the cClient) to simulate the spawing at this time
		// because the spawing is caused probably by conditions of the environment like collision with worm/cClient->getMap()
		pi->GeneralSpawnInfo.apply_Event(prj, currentTime, attribs.pureLX56);
	}
	
	for(std::list<const Proj_SpawnInfo*>::iterator i = otherSpawns.begin(); i != otherSpawns.end(); ++i) {
		// we use currentTime (= the simulation time of the cClient) to simulate the spawing at this time
		// because the spawing is caused probably by conditions of the environment like collision with worm/cClient->getMap()
		(*i)->apply_Event(prj, currentTime, false);
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

	// Check for collisions and move
	ProjCollisionType res = LX56Projectile_checkCollAndMove(proj, attribs, dt, cClient->getMap(), worms);
	
	proj->life() += dt.seconds();
	proj->extra() += dt.seconds();
	
	
	
	/*
		vOldPos = vPos;
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
			const int NumFrames = pi->bmpImage->w / pi->bmpImage->h;
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
			SpawnEntity(ENT_SMOKE,0,proj->getPos(),CVec(0,0),Color(),NULL);
		}
		break;
	case TRL_CHEMSMOKE:
		if(proj->extra() >= 0.075f) {
			proj->extra() = 0.0;
			SpawnEntity(ENT_CHEMSMOKE,0,proj->getPos(),CVec(0,0),Color(),NULL);
		}
		break;
	case TRL_DOOMSDAY:
		if(proj->extra() >= 0.05f) {
			proj->extra() = 0.0;
			SpawnEntity(ENT_DOOMSDAY,0,proj->getPos(),proj->getVelocity(),Color(),NULL);
		}
		break;
	case TRL_EXPLOSIVE:
		if(proj->extra() >= 0.05f) {
			proj->extra() = 0.0;
			SpawnEntity(ENT_EXPLOSION,10,proj->getPos(),CVec(0,0),Color(),NULL);
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
		
		const Proj_EventOccurInfo eventInfo = Proj_EventOccurInfo::Col(serverTime, dt, &result);
		
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
			projInfo.PlyHit.applyTo(eventInfo, prj, &doActionInfo);
		}
				
		for(size_t i = 0; i < projInfo.actions.size(); ++i) {
			projInfo.actions[i].checkAndApply(eventInfo, prj, attribs, &doActionInfo);
		}
		
		doActionInfo.execute(prj, currentTime, attribs);
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
		const LX56ProjAttribs attribs = { prj->getRadius(), prj->getProjInfo(), false };
		return doFrame(currentTime, dt, *prj->GetProjInfo(), prj, attribs);
	}
} lx56default;

struct LX56ProjHandler_Radius1pureLX56 : LX56ProjectileHandler {
	bool frame(const AbsTime currentTime, TimeDiff dt, CProjectile* const prj) {
		const LX56ProjAttribs attribs = { VectorD2<int>(1,1), prj->getProjInfo(), true };
		return doFrame(currentTime, dt, *prj->GetProjInfo(), prj, attribs);
	}
} lx56radius1;

struct LX56ProjHandler_Radius2pureLX56 : LX56ProjectileHandler {
	bool frame(const AbsTime currentTime, TimeDiff dt, CProjectile* const prj) {
		const LX56ProjAttribs attribs = { VectorD2<int>(2,2), prj->getProjInfo(), true };
		return doFrame(currentTime, dt, *prj->GetProjInfo(), prj, attribs);
	}
} lx56radius2;


struct LX56ProjHandler_CollideProjectiles : LX56ProjectileHandler {
	static Proj_ProjHitEvent hitEvent;
	static proj_t temporaryProj;

	LX56ProjHandler_CollideProjectiles()
	{
		if( temporaryProj.Timer.Type != PJ_DISAPPEAR2 ) // One-time init
		{
			hitEvent.Width = 5;
			hitEvent.Height = 5;
			//hitEvent.ownerWorm.DiffWormAsProjOwner = true; // Those checks fail for some reason
			//hitEvent.ownerWorm.EnemyOfProjOwner = true;
			
			temporaryProj.Timer.Type = PJ_DISAPPEAR2;
			//temporaryProj.Timer.Time = 0.00001f; // Fail
			temporaryProj.Hit.Type = PJ_DISAPPEAR2;
			temporaryProj.Hit.Type = PJ_DISAPPEAR2;
			temporaryProj.Tch.Type = PJ_DISAPPEAR2;
			temporaryProj.PlyHit.Type = PJ_DISAPPEAR2;
			temporaryProj.Type = PRJ_IMAGE;
			//temporaryProj.Type = PRJ_PIXEL;
			//temporaryProj.Colour.push_back( Color(255,255,255) );
			temporaryProj.Width = 0;
			temporaryProj.Height = 0;
		}
	}
	bool frame(const AbsTime currentTime, TimeDiff dt, CProjectile* const prj) {

		const proj_t* projInfo = prj->GetProjInfo();

		if( projInfo == &temporaryProj )
		{
			// Remove it after one frame
			if( prj->life() >= 1.0f )
				prj->setUnused();
			else
				prj->life() = 1.0f;
			return false;
		}

		const LX56ProjAttribs attribs = { prj->getRadius(), projInfo, false };
		//const LX56ProjAttribs attribs = { VectorD2<int>(hitEvent.Width, hitEvent.Height), &projInfo, false };

		bool ret = doFrame(currentTime, dt, *projInfo, prj, attribs);
		
		if( !ret )
			return ret;
		
		const LX56ProjAttribs attribs1 = { VectorD2<int>(5, 5), projInfo, false };
		//const LX56ProjAttribs attribs1 = attribs;
		
		ProjCollisionType result;
		Proj_DoActionInfo doActionInfo;
		
		Proj_EventOccurInfo eventInfo = Proj_EventOccurInfo::Col(cClient->serverTime(), dt, &result);
		
		if( hitEvent.checkEvent(eventInfo, prj, attribs1, &doActionInfo) )
		{
			bool enemyHit = false;
			for( Proj_EventOccurInfo::Targets::iterator it = eventInfo.targets.begin();
					it != eventInfo.targets.end(); it++ )
			{
				CProjectile* const prj1 = reinterpret_cast<CProjectile * const> (*it);
				
				const int projTeam = (prj->GetOwner() >= 0 && prj->GetOwner() < MAX_WORMS) ? cClient->getRemoteWorms()[prj->GetOwner()].getTeam() : -1;
				const int projTeam1 = (prj1->GetOwner() >= 0 && prj1->GetOwner() < MAX_WORMS) ? cClient->getRemoteWorms()[prj1->GetOwner()].getTeam() : -1;
				if( ( prj->GetOwner() != prj1->GetOwner() ) && ( !cClient->isTeamGame() || projTeam != projTeam1 ) )
				{
					enemyHit = true;
					// Spawn temp projectile, so other projectile will explode too
					cClient->SpawnProjectile(prj1->getPos(), prj1->getVelocity(), 0, prj->GetOwner(), &temporaryProj, 0, currentTime, AbsTime(0) );
				}
			}
			if( enemyHit )
			{
				projInfo->Hit.applyTo(eventInfo, prj, &doActionInfo);
				doActionInfo.execute(prj, currentTime, attribs1); // Projectile likely to be removed here
				return !doActionInfo.deleteAfter;
			}
		};
		return true;
	}
	
} lx56CollideProjectiles;

proj_t LX56ProjHandler_CollideProjectiles::temporaryProj;
Proj_ProjHitEvent LX56ProjHandler_CollideProjectiles::hitEvent;


static void LX56_simulateProjectile(const AbsTime currentTime, CProjectile* const prj) {
	static const TimeDiff orig_dt = LX56PhysicsDT;
	const TimeDiff dt = orig_dt * (float)cClient->getGameLobby()->features[FT_GameSpeed];
	
	VectorD2<int> oldPos(prj->getPos());
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
	AbsTime realSimulationTime = GetTime();
	
simulateProjectilesStart:
	if(cClient->fLastSimulationTime + orig_dt > currentTime) return;

	// HINT: if the computer is too slow and doesn't manage to simulate everything, just skip few frames
	// Better an incorrect simulation than a game that is not controllable
	if (GetTime() - realSimulationTime > orig_dt)  {
		cClient->fLastSimulationTime = currentTime;
		return;
	}
	realSimulationTime = GetTime();
	
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
	const bool pureLX56 = cClient->getGameScript()->GetHeader()->Version == GS_LX56_VERSION;
	if(radius.x == 1 && radius.y == 1 && pureLX56)
		lx56handler = &lx56radius1;
	else if(radius.x == 2 && radius.y == 2 && pureLX56)
		lx56handler = &lx56radius2;
	else
		lx56handler = &lx56default;
	if(bool(cClient->getGameLobby()->features[FT_CollideProjectiles]))
		lx56handler = &lx56CollideProjectiles;
}

