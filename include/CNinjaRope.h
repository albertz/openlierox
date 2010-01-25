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

#include "CVec.h"

class CWorm;
struct SDL_Surface;
class CBytestream;


class CNinjaRope {
public:
	// Constructor
	CNinjaRope() {
		Clear();

		LastWrite = AbsTime();
		LastPosUpdate = AbsTime();
		Strength = 0.4f;
	}


private:
	// Attributes
	
	bool		Released;
	bool		HookShooting;
	bool		HookAttached;
	bool		PlayerAttached;
	CWorm		*Worm;
	float		RopeLength;
	float		RestLength;

	float		MinLength;

	float		Strength;
	
	CVec		HookVelocity;

	CVec		HookPos;
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
	void		Setup();
	void		Clear();

	void		Draw(SDL_Surface * bmpDest, CViewport *view, CVec ppos);
	void		Shoot(CWorm* owner, CVec pos, CVec dir);

	CVec		GetForce(CVec playerpos);
	CVec		CalculateForce(CVec playerpos);
    
	bool		isReleased()		{ return Released; }
	void		Release();
	void		UnAttachPlayer();
	void		AttachToPlayer(CWorm *worm, CWorm *owner);

	void		updateCheckVariables();
	bool		writeNeeded();
	void		write(CBytestream *bs);
	void		read(CBytestream *bs, CWorm *worms, int owner);

    CVec getHookPos() const       { return HookPos; }
    bool   isAttached() const       { return HookAttached; }
    bool   isShooting()  const      { return HookShooting; }
	void	setShooting(bool s)			{ HookShooting = s; }
	void	setAttached(bool a)			{ HookAttached = a; }
    float getRestLength() const    { return RestLength; }
	float getMaxLength()	const	{ return RopeLength; }

	void		updateOldHookPos()		{ OldHookPos = HookPos; }

	const CVec&	getHookVel() const		{ return HookVelocity; }
	CVec&		hookVelocity()			{ return HookVelocity; }
	CVec&		hookPos()				{ return HookPos; }
	
	bool		isPlayerAttached()		{ return PlayerAttached; }
	CWorm*		getAttachedPlayer()		{ return Worm; }

	void		changeRestLength(float);

};



#endif  //  __CNINJAROPE_H__
