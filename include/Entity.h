/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Entity structure
// Created 23/7/02
// Jason Boettcher


#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "CViewport.h"
#include "CMap.h"


#define		MAX_ENTITIES	1024


// Entity types
enum {
	ENT_PARTICLE=0,
	ENT_GIB,
	ENT_SPAWN,
	ENT_BLOOD,
	ENT_BLOODDROPPER,
	ENT_EXPLOSION,
	ENT_SMOKE,
	ENT_CHEMSMOKE,
	ENT_SPARKLE,
	ENT_DOOMSDAY,
	ENT_JETPACKSPRAY,
	ENT_BEAM,
	ENT_LASERSIGHT
};



class entity_t { public:
entity_t() : bUsed(false) {}
	bool	bUsed;
	int		iType;
	int		iType2;
	float	fLife;
	CVec	vPos;
	CVec	vVel;
	int		iAngle;
	Uint32	iColour;
	float	fAnglVel;
	int		iRotation;
	
	float	fFrame;
	float	fExtra;
	CachedDataPointer<SDL_Surface> bmpSurf;

};


// Entity routines
int		InitializeEntities(void);
void	ShutdownEntities(void);
void	ClearEntities(void);

void	SpawnEntity(int type, int type2, CVec pos, CVec vel, Uint32 colour, CachedDataPointer<SDL_Surface> img);
void	DrawEntities(SDL_Surface *bmpDest, CViewport *v);
void	SimulateEntities(float dt, CMap *map);
void	EntityBounce(entity_t *ent);


#endif  //  __ENTITY_H__
