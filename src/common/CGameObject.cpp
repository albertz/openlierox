/*
 *  CGameObject.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 27.07.09.
 *  code under LGPL
 *
 */

#include "game/CGameObject.h"
#include "game/Game.h"
#include "CGameScript.h"
#include "game/CWorm.h"
#include "PhysicsLX56.h"
#include "CVec.h"
#include "util/angle.h"
#include "game/WormInputHandler.h"
#include "gusanos/glua.h"
#include "gusanos/luaapi/context.h"
#include "gusanos/lua/bindings-objects.h"

LuaReference CGameObject::metaTable;

CGameObject::CGameObject() { gusInit(); }
CGameObject::CGameObject(CWormInputHandler* owner, Vec pos_, Vec spd_) { gusInit(owner, pos_, spd_); }
CGameObject::~CGameObject() { gusShutdown(); }


bool CGameObject::canUpdateAttribs(const BaseObject* obj, const AttrDesc* attrDesc) {
	const CWorm* w = dynamic_cast<const CWorm*>(obj);
	if(!w) return true; // not yet implemented/handled. pass on to other checks in Attr code
	if(!w->getAlive()) return game.isServer();
	return w->getLocal();
}

bool CGameObject::injure(float damage) {
	health -= damage;

	if(health < 0.0f) {
		health = 0.0f;
		return true;
	}

	return false;
}

void CGameObject::gusInit( CWormInputHandler* owner, Vec pos_, Vec spd_ )
{
	nextS_=(0); nextD_=(0); prevD_=(0); cellIndex_=(-1);
	deleteMe=(false);

	m_owner=(owner);
	vPos = CVec(pos_);
	vVelocity = CVec(spd_);
}

void CGameObject::gusShutdown()
{
	if(luaData)
		lua.destroyReference(luaData);
}

Vec CGameObject::getRenderPos()
{
	return pos();
}

Angle CGameObject::getPointingAngle()
{
	return Angle(0);
}

int CGameObject::getDir()
{
	return 1;
}

CWormInputHandler* CGameObject::getOwner()
{
	return m_owner;
}

void CGameObject::remove()
{
	deleteMe = true;
}

bool CGameObject::isCollidingWith( const Vec& point, float radius )
{
	return (Vec(pos()) - point).lengthSqr() < radius*radius;
}

void CGameObject::removeRefsToPlayer(CWormInputHandler* player)
{
	if ( m_owner == player )
		m_owner = NULL;
}

bool CGameObject::isInside(int x, int y) const {
	IVec s = size();
	return abs((int)pos().get().x - x) + 1 <= s.x && abs((int)pos().get().y - y) + 1 <= s.y;
}

/*
 LuaReference CGameObject::getLuaReference()
 {
 if(luaReference)
 return luaReference;
 else
 {
 lua.pushFullReference(*this, metaTable);
 luaReference = lua.createReference();
 return luaReference;
 }
 }*/

void CGameObject::makeReference()
{
	lua.pushFullReference(*this, metaTable);
}

/*
 void CGameObject::pushLuaReference()
 {
 lua.push(getLuaReference());
 }
 
 void CGameObject::deleteThis()
 {
 finalize();
 
 if(luaReference)
 {
 lua.destroyReference(luaReference);
 luaReference.reset();
 }
 else
 delete this;
 }
 */



// -------
// LX56PhysicsDT.seconds() default is 1/84 ~= 0.0119.

CGameObject::ScopedGusCompatibleSpeed::ScopedGusCompatibleSpeed(CGameObject& o) : obj(o) {
	if(dynamic_cast<CWorm*> (&obj) == NULL) return;
	// we do this if we use the LX56 Physics simulation on worms
	// Gusanos interprets the velocity in a different way, so we convert it while we are doing Gus stuff
	obj.velocity() *= LX56PhysicsDT.seconds();
}

CGameObject::ScopedGusCompatibleSpeed::~ScopedGusCompatibleSpeed() {
	if(dynamic_cast<CWorm*> (&obj) == NULL) return;
	obj.velocity() *= 1.0f / LX56PhysicsDT.seconds();
}

float convertSpeed_LXToGus(float v) {
	return v * LX56PhysicsDT.seconds();
}

float convertSpeed_GusToLX(float v) {
	return v / LX56PhysicsDT.seconds();
}

float convertAccel_LXToGus(float v) {
	return v * LX56PhysicsDT.seconds() * LX56PhysicsDT.seconds();
}

float convertAccel_GusToLX(float v) {
	return (v / LX56PhysicsDT.seconds()) / LX56PhysicsDT.seconds();
}


REGISTER_CLASS(CGameObject, ClassId(-1))
