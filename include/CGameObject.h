/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Class that represents an object on game screen - worm, projectile etc, 
// so we can bind viewport to guided missile in future
// Contains only coordinates and velocity


#ifndef __CGAMEOBJECT_H__
#define __CGAMEOBJECT_H__

#include "CVec.h"

// TODO: add angle here?
class CGameObject {
public:
	CGameObject() {};
	~CGameObject() {};

protected:
	CVec		vPos;
	CVec		vVelocity;

public:

	CVec		getPos() const				{ return vPos; }
	void		setPos(CVec v)				{ vPos = v; }
	CVec&		pos()						{ return vPos; }

	CVec		getVelocity() const			{ return vVelocity; }
	void		setVelocity(CVec v)			{ vVelocity = v; }
	CVec&		velocity()					{ return vVelocity; }
};

#endif  //  __CGAMEOBJECT_H__
