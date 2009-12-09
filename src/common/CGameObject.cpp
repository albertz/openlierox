/*
 *  CGameObject.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 27.07.09.
 *  code under LGPL
 *
 */

#include "CGameObject.h"

bool CGameObject::injure(float damage) {
	health -= damage;
	
	if(health < 0.0f) {
		health = 0.0f;
		return true;
	}
	
	return false;
}



#include "util/vec.h"
#include "util/angle.h"
#include "game/WormInputHandler.h"
#include "gusanos/glua.h"
#include "gusanos/lua51/luaapi/context.h"
#include "gusanos/lua/bindings-objects.h"

LuaReference CGameObject::metaTable;

void CGameObject::gusInit( CWormInputHandler* owner, Vec pos_, Vec spd_ )
{
	nextS_=(0); nextD_=(0); prevD_=(0); cellIndex_=(-1);
	deleteMe=(false); m_owner=(owner);
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
