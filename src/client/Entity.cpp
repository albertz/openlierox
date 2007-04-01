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
#include "Graphics.h"
#include "GfxPrimitives.h"

// TODO: check that this is working like the original lierox


std::list<entity_t> tEntities;

Uint32 doomsday[4];

///////////////////
// Initialzie the entity system
int InitializeEntities(void)
{
	// Clear
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
void SpawnEntity(int type, int type2, CVec pos, CVec vel, Uint32 colour, SDL_Surface *img)
{
	// If this is a particle type entity, and particles are switched off, just leave
	if(!tLXOptions->iParticles) {
		if(type == ENT_PARTICLE ||
		   type == ENT_BLOOD)
			return;
	}
	
	// No free entities
	if(tEntities.size()==MAX_ENTITIES)
		return;

	entity_t NewEntity;

	// Initialize
	NewEntity.iType = type;
	NewEntity.iType2 = type2;
	NewEntity.vPos = pos;
	NewEntity.vVel = vel;
	NewEntity.iColour = colour;
	NewEntity.bmpSurf = img;

	NewEntity.fExtra = 0;
	NewEntity.fFrame = 0;
	NewEntity.fLife = 0;
	NewEntity.iAngle = 0;	

	switch(NewEntity.iType) {
	case ENT_EXPLOSION:
		NewEntity.fFrame = (float)(15-type2);
		break;
	case ENT_GIB:
		NewEntity.fAnglVel = (float)fabs(GetRandomNum())*20;
		NewEntity.iRotation = (int)(fabs(GetRandomNum())*3);
		break;
	case ENT_JETPACKSPRAY:
		NewEntity.fLife = 3;
	}

	tEntities.push_back(NewEntity);
	
}


///////////////////
// Draw the entities
void DrawEntities(SDL_Surface *bmpDest, CViewport *v)
{
	std::list<entity_t>::const_iterator ent;
	CVec end;

	int wx = v->GetWorldX();
	int wy = v->GetWorldY();
	int l = v->GetLeft();
	int t = v->GetTop();


	int x,y;
	int x2,y2;
		
	for(ent=tEntities.begin();ent!=tEntities.end();ent++) {

		x=((int)ent->vPos.x-wx)*2+l;
		y=((int)ent->vPos.y-wy)*2+t;

		// Clipping
		if(ent->iType != ENT_BEAM && ent->iType != ENT_LASERSIGHT) {
			if((x < l || x > l + v->GetVirtW()))
				continue;
			if((y < t || y > t + v->GetVirtH()))
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
				Uint8 r,g,b;
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

///////////////////
// Simulate the entities
void SimulateEntities(float dt, CMap *map)
{
	// TODO: this simulation depends to much on dt
	
	std::list<entity_t>::iterator ent,tmp_it;

	for(ent=tEntities.begin();ent!=tEntities.end();) {
		
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
	
				// Check if the particle has hit the map
				uchar pf = map->GetPixelFlag((int)ent->vPos.x,(int)ent->vPos.y);

				if(!(pf & PX_EMPTY)) {
					switch(ent->iType) {

						// Blood
						case ENT_BLOOD:
							map->PutImagePixel((int)ent->vPos.x, (int)ent->vPos.y, ent->iColour);
							map->UpdateMiniMapRect((int)ent->vPos.x-1,(int)ent->vPos.y-1,3,3);

							break;

						// Giblet
						case ENT_GIB:
							if(ent->vVel.GetLength2() > 25600)
								EntityBounce(&(*ent));
							else {
								// Add the gib to the map
								int x = (int)ent->vPos.x-1;
								int y = (int)ent->vPos.y-1;

								DrawImageAdv(map->GetImage(),ent->bmpSurf,(int)ent->iRotation*4,8,x,y,4,4);
								DrawImageStretch2(map->GetDrawImage(),map->GetImage(),x,y,x*2,y*2,4,4);

								map->UpdateMiniMapRect(x,y,ent->bmpSurf->w,ent->bmpSurf->h);

							}
							break;
					}


					// Remove
					tmp_it = ent; tmp_it++;
					tEntities.erase(ent);
					ent = tmp_it;
					continue;

				}
				break;
		}


		// Other stuff
		switch(ent->iType) {

			// Explosion
			case ENT_EXPLOSION:
				ent->fFrame += dt*40;				
				if(ent->fFrame > 15)  {
					tmp_it = ent; tmp_it++;
					tEntities.erase(ent);
					ent = tmp_it;
				} else ent++;
				break;

			// Smoke
			case ENT_SMOKE:
				// Fallthrough
			case ENT_CHEMSMOKE:
				ent->fFrame += dt*15;
				if((int)ent->fFrame > 4)  {
					tmp_it = ent; tmp_it++;
					tEntities.erase(ent);
					ent = tmp_it;
				} else ent++;
				break;

			// Doomsday
			case ENT_DOOMSDAY:
				ent->fFrame += dt*15;
				if((int)ent->fFrame > 3)  {
					tmp_it = ent; tmp_it++;
					tEntities.erase(ent);
					ent = tmp_it;
				} else ent++;
				break;

			// Spawn
			case ENT_SPAWN:
				ent->fFrame += dt*15;
				if((int)ent->fFrame > 5)  {
					tmp_it = ent; tmp_it++;
					tEntities.erase(ent);
					ent = tmp_it;
				} else ent++;
				break;

			// Sparkle
			case ENT_SPARKLE:
				ent->vPos = ent->vPos + CVec(0,5.0f*dt);
				ent->fFrame += dt*5;
				if((int)ent->fFrame > 2)  {
					tmp_it = ent; tmp_it++;
					tEntities.erase(ent);
					ent = tmp_it;
				} else ent++;
				break;

			// Jetpack Spray
			case ENT_JETPACKSPRAY:
				ent->fFrame += dt*200;
				if((int)ent->fFrame > 150)  {
					tmp_it = ent; tmp_it++;
					tEntities.erase(ent);
					ent = tmp_it;
				} else ent++;
				break;

			// Beam & Laser Sight
			case ENT_BEAM:
			case ENT_LASERSIGHT:
				if((int)ent->fFrame == 1)  {
					tmp_it = ent; tmp_it++;
					tEntities.erase(ent);
					ent = tmp_it;
				} else ent++;
				ent->fFrame++;
				break;

			// Blood dropper
			case ENT_BLOODDROPPER:
				if(ent->fExtra > 0.1f) {
					int col = GetRandomInt(1);
					int colour[] = {128,200};
					SpawnEntity(ENT_BLOOD,0,ent->vPos,CVec(GetRandomNum(),GetRandomNum()),MakeColour(colour[col],0,0),NULL);
					ent->fExtra = 0;
				}
				ent->fExtra += dt;
				ent++;
				break;

			default: ent++;
		}
	}
}

///////////////////
// Bounce an entity
void EntityBounce(entity_t *ent)
{
	ent->vVel *= -0.4f;
	ent->fAnglVel *= 0.8f; // TODO: this is not physical correct!
}
