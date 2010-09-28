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
#include "CGameObject.h"

class CWorm;
struct SDL_Surface;
class CBytestream;


class CNinjaRope : public CGameObject {
public:
	// Constructor
	CNinjaRope() {
		Clear();

		LastWrite = AbsTime();
		LastPosUpdate = AbsTime();
	}


private:
	// Attributes
	
	CWorm*		owner;
	
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

    CVec getHookPos() const       { return getPos(); }
    bool   isAttached() const       { return HookAttached; }
    bool   isShooting()  const      { return HookShooting; }
	void	setShooting(bool s)			{ HookShooting = s; }
	void	setAttached(bool a)			{ HookAttached = a; }

	void		updateOldHookPos()		{ OldHookPos = getPos(); }

	CVec&		hookVelocity()			{ return HookVelocity; }
	CVec&		hookPos()				{ return pos(); }
	
	bool		isPlayerAttached()		{ return PlayerAttached; }
	CWorm*		getAttachedPlayer()		{ return Worm; }

	void		changeRestLength(float);

	virtual bool isInside(int x, int y);	
	virtual Color renderColorAt(/* relative coordinates */ int x, int y);
	
};



#endif  //  __CNINJAROPE_H__
