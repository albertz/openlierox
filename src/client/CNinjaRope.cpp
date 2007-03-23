/////////////////////////////////////////
//
//                  OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Ninja rope class
// Created 6/2/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "GfxPrimitives.h"


///////////////////
// Clear the ninja rope vars
void CNinjaRope::Clear(void)
{
	Released = false;
	HookShooting = false;
	HookAttached = false;
	//RopeLength = false;
	PlayerAttached = false;
	Worm = NULL;
}


///////////////////
// Release the ninja rope
void CNinjaRope::Release(void)
{
	// Un-hook the rope from the other worm
	if(HookAttached && PlayerAttached && Worm)
		Worm->setHooked(false,NULL);

	Clear();
}


///////////////////
// Shoot the fucka
void CNinjaRope::Shoot(CVec pos, CVec dir)
{
	Clear();

	Released = true;
	HookShooting = true;
	HookAttached = false;
	PlayerAttached = false;
	Worm = NULL;
	//RopeLength = 300;
	//RestLength = 20;

	HookPos = pos;
	HookDir = dir;
	HookVelocity = dir*250;
}


///////////////////
// Setup the details from the gamescript
void CNinjaRope::Setup(CGameScript *gs)
{
	assert( gs );

	RopeLength = (float)gs->getRopeLength();
	RestLength = (float)gs->getRestLength();
	Strength = gs->getStrength();
}


///////////////////
// Simulate the thing
void CNinjaRope::Simulate(float dt, CMap *map, CVec playerpos, CWorm *worms, int owner)
{
	if(!Released)
		return;

	float length2;
	int firsthit = !HookAttached;
	CVec force;

	if(HookShooting)
		force = CVec(0,100);
	else
		force = CVec(0,150);


	// Still flying in the air
	if(HookShooting) {

		// Gravity
		HookVelocity += force*dt;
		HookPos += HookVelocity*dt;

		length2 = (playerpos-HookPos).GetLength2();

		// Check if it's too long
		if(length2 > RopeLength*RopeLength) {
			HookVelocity = CVec(0,0);
			HookShooting = false;
		}
	}
	// Failing
	else if(!HookShooting && !HookAttached) {

		// Going towards the player
		length2 = (playerpos-HookPos).GetLength2();
		if(length2 > RestLength*RestLength) {

			// Pull the hook back towards the player
			CVec d = playerpos - HookPos;
			if(length2) d *= (float)(1.0f/sqrt(length2)); // normalize

			force += (d*10000)*dt;
		}

		HookVelocity += force*dt;
		HookPos += HookVelocity*dt;

		//HookPos = HookPos + CVec(0,170*dt);
	}

    bool outsideMap = false;

	// Hack to see if the hook went out of the map
	if(HookPos.x <= 0 || HookPos.y <= 0 ||
	   HookPos.x >= map->GetWidth()-1 || HookPos.y >= map->GetHeight()-1) {
		HookShooting = false;
		HookAttached = true;
		PlayerAttached = false;

		// Make the hook stay at an edge
		HookPos.x=( MAX((float)0,HookPos.x) );
		HookPos.y=( MAX((float)0,HookPos.y) );

		HookPos.x=( MIN(map->GetWidth()-(float)1,HookPos.x) );
		HookPos.y=( MIN(map->GetHeight()-(float)1,HookPos.y) );

        outsideMap = true;
	}


	// Check if the hook has hit anything on the map
	if(!PlayerAttached)
		HookAttached = false;

	uchar px = map->GetPixelFlag((int)HookPos.x,(int)HookPos.y);
	if((px & PX_ROCK || px & PX_DIRT || outsideMap) && !PlayerAttached) {
		HookShooting = false;
		HookAttached = true;
		PlayerAttached = false;
		HookVelocity = CVec(0,0);

		if(px & PX_DIRT && firsthit) {
			Uint32 col = GetPixel(map->GetImage(),(int)HookPos.x,(int)HookPos.y);
            for( short i=0; i<5; i++ )
			    SpawnEntity(ENT_PARTICLE,0,HookPos+CVec(0,2),CVec(GetRandomNum()*40,GetRandomNum()*40),col,NULL);
		}
	}


	// Check if the hook has hit another worm
	if(!HookAttached && !PlayerAttached) {
		PlayerAttached = false;

		for(short i=0; i<MAX_WORMS; i++) {
			// Don't check against the worm if they aren't used, dead or the ninja rope was shot by the worm
			if(!worms[i].isUsed())
				continue;
			if(!worms[i].getAlive())
				continue;
			if(worms[i].getID() == owner)
				continue;

			if( ( worms[i].getPos() - HookPos ).GetLength2() < 25 ) {
				HookAttached = true;
				PlayerAttached = true;
				Worm = &worms[i];
				Worm->setHooked(true,&worms[owner]);
				break;
			}
		}
	}

	// Put the hook where the worm is
	else if(HookAttached && PlayerAttached) {

		// If the worm has been killed, or dropped, drop the hook
		assert( Worm );
		if(!Worm->isUsed() || !Worm->getAlive()) {
			HookVelocity = CVec(0,0);
			HookShooting = false;
			HookAttached = false;
			PlayerAttached = false;
			Worm = NULL;
		} else {
			HookPos = Worm->getPos();
		}
	}
}


///////////////////
// Draw the thing
void CNinjaRope::Draw(SDL_Surface *bmpDest, CViewport *view, CVec ppos)
{
	if(!Released)
		return;

	int l = view->GetLeft();
	int t = view->GetTop();
	int wx = view->GetWorldX();
	int wy = view->GetWorldY();

	int hx = (int)HookPos.x;
	int hy = (int)HookPos.y;

	int px = (int)ppos.x;
	int py = (int)ppos.y;

	px = ((int)ppos.x-wx)*2+l;
	py = ((int)ppos.y-wy)*2+t;

	hx = (hx-wx)*2+l;
	hy = (hy-wy)*2+t;


	// Rope
	px -= px % 2;
	py -= py % 2;
	hx -= hx % 2;
	hy -= hy % 2;
	DrawRope(bmpDest, hx,hy,px,py,SDL_MapRGB(bmpDest->format,159,79,0));


	// Check clipping against viewport
	if(hx>=l && hx<=l+view->GetVirtW() &&
	   hy>=t && hy<=t+view->GetVirtH()) {

		// Hook
		DrawImage(bmpDest,gfxGame.bmpHook,hx-2,hy-2);
	}

	// The clipping on the viewport is done in the line function
}


///////////////////
// Return the pulling force
CVec CNinjaRope::GetForce(CVec playerpos)
{
	if(!HookAttached)
		return CVec(0,0);

	return CalculateForce(playerpos,HookPos);
}


///////////////////
// Calculate the pulling force
CVec CNinjaRope::CalculateForce(CVec playerpos, CVec hookpos)
{
	// TODO: what is the dif between hookpos and HookPos ???

	float length2 = (playerpos-HookPos).GetLength2();

	CVec dir = playerpos-hookpos;
	dir = dir.Normalize();

	//float l = MIN((float)0,RestLength-length);
	float l;

	if(length2 < RestLength*RestLength)
		return CVec(0,0);

	// Make sure the pull isn't huge
	//l = MAX(-40,l);
	l = -Strength;

	dir *= l*100;//*Strength;

	return dir;
}



///////////////////
// Write out the rope details to a bytestream
void CNinjaRope::write(CBytestream *bs)
{
	int type = ROP_NONE;

	if(HookShooting)
		type = ROP_SHOOTING;
	else if(HookAttached) {
		if(PlayerAttached)
			type = ROP_PLYHOOKED;
		else
			type = ROP_HOOKED;
	}
	else
		type = ROP_FALLING;

	bs->writeByte( type );

	// Position
	short x = (short)HookPos.x;
	short y = (short)HookPos.y;

	// Write out position of the hook
	bs->write2Int12( x, y );


	// Calculate the heading angle that the hook is travelling
	float heading = (float)( -atan2(HookDir.x,HookDir.y) * (180.0f/PI) );
	heading+=90;
	if(heading < 0)
		heading+=360;
	if(heading > 360)
		heading-=360;
	if(heading == 360)
		heading=0;

	// Write out the direction is shooting out
	if(type == ROP_SHOOTING) {
		// Convert angle to fixed 256
		int a = (int)(256*heading/360)&255;
		bs->writeByte( a );
	}

	// Write out the worm id the hook is stuck to
	if(type == ROP_PLYHOOKED) {
		bs->writeByte( Worm->getID() );
	}
}


///////////////////
// Read rope details from a bytestream
void CNinjaRope::read(CBytestream *bs, CWorm *worms, int owner)
{
	int type = bs->readByte();
	Released = true;
	Worm = NULL;

	switch(type) {
		case ROP_SHOOTING:
			HookShooting = true;
			HookAttached = false;
			PlayerAttached = false;
			break;
		case ROP_HOOKED:
			HookShooting = false;
			HookAttached = true;
			PlayerAttached = false;
			break;
		case ROP_FALLING:
			HookShooting = false;
			HookAttached = false;
			PlayerAttached = false;
			break;
		case ROP_PLYHOOKED:
			HookShooting = false;
			HookAttached = true;
			PlayerAttached = true;
			break;
	}

	// Position
	short x, y;
	bs->read2Int12( x, y );
	HookPos.x=( (float)x );
	HookPos.y=( (float)y );

	// Angle
	if(type == ROP_SHOOTING) {
		int a = bs->readByte();
		int angle = 360*a/256;
		GetAngles(angle, &HookDir, NULL);
	}

	// Worm id
	if(type == ROP_PLYHOOKED) {
		int id = bs->readByte();
		if(id >= 0 && id<MAX_WORMS-1) {
			Worm = &worms[id];
			Worm->setHooked(true, &worms[owner]);

            // Set the hook pos on the worm
            HookPos = Worm->getPos();
		}
	}
}
