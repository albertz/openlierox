/*
	OpenLieroX
	
	the LX56 physics

	code under LGPL
	created on 9/2/2008
*/

#include "Physics.h"

// TODO: perhaps split this file into the different simulations?

#include "CGameScript.h"
#include "Frame.h"
#include "MathLib.h"
#include "CWorm.h"
#include "Entity.h"
#include "CBonus.h"


class PhysicsLX56 : public PhysicsEngine {
	virtual ~PhysicsLX56() {}
	
	virtual std::string name() { return "LX56 physics"; }

// -----------------------------
// ------ worm -----------------

	virtual void simulateWorm(float dt, CWorm* worm, CWorm *worms, int local) {
		// If the delta time is too big, divide it and run the simulation twice
		if( dt > 0.25f ) {
			dt /= 2;
			simulateWorm(dt, worm, worms,local);
			simulateWorm(dt, worm, worms,local);
			return;
		}

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
				worm->setLastSparkle( tLX->fCurTime );
				CVec p = worm->getPos() + CVec(GetRandomNum()*3, GetRandomNum()*3);

				SpawnEntity(ENT_SPARKLE,0,p, CVec(0,0), 0,NULL);
			}
		}

		// If we're seriously injured (below 15% health) bleed
		if(worm->getHealth() < 15) {
			if(tLX->fCurTime - worm->getLastBlood() > 2) {
				worm->setLastBlood( tLX->fCurTime );

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
			simulateNinjarope( dt, worm->getNinjaRope(), worm->getMap(), worm->getPos(), worms, worm->getID() );
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
		worm->MoveAndCheckWormCollision( dt, worm->getPos(), worm->getVelocity(), worm->getPos(), ws->iJump );


		// Ultimate in friction
		if(worm->isOnGround()) {
	//		vVelocity.x *= pow(0.9f, dt * 100.0f); // wrong

			// TODO: if you manage to make it this way but exponential, it will be perfect :)
			/*float old_x = vVelocity.x;
			vVelocity.x -= SIGN(vVelocity.x) * 0.9f * (dt * 100.0f); // TODO: this is the bug described in
			if (SIGN(old_x) != SIGN(vVelocity.x))  // Make sure we don't dampen to opposite direction
				vVelocity.x = 0.0f;*/

			// Even though this is highly FPS dependent, it seems to work quite good (this is how old lx did it)
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
	
	virtual void simulateProjectile(float dt, CProjectile* proj, CMap *map, CWorm *worms, int *wormid, int* result) {
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
		// TODO: move CheckCollision in here
		int colret = proj->CheckCollision(dt, map, worms, &dt);
		if(colret == -1)
			res |= PJC_TERRAIN;
		else if(colret >= 0) {
			*wormid = colret;
			res |= PJC_WORM;
		}
		
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

		*result = res;
		//return res;
	
	}
	
	virtual void simulateNinjarope(float dt, CNinjaRope* rope, CMap *map, CVec playerpos, CWorm *worms, int owner) {
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
		if((rope->getHookVel() + force*dt).GetLength2() * dt * dt > 5) {
			simulateNinjarope( dt/2, rope, map, playerpos, worms, owner );
			simulateNinjarope( dt/2, rope, map, playerpos, worms, owner );
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
				if(worms[i].getID() == owner)
					continue;
				if(worms[i].getFlag())
					continue;

				if( ( worms[i].getPos() - rope->hookPos() ).GetLength2() < 25 ) {
					rope->setAttached( true );
					rope->setPlayerAttached( true );
					rope->setAttachedPlayer( &worms[i] );
					rope->getAttachedPlayer()->setHooked(true, &worms[owner]);
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

	virtual void simulateBonus(float dt, CBonus* bonus, CMap* map) {
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
		
	}

	
};

PhysicsEngine* CreatePhysicsEngineLX56() {
	return new PhysicsLX56();
}
