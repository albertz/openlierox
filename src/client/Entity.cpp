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


#include "defs.h"
#include "LieroX.h"
#include "GfxPrimitives.h"
#include "Graphics.h"

entity_t *tEntities = NULL;

Uint32 doomsday[4];

///////////////////
// Initialzie the entity system
int InitializeEntities(void)
{
	tEntities = new entity_t[MAX_ENTITIES];
	if(tEntities == NULL)
		return false;

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
	if(tEntities) {
		delete[] tEntities;
		tEntities = NULL;
	}
}


///////////////////
// Clear all the entities
void ClearEntities(void)
{
	// Set all the entities to false
	for(ushort i=0;i<MAX_ENTITIES;i++)
		tEntities[i].bUsed = false;
}


///////////////////
// Spawn an entity
void SpawnEntity(int type, int type2, CVec pos, CVec vel, Uint32 colour, SDL_Surface *img)
{
	entity_t *ent = tEntities;

	// If this is a particle type entity, and particles are switched off, just leave
	if(!tLXOptions->iParticles) {
		if(type == ENT_PARTICLE ||
		   type == ENT_BLOOD)
			return;
	}

	// Find a free entity
	register ushort e;
	bool found = false;
	for(e=0;e<MAX_ENTITIES;e++,ent++) {
		if(!ent->bUsed) {
			found = true;
			break;
		}
	}
	if(!found) return; // nothing free
	
	ent->bUsed = true;
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
void DrawEntities(SDL_Surface *bmpDest, CViewport *v)
{
	entity_t *ent = tEntities;
	static CVec end;

	int wx = v->GetWorldX();
	int wy = v->GetWorldY();
	int l = v->GetLeft();
	int t = v->GetTop();


	static int x,y;
	static int x2,y2;
	static ushort curcount;
		
	for(curcount=0;curcount<MAX_ENTITIES;ent++,curcount++) {
		if(ent->bUsed)  {
			x=((int)ent->vPos.x-wx)*2+l;
			y=((int)ent->vPos.y-wy)*2+t;

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
					r = (Uint8)(0.314f * (255-ent->fFrame));
					g = (Uint8)(0.588f * (255-ent->fFrame));
					b = (Uint8)(0.784f * (255-ent->fFrame));
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
}


///////////////////
// Simulate the entities
void SimulateEntities(float dt, CMap *map)
{
	if (!MAX_ENTITIES)
		return;

	entity_t *ent = tEntities;

	for(register ushort e=0;e<MAX_ENTITIES;e++,ent++) {
		if(!ent->bUsed)
			continue;


		// Collisions and stuff
		switch(ent->iType) {

			case ENT_GIB:			
				ent->iRotation += (int) (ent->fAnglVel * dt);
				if(ent->iRotation > 4)
					ent->iRotation = 0;
			
			// Fallthrough
			case ENT_JETPACKSPRAY:
			case ENT_PARTICLE:
			case ENT_BLOOD:
			case ENT_BLOODDROPPER:
				ent->vVel.y += 100*dt;//vGravity;
				ent->vPos += ent->vVel * dt;
	
			

				// Clipping
				if(ent->vPos.x < 0 || ent->vPos.y < 0 ||
					(int)ent->vPos.x >= (int)map->GetWidth() || (int)ent->vPos.y >= (int)map->GetHeight()) {
					ent->bUsed = false;
					continue;
				}

				// Check if the particle has hit the map
				uchar pf = map->GetPixelFlag((uint)ent->vPos.x,(uint)ent->vPos.y);

				if((pf & PX_ROCK || pf & PX_DIRT)) {
					ent->bUsed = false;

					switch(ent->iType) {

						// Blood
						case ENT_BLOOD:
							PutPixel(map->GetImage(),(int)ent->vPos.x, (int)ent->vPos.y,ent->iColour);

							PutPixel(map->GetDrawImage(),(int)ent->vPos.x*2, (int)ent->vPos.y*2,ent->iColour);
							PutPixel(map->GetDrawImage(),(int)ent->vPos.x*2+1, (int)ent->vPos.y*2,ent->iColour);
							PutPixel(map->GetDrawImage(),(int)ent->vPos.x*2, (int)ent->vPos.y*2+1,ent->iColour);
							PutPixel(map->GetDrawImage(),(int)ent->vPos.x*2+1, (int)ent->vPos.y*2+1,ent->iColour);
							break;

						// Giblet
						case ENT_GIB:
							if(ent->vVel.GetLength2() > 25600)  {
								EntityBounce(ent);
								ent->bUsed = true; // Still alive
							}
							else {
								// Add the gib to the map
								int x = (int)ent->vPos.x-1;
								int y = (int)ent->vPos.y-1;

								// Clipping
								if(x < 0)
									x = 0;
								if(x+4 > (int)map->GetWidth())
									x = (int)map->GetWidth()-4;
								if(y < 0)
									y = 0;
								if(y+4 > (int)map->GetHeight())
									y = (int)map->GetHeight()-4;

								DrawImageAdv(map->GetImage(),ent->bmpSurf,(int)ent->iRotation*4,8,x,y,4,4);
								DrawImageStretch2(map->GetDrawImage(),map->GetImage(),x,y,x*2,y*2,4,4);
							}
							break;
					}
				}
				break;
		}


		// Other stuff
		switch(ent->iType) {

			// Explosion
			case ENT_EXPLOSION:
				ent->fFrame += dt*40;
				if(ent->fFrame > 15)  {
					ent->bUsed = false;
				}
				break;

			// Smoke
			case ENT_SMOKE:
				// Fallthrough
			case ENT_CHEMSMOKE:
				ent->fFrame += dt*15;
				if((int)ent->fFrame > 4)  {
					ent->bUsed = false;
				}
				break;

			// Doomsday
			case ENT_DOOMSDAY:
				ent->fFrame += dt*15;
				if((int)ent->fFrame > 3)  {
					ent->bUsed = false;
				}
				break;

			// Spawn
			case ENT_SPAWN:
				ent->fFrame += dt*15;
				if((int)ent->fFrame > 5)  {
					ent->bUsed = false;
				}
				break;

			// Sparkle
			case ENT_SPARKLE:
				ent->vPos = ent->vPos + CVec(0,5.0f*dt);
				ent->fFrame += dt*5;
				if((int)ent->fFrame > 2)  {
					ent->bUsed = false;
				}
				break;

			// Jetpack Spray
			case ENT_JETPACKSPRAY:
				ent->fFrame += dt*200;
				if((int)ent->fFrame > 150)  {
					ent->bUsed = false;
				}
				break;

			// Beam & Laser Sight
			case ENT_BEAM:
			case ENT_LASERSIGHT:
				if((int)ent->fFrame == 1)  {
					ent->bUsed = false;
				}
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
