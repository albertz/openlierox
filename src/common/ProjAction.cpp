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
#include "ProjectileDesc.h"
#include "Physics.h"
#include "ConfigHandler.h"
#include "EndianSwap.h"

int Proj_SpawnParent::ownerWorm() const {
	switch(type) {
		case PSPT_NOTHING: return -1;
		case PSPT_SHOT: return shot->nWormID;
		case PSPT_PROJ: return proj->GetOwner(); 
	}
	return -1;
}

int Proj_SpawnParent::fixedRandomIndex() const {
	switch(type) {
		case PSPT_NOTHING: return -1;
		case PSPT_SHOT: return shot->nRandom;
		case PSPT_PROJ: return proj->getRandomIndex() + 1; 
	}
	return -1;	
}

float Proj_SpawnParent::fixedRandomFloat() const {
	switch(type) {
		case PSPT_NOTHING: return -1;
		case PSPT_SHOT: return GetFixedRandomNum(shot->nRandom);
		case PSPT_PROJ: return proj->getRandomFloat(); 
	}
	return -1;	
}

CVec Proj_SpawnParent::position() const {
	switch(type) {
		case PSPT_NOTHING: return CVec(0,0);
		case PSPT_SHOT: {
			CVec dir;
			GetVecsFromAngle(shot->nAngle, &dir, NULL);
			CVec pos = shot->cPos + dir*8;
			return pos;
		}
		case PSPT_PROJ: return proj->GetPosition(); 
	}
	return CVec(0,0);
}

CVec Proj_SpawnParent::velocity() const {
	switch(type) {
		case PSPT_NOTHING: return CVec(0,0);
		case PSPT_SHOT: return shot->cWormVel;
		case PSPT_PROJ: return proj->GetVelocity(); 
	}
	return CVec(0,0);
}

float Proj_SpawnParent::angle() const {
	switch(type) {
		case PSPT_NOTHING: return 0;
		case PSPT_SHOT: return (float)shot->nAngle;
		case PSPT_PROJ: {
			CVec v = velocity();
			NormalizeVector(&v);
			float heading = (float)( -atan2(v.x,v.y) * (180.0f/PI) );
			heading+=90;
			FMOD(heading, 360.0f);
			return heading;
		}
	}
	return 0;
}



void Proj_SpawnInfo::apply(Proj_SpawnParent parent, AbsTime spawnTime) const {
	// Calculate the angle of the direction the projectile is heading
	float heading = 0;
	if(Useangle)
		heading = parent.angle();
	
	for(int i=0; i < Amount; i++) {		
		CVec sprd;
		if(UseParentVelocityForSpread)
			sprd = parent.velocity() * ParentVelSpreadFactor;
		else {
			int a = (int)( (float)Angle + heading + parent.fixedRandomFloat() * (float)Spread );
			GetVecsFromAngle(a, &sprd, NULL);
		}
		
		int rot = 0;
		if(UseRandomRot) {
			// Calculate a random starting angle for the projectile rotation (if used)
			if(Proj->Rotating) {
				// Prevent div by zero
				if(Proj->RotIncrement == 0)
					Proj->RotIncrement = 1;
				rot = GetRandomInt( 360 / Proj->RotIncrement ) * Proj->RotIncrement;
			}
		}
		
		if(parent.type == Proj_SpawnParent::PSPT_SHOT) {
			parent.shot->nRandom++;
			parent.shot->nRandom %= 255;		
		}
		
		CVec v = sprd * (float)Speed;
		CVec speedVarVec = sprd;
		if(UseSpecial11VecForSpeedVar) speedVarVec = CVec(1,1);
		v += speedVarVec * (float)SpeedVar * parent.fixedRandomFloat();
		if(AddParentVel)
			v += parent.velocity();
		
		if(parent.type == Proj_SpawnParent::PSPT_SHOT) {
			parent.shot->nRandom *= 5;
			parent.shot->nRandom %= 255;		
		}		
		
		AbsTime ignoreWormCollBeforeTime = spawnTime;
		if(parent.type == Proj_SpawnParent::PSPT_PROJ)
			ignoreWormCollBeforeTime = parent.proj->getIgnoreWormCollBeforeTime();
		else
			// we set the ignoreWormCollBeforeTime to the current time to let the physics engine
			// first emulate the projectiles to the curtime and ignore earlier colls as the worm-pos
			// is probably outdated at this time
			ignoreWormCollBeforeTime = GetPhysicsTime() + 0.1f; // HINT: we add 100ms (it was dt before) because the projectile is spawned -> worms are simulated (pos change) -> projectiles are simulated
		
		int random = parent.fixedRandomIndex();

		cClient->SpawnProjectile(parent.position(), v, rot, parent.ownerWorm(), Proj, random, spawnTime, ignoreWormCollBeforeTime);

		if(parent.type == Proj_SpawnParent::PSPT_SHOT) {
			parent.shot->nRandom++;
			parent.shot->nRandom %= 255;		
		}		
	}
	
}

std::string Proj_SpawnInfo::readFromIni(const std::string& file, const std::string& section) {
	ReadKeyword(file, section, "AddParentVel", &AddParentVel, AddParentVel); // new in OLX beta9
	
	ReadKeyword(file, section, "Useangle", &Useangle, Useangle);
	ReadInteger(file, section, "Angle", &Angle, Angle);
	
	ReadKeyword(file, section, "UseProjVelocity", &UseParentVelocityForSpread, UseParentVelocityForSpread);
	
	ReadInteger(file, section, "Amount", &Amount, Amount);
	ReadInteger(file, section, "Speed",  &Speed, Speed);
	ReadFloat(file, section, "SpeedVar",  &SpeedVar, SpeedVar);
	ReadFloat(file, section, "Spread", &Spread, Spread);

	std::string prjfile;
	ReadString(file, section, "Projectile", prjfile, "");
	return prjfile;
}

bool Proj_SpawnInfo::read(CGameScript* gs, FILE* fp) {
	fread_endian<char>(fp, AddParentVel);
	fread_endian<char>(fp, Useangle);
	fread_endian<int>(fp, Angle);
	fread_endian<int>(fp, Amount);
	fread_endian<int>(fp, Speed);
	fread_endian<float>(fp, SpeedVar);
	fread_endian<float>(fp, Spread);
	Proj = gs->LoadProjectile(fp);
	return Proj != NULL;
}

bool Proj_SpawnInfo::write(CGameScript* gs, FILE* fp) {
	fwrite_endian<char>(fp, AddParentVel);
	fwrite_endian<char>(fp, Useangle);
	fwrite_endian<int>(fp, Angle);
	fwrite_endian<int>(fp, Amount);
	fwrite_endian<int>(fp, Speed);
	fwrite_endian<float>(fp, SpeedVar);
	fwrite_endian<float>(fp, Spread);
	return gs->SaveProjectile(Proj, fp);
}

