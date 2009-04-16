/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Projectile Class
// Created 11/2/02
// Jason Boettcher


#include "LieroX.h"
#include "CGameScript.h" // for all PRJ_* and PJ_* constants only
#include "GfxPrimitives.h"
#include "CProjectile.h"
#include "Protocol.h"
#include "CWorm.h"
#include "Entity.h"
#include "MathLib.h"
#include "CClient.h"
#include "ProfileSystem.h"
#include "Debug.h"
#include "ProjectileDesc.h"
#include "Physics.h"
#include "Geometry.h"


void CProjectile::setUnused() {
	bUsed = false;
	timerInfo.clear();

	onInvalidation.occurred( EventData(this) );
}

///////////////////
// Spawn the projectile
// this function is called by CClient::SpawnProjectile()
void CProjectile::Spawn(proj_t *_proj, CVec _pos, CVec _vel, int _rot, int _owner, int _random, AbsTime time, AbsTime ignoreWormCollBeforeTime)
{
	if (_owner < 0 || _owner >= MAX_WORMS)
		iOwner = -1;
	else
		iOwner = _owner;
	
	bUsed = true;
	tProjInfo = _proj;
	fLife = 0;
	fExtra = 0;
	vOldPos = _pos;
	vPosition = _pos;
	vVelocity = _vel;
	fRotation = (float)_rot;
	radius.x = tProjInfo->Width / 2;
	radius.y = tProjInfo->Height / 2;
	health = 100;
	
	fLastTrailProj = AbsTime();
	iRandom = _random;
    iFrameX = 0;
	fIgnoreWormCollBeforeTime = ignoreWormCollBeforeTime;

    fTimeVarRandom = GetFixedRandomNum(iRandom);
	fLastSimulationTime = time;
	fSpawnTime = time;
	timerInfo.clear();

	fSpeed = _vel.GetLength();
	CalculateCheckSteps();

	fFrame = 0;
	bFrameDelta = true;

	firstbounce = true;

	switch(tProjInfo->Type) {
		case PRJ_RECT:
		case PRJ_PIXEL:
		case PRJ_CIRCLE:
		case PRJ_POLYGON: {
			// Choose a colour
			if(tProjInfo->Colour.size() > 0) {
				int c = GetRandomInt(tProjInfo->Colour.size()-1);
				iColour = tProjInfo->Colour[c];
			}
			else {
				// don't give the warning here, give it while loading GS
				iColour = Color();
			}
			break;
		}
		case PRJ_IMAGE: break;
		case __PRJ_LBOUND: case __PRJ_UBOUND: errors << "CProjectile::DrawShadow: hit __PRJ_BOUND" << endl;
	}

	fGravity = 100.0f; // Default
	if (tProjInfo->UseCustomGravity)
		fGravity = (float)tProjInfo->Gravity;

	fWallshootTime = 0.01f + getRandomFloat() / 1000; // Support wallshooting - ignore collisions before this time

	// TODO: the check was tProjInfo->Type != PJ_BOUNCE before, which didn't make sense. is it correct now? 
	bChangesSpeed = ((int)fGravity == 0) && ((int)tProjInfo->Dampening == 1)
		&& (tProjInfo->Hit.Type != PJ_BOUNCE || (int)tProjInfo->Hit.BounceCoeff == 1);  // Changes speed on bounce

	updateCollMapInfo();
}


///////////////////
// Gets a random float from a special list
// TODO: how does this belong to projectiles? move it perhaps out here
float CProjectile::getRandomFloat()
{
	float r = GetFixedRandomNum(iRandom++);

	iRandom %= 255;

	return r;
}


static ProjCollisionType FinalWormCollisionCheck(CProjectile* proj, const CVec& vFrameOldPos, const CVec& vFrameOldVel, CWorm* worms, float dt, float* enddt, ProjCollisionType curResult) {
	// do we get any worm?
	if(proj->GetProjInfo()->PlyHit.Type != PJ_NOTHING) {
		CVec dif = proj->GetPosition() - vFrameOldPos;
		float len = NormalizeVector( &dif );

		// the worm has a size of 4*4 in ProjWormColl, so it's save to check every second pixel here
		for (float p = 0.0f; p <= len; p += 2.0f) {
			CVec curpos = vFrameOldPos + dif * p;

			int ret = proj->ProjWormColl(curpos, worms);
			if (ret >= 0)  {
				if(proj->GetProjInfo()->PlyHit.Type != PJ_GOTHROUGH) {
					if (enddt) {
						if (len != 0)
							*enddt = dt * p / len;
						else
							*enddt = dt;
					}
					proj->setNewPosition( curpos ); // save the new position at the first collision
					proj->setNewVel( vFrameOldVel ); // don't get faster
				}
				return ProjCollisionType::Worm(ret);
			}
		}
	}

	return curResult;
}

//////////////////////
// Pre-calculates the check steps for collisions
void CProjectile::CalculateCheckSteps()
{
	MIN_CHECKSTEP = 4;
	MAX_CHECKSTEP = 6;
	AVG_CHECKSTEP = 4;

	iCheckSpeedLen = (int)vVelocity.GetLength2();
	if (iCheckSpeedLen < 14000)  {
		MIN_CHECKSTEP = 0;
		MAX_CHECKSTEP = 3;
		AVG_CHECKSTEP = 2;
	} else if (iCheckSpeedLen < 75000)  {
		MIN_CHECKSTEP = 0;
		MAX_CHECKSTEP = 4;
		AVG_CHECKSTEP = 2;
	} else if (iCheckSpeedLen < 250000)  {
		MIN_CHECKSTEP = 1;
		MAX_CHECKSTEP = 5;
		AVG_CHECKSTEP = 2;

	// HINT: add or substract some small random number for high speeds, it behaves more like original LX
	} else {
		int rnd = (getRandomIndex() % 3);
		rnd *= SIGN(getRandomFloat());
		MIN_CHECKSTEP = 6;
		if (tProjInfo->Hit.Type == PJ_BOUNCE)  { // HINT: this avoids fast bouncing projectiles to stay in a wall too often (for example zimm)
			MAX_CHECKSTEP = 2;
			AVG_CHECKSTEP = 2;
		} else {
			MAX_CHECKSTEP = 9 + rnd;
			AVG_CHECKSTEP = 6 + rnd;
		}
	}

	MAX_CHECKSTEP2 = MAX_CHECKSTEP * MAX_CHECKSTEP;
	MIN_CHECKSTEP2 = MIN_CHECKSTEP * MIN_CHECKSTEP;
}


///////////////////////
// Checks for collision with the level border
bool CProjectile::MapBoundsCollision(int px, int py)
{
	CMap* map = cClient->getMap();
	CollisionSide = 0;

	if (px - radius.x < 0)
		CollisionSide |= COL_LEFT;

	if (px + radius.x >= (int)map->GetWidth())
		CollisionSide |= COL_RIGHT;

	if (py - radius.y < 0)
		CollisionSide |= COL_TOP;

	if (py + radius.y >= (int)map->GetHeight())
		CollisionSide |= COL_BOTTOM;

	return CollisionSide != 0;
}

////////////////////////////
// Checks for collision with the terrain
// WARNING: assumed to be called only from SimulateFrame
CProjectile::ColInfo CProjectile::TerrainCollision(int px, int py)
{
	CMap* map = cClient->getMap();

	ColInfo res = { 0, 0, 0, 0, false, true };

	// if we are small, we can make a fast check
	if(radius.x*2 < map->getGridWidth() && radius.y*2 < map->getGridHeight()) {
		// If the current cells are empty, don't check for the collision
		const int gf1 = (py - radius.y) / map->getGridHeight() * map->getGridCols() + (px - radius.x) / map->getGridWidth();
		const int gf2 = (py - radius.y) / map->getGridHeight() * map->getGridCols() + (px + radius.x) / map->getGridWidth();
		const int gf3 = (py + radius.y) / map->getGridHeight() * map->getGridCols() + (px - radius.x) / map->getGridWidth();
		const int gf4 = (py + radius.y) / map->getGridHeight() * map->getGridCols() + (px + radius.x) / map->getGridWidth();
		const uchar *pf = map->getAbsoluteGridFlags();
		if ((pf[gf1] | pf[gf2] | pf[gf3] | pf[gf4]) == PX_EMPTY)
			return res;
	}

	// Check for the collision
	for(int y = py - radius.y; y <= py + radius.y; ++y) {
		// this is safe because in SimulateFrame, we do map bound checks
		uchar *pf = map->GetPixelFlags() + y * map->GetWidth() + px - radius.x;

		for(int x = px - radius.x; x <= px + radius.x; ++x, ++pf) {

			if(tProjInfo->Type == PRJ_CIRCLE && (VectorD2<int>(x,y) - VectorD2<int>(px,py)).GetLength2() > radius.GetLength2())
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
bool CProjectile::HandleCollision(const CProjectile::ColInfo &c, const CVec& oldpos, const CVec& oldvel, float dt)
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
			vPosition = oldpos + (vVelocity * dt) * 0.5f;
			vOldPos = vPosition; // TODO: this is a hack; we do it to not go back to real old position because of collision
			// HINT: The above velocity reduction is not exact. SimulateFrame is also executed only for one checkstep because of the collision.
			break;
		case PJ_GOTHROUGH:
			vPosition = oldpos + (vVelocity * dt) * tProjInfo->Hit.GoThroughSpeed;			
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
ProjCollisionType CProjectile::SimulateFrame(float dt, CMap *map, CWorm* worms, float* enddt)
{
	// Check if we need to recalculate the checksteps (projectile changed its velocity too much)
	if (bChangesSpeed)  {
		int len = (int)vVelocity.GetLength2();
		if (abs(len - iCheckSpeedLen) > 50000)
			CalculateCheckSteps();
	}

	CVec vOldVel = vVelocity;
	CVec newvel = vVelocity;

	// Gravity
	newvel.y += fGravity * dt;

	// Dampening
	// HINT: as this function is always called with fixed dt, we can do it this way
	newvel *= tProjInfo->Dampening;

	float checkstep = newvel.GetLength2(); // |v|^2
	if ((int)(checkstep * dt * dt) > MAX_CHECKSTEP2) { // |dp|^2=|v*dt|^2
		// calc new dt, so that we have |v*dt|=AVG_CHECKSTEP
		// checkstep is new dt
		checkstep = (float)AVG_CHECKSTEP / sqrt(checkstep);
		checkstep = MAX(checkstep, 0.001f);

		// In some bad cases (float accurance problems mainly),
		// it's possible that checkstep >= dt .
		// If we would not check this case, we get in an infinie
		// recursive loop.
		// Therefore if this is the case, we don't do multiple checksteps.
		if(checkstep < dt) {
			for(float time = 0; time < dt; time += checkstep) {
				ProjCollisionType ret = SimulateFrame((time + checkstep > dt) ? dt - time : checkstep, map,worms,enddt);
				if(ret) {
					if(enddt) *enddt += time;
					return ret;
				}
			}

			if(enddt) *enddt = dt;
			return ProjCollisionType::NoCol();
		}
	}

	vVelocity = newvel;
	if(enddt) *enddt = dt;
	CVec vFrameOldPos = vPosition;
	vPosition += vVelocity * dt;
	
	// if distance is to short to last check, just return here without a check
	if ((int)(vOldPos - vPosition).GetLength2() < MIN_CHECKSTEP2) {
/*		printf("pos dif = %f , ", (vOldPos - vPosition).GetLength());
		printf("len = %f , ", sqrt(len));
		printf("vel = %f , ", vVelocity.GetLength());
		printf("mincheckstep = %i\n", MIN_CHECKSTEP);	*/
		return FinalWormCollisionCheck(this, vFrameOldPos, vOldVel, worms, dt, enddt, ProjCollisionType::NoCol());
	}

	int px = (int)(vPosition.x);
	int py = (int)(vPosition.y);

	// Hit edges
	if (MapBoundsCollision(px, py))  {
		vPosition = vOldPos;
		vVelocity = vOldVel;

		return FinalWormCollisionCheck(this, vFrameOldPos, vOldVel, worms, dt, enddt, ProjCollisionType::Terrain(PJC_TERRAIN|PJC_MAPBORDER));
	}
	
	// Make wallshooting possible
	// NOTE: wallshooting is a bug in old LX physics that many players got used to
	if (GetPhysicsTime() - fSpawnTime <= fWallshootTime)
		return FinalWormCollisionCheck(this, vFrameOldPos, vOldVel, worms, dt, enddt, ProjCollisionType::NoCol());

	// Check collision with the terrain
	ColInfo c = TerrainCollision(px, py);

	// Check for a collision
	if(c.collided && HandleCollision(c, vFrameOldPos, vOldVel, dt)) {
		int colmask = PJC_TERRAIN;
		if(c.onlyDirt) colmask |= PJC_DIRT;
		return FinalWormCollisionCheck(this, vFrameOldPos, vOldVel, worms, dt, enddt, ProjCollisionType::Terrain(colmask));
	}

	// the move was safe, save the position
	vOldPos = vPosition;

	return FinalWormCollisionCheck(this, vFrameOldPos, vOldVel, worms, dt, enddt, ProjCollisionType::NoCol());
}

///////////////////
// Check for a collision (static version; doesnt do anything else then checking)
// Returns true if there was a collision, otherwise false is returned
int CProjectile::CheckCollision(proj_t* tProjInfo, float dt, CVec pos, CVec vel)
{
	// Check if it hit the terrain
	CMap* map = cClient->getMap();
	int mw = map->GetWidth();
	int mh = map->GetHeight();
	int w,h;

	if(tProjInfo->Type == PRJ_PIXEL)
		w=h=1;
	else
		w=h=2;

	float maxspeed2 = (float)(4*w*w+4*w+1); // (2w+1)^2
	if( (vel*dt).GetLength2() > maxspeed2) {
		dt *= 0.5f;

		int col = CheckCollision(tProjInfo,dt,pos,vel);
		if(col) return col;

		pos += vel*dt;

		return CheckCollision(tProjInfo,dt,pos,vel);
	}

	pos += vel*dt;

	int px = (int)pos.x;
	int py = (int)pos.y;

	// Hit edges
	if(px-w<0 || py-h<0 || px+w>=mw || py+h>=mh)
		return PJC_TERRAIN|PJC_MAPBORDER;

	const uchar* gridflags = map->getAbsoluteGridFlags();
	int grid_w = map->getGridWidth();
	int grid_h = map->getGridHeight();
	int grid_cols = map->getGridCols();
	if(grid_w < 2*w+1 || grid_h < 2*h+1 // this ensures, that this check is safe
	|| gridflags[((py-h)/grid_h)*grid_cols + (px-w)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((py+h)/grid_h)*grid_cols + (px-w)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((py-h)/grid_h)*grid_cols + (px+w)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((py+h)/grid_h)*grid_cols + (px+w)/grid_w] & (PX_ROCK|PX_DIRT))
	for(int y=py-h;y<=py+h;y++) {

		uchar *pf = map->GetPixelFlags() + y*mw + px-w;

		for(int x=px-w;x<=px+w;x++) {


			if(!(*pf & PX_EMPTY))
				return PJC_TERRAIN;

			pf++;
		}
	}


	// No collision
	return 0;
}

///////////////////
// Draw the projectile
void CProjectile::Draw(SDL_Surface * bmpDest, CViewport *view)
{
	int wx = view->GetWorldX();
	int wy = view->GetWorldY();
	int l = view->GetLeft();
	int t = view->GetTop();

	int x=((int)vPosition.x-wx)*2+l;
	int y=((int)vPosition.y-wy)*2+t;

	// Clipping on the viewport
	if((x<l || x>l+view->GetVirtW()))
		return;
	if((y<t || y>t+view->GetVirtH()))
		return;

    switch (tProjInfo->Type) {
		case PRJ_PIXEL:
			DrawRectFill2x2(bmpDest, x - 1, y - 1,iColour);
			return;
	
		case PRJ_IMAGE:  {
	
			if(tProjInfo->bmpImage == NULL)
				return;
	
			float framestep = 0;
			

			if(tProjInfo->Animating)
				framestep = fFrame;
			
			// Special angle
			// Basically another way of organising the angles in images
			// Straight up is in the middle, rotating left goes left, rotating right goes right in terms
			// of image index's from the centre
			else if(tProjInfo->UseSpecAngle) {
				CVec dir = vVelocity;
				float angle = (float)( -atan2(dir.x,dir.y) * (180.0f/PI) );
				int direct = 0;
	
				if(angle > 0)
					angle=180-angle;
				if(angle < 0) {
					angle=180+angle;
					direct = 1;
				}
				if(angle == 0)
					direct = 0;
	
	
				int num = (tProjInfo->AngleImages - 1) / 2;
				if(direct == 0)
					// Left side
					framestep = (float)(151-angle) / 151.0f * (float)num;
				else {
					// Right side
					framestep = (float)angle / 151.0f * (float)num;
					framestep += num+1;
				}
			}
	
			// Directed in the direction the projectile is travelling
			else if(tProjInfo->UseAngle) {
				CVec dir = vVelocity;
				float angle = (float)( -atan2(dir.x,dir.y) * (180.0f/PI) );
				float offset = 360.0f / (float)tProjInfo->AngleImages;
	
				FMOD(angle, 360.0f);
	
				framestep = angle / offset;
			}
			
			// Spinning projectile only when moving
			else if(tProjInfo->RotIncrement != 0 && tProjInfo->Rotating && (vVelocity.x != 0 || vVelocity.y != 0))
				framestep = fRotation / (float)tProjInfo->RotIncrement;
			
			
			int size = tProjInfo->bmpImage->h;
			int half = size/2;
			iFrameX = (int)framestep*size;
			MOD(iFrameX, tProjInfo->bmpImage->w);
	
			DrawImageAdv(bmpDest, tProjInfo->bmpImage, iFrameX, 0, x-half, y-half, size,size);
		
			return;
		}
		
		case PRJ_CIRCLE:
			DrawCircleFilled(bmpDest, x, y, radius.x*2, radius.y*2, iColour);
			return;
			
		case PRJ_RECT:
			DrawRectFill(bmpDest, x - radius.x*2, y - radius.y*2, x + radius.x*2, y + radius.x*2, iColour);
			return;
			
		case PRJ_POLYGON:
			getProjInfo()->polygon.drawFilled(bmpDest, (int)vPosition.x, (int)vPosition.y, view, iColour);
			return;
		
		case __PRJ_LBOUND: case __PRJ_UBOUND: errors << "CProjectile::Draw: hit __PRJ_BOUND" << endl;
	}
}


///////////////////
// Draw the projectiles shadow
void CProjectile::DrawShadow(SDL_Surface * bmpDest, CViewport *view)
{
	if (tLX->fDeltaTime >= 0.1f) // Don't draw projectile shadows with FPS <= 10 to get a little better performance
		return;

	CMap* map = cClient->getMap();
	
	int wx = view->GetWorldX();
	int wy = view->GetWorldY();
	int l = view->GetLeft();
	int t = view->GetTop();

	int x=((int)vPosition.x-wx)*2+l;
	int y=((int)vPosition.y-wy)*2+t;

	// Clipping on the viewport
	if((x<l || x>l+view->GetVirtW()))
		return;
	if((y<t || y>t+view->GetVirtH()))
		return;

	switch (tProjInfo->Type)  {
	
		// Pixel
		case PRJ_PIXEL:
			map->DrawPixelShadow(bmpDest, view, (int)vPosition.x, (int)vPosition.y);
			break;
	
		// Image
		case PRJ_IMAGE:  {
	
			if(tProjInfo->bmpImage == NULL)
				return;
			/*if (tProjInfo->bmpImage.get()->w <= 2 && tProjInfo->bmpImage.get()->h <= 2)  {
				map->DrawPixelShadow(bmpDest, view, (int)vPosition.x, (int)vPosition.y);
				return;
			}*/

			int size = tProjInfo->bmpImage->h;
			int half = size / 2;
			map->DrawObjectShadow(bmpDest, tProjInfo->bmpImage, iFrameX, 0, size,size, view, (int)vPosition.x-(half>>1), (int)vPosition.y-(half>>1));
		
			break;	
		}
		
		case PRJ_CIRCLE:
		case PRJ_POLYGON:
		case PRJ_RECT:
			// TODO ...
			break;
		
		case __PRJ_LBOUND: case __PRJ_UBOUND: errors << "CProjectile::DrawShadow: hit __PRJ_BOUND" << endl;
    }
}


///////////////////
// Bounce
// HINT: this is not exactly the way the original LX did it,
//		but this way is way more correct and it seems to work OK
//		(original LX resets the bounce-direction on each checked side)
void CProjectile::Bounce(float fCoeff)
{
	float x,y;
	x=y=1;

	// This code is right, it should be done like that
	// However, we want to keep compatibility with .56 and when on each client would be another simulation,
	// we couldn't call that compatibility at all

	// For now we just keep the old, wrong code, so noone will call OLX players as cheaters
/*	if(CollisionSide & (COL_TOP|COL_BOTTOM)) {
		y=-y;
	}
	if(CollisionSide & (COL_LEFT|COL_RIGHT)) {
		x=-x;
	}

	if(CollisionSide & COL_TOP) {
		y*=fCoeff;
	}
	if(CollisionSide & COL_BOTTOM) {
		y*=fCoeff;
	}
	if(CollisionSide & COL_LEFT) {
		x*=fCoeff;
	}
	if(CollisionSide & COL_RIGHT) {
		x*=fCoeff;
	}*/


	// WARNING: this code should not be used; it is simply wrong
	//	(this was the way the original LX did it)

	if (CollisionSide & COL_TOP)  {
		x = fCoeff;
		y = -fCoeff;
	}
	if (CollisionSide & COL_BOTTOM)  {
		x = fCoeff;
		y = -fCoeff;
	}
	if (CollisionSide & COL_LEFT)  {
		x = -fCoeff;
		y = fCoeff;
	}
	if (CollisionSide & COL_RIGHT)  {
		x = -fCoeff;
		y = fCoeff;
	}


	vVelocity.x *= x;
	vVelocity.y *= y;
}


///////////////////
// Check for collisions with worms
// HINT: this function is not used at the moment
//		(ProjWormColl is used directly from within CheckCollision)
int CProjectile::CheckWormCollision(CWorm *worms)
{
	static const float divisions = 5;
	CVec dir = vPosition - vOldPos;
	float length = NormalizeVector(&dir);

	// Length must be at least 'divisions' in size so we do at least 1 check
	// So stationary projectiles also get checked (mines)
	length = MAX(length, divisions);

	// Go through at fixed positions
	CVec pos = vOldPos;
	int wrm;
	for(float p=0; p<length; p+=divisions, pos += dir*divisions) {
		wrm = ProjWormColl(pos, worms);
		if( wrm >= 0)
			return wrm;
	}

	// AI hack (i know it's dirty, but it's fast)
	// Checks, whether this projectile is heading to any AI worm
	// If so, sets the worm's property Heading to ourself
	CVec mutual_speed;
	CWorm *w = worms;
	for(short i=0;i<MAX_WORMS;i++,w++) {

		// Only AI worms need this
		if (!w->isUsed() || !w->getAlive() || w->getType() != PRF_COMPUTER)
			continue;

		mutual_speed = vVelocity - (*w->getVelocity());

		// This projectile is heading to the worm
		if (SIGN(mutual_speed.x) == SIGN(w->getPos().x - vPosition.x) &&
			SIGN(mutual_speed.y) == SIGN(w->getPos().y - vPosition.y))  {

			int len = (int)mutual_speed.GetLength();
			float dist = 60.0f;

			// Get the dangerous distance for various speeds
			if (len < 30)
				dist = 20.0f;
			else if (len < 60)
				dist = 30.0f;
			else if (len < 90)
				dist = 50.0f;

		}
	}


	// No worms hit
	return -1;
}


///////////////////
// Lower level projectile-worm collision test
// TODO: move to physics?
int CProjectile::ProjWormColl(CVec pos, CWorm *worms)
{
	Shape<int> s; s.pos = pos; s.radius = radius;
	if(tProjInfo->Type == PRJ_CIRCLE)
		s.type = Shape<int>::ST_CIRCLE;
	else {
		// that's LX56 behaviour...
		if(s.radius.x <= 2) s.radius.x = 0;
		if(s.radius.y <= 2) s.radius.y = 0;
	}
	
	CWorm* ownerWorm = NULL;
	if(this->iOwner >= 0 && this->iOwner < MAX_WORMS) {
		ownerWorm = &worms[this->iOwner];
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

bool CProjectile::CollisionWith(const CProjectile* prj) const {
	return CollisionWith(prj, radius.x, radius.y);
}

bool CProjectile::CollisionWith(const CProjectile* prj, int rx, int ry) const {
	Shape<int> s1; s1.pos = vPosition; s1.radius.x = rx; s1.radius.y = ry;
	Shape<int> s2; s2.pos = prj->vPosition; s2.radius = prj->radius;
	if(tProjInfo->Type == PRJ_CIRCLE) s1.type = Shape<int>::ST_CIRCLE;
	if(prj->tProjInfo->Type == PRJ_CIRCLE) s2.type = Shape<int>::ST_CIRCLE;
	
	return s1.CollisionWith(s2);
}


template<bool TOP, bool LEFT>
static CClient::MapPosIndex MPI(const VectorD2<int>& p, const VectorD2<int>& r) {
	return CClient::MapPosIndex( p + VectorD2<int>(LEFT ? -r.x : r.x, TOP ? -r.y : r.y) );
}

template<bool INSERT>
static void updateMap(CProjectile* prj, const VectorD2<int>& p, const VectorD2<int>& r) {
	for(int x = MPI<true,true>(p,r).x; x <= MPI<true,false>(p,r).x; ++x)
		for(int y = MPI<true,true>(p,r).y; y <= MPI<false,true>(p,r).y; ++y) {
			CClient::ProjectileSet* projs = cClient->projPosMap[CClient::MapPosIndex(x,y).index(cClient->getMap())];
			if(projs == NULL) continue;
			if(INSERT)
				projs->insert(prj);
			else
				projs->erase(prj);
		}
}

void CProjectile::updateCollMapInfo(const VectorD2<int>* oldPos, const VectorD2<int>* oldRadius) {
	if(!cClient->getGameScript()->getNeedCollisionInfo()) return;
	
	if(!isUsed()) { // not used anymore
		if(oldPos && oldRadius)
			updateMap<false>(this, *oldPos, *oldRadius);
		return;
	}
	
	if(oldPos && oldRadius) {
		if(
		   (MPI<true,true>(*oldPos,*oldRadius) == MPI<true,true>(vPosition,radius)) &&
		   (MPI<true,false>(*oldPos,*oldRadius) == MPI<true,false>(vPosition,radius)) &&
		   (MPI<false,true>(*oldPos,*oldRadius) == MPI<false,true>(vPosition,radius)) &&
		   (MPI<false,false>(*oldPos,*oldRadius) == MPI<false,false>(vPosition,radius))) {
			return; // nothing has changed
		}
		
		// delete from all
		updateMap<false>(this, *oldPos, *oldRadius);
	}
	
	// add to all
	updateMap<true>(this, vPosition, radius);
}
