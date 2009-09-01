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
void CNinjaRope::Clear()
{
	Released = false;
	HookShooting = false;
	HookAttached = false;
	//RopeLength = false;
	PlayerAttached = false;
	Worm = NULL;
	CrossedHorizontal = CrossedVertical = 0;
	OldReceivedPos = VectorD2<int>(0, 0);

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
	
	Released = true;
	HookShooting = true;
	HookAttached = false;
	PlayerAttached = false;
	Worm = NULL;
	CrossedHorizontal = owner->crossedHorizontal();
	CrossedVertical = owner->crossedVertical();
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
void CNinjaRope::Draw(SDL_Surface * bmpDest, CViewport *view, CWorm *owner)
{
	if(!Released)
		return;

	const int mapw = cClient->getMap()->GetWidth();
	const int maph = cClient->getMap()->GetHeight();
	VectorD2<int> ppos((int)owner->getPos().x, (int)owner->getPos().y);
	const bool wrapAround = cClient->getGameLobby()->features[FT_InfiniteMap];

	int l = view->GetLeft();
	int t = view->GetTop();
	int wx = view->GetWorldX();
	int wy = view->GetWorldY();

	int hx = (int)HookPos.x - wx;
	int hy = (int)HookPos.y - wy;

	int dx = Round(HookPos.x - owner->getPos().x);
	int dy = Round(HookPos.y - owner->getPos().y);

	// HINT: the hooked worm position could change since the Simulate procedure was called,
	// because the worms are being processed in a "random" order -> we simulate and then the hook worm
	// is simulated -> we have a wrong position, that's why we are correcting it here:
	if(HookAttached && PlayerAttached && Worm) {
		hx = (int)Worm->getPos().x - wx;
		hy = (int)Worm->getPos().y - wy;
		// HINT: don't change HookPos directly here, this should only be done by the simulation-function
	}

	int px = ppos.x - wx;
	int py = ppos.y - wy;

	if (wrapAround)  {
		px += mapw; px %= mapw;
		py += maph; py %= maph;

		hx = px + dx;
		hy = py + dy;
	}

	px = px*2+l;
	py = py*2+t;

	hx = hx*2+l;
	hy = hy*2+t;


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
	float length2 = (playerpos-HookPos).GetLength2();

	CVec dir = playerpos-HookPos;
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

//////////////////
// Adjusts the hook position in infinite map
void CNinjaRope::wrapAround(CWorm *owner, CVec playerpos)
{
	if (!cClient->getGameLobby()->features[FT_InfiniteMap] || !Released)
		return;

	float mapW = (float)cClient->getMap()->GetWidth();
	float mapH = (float)cClient->getMap()->GetHeight();

	int hdiff = CrossedHorizontal - owner->crossedHorizontal();
	int vdiff = CrossedVertical - owner->crossedVertical();

	HookPos.x -= CrossedHorizontal * mapW;
	FMOD<float>(HookPos.x, mapW);
	HookPos.x += hdiff * mapW;

	HookPos.y -= CrossedVertical * mapH;
	FMOD<float>(HookPos.y, mapH);
	HookPos.y += vdiff * mapH;
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
	short x = cClient->getMap()->WrapAroundX((short)HookPos.x);
	short y = cClient->getMap()->WrapAroundY((short)HookPos.y);

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
// Check if the ninja rope has crossed the map border and update crossed* variables accordingly
// HINT: only for remote worms
void CNinjaRope::checkWrapAround(int x, int y, CWorm *owner)
{
	if (!cClient->getGameLobby()->features[FT_InfiniteMap])
		return;

	int old_x = OldReceivedPos.x;
	int old_y = OldReceivedPos.y;
	int mapw = cClient->getMap()->GetWidth();
	int maph = cClient->getMap()->GetHeight();
	static const int tolerance = 8;

	if (abs(old_x - x) >= mapw - tolerance)
		CrossedHorizontal += SIGN(old_x - x);

	if (abs(old_y - y) >= maph - tolerance)
		CrossedVertical += SIGN(old_y - y);

	OldReceivedPos.x = x;
	OldReceivedPos.y = y;
}

///////////////////
// Read rope details from a bytestream
void CNinjaRope::read(CBytestream *bs, CWorm *worms, int owner)
{
	if (owner < 0 || owner >= MAX_WORMS)  {
		warnings << "CNinjaRope::read: invalid owner " << owner << endl;
		return;
	}

	if (!worms[owner].isUsed() || worms[owner].getLives() == WRM_OUT)  {
		warnings << "CNinjaRope::read: owner worm " << owner << " is not playing" << endl;
		return;
	}

	// If we are shooting the rope, clear the cross* variables because ::Shoot is not called for remote worms
	bool justShot = false;
	if (!Released)  {
		justShot = true;
		CrossedHorizontal = worms[owner].crossedHorizontal();
		CrossedVertical = worms[owner].crossedVertical();
	}

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
	if (justShot)
		OldReceivedPos = VectorD2<int>(x, y);

	// Check if it has wrapped around
	checkWrapAround(x, y, &worms[owner]);

	HookPos.x=( (float)x );
	HookPos.y=( (float)y );

	// Angle
	if(type == ROP_SHOOTING) {
		int a = bs->readByte();
		int angle = 360*a/256;
		GetVecsFromAngle(angle, &HookDir, NULL);
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
