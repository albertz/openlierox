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

