/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Entity routines
// Created 23/7/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


entity_t *tEntities = NULL;

Uint32 doomsday[4];

///////////////////
// Initialzie the entity system
int InitializeEntities(void)
{
	tEntities = new entity_t[MAX_ENTITIES];
	if(tEntities == NULL)
		return false;

	// Set all the entities to false
	for(int i=0;i<MAX_ENTITIES;i++)
		tEntities[i].iUsed = false;

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
	for(int i=0;i<MAX_ENTITIES;i++)
		tEntities[i].iUsed = false;
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
	int e;
	for(e=0;e<MAX_ENTITIES;e++,ent++) {
		if(!ent->iUsed)
			break;
	}

	// No free entities
	if(e==MAX_ENTITIES-1)
		return;

	ent->iUsed = true;
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

	if(ent->iType == ENT_EXPLOSION)
		ent->fFrame = (float)(15-type2);

	if(ent->iType == ENT_GIB) {
		ent->fAnglVel = (float)fabs(GetRandomNum())*20;
		ent->iRotation = (int)(fabs(GetRandomNum())*3);
	}

	if(ent->iType == ENT_JETPACKSPRAY)
		ent->fLife = 3;
}


///////////////////
// Draw the entities
void DrawEntities(SDL_Surface *bmpDest, CViewport *v)
{
	entity_t *ent = tEntities;
	CVec end;

	int wx = v->GetWorldX();
	int wy = v->GetWorldY();
	int l = v->GetLeft();
	int t = v->GetTop();


	int x,y;
	int x2,y2;
	int r,g,b;
		
	for(int e=0;e<MAX_ENTITIES;e++,ent++) {
		if(ent->iUsed)  {
			//continue;}

			x=((int)ent->vPos.GetX()-wx)*2+l;
			y=((int)ent->vPos.GetY()-wy)*2+t;

			// Clipping
			if(ent->iType != ENT_BEAM && ent->iType != ENT_LASERSIGHT) {
				if(x<l || x>l+v->GetVirtW())
					continue;
				if(y<t || y>t+v->GetVirtH())
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
					r = (int)(0.314f * (255-ent->fFrame));
					g = (int)(0.588f * (255-ent->fFrame));
					b = (int)(0.784f * (255-ent->fFrame));
					DrawRectFill(bmpDest,x-1,y-1,x+1,y+1,MakeColour(r,g,b));
					break;

				// Beam
				case ENT_BEAM:
					end = ent->vPos + ent->vVel*(float)ent->iType2;
					x2=((int)end.GetX()-wx)*2+l;
					y2=((int)end.GetY()-wy)*2+t;
					DrawBeam(bmpDest, x,y, x2,y2, ent->iColour);
					break;

				// Laser Sight
				case ENT_LASERSIGHT:
					end = ent->vPos + ent->vVel*(float)ent->iType2;
					x2=((int)end.GetX()-wx)*2+l;
					y2=((int)end.GetY()-wy)*2+t;
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
	entity_t *ent = tEntities;

	for(int e=0;e<MAX_ENTITIES;e++,ent++) {
		if(!ent->iUsed)
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
				ent->vVel = ent->vVel + CVec(0,100)*dt;//vGravity;
				ent->vPos = ent->vPos + ent->vVel * dt;
	
			

				// Clipping
				if(ent->vPos.GetX() < 0 || ent->vPos.GetY() < 0 ||
					(int)ent->vPos.GetX() >= map->GetWidth() || (int)ent->vPos.GetY() >= map->GetHeight()) {
					ent->iUsed = false;
					continue;
				}

				// Check if the particle has hit the map
				uchar pf = map->GetPixelFlag((int)ent->vPos.GetX(),(int)ent->vPos.GetY());

				if(pf & PX_ROCK || pf & PX_DIRT) {
					ent->iUsed = false;

					switch(ent->iType) {

						// Blood
						case ENT_BLOOD:
							PutPixel(map->GetImage(),(int)ent->vPos.GetX(), (int)ent->vPos.GetY(),ent->iColour);

							PutPixel(map->GetDrawImage(),(int)ent->vPos.GetX()*2, (int)ent->vPos.GetY()*2,ent->iColour);
							PutPixel(map->GetDrawImage(),(int)ent->vPos.GetX()*2+1, (int)ent->vPos.GetY()*2,ent->iColour);
							PutPixel(map->GetDrawImage(),(int)ent->vPos.GetX()*2, (int)ent->vPos.GetY()*2+1,ent->iColour);
							PutPixel(map->GetDrawImage(),(int)ent->vPos.GetX()*2+1, (int)ent->vPos.GetY()*2+1,ent->iColour);
							break;

						// Giblet
						case ENT_GIB:
							if(VectorLength(ent->vVel) > 160)
								EntityBounce(ent);
							else {
								// Add the gib to the map
								int x = (int)ent->vPos.GetX()-1;
								int y = (int)ent->vPos.GetY()-1;

								// Clipping
								if(x < 0)
									x = 0;
								if(x+4 > map->GetWidth())
									x = map->GetWidth()-4;
								if(y < 0)
									y = 0;
								if(y+4 > map->GetHeight())
									y = map->GetHeight()-4;

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
				if(ent->fFrame > 15)
					ent->iUsed = false;
				break;

			// Smoke
			case ENT_SMOKE:
				// Fallthrough
			case ENT_CHEMSMOKE:
				ent->fFrame += dt*15;
				if((int)ent->fFrame > 4)
					ent->iUsed = false;
				break;

			// Doomsday
			case ENT_DOOMSDAY:
				ent->fFrame += dt*15;
				if((int)ent->fFrame > 3)
					ent->iUsed = false;
				break;

			// Spawn
			case ENT_SPAWN:
				ent->fFrame += dt*15;
				if((int)ent->fFrame > 5)
					ent->iUsed = false;
				break;

			// Sparkle
			case ENT_SPARKLE:
				ent->vPos = ent->vPos + CVec(0,5)*dt;
				ent->fFrame += dt*5;
				if((int)ent->fFrame > 2)
					ent->iUsed = false;
				break;

			// Jetpack Spray
			case ENT_JETPACKSPRAY:
				ent->fFrame += dt*200;
				if((int)ent->fFrame > 150)
					ent->iUsed = false;
				break;

			// Beam & Laser Sight
			case ENT_BEAM:
			case ENT_LASERSIGHT:
				if((int)ent->fFrame == 1)
					ent->iUsed = false;
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
				break;
		}
	}
}


///////////////////
// Bounce an entity
void EntityBounce(entity_t *ent)
{
	ent->vVel = ent->vVel * CVec(-0.4f, -0.4f);
	ent->fAnglVel *= 0.8f;
	
	// Still alive
	ent->iUsed = true;
}
