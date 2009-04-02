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
#include "CClient.h"

int Proj_SpawnParent::ownerWorm() const {
	switch(type) {
		case PSPT_NOTHING: return -1;
		case PSPT_WORM: return worm->getID();
		case PSPT_PROJ: return proj->GetOwner(); 
	}
	return -1;
}

CVec Proj_SpawnParent::position() const {
	switch(type) {
		case PSPT_NOTHING: return CVec(0,0);
		case PSPT_WORM: return worm->pos();
		case PSPT_PROJ: return proj->GetPosition(); 
	}
	return CVec(0,0);
}

CVec Proj_SpawnParent::velocity() const {
	switch(type) {
		case PSPT_NOTHING: return CVec(0,0);
		case PSPT_WORM: return worm->velocity();
		case PSPT_PROJ: return proj->GetVelocity(); 
	}
	return CVec(0,0);
}


void Proj_SpawnInfo::apply(CGameScript* script, Proj_SpawnParent parent, AbsTime spawnTime) {
	CVec sprd;
	if(UseParentVelocity) {
		sprd = parent.velocity() * ParentVelFactor;
	}
	
	for(int i=0; i < Amount; i++) {
		if(!UseParentVelocity && parent.type == Proj_SpawnParent::PSPT_PROJ)
			GetVecsFromAngle((int)((float)Spread * parent.proj->getRandomFloat()),&sprd,NULL);
		
		CVec v = sprd * (float)Speed;
		if(parent.type == Proj_SpawnParent::PSPT_PROJ)
			v += CVec(1,1) * (float)SpeedVar * parent.proj->getRandomFloat();
		
		AbsTime ignoreWormCollBeforeTime = spawnTime;
		if(parent.type == Proj_SpawnParent::PSPT_PROJ)
			ignoreWormCollBeforeTime = parent.proj->getIgnoreWormCollBeforeTime();
		int random = 0;
		if(parent.type == Proj_SpawnParent::PSPT_PROJ)
			random = parent.proj->getRandomIndex() + 1;
		cClient->SpawnProjectile(parent.position(), v, 0, parent.ownerWorm(), Proj, random, spawnTime, ignoreWormCollBeforeTime);
	}
	
}
