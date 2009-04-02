/*
 *  ProjAction.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 02.04.09.
 *  code under LGPL
 *
 */

#include "ProjAction.h"
#include "CGameScript.h"
#include "CWorm.h"
#include "CProjectile.h"

int Proj_SpawnParent::ownerWorm() const {
	switch(type) {
		case PSPT_NOTHING: return -1;
		case PSPT_WORM: return worm->getID();
		case PSPT_PROJ: return proj->GetOwner(); 
	}
	return -1;
}

void Proj_SpawnInfo::apply(CGameScript* script, Proj_SpawnParent parent, AbsTime spawnTime) {
	
}
