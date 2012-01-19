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

#include "Utils.h"
#include "util/angle.h"
#include "util/vec.h"
#include "gusanos/luaapi/types.h"
#include "gusanos/glua.h"
#include "Color.h"
#include "util/WeakRef.h"
#include "CVec.h"

struct ALLEGRO_BITMAP;
class CViewport;
class CWormInputHandler;

class CGameObject : public LuaObject {
public:
	CGameObject();
	CGameObject(CWormInputHandler* owner, Vec pos_ = Vec(), Vec spd_ = Vec() );
	~CGameObject();

protected:
	// Gusanos comment:
	// IMPORTANT: The pos and spd vectors should be used as read only. ( Because of netplay needs )
	// To change their values use the setters provided.
	CVec		vPos;
	CVec		vVelocity;
	float		health;

public:

	CVec		getPos() const				{ return vPos; }
	void		setPos(const CVec& v)		{ vPos = v; }
	CVec&		pos()						{ return vPos; }

	CVec		getVelocity() const			{ return vVelocity; }
	void		setVelocity(const CVec& v)	{ vVelocity = v; }
	CVec&		velocity()					{ return vVelocity; }
	
	void		setHealth(float _h)			{ health = _h; }
	float		getHealth() const			{ return health; }
	bool		injure(float damage); // returns true if object has died
	

	virtual bool isInside(int x, int y);
	virtual IVec size() { return IVec(); }

	virtual Color renderColorAt(/* relative coordinates */ int x, int y) { return Color(0,0,0,SDL_ALPHA_TRANSPARENT); }
	
	
	// -------------------------------------------------------------
	// ------------------------- Gusanos ---------------------------
	
public:
	static LuaReference metaTable;
	//static int const luaID = 1;
	
	void gusInit( CWormInputHandler* owner = 0, Vec pos_ = Vec(), Vec spd_ = Vec() );
	void gusShutdown();
	
#ifndef DEDICATED_ONLY
	// Draw the object in the where bitmap with an offset ( used for camera )
	//virtual void draw(ALLEGRO_BITMAP* where, int xOff, int yOff) {}
	virtual void draw(CViewport* viewport)
	{}
#endif
	// All the object logic here
	virtual void think()
	{}
	
	// Gets the position on the level where the object wants to be rendered
	virtual Vec getRenderPos();
	
	// Gets a pointer to the CWormHumanInputHandler that owns this object ( NULL if it has no owner )
	virtual CWormInputHandler* getOwner();
	
	// Gets the angle the object is pointing
	virtual Angle getPointingAngle();
	
	// Gets the object's dir ( left = -1 right = 1 )
	virtual int getDir();
	
	// Returns true if the object is colliding with a circle of the given radius in the given point
	virtual bool isCollidingWith( const Vec& point, float radius );
	
	// Removes all the connections this object may have to the given player
	virtual void removeRefsToPlayer( CWormInputHandler* player );
	
	// Adds the speed value to the current angle speed of the object
	virtual void addAngleSpeed( AngleDiff speed )
	{}
	
	// Tells the object to remove itself ( The object may not agree and nothing will happen )
	virtual void remove();
	
	// Deletes the object
	//void deleteThis();
	
	virtual void makeReference();
	
#ifndef DEDICATED_ONLY
	// Sets a destination alpha value and the time in logic frames it will take to reach that value
	virtual void setAlphaFade( int frames, int dest )
	{}
#endif
	
	// Runs the custom event number "index" of the object
	virtual void customEvent ( size_t index )
	{}
	
	// Effects amount damage to the object and sets the last damager to the passed CWormInputHandler pointer
	virtual void damage(float amount, CWormInputHandler* damager )
	{}
	
	// Adds the speed vector to the current speed
	virtual void addSpeed( Vec spd_ )
	{
		vVelocity += CVec(spd_.x, spd_.y);
	}
	
	// Moves the object somewhere else
	virtual void setPos( Vec pos_ )
	{
		vPos = CVec(pos_.x, pos_.y);
	}
	
	struct ScopedGusCompatibleSpeed : DontCopyTag {
		CGameObject& obj;
		ScopedGusCompatibleSpeed(CGameObject& o);
		~ScopedGusCompatibleSpeed();
	};
	
	LuaReference luaData;
	
	CGameObject* nextS_;
	CGameObject* nextD_;
	CGameObject* prevD_;
	int cellIndex_;
	
	// If this is true the object will be removed from the objects list in the next frame
	bool deleteMe;
	
	WeakRef<CGameObject> thisWeakRef;
	uint32_t uniqueObjId;
	
protected:
	//LuaReference luaReference; //Defined in LuaObject
	CWormInputHandler* m_owner;
	
};

#endif  //  __CGAMEOBJECT_H__
