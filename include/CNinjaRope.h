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
	// Constructor
	CNinjaRope(CWorm* _owner)
	: owner(_owner), m_sprite(NULL), m_animator(NULL) {
		assert(owner != NULL);
		Clear();

		LastWrite = AbsTime();
		LastPosUpdate = AbsTime();
	}
	~CNinjaRope();
	CNinjaRope& operator=(const CNinjaRope&);

	virtual BaseObject* parentObject() const { return (BaseObject*)owner; }

private:	
	// Attributes
	
	CWorm* const owner;
	
	bool		Released;
	bool		HookShooting;
	bool		HookAttached;
	bool		PlayerAttached;
	CWorm		*Worm;

	float		MinLength;

	CVec		HookVelocity;

	CVec		HookDir;
	CVec		OldHookPos;

	// Used for writeNeeded check
	bool		LastReleased;
	bool		LastHookShooting;
	bool		LastHookAttached;
	bool		LastPlayerAttached;
	CWorm		*LastWorm;
	AbsTime		LastWrite;
	AbsTime		LastPosUpdate; // Used for velocity counting (client don't send velocity)
	

public:
	// Methods
	void		Clear();

	void		Draw(SDL_Surface * bmpDest, CViewport *view, CVec ppos);
	void		Shoot(CWorm* owner, CVec pos, CVec dir);

	CVec		GetForce();
    
	bool		isReleased()		{ return Released; }
	void		UnAttachPlayer();
	void		AttachToPlayer(CWorm *worm);

	void		updateCheckVariables();
	bool		writeNeeded();
	void		write(CBytestream *bs);
	void		read(CBytestream *bs, int owner);

    CVec getHookPos() const       { return getPos(); }
    bool   isAttached() const       { return HookAttached; }
    bool   isShooting()  const      { return HookShooting; }
	void	setShooting(bool s)			{ HookShooting = s; }
	void	setAttached(bool a)			{ HookAttached = a; }

	void		updateOldHookPos()		{ OldHookPos = getPos(); }

	CVec&		hookVelocity()			{ return HookVelocity; }
	vPos_Type&	hookPos()				{ return pos(); }
	
	bool		isPlayerAttached()		{ return PlayerAttached; }
	CWorm*		getAttachedPlayer()		{ return Worm; }

	void		changeRestLength(float);

	virtual bool isInside(int x, int y);	
	virtual Color renderColorAt(/* relative coordinates */ int x, int y);
	
	
	// ----------------------
	// --------- Gusanos
	// ----------------------
	
public:
	static LuaReference metaTable;

	void gusInit();
	void shoot(Vec _pos, Vec _spd);
	void remove();
	
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
	
	int getColour();
	float& getLengthReference()
	{
		return m_length;
	}
	
	bool active;
	bool attached;

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
