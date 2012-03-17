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
#include "game/CWorm.h"
#include "MathLib.h"
#include "Entity.h"
#include "CClient.h"
#include "game/Game.h"
#include "Geometry.h"
#include "gusanos/luaapi/classes.h"


LuaReference CNinjaRope::metaTable;

CNinjaRope::CNinjaRope()
: m_sprite(NULL), m_animator(NULL) {
	thisRef.classId = LuaID<CNinjaRope>::value;
	Clear();
}

CNinjaRope::~CNinjaRope() {
	// We must delete the object now out of the list because this destructor
	// is not called from Gusanos but from CClient.
	// NOTE: Not really the best way but I don't know a better way
	// TODO: move this out here
	for ( Grid::iterator iter = game.objects.beginAll(); iter;)
	{
		if( &*iter == this )
			iter.erase();
		else
			++iter;
	}
}

BaseObject* CNinjaRope::parentObject() const {
	for_each_iterator(CWorm*, w, game.worms()) {
		if(&w->get()->cNinjaRope.get() == this)
			return w->get();
	}
	return NULL;
}

CWorm* CNinjaRope::owner() const {
	return dynamic_cast<CWorm*>(parentObject());
}


bool CNinjaRope::isPlayerAttached() const {
	if(PlayerAttached < 0) return false;
	return game.wormById(PlayerAttached, false) != NULL;
}

CWorm* CNinjaRope::getAttachedPlayer() const {
	return game.wormById(PlayerAttached, false);
}

///////////////////
// Clear the ninja rope vars
void CNinjaRope::Clear()
{	
	Released = false;
	HookShooting = false;
	HookAttached = false;
	PlayerAttached = -1;
}



///////////////////
// Shoot the rope
void CNinjaRope::Shoot(CVec dir)
{
	Clear();

	if(owner() == NULL) {
		errors << "CNinjaRope::Shoot: owner unknown" << endl;
		return;
	}

	if(!owner()->canUseNinja()) return;
	
	Released = true;
	HookShooting = true;
	HookAttached = false;
	PlayerAttached = -1;

	pos() = owner()->pos();
	velocity() = dir * (float)cClient->getGameLobby()[FT_RopeSpeed];
	
	if(cClient->getGameLobby()[FT_RopeAddParentSpeed])
		velocity() += owner()->getVelocity();
}



///////////////////
// Draw the thing
void CNinjaRope::Draw(SDL_Surface * bmpDest, CViewport *view, CVec ppos) const
{
	if(!Released)
		return;

	int l = view->GetLeft();
	int t = view->GetTop();
	int wx = view->GetWorldX();
	int wy = view->GetWorldY();

	int hx = (int)pos().get().x;
	int hy = (int)pos().get().y;

	// HINT: the hooked worm position could change since the Simulate procedure was called,
	// because the worms are being processed in a "random" order -> we simulate and then the hook worm
	// is simulated -> we have a wrong position, that's why we are correcting it here:
	if(HookAttached && PlayerAttached >= 0) {
		CWorm* w = game.wormById(PlayerAttached, false);
		if(w) {
			hx = (int)w->getPos().x;
			hy = (int)w->getPos().y;
		}
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

bool CNinjaRope::isInside(int x, int y) const {
	if(!Released) return false;
	if(!owner()) return false;
	if(!owner()->getAlive()) return false;
	
	Line l(owner()->pos(), pos());
	return l.distFromPoint2(VectorD2<int>(x, y)) < 2.0f;
}

Color CNinjaRope::renderColorAt(/* relative coordinates */ int x, int y) const {
	x += (int)pos().get().x;
	y += (int)pos().get().y;
	if(isInside(x, y)) return Color(159,79,0);
	return Color(0,0,0,SDL_ALPHA_TRANSPARENT);
}

////////////////////
// Unattaches rope from a worm
void CNinjaRope::UnAttachPlayer()
{
	if (PlayerAttached < 0)
		return;

	velocity() = CVec();
	HookShooting = false;
	HookAttached = false;
	PlayerAttached = -1;
}

void CNinjaRope::AttachToPlayer(CWorm *worm)
{
	HookShooting = false;
	HookAttached = true;
	PlayerAttached = worm->getID();
}


///////////////////
// Return the pulling force
CVec CNinjaRope::GetForce() const
{
	if(!HookAttached)
		return CVec(0,0);

	CVec dir = owner()->pos() - pos();
	dir = dir.Normalize();
	
	const float RestLength = (float)(int)cClient->getGameLobby()[FT_RopeRestLength];
	if((owner()->pos() - pos()).GetLength2() < RestLength*RestLength)
		return CVec(0,0);
	
	dir *= (float)cClient->getGameLobby()[FT_RopeStrength] * -100;
	
	return dir;
}



///////////////////
// Write out the rope details to a bytestream
void CNinjaRope::write(CBytestream *bs) const
{
	int type = ROP_NONE;

	if(HookShooting)
		type = ROP_SHOOTING;
	else if(HookAttached) {
		if(PlayerAttached >= 0)
			type = ROP_PLYHOOKED;
		else
			type = ROP_HOOKED;
	}
	else
		type = ROP_FALLING;

	bs->writeByte( type );

	// Position
	short x = (short)pos().get().x;
	short y = (short)pos().get().y;

	// Write out position of the hook
	bs->write2Int12( x, y );


	// Write out the direction is shooting out
	if(type == ROP_SHOOTING) {
		// HookDir was removed as it is not really used
		bs->writeByte( 0 );
	}

	// Write out the worm id the hook is stuck to
	if(type == ROP_PLYHOOKED) {
		bs->writeByte( PlayerAttached );
	}
}



///////////////////
// Read rope details from a bytestream
void CNinjaRope::read(CBytestream *bs, int owner)
{
	if(this->owner() == NULL)
		errors << "CNinjaRope::read: owner unknown" << endl;
	else if(this->owner() != game.wormById(owner, false))
		errors << "CNinjaRope::read: owner (" << this->owner()->getID() << ") differs from param " << owner << endl;
	
	int type = bs->readByte();
	Released = true;

	switch(type) {
		case ROP_SHOOTING:
			HookShooting = true;
			HookAttached = false;
			PlayerAttached = -1;
			break;
		case ROP_HOOKED:
			HookShooting = false;
			HookAttached = true;
			PlayerAttached = -1;
			break;
		case ROP_FALLING:
			HookShooting = false;
			HookAttached = false;
			PlayerAttached = -1;
			break;
		case ROP_PLYHOOKED:
			HookShooting = false;
			HookAttached = true;
			break;
	}

	// Position
	short x, y;
	bs->read2Int12( x, y );
	pos() = CVec( float(x), float(y) );

	// Angle
	if(type == ROP_SHOOTING) {
		// we don't use/need the angle
		/* angle = */ bs->readByte();
	}

	// Worm id
	if(type == ROP_PLYHOOKED) {
		int id = bs->readByte();
		if(id >= 0 && id < MAX_WORMS) {
			PlayerAttached = id;
			CWorm* w = game.wormById(id, false);
			if(w) {
				// Set the hook pos on the worm
				pos() = w->getPos();
			}
		}
	}
}

REGISTER_CLASS(CNinjaRope, LuaID<CGameObject>::value)
