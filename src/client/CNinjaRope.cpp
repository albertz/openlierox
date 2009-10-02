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


#include <assert.h>
#include "LieroX.h"

#include "DeprecatedGUI/Graphics.h"
#include "GfxPrimitives.h"
#include "CWorm.h"
#include "MathLib.h"
#include "Entity.h"
#include "CClient.h"
#include "CGameScript.h"


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

	LastReleased = Released;
	LastHookShooting = HookShooting;
	LastHookAttached = HookAttached;
	LastPlayerAttached = PlayerAttached;
	LastWorm = NULL;
}


///////////////////
// Release the ninja rope
// TODO: there is a name-inconvesion between isReleased and Release
void CNinjaRope::Release(void)
{
	// Un-hook the rope from the other worm
	if(HookAttached && PlayerAttached && Worm)
		Worm->setHooked(false,NULL);

	Clear();
}


///////////////////
// Shoot the rope
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
// Draw the thing
void CNinjaRope::Draw(SDL_Surface * bmpDest, CViewport *view, CVec ppos)
{
	if(!Released)
		return;

	int l = view->GetLeft();
	int t = view->GetTop();
	int wx = view->GetWorldX();
	int wy = view->GetWorldY();

	int hx = (int)HookPos.x;
	int hy = (int)HookPos.y;

	// HINT: the hooked worm position could change since the Simulate procedure was called,
	// because the worms are being processed in a "random" order -> we simulate and then the hook worm
	// is simulated -> we have a wrong position, that's why we are correcting it here:
	if(HookAttached && PlayerAttached && Worm) {
		hx = (int)Worm->getPos().x;
		hy = (int)Worm->getPos().y;
		// HINT: don't change HookPos directly here, this should only be done by the simulation-function
	}

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
	DrawRope(bmpDest, hx,hy,px,py,MakeColour(159,79,0));


	// Check clipping against viewport
	if(hx>=l && hx<=l+view->GetVirtW() &&
	   hy>=t && hy<=t+view->GetVirtH()) {

		// Hook
		DrawImage(bmpDest,DeprecatedGUI::gfxGame.bmpHook,hx-2,hy-2);
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

//////////////
// Synchronizes the variables used for check below
void CNinjaRope::updateCheckVariables()
{
	LastReleased = Released;
	LastHookShooting = HookShooting;
	LastHookAttached = HookAttached;
	LastPlayerAttached = PlayerAttached;
	LastWorm = Worm;
	LastWrite = tLX->fCurTime;
}

//////////////
// Returns true if the write function needs to be called
bool CNinjaRope::writeNeeded()
{
	// TODO: why that?
	/*if (!Released)
		return false;*/

	if		((LastReleased != Released) ||
			(LastHookShooting != HookShooting) ||
			(LastHookAttached != HookAttached) ||
			(LastPlayerAttached != PlayerAttached) ||
			(LastWorm != Worm))
				return true;

	return HookPos != OldHookPos;
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

	// Update the "last" variables
	updateCheckVariables();
}


///////////////////
// Read rope details from a bytestream
void CNinjaRope::read(CBytestream *bs, CWorm *worms, int owner)
{
	OldHookPos = HookPos;
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
		if(id >= 0 && id < MAX_WORMS) {
			Worm = &worms[id];
			Worm->setHooked(true, &worms[owner]);

            // Set the hook pos on the worm
            HookPos = Worm->getPos();
		}
	}
}
