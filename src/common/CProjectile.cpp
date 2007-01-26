/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Projectile Class
// Created 11/2/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


///////////////////
// Spawn the projectile
void CProjectile::Spawn(proj_t *_proj, CVec _pos, CVec _vel, int _rot, int _owner, int _random, int _remote, float _remotetime)
{
	tProjInfo = _proj;
	fLife = 0;
	fExtra = 0;
	vOldPos = _pos;
	vPosition = _pos;
	vVelocity = _vel;
	fRotation = (float)_rot;
	iOwner = _owner;
	iUsed = true;
	iSpawnPrjTrl = false;
	fLastTrailProj = -99999;
	iRandom = _random;
	iRemote = _remote;
    iFrameX = 0;

    fTimeVarRandom = GetFixedRandomNum(iRandom);

	if(iRemote)
		fRemoteFrameTime = _remotetime;

	// this produce a memory leak
	fSpeed = _vel.GetLength();

	fFrame = 0;
	iFrameDelta = true;

	firstbounce = true;

	// Choose a colour
	if(tProjInfo->Type == PRJ_PIXEL) {
		int c = GetRandomInt(tProjInfo->NumColours-1);
		if(c == 0)
			iColour = MakeColour(tProjInfo->Colour1[0], tProjInfo->Colour1[1], tProjInfo->Colour1[2]);
		else if(c==1)
			iColour = MakeColour(tProjInfo->Colour2[0], tProjInfo->Colour2[1], tProjInfo->Colour2[2]);
	}

    // Events
    nExplode = false;
    nTouched = false;
}


///////////////////
// Gets a random float from a special list
float CProjectile::getRandomFloat(void)
{
	float r = GetFixedRandomNum(iRandom++);

	iRandom %= 255;

	return r;
}


///////////////////
// Simulate the projectile
//
// Returns flags for anything that happened to the projectile
int CProjectile::Simulate(float dt, CMap *map, CWorm *worms, int *wormid)
{
    int res = PJC_NONE;

	// If this is a remote projectile, the first frame is simulated with a longer delta time
	if(iRemote) {
		iRemote = false;

		// Only do it for a positive delta time
		if(fRemoteFrameTime>0) {
			res = Simulate(fRemoteFrameTime, map,worms,wormid);
            if( res != PJC_NONE )
				return res;
		}

		// Don't leave, coz we still need to process it with a normal delta time
	}

	// If the dt is too great, half the simulation time & run it twice
	if(dt > 0.15f) {
		dt /= 2;
		res = Simulate(dt,map,worms,wormid);
        if( res != PJC_NONE )
			return res;

		return Simulate(dt,map,worms,wormid);
	}


    // If any of the events have been triggered, add that onto the flags
    if( nExplode && tLX->fCurTime > fExplodeTime) {
        res |= PJC_EXPLODE;
        nExplode = false;
    }
    if( nTouched ) {
        res |= PJC_TOUCH;
        nTouched = false;
    }


	fLife += dt;
	fExtra += dt;


	vOldPos = vPosition;
	if(tProjInfo->UseCustomGravity)
		vVelocity.y += (float)(tProjInfo->Gravity)*dt;
	else
		vVelocity.y += 100*dt;

	vPosition += vVelocity*dt;

	// Dampening
	if(tProjInfo->Dampening != 1)
		vVelocity *= tProjInfo->Dampening;

	if(tProjInfo->Rotating)
		fRotation += (float)tProjInfo->RotSpeed*dt;
	if(fRotation < 0)
		fRotation = 360;
	else if(fRotation > 360)
		fRotation = 0;

	// Animation
	if(tProjInfo->Animating) {
		if(iFrameDelta)
			fFrame += (float)tProjInfo->AnimRate*dt;
		else
			fFrame -= (float)tProjInfo->AnimRate*dt;

		if(tProjInfo->bmpImage) {
			int NumFrames = tProjInfo->bmpImage->w / tProjInfo->bmpImage->h;
			if(fFrame >= NumFrames) {
				switch(tProjInfo->AnimType) {
				case ANI_ONCE:
					iUsed = false;
					break;
				case ANI_LOOP:
					fFrame = 0;
					break;
				case ANI_PINGPONG:
					iFrameDelta = !iFrameDelta;
					fFrame = (float)NumFrames-1;
				}
			}
			if(fFrame < 0) {
				if(tProjInfo->AnimType == ANI_PINGPONG) {
					iFrameDelta = !iFrameDelta;
					fFrame = 0.0f;
				}
			}
		}
	}

	// Trails
	switch(tProjInfo->Trail) {
	case TRL_SMOKE:
		if(fExtra > 0.075f) {
			fExtra=0;
			SpawnEntity(ENT_SMOKE,0,vPosition,CVec(0,0),0,NULL);
		}
		break;
	case TRL_CHEMSMOKE:
		if(fExtra > 0.075f) {
			fExtra=0;
			SpawnEntity(ENT_CHEMSMOKE,0,vPosition,CVec(0,0),0,NULL);
		}
		break;
	case TRL_DOOMSDAY:
		if(fExtra > 0.05f) {
			fExtra=0;
			SpawnEntity(ENT_DOOMSDAY,0,vPosition,vVelocity,0,NULL);
		}
		break;
	case TRL_EXPLOSIVE:
		if(fExtra > 0.05f) {
			fExtra=0;
			SpawnEntity(ENT_EXPLOSION,10,vPosition,CVec(0,0),0,NULL);
		}
		break;
	case TRL_PROJECTILE: // Projectile trail
		if(tLX->fCurTime > fLastTrailProj) {
			fLastTrailProj = tLX->fCurTime + tProjInfo->PrjTrl_Delay;

			// Set the spawning to true so the upper layers of code (client) will spawn the projectiles
			iSpawnPrjTrl = true;
		}
	}


    // Check worm collisions
    int w = CheckWormCollision(worms);
    if( w >= 0 ) {
        *wormid = w;
        res |= PJC_WORM;
    }


	// Hack!!!
	if(tProjInfo->Hit_Type == PJ_EXPLODE && tProjInfo->Type == PRJ_PIXEL) {
		int px = (int)vPosition.x;
		int py = (int)vPosition.y;

		// Edge checks
		if(px<=0 || py<=0 || px>=map->GetWidth()-1 || py>=map->GetHeight()-1) {
			// Clamp the position
			px = MAX(px,0);
			py = MAX(py,0);
			px = MIN(map->GetWidth()-1,px);
			py = MIN(map->GetHeight()-1,py);
            res |= PJC_TERRAIN;
            return res;
		}

		uchar pf = map->GetPixelFlag(px, py);
		if(pf & PX_DIRT || pf & PX_ROCK)
			res |= PJC_TERRAIN;

        return res;
	}


	// Check for collisions
    if( CheckCollision(dt, map, vOldPos, vVelocity) )
        res |= PJC_TERRAIN;

    return res;
}


///////////////////
// Check for a collision
// Returns true if there was a collision, otherwise false is returned
int CProjectile::CheckCollision(float dt, CMap *map, CVec pos, CVec vel)
{
	// Check if it hit the terrain
	int mw = map->GetWidth();
	int mh = map->GetHeight();
	int w,h;
	int px,py,x,y;

	if(tProjInfo->Type == PRJ_PIXEL)
		w=h=1;
	else // TODO: was this 'else' missing here?
		w=h=2;

	float maxspeed2 = (float)(4*w*w+4*w+1); // (2w+1)^2
	if( (vel*dt).GetLength2() > maxspeed2) {
		dt *= 0.5f;

		if(CheckCollision(dt,map,pos,vel))
			return true;

		pos += vel*dt;

		return CheckCollision(dt,map,pos,vel);
	}

	pos += vel*dt;

	px=(int)pos.x;
	py=(int)pos.y;

	CollisionSide = 0;
	short top,bottom,left,right;
	top=bottom=left=right=0;

	// Hit edges
	if(px-w<0 || py-h<0 || px+w>=mw || py+h>=mh) {

		// Check the collision side
		if(px-w<0) {
			px = w;
			CollisionSide |= COL_LEFT;
		}
		if(py-h<0) {
			py = h;
			CollisionSide |= COL_TOP;
		}
		if(px+w>=mw) {
			px = mw-w;
			CollisionSide |= COL_RIGHT;
		}
		if(py+h>=mh) {
			py = mh-h;
			CollisionSide |= COL_BOTTOM;
		}

		vPosition.x = (float)px;
		vPosition.y = (float)py;

		return true;
	}

	const uchar* gridflags = map->getAbsoluteGridFlags();
	int grid_w = map->getGridWidth();
	int grid_h = map->getGridHeight();
	int grid_cols = map->getGridCols();
	if(grid_w < 2*w+1 || grid_h < 2*h+1 // this ensures, that this check is safe
	|| gridflags[((py-h)/grid_h)*grid_cols + (px-w)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((py+h)/grid_h)*grid_cols + (px-w)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((py-h)/grid_h)*grid_cols + (px+w)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((py+h)/grid_h)*grid_cols + (px+w)/grid_w] & (PX_ROCK|PX_DIRT))
	for(y=py-h;y<=py+h;y++) {

		// Clipping means that it has collided
		if(y<0)	{
			CollisionSide |= COL_TOP;
			return true;
		}
		if(y>=mh) {
			CollisionSide |= COL_BOTTOM;
			return true;
		}


		const uchar *pf = map->GetPixelFlags() + y*mw + px-w;

		for(x=px-w;x<=px+w;x++) {

			// Clipping
			if(x<0) {
				CollisionSide |= COL_LEFT;
				return true;
			}
			if(x>=mw) {
				CollisionSide |= COL_RIGHT;
				return true;
			}

			if(*pf & PX_DIRT || *pf & PX_ROCK) {
				if(y<py)
					top++;
				else if(y>py)
					bottom++;
				if(x<px)
					left++;
				else if(x>px)
					right++;
			}

			pf++;
		}
	}


	// Check for a collision
	if(top || bottom || left || right) {
		CollisionSide = 0;

		if(tProjInfo->Hit_Type == PJ_EXPLODE) {
			//vPosition = pos;
			return true;
		}


		// Bit of a hack
		if(tProjInfo->Hit_Type == PJ_BOUNCE)
			pos = vOldPos;
		else
			vPosition = pos;

		// Find the collision side
		if( (left>right || left>2) && left>1 && vVelocity.x < 0) {
			if(tProjInfo->Hit_Type == PJ_BOUNCE)
				vPosition.x=( pos.x );
			CollisionSide |= COL_LEFT;
		}

		if( (right>left || right>2) && right>1 && vVelocity.x > 0) {
			if(tProjInfo->Hit_Type == PJ_BOUNCE)
				vPosition.x=( pos.x );
			CollisionSide |= COL_RIGHT;
		}

		if(top>1 && vVelocity.y < 0) {
			if(tProjInfo->Hit_Type == PJ_BOUNCE)
				vPosition.y=( pos.y );
			CollisionSide |= COL_TOP;
		}

		if(bottom>1 && vVelocity.y > 0) {
			if(tProjInfo->Hit_Type == PJ_BOUNCE)
				vPosition.y=( pos.y );
			CollisionSide |= COL_BOTTOM;
		}

		// If the velocity is too low, just stop me
		/*if(fabs(vVelocity.x) < 2)
			vVelocity.x=(0);
		if(fabs(vVelocity.y) < 2)
			vVelocity.y=(0);*/

		return true;
	}

	return false;
}

///////////////////
// Check for a collision (static version; doesnt do anything else then checking)
// Returns true if there was a collision, otherwise false is returned
int CProjectile::CheckCollision(proj_t* tProjInfo, float dt, CMap *map, CVec pos, CVec vel)
{
	// Check if it hit the terrain
	int mw = map->GetWidth();
	int mh = map->GetHeight();
	int w,h;
	int px,py,x,y;

	if(tProjInfo->Type == PRJ_PIXEL)
		w=h=1;
	else // TODO: was this 'else' missing here?
		w=h=2;

	float maxspeed2 = (float)(4*w*w+4*w+1); // (2w+1)^2
	if( (vel*dt).GetLength2() > maxspeed2) {
		dt *= 0.5f;

		if(CheckCollision(tProjInfo,dt,map,pos,vel))
			return true;

		pos += vel*dt;

		return CheckCollision(tProjInfo,dt,map,pos,vel);
	}

	pos += vel*dt;

	px=(int)pos.x;
	py=(int)pos.y;

	short top,bottom,left,right;
	top=bottom=left=right=0;

	// Hit edges
	if(px-w<0 || py-h<0 || px+w>=mw || py+h>=mh) {

		return true;
	}

	const uchar* gridflags = map->getAbsoluteGridFlags();
	int grid_w = map->getGridWidth();
	int grid_h = map->getGridHeight();
	int grid_cols = map->getGridCols();
	if(grid_w < 2*w+1 || grid_h < 2*h+1 // this ensures, that this check is safe
	|| gridflags[((py-h)/grid_h)*grid_cols + (px-w)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((py+h)/grid_h)*grid_cols + (px-w)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((py-h)/grid_h)*grid_cols + (px+w)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((py+h)/grid_h)*grid_cols + (px+w)/grid_w] & (PX_ROCK|PX_DIRT))
	for(y=py-h;y<=py+h;y++) {

		// Clipping means that it has collided
		if(y<0 || y>=mh)	{
			return true;
		}

		const uchar *pf = map->GetPixelFlags() + y*mw + px-w;

		for(x=px-w;x<=px+w;x++) {

			// Clipping
			if(x<0 || x>=mw) {
				return true;
			}

			if(*pf & PX_DIRT || *pf & PX_ROCK) {
				if(y<py)
					top++;
				else if(y>py)
					bottom++;
				if(x<px)
					left++;
				else if(x>px)
					right++;
			}

			pf++;
		}
	}


	// Check for a collision
	if(top || bottom || left || right) {
		return true;
	}

	return false;
}

///////////////////
// Draw the projectile
void CProjectile::Draw(SDL_Surface *bmpDest, CViewport *view)
{
	int wx = view->GetWorldX();
	int wy = view->GetWorldY();
	int l = view->GetLeft();
	int t = view->GetTop();
	float framestep;

	int x=((int)vPosition.x-wx)*2+l;
	int y=((int)vPosition.y-wy)*2+t;

	// Clipping on the viewport
	if(x<l || x>l+view->GetVirtW())
		return;
	if(y<t || y>t+view->GetVirtH())
		return;

    if(tProjInfo->Type == PRJ_PIXEL) {
		DrawRectFill(bmpDest,x-1,y-1,x+1,y+1,iColour);
        return;
    }
	else if(tProjInfo->Type == PRJ_IMAGE) {
		if(tProjInfo->bmpImage == NULL)
			return;

		// Spinning projectile only when moving
		if(tProjInfo->Rotating && (vVelocity.x != 0 || vVelocity.y != 0))
			framestep = fRotation / (float)tProjInfo->RotIncrement;
		else
			framestep = 0;

		// Directed in the direction the projectile is travelling
		if(tProjInfo->UseAngle) {
			CVec dir = vVelocity;
			float angle = (float)( -atan2(dir.x,dir.y) * (180.0f/PI) );
			float offset = 360.0f / (float)tProjInfo->AngleImages;

			if(angle < 0)
				angle+=360;
			if(angle > 360)
				angle-=360;

			if(angle == 360)
				angle=0;

			framestep = angle / offset;
		}

		// Special angle
		// Basically another way of organising the angles in images
		// Straight up is in the middle, rotating left goes left, rotating right goes right in terms
		// of image index's from the centre
		if(tProjInfo->UseSpecAngle) {
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
//			int middle = num*tProjInfo->bmpImage->h; // TODO: not used
			if(direct == 0)
				// Left side
				framestep = (float)(151-angle) / 151.0f * (float)num;
			else {
				// Right side
				framestep = (float)angle / 151.0f * (float)num;
				framestep += num+1;
			}
		}

		if(tProjInfo->Animating)
			framestep = fFrame;

		int size = tProjInfo->bmpImage->h;
		int half = size/2;
        iFrameX = (int)framestep*size;

		DrawImageAdv(bmpDest, tProjInfo->bmpImage, (int)framestep*size, 0, x-half, y-half, size,size);
	}
}


///////////////////
// Draw the projectiles shadow
void CProjectile::DrawShadow(SDL_Surface *bmpDest, CViewport *view, CMap *map)
{
    int wx = view->GetWorldX();
	int wy = view->GetWorldY();
	int l = view->GetLeft();
	int t = view->GetTop();

    int x=((int)vPosition.x-wx)*2+l;
	int y=((int)vPosition.y-wy)*2+t;

	// Clipping on the viewport
	if(x<l || x>l+view->GetVirtW())
		return;
	if(y<t || y>t+view->GetVirtH())
		return;

    // Pixel
    if(tProjInfo->Type == PRJ_PIXEL)
        map->DrawPixelShadow(bmpDest, view, (int)vPosition.x, (int)vPosition.y);

    // Image
    else if(tProjInfo->Type == PRJ_IMAGE) {
        if(tProjInfo->bmpImage == NULL)
            return;

        int size = tProjInfo->bmpImage->h;
        int half = size/2;
        map->DrawObjectShadow(bmpDest, tProjInfo->bmpImage, iFrameX, 0, size,size, view, (int)vPosition.x-(half>>1), (int)vPosition.y-(half>>1));
    }
}


///////////////////
// Bounce
void CProjectile::Bounce(float fCoeff)
{
	float x,y;
	x=y=1;

	float Bounce = fCoeff;

	/*if(firstbounce) {
		printf("Bounce; %d, %d, %d, %d\n",(CollisionSide & COL_TOP)==COL_TOP,
										  (CollisionSide & COL_BOTTOM)==COL_BOTTOM,
										  (CollisionSide & COL_LEFT)==COL_LEFT,
										  (CollisionSide & COL_RIGHT)==COL_RIGHT);
		firstbounce = false;
	}*/

//	int w,h;
//	w=h=3;

//	CVec pos = vPosition;
	if(CollisionSide & COL_TOP) {
		x=Bounce; y=-Bounce;
		//vPosition.y=( vOldPos.y );
	}
	if(CollisionSide & COL_BOTTOM) {
		x=Bounce; y=-Bounce;
		//vPosition.y=( vOldPos.y );
	}

	if(CollisionSide & COL_LEFT) {
		x=-Bounce; y=Bounce;
		//vPosition.x=( vOldPos.x );
	}
	if(CollisionSide & COL_RIGHT) {
		x=-Bounce; y=Bounce;
		//vPosition.x=( vOldPos.x );
	}

	vVelocity.x *= x;
	vVelocity.y *= y;

//	CVec dir = vVelocity;
//	NormalizeVector(&dir);

	//vPosition = vPosition + dir*2;
}


///////////////////
// Check for collisions with worms
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

			float real_dist2 = (vPosition - w->getPos()).GetLength2();
			if (real_dist2 <= dist*dist) {
				// If any projectile is already heading to the worm and is closer than we, don't set us as heading
				if (w->getHeading())  {
					if ((w->getPos() - w->getHeading()->GetPosition()).GetLength2() >= dist*dist)
						w->setHeading(this);
				}
				// No projectile heading to the worm
				else
					w->setHeading(this);
			}

		}
	}


	// No worms hit
	return -1;
}


///////////////////
// Lower level projectile-worm collision test
int CProjectile::ProjWormColl(CVec pos, CWorm *worms)
{
	int px = (int)pos.x;
	int py = (int)pos.y;
	int wx,wy;

	const static int wsize = 4;

	CWorm *w = worms;
	for(short i=0;i<MAX_WORMS;i++,w++) {
		if(!w->isUsed() || !w->getAlive())
			continue;

		wx = (int)w->getPos().x;
		wy = (int)w->getPos().y;

		// AABB - Point test
		if( abs(wx-px) < wsize && abs(wy-py) < wsize) {

			CollisionSide = 0;

			// Calculate the side of the collision (obsolete??)
			if(px < wx-2)
				CollisionSide |= COL_LEFT;
			else if(px > wx+2)
				CollisionSide |= COL_RIGHT;
			if(py < wy-2)
				CollisionSide |= COL_TOP;
			else if(py > wy+2)
				CollisionSide |= COL_BOTTOM;

			return i;
		}
	}

	// No worm was hit
	return -1;
}
