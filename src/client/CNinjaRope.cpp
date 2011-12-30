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
#include "game/Game.h"
#include "Geometry.h"


///////////////////
// Clear the ninja rope vars
void CNinjaRope::Clear()
{
	owner = NULL;
	
	Released = false;
	HookShooting = false;
	HookAttached = false;
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
void CNinjaRope::Release()
{
	// Un-hook the rope from the other worm
	if(HookAttached && PlayerAttached && Worm)
		Worm->setHooked(false,NULL);

	Clear();
}


///////////////////
// Shoot the rope
void CNinjaRope::Shoot(CWorm* owner, CVec pos, CVec dir)
{
	Clear();

	if(!owner->canUseNinja()) return;
	
	this->owner = owner;
	
	Released = true;
	HookShooting = true;
	HookAttached = false;
	PlayerAttached = false;
	Worm = NULL;

	this->pos() = pos;
	HookDir = dir;
	HookVelocity = dir * (float)cClient->getGameLobby()[FT_RopeSpeed];
	
	if(cClient->getGameLobby()[FT_RopeAddParentSpeed])
		HookVelocity += owner->getVelocity();
}


///////////////////
// Setup the details from the gamescript
void CNinjaRope::Setup()
{}



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

	int hx = (int)pos().x;
	int hy = (int)pos().y;

	// HINT: the hooked worm position could change since the Simulate procedure was called,
	// because the worms are being processed in a "random" order -> we simulate and then the hook worm
	// is simulated -> we have a wrong position, that's why we are correcting it here:
	if(HookAttached && PlayerAttached && Worm) {
		hx = (int)Worm->getPos().x;
		hy = (int)Worm->getPos().y;
		// HINT: don't change HookPos directly here, this should only be done by the simulation-function
	}

	hx = (hx-wx)*2+l;
	hy = (hy-wy)*2+t;

	int px = ((int)ppos.x-wx)*2+l;
	int py = ((int)ppos.y-wy)*2+t;

	// Rope
	px -= px % 2;
	py -= py % 2;
	hx -= hx % 2;
	hy -= hy % 2;
	DrawRope(bmpDest, hx,hy,px,py,Color(159,79,0));


	// Check clipping against viewport
	if(hx>=l && hx<=l+view->GetVirtW() &&
	   hy>=t && hy<=t+view->GetVirtH()) {

		// Hook
		DrawImage(bmpDest,DeprecatedGUI::gfxGame.bmpHook,hx-2,hy-2);
	}

	// The clipping on the viewport is done in the line function
}

bool CNinjaRope::isInside(int x, int y) {
	if(!Released) return false;
	if(!owner) return false;
	if(!owner->getAlive()) return false;
	
	Line l(owner->pos(), pos());
	return l.distFromPoint2(VectorD2<int>(x, y)) < 2.0f;
}

Color CNinjaRope::renderColorAt(/* relative coordinates */ int x, int y) {
	x += (int)pos().x;
	y += (int)pos().y;
	if(isInside(x, y)) return Color(159,79,0);
	return Color(0,0,0,SDL_ALPHA_TRANSPARENT);
}

////////////////////
// Unattaches rope from a worm
void CNinjaRope::UnAttachPlayer()
{
	if (!Worm)
		return;

	HookVelocity.x = HookVelocity.y = 0;
	HookShooting = false;
	HookAttached = false;
	PlayerAttached = false;
	Worm->setHooked(false, NULL);
	Worm = NULL;
}

void CNinjaRope::AttachToPlayer(CWorm *worm, CWorm *owner)
{
	HookShooting = false;
	HookAttached = true;
	PlayerAttached = true;
	Worm = worm;
	worm->setHooked(true, owner);
}


///////////////////
// Return the pulling force
CVec CNinjaRope::GetForce(CVec playerpos)
{
	if(!HookAttached)
		return CVec(0,0);

	return CalculateForce(playerpos);//,HookPos);
}


///////////////////
// Calculate the pulling force
CVec CNinjaRope::CalculateForce(CVec playerpos)
{
	CVec dir = playerpos-pos();
	dir = dir.Normalize();

	const int RestLength = cClient->getGameLobby()[FT_RopeRestLength];
	if((playerpos-pos()).GetLength2() < RestLength*RestLength)
		return CVec(0,0);

	// Make sure the pull isn't huge
	const float Strength = cClient->getGameLobby()[FT_RopeStrength];
	float l = -Strength;

	dir *= l*100;

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
	LastWrite = tLX->currentTime;
}

//////////////
// Returns true if the write function needs to be called
bool CNinjaRope::writeNeeded()
{
	if		((LastReleased != Released) ||
			(LastHookShooting != HookShooting) ||
			(LastHookAttached != HookAttached) ||
			(LastPlayerAttached != PlayerAttached) ||
			(LastWorm != Worm))
				return true;

	return pos() != OldHookPos;
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
	short x = (short)pos().x;
	short y = (short)pos().y;

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
	this->owner = &worms[owner];
	OldHookPos = pos();
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
	pos() = CVec( float(x), float(y) );

	// Angle
	if(type == ROP_SHOOTING) {
		int a = bs->readByte();
		int angle = 360*a/256;
		GetVecsFromAngle((float)angle, &HookDir, NULL);
	}

	// Worm id
	if(type == ROP_PLYHOOKED) {
		int id = bs->readByte();
		if(id >= 0 && id < MAX_WORMS) {
			Worm = &worms[id];
			Worm->setHooked(true, &worms[owner]);

            // Set the hook pos on the worm
            pos() = Worm->getPos();
		}
	}
}
