/*
	OpenLieroX
	
	the LX56 physics

	code under LGPL
	created on 9/2/2008
*/

#include "Physics.h"

// TODO: clean up this code!


#include "CGameScript.h"
#include "Frame.h"
#include "MathLib.h"
#include "CWorm.h"
#include "Entity.h"
#include "CBonus.h"
#include "CClient.h"
#include "console.h"


class PhysicsLX56 : public PhysicsEngine {
public:
	CMap* map;

// ---------
	PhysicsLX56() : map(NULL) {}
	virtual ~PhysicsLX56() {}
	
	virtual std::string name() { return "LX56 physics"; }

	virtual void initGame( CMap* m ) { map = m; }
	virtual void uninitGame() { map = NULL; }
	
// -----------------------------
// ------ worm -----------------

	// Check collisions with the level
	// HINT: it directly manipulates vPos!
	bool moveAndCheckWormCollision(float dt, CWorm* worm, CVec pos, CVec *vel, CVec vOldPos, int jump ) {
		static const int maxspeed2 = 10;

		// Can happen when starting a game
		if (!map)
			return false;
		
		// If the worm is going too fast, divide the speed by 2 and perform 2 collision checks
		// TODO: is this still needed? we call this function with a fixed dt
		// though perhaps it is as with higher speed the way we have to check is longer
		if( (*vel*dt).GetLength2() > maxspeed2) {
			dt /= 2;
			if(moveAndCheckWormCollision(dt,worm,pos,vel,vOldPos,jump)) return true;
			return moveAndCheckWormCollision(dt,worm,worm->getPos(),vel,vOldPos,jump);
		}

		pos += *vel * dt;
		worm->pos() = pos;


		int x,y;
		x = (int)pos.x;
		y = (int)pos.y;
		short clip = 0; // 0x1=left, 0x2=right, 0x4=top, 0x8=bottom
		bool coll = false;
		bool check_needed = false;

		const uchar* gridflags = map->getAbsoluteGridFlags();
		uint grid_w = map->getGridWidth();
		uint grid_h = map->getGridHeight();
		uint grid_cols = map->getGridCols();
		if(y-4 < 0 || (uint)y+5 > map->GetHeight()-1
		|| x-3 < 0 || (uint)x+3 > map->GetWidth()-1)
			check_needed = true; // we will check later, what to do here
		else if(grid_w < 7 || grid_h < 10 // this ensures, that this check is safe
		|| (gridflags[((y-4)/grid_h)*grid_cols + (x-3)/grid_w] & (PX_ROCK|PX_DIRT))
		|| (gridflags[((y+5)/grid_h)*grid_cols + (x-3)/grid_w] & (PX_ROCK|PX_DIRT))
		|| (gridflags[((y-4)/grid_h)*grid_cols + (x+3)/grid_w] & (PX_ROCK|PX_DIRT))
		|| (gridflags[((y+5)/grid_h)*grid_cols + (x+3)/grid_w] & (PX_ROCK|PX_DIRT)))
			check_needed = true;

		if(check_needed && y >= 0 && (uint)y < map->GetHeight()) {
			for(x=-3;x<4;x++) {
				// Optimize: pixelflag++

				// Left side clipping
				if(pos.x+x <= 2) {
					clip |= 0x01;
					worm->pos().x=( 5 );
					coll = true;
					if(fabs(vel->x) > 40)
						vel->x *=  -0.4f;
					else
						vel->x=(0);
					continue;
				}

				// Right side clipping
				if(pos.x+x >= map->GetWidth()) {
					worm->pos().x=( (float)map->GetWidth() - 5 );
					coll = true;
					clip |= 0x02;
					if(fabs(vel->x) > 40)
						vel->x *= -0.4f;
					else
						vel->x=(0);
					continue;
				}


				if(!(map->GetPixelFlag((int)pos.x+x,y) & PX_EMPTY)) {
					coll = true;
					
					if(x<0) {
						clip |= 0x01;
						worm->pos().x=( pos.x+x+4 );
					}
					else {
						clip |= 0x02;
						worm->pos().x=( pos.x+x-4 );
					}

					// Bounce
					if(fabs(vel->x) > 30)
						vel->x *= -0.4f;
					else
						vel->x=(0);
				}
			}
		}

		worm->setOnGround( false );

		bool hit = false;
		x = (int)pos.x;

		if(check_needed && (uint)x < map->GetWidth()) {
			for(y=5;y>-5;y--) {
				// Optimize: pixelflag + Width

				// Top side clipping
				if(pos.y+y <= 1) {
					worm->pos().y=( 6 );
					coll = true;
					clip |= 0x04;
					if(fabs(vel->y) > 40)
						vel->y *= -0.4f;
					continue;
				}

				// Bottom side clipping
				if(pos.y+y >= map->GetHeight()) {
					worm->pos().y=( (float)map->GetHeight() - 5 );
					clip |= 0x08;
					coll = true;
					worm->setOnGround( true );
					if(fabs(vel->y) > 40)
						vel->y *= -0.4f;
					else
						vel->y=(0);
					continue;
				}


				if(!(map->GetPixelFlag(x,(int)pos.y+y) & PX_EMPTY)) {
					coll = true;

					if(!hit && !jump) {
						if(fabs(vel->y) > 40 && ((vel->y > 0 && y>0) || (vel->y < 0 && y<0)))
							vel->y *= -0.4f;
						else
							vel->y=(0);
					}

					hit = true;
					worm->setOnGround( true );

					if(y<0) {
						clip |= 0x04;
						worm->pos().y=( pos.y+y+5 );
					}
					else {
						clip |= 0x08;
						worm->pos().y=( pos.y+y-5 );
					}

					//if(y>3 && !jump) {
						//vVelocity.y=(-10);
						//Velocity.y=(0);
					//	break;
					//}
				}
			}
		}

		// If we are stuck in left & right or top & bottom, just don't move in that direction
		if ((clip & 0x01) && (clip & 0x02))
			worm->pos().x = vOldPos.x;

		// HINT: when stucked horizontal we move slower - it's more like original LX
		if ((clip & 0x04) && (clip & 0x08))  {
			worm->pos().y = vOldPos.y;
			worm->pos().x -= (worm->pos().x - vOldPos.x) / 2;
		}

		// If we collided with the ground and we were going pretty fast, make a bump sound
		if(coll) {
			if( fabs(vel->x) > 30 && (clip & 0x01 || clip & 0x02) )
				StartSound( sfxGame.smpBump, worm->pos(), worm->getLocal(), -1, worm );
			else if( fabs(vel->y) > 30 && (clip & 0x04 || clip & 0x08) )
				StartSound( sfxGame.smpBump, worm->pos(), worm->getLocal(), -1, worm );
		}

		return coll;
	}


	virtual void simulateWorm(CWorm* worm, CClient* client, CWorm *worms, int local) {
		const static float dt = 0.01f;
		if(worm->fLastSimulationTime + dt > tLX->fCurTime) return;
		
		// get input max once a frame (and not at all if we don't simulate this frame)
		
			/*
				Only get input for this worm on certain conditions:
				1) This worm is a local worm (ie, owned by me)
				2) We're not in a game menu
				3) We're not typing a message
				4) weapons selected
			*/
		
		if(client && local && !client->isGameMenu() && !client->isChatTyping() && !client->isGameOver() && !Con_IsUsed() && worm->getWeaponsReady()) {
			int old_weapon = worm->getCurrentWeapon();

			// TODO: use one getInput for both
			if(worm->getType() == PRF_HUMAN)
				worm->getInput();
			else
				worm->AI_GetInput(tGameInfo.iGameType, tGameInfo.iGameType == GMT_TEAMDEATH, tGameInfo.iGameType == GMT_TAG, tGameInfo.iGameType == GMT_VIP, tGameInfo.iGameType == GMT_CTF, tGameInfo.iGameType == GMT_TEAMCTF);

			if (worm->isShooting() || old_weapon != worm->getCurrentWeapon())  // The weapon bar is changing
				client->shouldRepaintInfo() = true;
		}
		
	simulateWormStart:
		if(worm->fLastSimulationTime + dt > tLX->fCurTime) return;
		worm->fLastSimulationTime += dt;
	
		float speed;
		CVec dir;
		const gs_worm_t *wd = worm->getGameScript()->getWorm();

		float	fFrameRate = 7.5f;

		worm_state_t *ws = worm->getWormState();

		// Simulate the viewport
		//cViewport.Process(vPos,map->GetWidth(),map->GetHeight());


		// Special things for remote worms
		if(!local) {
			worm->setAngle( (float)ws->iAngle );
			worm->setDirection( ws->iDirection );
		}


		// If we're IT, spawn some sparkles
		if(worm->getTagIT() && tGameInfo.iGameMode == GMT_TAG) {
			if(tLX->fCurTime - worm->getLastSparkle() > 0.15f) {
				worm->setLastSparkle( worm->fLastSimulationTime );
				CVec p = worm->getPos() + CVec(GetRandomNum()*3, GetRandomNum()*3);

				SpawnEntity(ENT_SPARKLE,0,p, CVec(0,0), 0,NULL);
			}
		}

		// If we're seriously injured (below 15% health) bleed
		if(worm->getHealth() < 15) {
			if(tLX->fCurTime - worm->getLastBlood() > 2) {
				worm->setLastBlood( worm->fLastSimulationTime );

				float amount = ((float)tLXOptions->iBloodAmount / 100.0f) * 10;
				CVec v;
				for(short i=0;i<amount;i++) {
					v = CVec(GetRandomNum(), GetRandomNum()) * 30;
					SpawnEntity(ENT_BLOOD,0,worm->getPos(),v,MakeColour(200,0,0),NULL);
					SpawnEntity(ENT_BLOOD,0,worm->getPos(),v,MakeColour(180,0,0),NULL);
				}
			}
		}

		// Calculate dir
		dir.x=( (float)cos(worm->getAngle() * (PI/180)) );
		dir.y=( (float)sin(worm->getAngle() * (PI/180)) );
		if(worm->getDirection()==DIR_LEFT)
			dir.x=(-dir.x);

		// Process the carving
		if(ws->iCarve) {
			worm->incrementDirtCount( CarveHole(worm->getMap(), worm->getPos() + dir*4) );
			//cClient->SendCarve(vPos + dir*4);
		}

		if(ws->iMove)
			worm->frame() += fFrameRate * dt;

		if(worm->frame() >= 3.0f || !ws->iMove)
			worm->frame() = (0);

		speed = worm->isOnGround() ? wd->GroundSpeed : wd->AirSpeed;

		// Process the ninja rope
		if(worm->getNinjaRope()->isReleased() && worms) {
			simulateNinjarope( dt, worm, worms );
			// TODO: move 'getForce' here?
			worm->velocity() += worm->getNinjaRope()->GetForce(worm->getPos()) * dt;
		}

		// Process the moving
		if(ws->iMove) {
			if(worm->getDirection() == DIR_RIGHT) {
				// Right
				if(worm->getVelocity()->x < 30)
					worm->getVelocity()->x += speed * dt * 90.0f;
			} else {
				// Left
				if(worm->getVelocity()->x > -30)
					worm->getVelocity()->x -= speed * dt * 90.0f;
			}
		}

		// TODO: move this to CWorm::getInput() ?
		if(worm->getStrafeInput()->isDown())
			worm->setDirection( worm->getStrafeDirection() );


		// Process the jump
		if(ws->iJump && worm->CheckOnGround()) {
			worm->getVelocity()->y = wd->JumpForce;
			worm->setOnGround( false );
		}


		// Air drag (Mainly to dampen the ninja rope)
		float Drag = wd->AirFriction;

		if(!worm->isOnGround())	{
			worm->getVelocity()->x -= SQR(worm->getVelocity()->x) * SIGN(worm->getVelocity()->x) * Drag * dt;
			worm->getVelocity()->y += -SQR(worm->getVelocity()->y) * SIGN(worm->getVelocity()->y) * Drag * dt;
		}


		// Gravity
		worm->getVelocity()->y += wd->Gravity*dt;


		//resetFollow(); // reset follow here, projectiles will maybe re-enable it...

		// Check collisions and move
		moveAndCheckWormCollision( dt, worm, worm->getPos(), worm->getVelocity(), worm->getPos(), ws->iJump );


		// Ultimate in friction
		if(worm->isOnGround()) {
			worm->velocity().x *= 0.9f;

			//vVelocity = vVelocity * CVec(/*wd->GroundFriction*/ 0.9f,1);        // Hack until new game script is done

			// Too slow, just stop
			if(fabs(worm->getVelocity()->x) < 5 && !ws->iMove)
				worm->getVelocity()->x = 0;
		}

		simulateWormWeapon(dt, worm);
		

		// Fill in the info for sending
		if(local) {
			ws->iAngle = (int)worm->getAngle();
			ws->iDirection = worm->getDirection();
			ws->iX = (int)worm->getPos().x;
			ws->iY = (int)worm->getPos().y;
		}
	
		goto simulateWormStart;
	}

	virtual void simulateWormWeapon(float dt, CWorm* worm) {
		// Weird
		if (worm->getCurrentWeapon() < 0 || worm->getCurrentWeapon() >= 5) {
			printf("WARNING: SimulateWeapon: iCurrentWeapon is bad\n");
			return;
		}

		wpnslot_t *Slot = worm->getWeapon(worm->getCurrentWeapon());

		if(Slot->LastFire>0)
			Slot->LastFire-=dt;

		if(Slot->Reloading) {

			// Prevent a div by zero error
			if(worm->getLoadingTime() == 0)
				worm->setLoadingTime( 0.00001f );

			Slot->Charge += dt * (Slot->Weapon->Recharge * (1.0f/worm->getLoadingTime()));
			if(Slot->Charge > 1) {
				Slot->Charge = 1;
				Slot->Reloading = false;
			}
		}	
	}
	
	virtual void simulateProjectile(CProjectile* proj, CWorm *worms, int *wormid, int* result) {
		const static float fixed_dt = 0.01f;
		*result = PJC_NONE;
		
		// HINT: this is a bit hacky here because there is a CheckCollision used which manipulates dt
	simulateProjectileStart:
		if(proj->fLastSimulationTime + fixed_dt > tLX->fCurTime) return;
		
		int res = PJC_NONE;

		// If this is a remote projectile, the first frame is simulated with a longer delta time
		// HINT: replaced with ping simulation, see CClient_Game.cpp, line 304
		/*if(bRemote) {
			bRemote = false;

			// Only do it for a positive delta time
			if(fRemoteFrameTime>0) {
				res = Simulate(fRemoteFrameTime, map,worms,wormid);
				//if( res != PJC_NONE )
				//	return res;
				return res;
			}
		}*/
		proj->setRemote( false );
		
		// Check for collisions
		// ATENTION: dt will manipulated directly here!
		// TODO: use a more general CheckCollision here
		float dt = fixed_dt;
		int colret = proj->CheckCollision(dt, map, worms, &dt);
		if(colret == -1)
			res |= PJC_TERRAIN;
		else if(colret >= 0) {
			*wormid = colret;
			res |= PJC_WORM;
		}
		proj->fLastSimulationTime += dt;
		
		// HINT: in original LX, we have this simulate code with lower dt
		//		(this is now the work of CheckCollision)
		//		but we also do this everytime without checks for a collision
		//		so this new code should behave like the original LX on high FPS
		// PERHAPS: do this before correction of dt; this is perhaps more like
		//		the original behavior (with lower FPS)
		//		(or leave it this way, if it works)
		proj->life() += dt;
		proj->extra() += dt;

		// If any of the events have been triggered, add that onto the flags
		if( proj->explode() && tLX->fCurTime > proj->explodeTime()) {
			res |= PJC_EXPLODE;
			proj->explode() = false;
		}
		if( proj->touched() ) {
			res |= PJC_TOUCH;
			proj->touched() = false;
		}



	/*
		vOldPos = vPosition;
	*/

		if(proj->getProjInfo()->Rotating)
			proj->rotation() += (float)proj->getProjInfo()->RotSpeed * dt;
		if(proj->rotation() < 0)
			proj->rotation() = 360;
		else if(proj->rotation() > 360)
			proj->rotation() = 0;

		// Animation
		if(proj->getProjInfo()->Animating) {
			if(proj->getFrameDelta())
				proj->frame() += (float)proj->getProjInfo()->AnimRate * dt;
			else
				proj->frame() -= (float)proj->getProjInfo()->AnimRate * dt;

			if(proj->getProjInfo()->bmpImage) {
				int NumFrames = proj->getProjInfo()->bmpImage->w / proj->getProjInfo()->bmpImage->h;
				if(proj->frame() >= NumFrames) {
					switch(proj->getProjInfo()->AnimType) {
					case ANI_ONCE:
						proj->setUsed( false );
						break;
					case ANI_LOOP:
						proj->frame() = 0;
						break;
					case ANI_PINGPONG:
						// TODO: why is an integer negated here?
						proj->setFrameDelta( ! proj->getFrameDelta() );
						proj->frame() = (float)NumFrames - 1;
					}
				}
				if(proj->frame() < 0) {
					if(proj->getProjInfo()->AnimType == ANI_PINGPONG) {
						// TODO: why is an integer negated here?
						proj->setFrameDelta( ! proj->getFrameDelta() );
						proj->frame() = 0.0f;
					}
				}
			}
		}

		// Trails	
		switch(proj->getProjInfo()->Trail) {
		case TRL_SMOKE:
			if(proj->extra() >= 0.075f) {
				proj->extra()-=0.075f;
				SpawnEntity(ENT_SMOKE,0,proj->GetPosition(),CVec(0,0),0,NULL);
			}
			break;
		case TRL_CHEMSMOKE:
			if(proj->extra() >= 0.075f) {
				proj->extra()-=0.075f;
				SpawnEntity(ENT_CHEMSMOKE,0,proj->GetPosition(),CVec(0,0),0,NULL);
			}
			break;
		case TRL_DOOMSDAY:
			if(proj->extra() >= 0.05f) {
				proj->extra()-=0.05f;
				SpawnEntity(ENT_DOOMSDAY,0,proj->GetPosition(),proj->GetVelocity(),0,NULL);
			}
			break;
		case TRL_EXPLOSIVE:
			if(proj->extra() >= 0.05f) {
				proj->extra()-=0.05f;
				SpawnEntity(ENT_EXPLOSION,10,proj->GetPosition(),CVec(0,0),0,NULL);
			}
			break;
		case TRL_PROJECTILE: // Projectile trail
			if(tLX->fCurTime > proj->lastTrailProj()) {
				proj->lastTrailProj() = tLX->fCurTime + proj->getProjInfo()->PrjTrl_Delay;

				// Set the spawning to true so the upper layers of code (client) will spawn the projectiles
				// TODO: why is boolean used for an integer here?
				proj->setSpawnPrjTrl( true );
			}
		}

	/*
		// Check worm collisions
		int w = CheckWormCollision(worms);
		if( w >= 0 ) {
			*wormid = w;
			res |= PJC_WORM;
		}
	*/

	/*
		// Hack!!!
		if(tProjInfo->Hit_Type == PJ_EXPLODE && tProjInfo->Type == PRJ_PIXEL) {
			int px = (int)vPosition.x;
			int py = (int)vPosition.y;

			// Edge checks
			if(px<=0 || py<=0 || px>=map->GetWidth()-1 || py>=map->GetHeight()-1) {
				// Clamp the position			
				px = MAX(px,0); // not needed
				py = MAX(py,0);
				px = MIN(map->GetWidth()-1,px);
				py = MIN(map->GetHeight()-1,py);
				res |= PJC_TERRAIN;
				return res;
			}

			uchar pf = map->GetPixelFlag(px, py);
			if(pf & PX_DIRT || pf & PX_ROCK)
				res |= PJC_TERRAIN;

			return res;
		}

	*/
		if(res != PJC_NONE) {
			*result = res;
			//return res;
			return;
		}
		
		goto simulateProjectileStart;	
	}
	
	void simulateNinjarope(float dt, CWorm* owner, CWorm *worms) {
		CNinjaRope* rope = owner->getNinjaRope();
		CVec playerpos = owner->getPos();
		
		rope->updateOldHookPos();

		if(!rope->isReleased())
			return;

		float length2;
		// TODO: why is int used as a boolean here?
		int firsthit = !rope->isAttached();
		CVec force;

		if(rope->isShooting())
			force = CVec(0,100);
		else
			force = CVec(0,150);

		// TODO: does this need more improvement/optimisation ?
		// TODO: is this still needed? we get in any case a fixed dt here
		// though perhaps it is if the rope is very fast?
		if((rope->getHookVel() + force*dt).GetLength2() * dt * dt > 5) {
			simulateNinjarope( dt/2, owner, worms );
			simulateNinjarope( dt/2, owner, worms );
			return;
		}

		// Still flying in the air
		if(rope->isShooting()) {

			// Gravity
			rope->hookVelocity() += force*dt;
			rope->hookPos() += rope->hookVelocity() * dt;

			length2 = (playerpos - rope->hookPos()) . GetLength2();

			// Check if it's too long
			if(length2 > rope->getMaxLength() * rope->getMaxLength()) {
				rope->hookVelocity() = CVec(0,0);
				rope->setShooting( false );
			}
		}
		// Failing
		else if(!rope->isShooting() && !rope->isAttached()) {

			// Going towards the player
			length2 = (playerpos - rope->hookPos()) . GetLength2();
			if(length2 > rope->getRestLength() * rope->getRestLength()) {

				// Pull the hook back towards the player
				CVec d = playerpos - rope->hookPos();
				if(length2) d *= (float)(1.0f/sqrt(length2)); // normalize

				force += (d*10000)*dt;
			}

			rope->hookVelocity() += force * dt;
			rope->hookPos() += rope->hookVelocity() * dt;

			//HookPos = HookPos + CVec(0,170*dt);
		}

		bool outsideMap = false;

		// Hack to see if the hook went out of the map
		if(rope->hookPos().x <= 0 || rope->hookPos().y <= 0 ||
				rope->hookPos().x >= map->GetWidth()-1 ||
				rope->hookPos().y >= map->GetHeight()-1) {
			rope->setShooting( false );
			rope->setAttached( true );
			rope->setPlayerAttached( false );

			// Make the hook stay at an edge
			rope->hookPos().x = ( MAX((float)0, rope->hookPos().x) );
			rope->hookPos().y = ( MAX((float)0, rope->hookPos().y) );

			rope->hookPos().x = ( MIN(map->GetWidth()-(float)1, rope->hookPos().x) );
			rope->hookPos().y = ( MIN(map->GetHeight()-(float)1, rope->hookPos().y) );

			outsideMap = true;
		}


		// Check if the hook has hit anything on the map
		if(!rope->isPlayerAttached())
			rope->setAttached( false );

		LockSurface(map->GetImage());
		uchar px = map->GetPixelFlag((int)rope->hookPos().x, (int)rope->hookPos().y);
		if((px & PX_ROCK || px & PX_DIRT || outsideMap) && !rope->isPlayerAttached()) {
			rope->setShooting( false );
			rope->setAttached( true );
			rope->setPlayerAttached( false );
			rope->hookVelocity() = CVec(0,0);

			if(px & PX_DIRT && firsthit) {
				Uint32 col = GetPixel(map->GetImage(), (int)rope->hookPos().x, (int)rope->hookPos().y);
				for( short i=0; i<5; i++ )
					SpawnEntity(ENT_PARTICLE,0, rope->hookPos() + CVec(0,2), CVec(GetRandomNum()*40,GetRandomNum()*40),col,NULL);
			}
		}
		UnlockSurface(map->GetImage());


		// Check if the hook has hit another worm
		if(!rope->isAttached() && !rope->isPlayerAttached()) {
			rope->setPlayerAttached( false );

			for(short i=0; i<MAX_WORMS; i++) {
				// Don't check against the worm if they aren't used, dead, a flag or the ninja rope was shot by the worm
				if(!worms[i].isUsed())
					continue;
				if(!worms[i].getAlive())
					continue;
				if(worms[i].getID() == owner->getID())
					continue;
				if(worms[i].getFlag())
					continue;

				if( ( worms[i].getPos() - rope->hookPos() ).GetLength2() < 25 ) {
					rope->setAttached( true );
					rope->setPlayerAttached( true );
					rope->setAttachedPlayer( &worms[i] );
					rope->getAttachedPlayer()->setHooked(true, owner);
					break;
				}
			}
		}

		// Put the hook where the worm is
		else if(rope->isAttached() && rope->isPlayerAttached()) {

			// If the worm has been killed, or dropped, drop the hook
			assert( rope->getAttachedPlayer() );
			if(!rope->getAttachedPlayer()->isUsed() || !rope->getAttachedPlayer()->getAlive()) {
				rope->hookVelocity() = CVec(0,0);
				rope->setShooting( false );
				rope->setAttached( false );
				rope->setPlayerAttached( false );
				rope->setAttachedPlayer( NULL );
			} else {
				rope->hookPos() = rope->getAttachedPlayer()->getPos();
			}
		}
	
	}



// --------------------
// ---- Bonus ---------

	void colideBonus(CBonus* bonus, int x, int y) {
		bonus->velocity() = CVec(0,0);
		bonus->pos().y = (float)y - 2;	
	}

	virtual void simulateBonus(CBonus* bonus) {
		const static float dt = 0.01f;
		
	simulateBonusStart:
		if(bonus->fLastSimulationTime + dt > tLX->fCurTime) return;
		bonus->fLastSimulationTime += dt;
		
		int x,  y;
		int mw, mh;
		int px, py;


		bonus->life() += dt;
		if(bonus->life() > BONUS_LIFETIME-3) {
			bonus->flashTime() += dt;

			if(bonus->flashTime() > 0.3f)
				bonus->flashTime() = 0;
		}

		//
		// Position & Velocity
		//
		bonus->velocity().y += 80 * dt;
		bonus->pos() += bonus->velocity() * dt;


		//
		// Check if we are hitting the ground
		//


		px = (int)bonus->pos().x;
		py = (int)bonus->pos().y;

		// Initialize
		x = px-2;
		y = py-2;

		
		mw = map->GetWidth();
		mh = map->GetHeight();

		for(y=py-2; y<=py+2; y++) {

			// Clipping
			if(y<0)
				continue;
			if(y>=mh) {
				colideBonus(bonus, x,y);
				return;
			}

			const uchar *pf = map->GetPixelFlags() + y*mw + px-2;

			for(x=px-2; x<=px+2; x++) {

				// Clipping
				if(x<0) {
					pf++;
					continue;
				}
				if(x>=mw) {
					colideBonus(bonus, x,y);
					return;
				}

				if(*pf & PX_DIRT || *pf & PX_ROCK) {
					colideBonus(bonus, x,y);
					return;
				}

				pf++;
			}
		}
		
		goto simulateBonusStart;
	}

	virtual void simulateBonuses(CBonus* bonuses, size_t count) {
		if(!tGameInfo.bBonusesOn)
			return;

		CBonus *b = bonuses;

		for(size_t i=0; i < count; i++,b++) {
			if(!b->getUsed())
				continue;

			simulateBonus(b);
		}		
	}
	
};

PhysicsEngine* CreatePhysicsEngineLX56() {
	return new PhysicsLX56();
}
