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


class	CWorm;

class CNinjaRope {
public:
	// Constructor
	CNinjaRope() {
		Clear();

		Strength = 0.4f;
	}


private:
	// Attributes
	
	int			Released;
	int			HookShooting;
	int			HookAttached;
	int			PlayerAttached;
	CWorm		*Worm;
	float		RopeLength;
	float		RestLength;
	float		Strength;
	
	CVec		HookVelocity;

	CVec		HookPos;
	CVec		HookDir;
	

public:
	// Methods

	void		Clear();

	void		Draw(SDL_Surface *bmpDest, CViewport *view, CVec ppos);
	void		Shoot(CVec pos, CVec dir);

	void		Simulate(float dt, CMap *map, CVec playerpos, CWorm *worms, int owner);

	CVec		GetForce(CVec playerpos);
	CVec		CalculateForce(CVec playerpos, CVec hookpos);
    

	void		Setup(CGameScript *gs);

	int			isReleased(void)		{ return Released; }
	void		Release(void);

	void		write(CBytestream *bs);
	void		read(CBytestream *bs, CWorm *worms, int owner);

    inline CVec getHookPos(void) const       { return HookPos; }
    inline int   isAttached(void) const       { return HookAttached; }
    inline int   isShooting(void)  const      { return HookShooting; }
    inline float getRestLength(void) const    { return RestLength; }
	inline float getMaxLength(void)	const	{ return RopeLength; }







};



#endif  //  __CNINJAROPE_H__
