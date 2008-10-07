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

class CGameScript;
class CWorm;
struct SDL_Surface;
class CBytestream;


class CNinjaRope {
public:
	// Constructor
	CNinjaRope() {
		Clear();

		LastWrite = -9999;
		LastPosUpdate = -9999;
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
	float		LastWrite;
	float		LastPosUpdate; // Used for velocity counting (client don't send velocity)
	

public:
	// Methods

	void		Clear();

	void		Draw(SDL_Surface * bmpDest, CViewport *view, CVec ppos);
	void		Shoot(CVec pos, CVec dir);

	CVec		GetForce(CVec playerpos);
	CVec		CalculateForce(CVec playerpos, CVec hookpos);
    

	void		Setup(CGameScript *gs);

	int			isReleased(void)		{ return Released; }
	void		Release(void);

	void		updateCheckVariables();
	bool		writeNeeded();
	void		write(CBytestream *bs);
	void		read(CBytestream *bs, CWorm *worms, int owner);

    CVec getHookPos(void) const       { return HookPos; }
    bool   isAttached(void) const       { return HookAttached; }
    bool   isShooting(void)  const      { return HookShooting; }
	void	setShooting(bool s)			{ HookShooting = s; }
	void	setAttached(bool a)			{ HookAttached = a; }
    float getRestLength(void) const    { return RestLength; }
	float getMaxLength(void)	const	{ return RopeLength; }

	void		updateOldHookPos()		{ OldHookPos = HookPos; }

	const CVec&	getHookVel() const		{ return HookVelocity; }
	CVec&		hookVelocity()			{ return HookVelocity; }
	CVec&		hookPos()				{ return HookPos; }
	
	bool		isPlayerAttached()		{ return PlayerAttached; }
	void		setPlayerAttached(bool a)		{ PlayerAttached = a; }
	// TODO: remove PlayerAttached and just use Worm (and if Worm==NULL => no player attached)
	void		setAttachedPlayer(CWorm* w)	{ Worm = w; }
	CWorm*		getAttachedPlayer()		{ return Worm; }
	
};



#endif  //  __CNINJAROPE_H__
