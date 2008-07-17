/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Entity routines
// Created 23/7/02
// Jason Boettcher


#include "LieroX.h"
#include "GfxPrimitives.h"
#include "Graphics.h"
#include "Entity.h"
#include "MathLib.h"
#include "FastVector.h"

typedef FastVector<entity_t,MAX_ENTITIES> Entities;
Entities tEntities;

Uint32 doomsday[4];

///////////////////
// Initialzie the entity system
int InitializeEntities(void)
{
	tEntities.clear();

    // Pre-calculate the doomsday colour
    doomsday[0] = MakeColour(244,244,112);
    doomsday[1] = MakeColour(248,192,36);
	doomsday[2] = MakeColour(248,108,20);
    doomsday[3] = MakeColour(248,108,20);

	return true;
}


///////////////////
// Shutdown the entity system
void ShutdownEntities(void)
{
	tEntities.clear();
}


///////////////////
// Clear all the entities
void ClearEntities(void)
{
	tEntities.clear();
}


///////////////////
// Spawn an entity
void SpawnEntity(int type, int type2, CVec pos, CVec vel, Uint32 colour, const SmartPointer<SDL_Surface> & img)
{
	// If this is a particle type entity, and particles are switched off, just leave
	if(!tLXOptions->bParticles) {
		if(type == ENT_PARTICLE ||
		   type == ENT_BLOOD)
			return;
	}

	entity_t *ent = tEntities.getNewObj();

	// Are there any free entitiy slots?
	if (!ent)
		return;
	
	ent->Spawn();
	ent->iType = type;
	ent->iType2 = type2;
	ent->vPos = pos;
	ent->vVel = vel;
	ent->iColour = colour;
	ent->bmpSurf = img;

	ent->fExtra = 0;
	ent->fFrame = 0;
	ent->fLife = 0;
	ent->iAngle = 0;	

	switch(ent->iType) {
	case ENT_EXPLOSION:
		ent->fFrame = (float)(15-type2);
		break;
	case ENT_GIB:
		ent->fAnglVel = (float)fabs(GetRandomNum())*20;
		ent->iRotation = (int)(fabs(GetRandomNum())*3);
		break;
	case ENT_JETPACKSPRAY:
		ent->fLife = 3;
	}
}


///////////////////
// Draw the entities
void DrawEntities(SDL_Surface * bmpDest, CViewport *v)
{
	CVec end;

	int wx = v->GetWorldX();
	int wy = v->GetWorldY();
	int l = v->GetLeft();
	int t = v->GetTop();


	int x,y;
	int x2,y2;
		
	for (Entities::Iterator::Ref e = tEntities.begin(); e->isValid(); e->next()) {

		entity_t *ent = &e->get();

		x=((int)ent->vPos.x - wx)*2 + l;
		y=((int)ent->vPos.y - wy)*2 + t;

		// Clipping
		if(ent->iType != ENT_BEAM && ent->iType != ENT_LASERSIGHT) {
			if((x<l || x>l+v->GetVirtW()))
				continue;
			if((y<t || y>t+v->GetVirtH()))
				continue;
		}

		switch(ent->iType) {

			// Particle & Blood
			case ENT_PARTICLE:
			case ENT_BLOOD:				// Fallthrough
			case ENT_BLOODDROPPER:		// Fallthrough
				DrawRectFill(bmpDest,x-1,y-1,x+1,y+1,ent->iColour);
				break;

			// Explosion
			case ENT_EXPLOSION:
				DrawImageAdv(bmpDest,gfxGame.bmpExplosion,(int)ent->fFrame*32,0,x-16,y-16,32,32);
				break;

			// Smoke
			case ENT_SMOKE:
				DrawImageAdv(bmpDest, gfxGame.bmpSmoke, (int)ent->fFrame*14,0,x-7,y-7,14,14);
				break;

			// Chemical smoke
			case ENT_CHEMSMOKE:
				DrawImageAdv(bmpDest, gfxGame.bmpChemSmoke, (int)ent->fFrame*10,0,x-5,y-5,10,10);
				break;

			// Spawn
			case ENT_SPAWN:
				DrawImageAdv(bmpDest, gfxGame.bmpSpawn, (int)ent->fFrame*32,0,x-16,y-16,32,32);
				break;

			// Giblet
			case ENT_GIB:
				DrawImageAdv(bmpDest,ent->bmpSurf,(int)ent->iRotation*8,0,x-2,y-2,8,8);
				break;

			// Sparkle
			case ENT_SPARKLE:
				DrawImageAdv(bmpDest, gfxGame.bmpSparkle, (int)ent->fFrame*10,0, x-5,y-5,10,10);
				break;

			// Doomsday
			case ENT_DOOMSDAY:
				DrawRectFill(bmpDest,x-1,y-1,x+1,y+1,doomsday[(int)ent->fFrame]);
				break;

			// Jetpack spray
			case ENT_JETPACKSPRAY:
				static Uint8 r,g,b;
				r = (Uint8)((float)MIN(0.314f * (255-ent->fFrame),255.0f));
				g = (Uint8)((float)MIN(0.588f * (255-ent->fFrame),255.0f));
				b = (Uint8)((float)MIN(0.784f * (255-ent->fFrame),255.0f));
				DrawRectFill(bmpDest,x-1,y-1,x+1,y+1,MakeColour(r,g,b));
				break;

			// Beam
			case ENT_BEAM:
				end = ent->vPos + ent->vVel*(float)ent->iType2;
				x2=((int)end.x-wx)*2+l;
				y2=((int)end.y-wy)*2+t;
				DrawBeam(bmpDest, x,y, x2,y2, ent->iColour);
				break;

			// Laser Sight
			case ENT_LASERSIGHT:
				end = ent->vPos + ent->vVel*(float)ent->iType2;
				x2=((int)end.x-wx)*2+l;
				y2=((int)end.y-wy)*2+t;
				DrawLaserSight(bmpDest, x,y, x2,y2, ent->iColour);
				break;
		}
	}
}


///////////////////
// Simulate the entities
void SimulateEntities(float dt, CMap *map)
{
	if (!map) { // Weird
		printf("WARNING. SimulateEntities gots no map\n");
		return;
	}

	float realdt = tLX->fRealDeltaTime;

	for (Entities::Iterator::Ref e = tEntities.begin(); e->isValid(); e->next()) {

		entity_t *ent = &e->get();

		// Collisions and stuff
		switch(ent->iType) {

			case ENT_GIB:			
				ent->iRotation += (int) (ent->fAnglVel * realdt);
				ent->iRotation %= 5;
			
			// Fallthrough
			case ENT_JETPACKSPRAY:
			case ENT_PARTICLE:
			case ENT_BLOOD:
			case ENT_BLOODDROPPER:
				ent->vVel.y += 100*dt;//vGravity;
				ent->vPos += ent->vVel * dt;
	
				// Clipping
				if((uint)ent->vPos.x >= (uint)map->GetWidth() || (uint)ent->vPos.y >= (uint)map->GetHeight()) {
					ent->setUnused();
					break;
				}

				// Check if the particle has hit the map
				uchar pf = map->GetPixelFlag((uint)ent->vPos.x, (uint)ent->vPos.y);

				if((pf & (PX_ROCK|PX_DIRT))) {

					switch(ent->iType) {

						// Blood
						case ENT_BLOOD:  {
								int x = (int)ent->vPos.x;
								int y = (int)ent->vPos.y;
								LOCK_OR_QUIT(map->GetImage());
								PutPixel(map->GetImage().get(),(int)ent->vPos.x, (int)ent->vPos.y,ent->iColour);
								UnlockSurface(map->GetImage());

								x *= 2;
								y *= 2;
								DrawRectFill(map->GetDrawImage().get(), x, y, x + 2, y + 2, ent->iColour);

								ent->setUnused();
							} break;

						// Giblet
						case ENT_GIB:
							if((int)ent->vVel.GetLength2() > 25600)  {
								EntityBounce(ent);
								// Still alive
							}
							else {

								// Add the gib to the map
								int x = (int)ent->vPos.x-1;
								int y = (int)ent->vPos.y-1;

								DrawImageAdv(map->GetImage().get(),ent->bmpSurf,(int)ent->iRotation*4,8,x,y,4,4);
								DrawImageStretch2(map->GetDrawImage().get(),map->GetImage(),x,y,x*2,y*2,4,4);

								ent->setUnused();
							}
							break;

						default:
							ent->setUnused();
					}
				}
				break;
		}


		// Other stuff
		switch(ent->iType) {

			// Explosion
			case ENT_EXPLOSION:
				ent->fFrame += realdt * 40;
				if(ent->fFrame > 15) ent->setUnused();
				break;

			// Smoke
			case ENT_SMOKE:
				// Fallthrough
			case ENT_CHEMSMOKE:
				ent->fFrame += realdt * 15;
				if((int)ent->fFrame > 4) ent->setUnused();
				break;

			// Doomsday
			case ENT_DOOMSDAY:
				ent->fFrame += realdt * 15;
				if((int)ent->fFrame > 3) ent->setUnused();
				break;

			// Spawn
			case ENT_SPAWN:
				ent->fFrame += realdt * 15;
				if((int)ent->fFrame > 5) ent->setUnused();
				break;

			// Sparkle
			case ENT_SPARKLE:
				ent->vPos = ent->vPos + CVec(0, 5.0f * dt);
				ent->fFrame += realdt * 5;
				if((int)ent->fFrame > 2) ent->setUnused();
				break;

			// Jetpack Spray
			case ENT_JETPACKSPRAY:
				ent->fFrame += realdt * 200;
				if((int)ent->fFrame > 150) ent->setUnused();
				break;

			// Beam & Laser Sight
			case ENT_BEAM:
			case ENT_LASERSIGHT:
				if((int)ent->fFrame == 1) ent->setUnused();
				ent->fFrame++;
				break;

			// Blood dropper
			case ENT_BLOODDROPPER:
				if(ent->fExtra > 0.1f) {
					int col = GetRandomInt(1);
					static const int colour[] = {128,200};
					SpawnEntity(ENT_BLOOD,0,ent->vPos,CVec(GetRandomNum(),GetRandomNum()),MakeColour(colour[col],0,0),NULL);
					ent->fExtra = 0;
				}
				ent->fExtra += dt;
				break;
		}
	}
}


///////////////////
// Bounce an entity
void EntityBounce(entity_t *ent)
{
	ent->vVel.x *= -0.4f;
	ent->vVel.y *= -0.4f;
	ent->fAnglVel *= 0.8f;
}
