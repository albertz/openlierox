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


#ifndef __CNINJAROPE_H__
#define __CNINJAROPE_H__

#include "game/CGameObject.h"
#include "gusanos/events.h"
#include "CVec.h"
#include "util/angle.h"
#include "gusanos/timer_event.h"
#include "gusanos/particle.h"
#include <vector>

// Rope types
#define		ROP_NONE		0x00
#define		ROP_SHOOTING	0x01
#define		ROP_HOOKED		0x02
#define		ROP_FALLING		0x04
#define		ROP_PLYHOOKED	0x08


#ifndef DEDICATED_ONLY
class SpriteSet;
class BaseAnimator;
#endif

#include "CVec.h"

class CWorm;
struct SDL_Surface;
class CBytestream;


class CNinjaRope : public CGameObject {
public:
	CNinjaRope();
	~CNinjaRope();

	virtual BaseObject* parentObject() const;
	CWorm* owner() const;

private:	
	ATTR(CNinjaRope, bool,		Released, 1, {serverside = false;})
	ATTR(CNinjaRope, bool,		HookShooting, 2, {serverside = false;})
	ATTR(CNinjaRope, bool,		HookAttached, 3, {serverside = false;})
	ATTR(CNinjaRope, int,		PlayerAttached, 4, { serverside = false; defaultValue = -1; })

public:
	// Methods
	void		Clear();

	void		Draw(SDL_Surface * bmpDest, CViewport *view, CVec ppos) const;
	void		Shoot(CVec dir);

	CVec		GetForce() const;
    
	bool		isReleased() const	{ return Released; }
	void		UnAttachPlayer();
	void		AttachToPlayer(CWorm *worm);

	void		write(CBytestream *bs) const;
	void		read(CBytestream *bs, int owner);

    CVec getHookPos() const       { return getPos(); }
    bool   isAttached() const       { return HookAttached; }
    bool   isShooting()  const      { return HookShooting; }
	void	setShooting(bool s)			{ HookShooting = s; }
	void	setAttached(bool a)			{ HookAttached = a; }

	vVelocity_Type&		hookVelocity()			{ return velocity(); }
	vPos_Type&	hookPos()				{ return pos(); }
	
	bool		isPlayerAttached() const;
	CWorm*		getAttachedPlayer()	const;

	virtual bool isInside(int x, int y) const;
	virtual Color renderColorAt(/* relative coordinates */ int x, int y) const;
	
	
	// ----------------------
	// --------- Gusanos
	// ----------------------
	
public:
	static LuaReference metaTable;

	void gusInit();
	void remove() { Clear(); /* but dont delete */ }

#ifndef DEDICATED_ONLY
	void draw(CViewport *viewport);
#endif
	void think();
	
	Angle getPointingAngle();
	void addAngleSpeed(AngleDiff);
	
	void addLength(float length_)
	{
		m_length += length_;
		if ( m_length < 0.f )
			m_length = 0.f;
	}
	
	int getColour() const;
	float& getLengthReference()
	{
		return m_length;
	}
	
	void deleteThis();
	
private:
	
	std::vector< TimerEvent::State > timer;
	Angle m_angle;
	AngleDiff m_angleSpeed;
	float m_length;
#ifndef DEDICATED_ONLY
	SpriteSet* m_sprite;
	BaseAnimator* m_animator;
#endif
	
	bool justCreated;
	
};



#endif  //  __CNINJAROPE_H__
