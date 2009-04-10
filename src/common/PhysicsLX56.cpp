/*
	OpenLieroX

	the LX56 physics

	code under LGPL
	created on 9/2/2008
*/

/*
 some references:
 
 LX56 projectile simulation:
 http://openlierox.svn.sourceforge.net/viewvc/openlierox/src/client/CClient_Game.cpp?revision=1&view=markup&pathrev=1
 
 
 */


#include "LieroX.h"
#include "ProfileSystem.h"
#include "Physics.h"
#include "CGameScript.h"
#include "Frame.h"
#include "MathLib.h"
#include "CWorm.h"
#include "Entity.h"
#include "CBonus.h"
#include "CClient.h"
#include "console.h"
#include "Debug.h"
#include "CServer.h"
#include "FlagInfo.h"
#include "ProjectileDesc.h"
#include "WeaponDesc.h"



// TODO: clean up this code!



class PhysicsLX56 : public PhysicsEngine {
public:
	bool m_inited;

// ---------
	PhysicsLX56() : m_inited(false) {}
	virtual ~PhysicsLX56() { uninitGame(); }

	virtual std::string name() { return "LX56 physics"; }

	virtual void initGame() { m_inited = true; }
	virtual void uninitGame() { m_inited = false; }
	virtual bool isInitialised() { return m_inited; }


// -----------------------------
// ------ worm -----------------

	// Check collisions with the level
	// HINT: it directly manipulates vPos!
	bool moveAndCheckWormCollision(AbsTime currentTime, float dt, CWorm* worm, CVec pos, CVec *vel, CVec vOldPos, int jump ) {
		static const int maxspeed2 = 10; // this should not be too high as we could run out of the cClient->getMap() without checking else

		// Can happen when starting a game
		if (!cClient->getMap())
			return false;

		// check if the vel is really too high (or infinity), in this case just ignore
		if( (*vel*dt*worm->speedFactor()).GetLength2() > (float)cClient->getMap()->GetWidth() * (float)cClient->getMap()->GetHeight() )
			return true;

		// If the worm is going too fast, divide the speed by 2 and perform 2 collision checks
		// TODO: is this still needed? we call this function with a fixed dt
		// though perhaps it is as with higher speed the way we have to check is longer
		if( (*vel * dt * worm->speedFactor()).GetLength2() > maxspeed2 && dt > 0.001f ) {
			dt /= 2;
			if(moveAndCheckWormCollision(currentTime, dt,worm,pos,vel,vOldPos,jump)) return true;
			return moveAndCheckWormCollision(currentTime, dt,worm,worm->getPos(),vel,vOldPos,jump);
		}

		pos += *vel * dt * worm->speedFactor();
		worm->pos() = pos;
		
			
		int x,y;
		x = (int)pos.x;
		y = (int)pos.y;
		short clip = 0; // 0x1=left, 0x2=right, 0x4=top, 0x8=bottom
		bool coll = false;
		bool check_needed = false;

		const uchar* gridflags = cClient->getMap()->getAbsoluteGridFlags();
		uint grid_w = cClient->getMap()->getGridWidth();
		uint grid_h = cClient->getMap()->getGridHeight();
		uint grid_cols = cClient->getMap()->getGridCols();
		if(y-4 < 0 || (uint)y+5 >= cClient->getMap()->GetHeight()
		|| x-3 < 0 || (uint)x+3 >= cClient->getMap()->GetWidth())
			check_needed = true; // we will check later, what to do here
		else if(grid_w < 7 || grid_h < 10 // this ensures, that this check is safe
		|| (gridflags[((y-4)/grid_h)*grid_cols + (x-3)/grid_w] & (PX_ROCK|PX_DIRT))
		|| (gridflags[((y+5)/grid_h)*grid_cols + (x-3)/grid_w] & (PX_ROCK|PX_DIRT))
		|| (gridflags[((y-4)/grid_h)*grid_cols + (x+3)/grid_w] & (PX_ROCK|PX_DIRT))
		|| (gridflags[((y+5)/grid_h)*grid_cols + (x+3)/grid_w] & (PX_ROCK|PX_DIRT)))
			check_needed = true;

		if(check_needed && y >= 0 && (uint)y < cClient->getMap()->GetHeight()) {
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
				if(pos.x+x >= cClient->getMap()->GetWidth()) {
					worm->pos().x=( (float)cClient->getMap()->GetWidth() - 5 );
					coll = true;
					clip |= 0x02;
					if(fabs(vel->x) > 40)
						vel->x *= -0.4f;
					else
						vel->x=(0);
					continue;
				}


				if(!(cClient->getMap()->GetPixelFlag((int)pos.x+x,y) & PX_EMPTY)) {
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

		if(check_needed && x >= 0 && (uint)x < cClient->getMap()->GetWidth()) {
			for(y=5;y>-5;y--) {
				// Optimize: pixelflag + Width

				// Top side clipping
				if(pos.y+y <= 1) {
					worm->pos().y=( 6 );
					coll = true;
					clip |= 0x04;
					if(fabs(vel->y) > 40)
						vel->y *= -0.4f;
					else
						vel->y = (0);
					continue;
				}

				// Bottom side clipping
				if(pos.y+y >= cClient->getMap()->GetHeight()) {
					worm->pos().y=( (float)cClient->getMap()->GetHeight() - 5 );
					clip |= 0x08;
					coll = true;
					worm->setOnGround( true );
					if(fabs(vel->y) > 40)
						vel->y *= -0.4f;
					else
						vel->y=(0);
					continue;
				}


				if(!(cClient->getMap()->GetPixelFlag(x,(int)pos.y+y) & PX_EMPTY)) {
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
			if (!worm->getWormState()->bJump)  // HINT: this is almost exact as old LX
				worm->pos().x = vOldPos.x;
		}

		// If we collided with the ground and we were going pretty fast, make a bump sound
		// Also save the collision time and velocity
		if(coll) {
			if( fabs(vel->x) > 30 && (clip & 0x01 || clip & 0x02) )
				StartSound( sfxGame.smpBump, worm->pos(), worm->getLocal(), -1, worm );
			else if( fabs(vel->y) > 30 && (clip & 0x04 || clip & 0x08) )
				StartSound( sfxGame.smpBump, worm->pos(), worm->getLocal(), -1, worm );

			// Set the collision information
			if (!worm->hasCollidedLastFrame())  {
				worm->setCollisionTime(currentTime);
				worm->setCollisionVel(*worm->getVelocity());
				worm->setCollidedLastFrame(true);
			}
		} else
			worm->setCollidedLastFrame(false);

		return coll;
	}


	virtual void simulateWorm(CWorm* worm, CWorm* worms, bool local, AbsTime simulationTime) {
		const float orig_dt = 0.01f;
		const float dt = orig_dt * (float)cClient->getGameLobby()->features[FT_GameSpeed];
		if(worm->fLastSimulationTime + orig_dt > simulationTime) return;

		// TODO: Later, we should have a message bus for input-events which is filled
		// by goleft/goright/stopleft/stopright/shoot/etc signals. These signals are handled in here.
		// Though the key/mouse event handling (what CWorm::getInput() is doing atm)
		// is then handled in the InputEngine and not called from here because
		// the PhysicEngine should not care about when the key/mouse events are handled.

		// TODO: count the amount of shootings
		// The problem is that we eventually count one shoot-press twice or also the
		// other way around, twice shoot-presses only once.
		// To solve this in a clean way, we really need this message bus. A dirty way
		// to solve it would be to send wormstate-updates more often if needed
		// with some checks here. (In the end, also the clean way would result in more
		// worm updates.)

		// get input max once a frame (and not at all if we don't simulate this frame)

			/*
				Only get input for this worm on certain conditions:
				1) This worm is a local worm (ie, owned by me)
				2) We're not in a game menu
				3) We're not typing a message
				4) weapons selected
			*/

		if(cClient && local && !cClient->isGameMenu() && !cClient->isChatTyping() && !cClient->isGameOver() && !Con_IsVisible() && worm->getWeaponsReady()) {
			int old_weapon = worm->getCurrentWeapon();

			worm->getInput();
			
			if (worm->isShooting() || old_weapon != worm->getCurrentWeapon())  // The weapon bar is changing
				cClient->shouldRepaintInfo() = true;
		}

		const gs_worm_t *wd = worm->getGameScript()->getWorm();
		worm_state_t *ws = worm->getWormState();


	simulateWormStart:
	
		if(worm->fLastSimulationTime + orig_dt > simulationTime) return;
		worm->fLastSimulationTime += TimeDiff(orig_dt);


		float speed;
		float	fFrameRate = 7.5f;

		// If we're seriously injured (below 15% health) and visible, bleed
		// HINT: We have to check the visibility for everybody as we don't have entities for specific teams/worms.
		// If you want to make that better, you would have to give the CViewport to simulateWorm (but that would be really stupid).
		if(worm->getHealth() < 15 && worm->isVisibleForEverybody()) {
			if(simulationTime > worm->getLastBlood() + 2.0f) {
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


		// Process the carving
		if(ws->bCarve) {
			// Calculate dir
			CVec dir;
			dir.x=( (float)cos(worm->getAngle() * (PI/180)) );
			dir.y=( (float)sin(worm->getAngle() * (PI/180)) );
			if(worm->getMoveDirection()==DIR_LEFT)
				dir.x=(-dir.x);

			worm->incrementDirtCount( CarveHole(worm->getPos() + dir*4) );
			//cClient->SendCarve(vPos + dir*4);
		}

		if(ws->bMove)
			worm->frame() += fFrameRate * dt;

		if(worm->frame() >= 3.0f || !ws->bMove)
			worm->frame() = 0.0f;
		if(worm->frame() < 0)
			worm->frame() = 2.99f;

		speed = worm->isOnGround() ? wd->GroundSpeed : wd->AirSpeed;

		// Process the ninja rope
		if(worm->getNinjaRope()->isReleased() && worms) {
			simulateNinjarope( dt, worm, worms );
			// TODO: move 'getForce' here?
			worm->velocity() += worm->getNinjaRope()->GetForce(worm->getPos()) * dt;
		}

		// Process the moving
		if(ws->bMove) {
			if(worm->getMoveDirection() == DIR_RIGHT) {
				// Right
				if(worm->getVelocity()->x < 30)
					worm->getVelocity()->x += speed * dt * 90.0f;
			} else {
				// Left
				if(worm->getVelocity()->x > -30)
					worm->getVelocity()->x -= speed * dt * 90.0f;
			}
		}


		// Process the jump
		bool onGround = worm->CheckOnGround();
		if( onGround )
			worm->setLastAirJumpTime(AbsTime());
		if(ws->bJump && ( onGround || worm->canAirJump() ||
			( bool(cClient->getGameLobby()->features[FT_RelativeAirJump]) && GetPhysicsTime() > 
				worm->getLastAirJumpTime() + float( cClient->getGameLobby()->features[FT_RelativeAirJumpDelay] ) ) )) 
		{
			if( onGround )
			{
				worm->getVelocity()->y = wd->JumpForce;
				// HINT: if we are on ground for a short time, make the jump X-velocity
				// the same as it was in the time of the collision (before the ground had dampened the worm)
				// This behavior is more like old LX
				if (simulationTime - worm->getCollisionTime() <= 0.15f)
					worm->getVelocity()->x = worm->getCollisionVel().x * 0.8f; // Dampen only a bit
			}
			else
			{
				// GFX effect, as in TeeWorlds (we'll change velocity after that)
				SpawnEntity(ENT_SPARKLE, 10, worm->getPos() + CVec( 0, 4 ), worm->velocity() + CVec( 0, 40 ), 0, NULL );
				SpawnEntity(ENT_SPARKLE, 10, worm->getPos() + CVec( 2, 4 ), worm->velocity() + CVec( 20, 40 ), 0, NULL );
				SpawnEntity(ENT_SPARKLE, 10, worm->getPos() + CVec( -2, 4 ), worm->velocity() + CVec( -20, 40 ), 0, NULL );

				if( worm->canAirJump() && worm->getVelocity()->y > wd->JumpForce ) // Negative Y coord = moving up
					worm->getVelocity()->y = wd->JumpForce; // Absolute velocity - instant air jump
				else
					worm->getVelocity()->y += wd->JumpForce; // Relative velocity - relative air jump
			}
			worm->setLastAirJumpTime(GetPhysicsTime());
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
		moveAndCheckWormCollision( simulationTime, dt, worm, worm->getPos(), worm->getVelocity(), worm->getPos(), ws->bJump );


		// Ultimate in friction
		if(worm->isOnGround()) {
			worm->velocity().x *= 0.9f;

			//vVelocity = vVelocity * CVec(/*wd->GroundFriction*/ 0.9f,1);        // Hack until new game script is done

			// Too slow, just stop
			if(fabs(worm->getVelocity()->x) < 5 && !ws->bMove)
				worm->getVelocity()->x = 0;
		}

		simulateWormWeapon(TimeDiff(dt), worm);


		// Fill in the info for sending
		if(local) {
			ws->iAngle = (int)worm->getAngle();
			ws->iDirection = worm->getDirection();
			ws->iX = (int)worm->getPos().x;
			ws->iY = (int)worm->getPos().y;
		}

		goto simulateWormStart;
	}

	virtual void simulateWormWeapon(TimeDiff dt, CWorm* worm) {
		// Weird
		if (worm->getCurrentWeapon() < 0 || worm->getCurrentWeapon() >= 5) {
			warnings("WARNING: SimulateWeapon: iCurrentWeapon is bad\n");
			return;
		}

		wpnslot_t *Slot = worm->getWeapon(worm->getCurrentWeapon());

		if(!Slot->Weapon) return;

		if(Slot->LastFire > 0)
			Slot->LastFire -= dt.seconds();

		if(Slot->Reloading) {

			if(worm->getLoadingTime() == 0)
				Slot->Charge = 1;
			else
				Slot->Charge += fabs((float)dt.seconds()) * (Slot->Weapon->Recharge * (1.0f/worm->getLoadingTime()));

			if(Slot->Charge >= 1) {
				Slot->Charge = 1;
				Slot->Reloading = false;
			}
		}
	}

	ProjCollisionType simulateProjectile_LowLevel(AbsTime currentTime, float dt, CProjectile* proj, CWorm *worms, bool* projspawn, bool* deleteAfter) {
		// If this is a remote projectile, we have already set the correct fLastSimulationTime
		//proj->setRemote( false );

		// Check for collisions
		// ATENTION: dt will manipulated directly here!
		// TODO: use a more general CheckCollision here
		ProjCollisionType res = proj->SimulateFrame(dt, cClient->getMap(), worms, &dt);


		// HINT: in original LX, we have this simulate code with lower dt
		//		(this is now the work of CheckCollision)
		//		but we also do this everytime without checks for a collision
		//		so this new code should behave like the original LX on high FPS
		// PERHAPS: do this before correction of dt; this is perhaps more like
		//		the original behavior (with lower FPS)
		//		(or leave it this way, if it works)
		proj->life() += dt;
		proj->extra() += dt;



	/*
		vOldPos = vPosition;
	*/

		const proj_t *pi = proj->GetProjInfo();

		if(pi->Rotating)  {
			proj->rotation() += (float)pi->RotSpeed * dt;
			FMOD(proj->rotation(), 360.0f);
		}

		// Animation
		if(pi->Animating) {
			if(proj->getFrameDelta())
				proj->frame() += (float)pi->AnimRate * dt;
			else
				proj->frame() -= (float)pi->AnimRate * dt;

			if(pi->bmpImage) {
				int NumFrames = pi->bmpImage->w / pi->bmpImage->h;
				if(proj->frame() >= NumFrames) {
					switch(pi->AnimType) {
					case ANI_ONCE:
						*deleteAfter = true;
						break;
					case ANI_LOOP:
						proj->frame() = 0;
						break;
					case ANI_PINGPONG:
						proj->setFrameDelta( ! proj->getFrameDelta() );
						proj->frame() = (float)NumFrames - 1;
						break;
					case __ANI_LBOUND: case __ANI_UBOUND: errors << "simulateProjectile_LowLevel: hit __ANI_BOUND" << endl;
					}
				}
				else if(proj->frame() < 0) {
					if(proj->getProjInfo()->AnimType == ANI_PINGPONG) {
						proj->setFrameDelta( ! proj->getFrameDelta() );
						proj->frame() = 0.0f;
					}
				}
			}
		}

		// Trails
		switch(pi->Trail.Type) {
		case TRL_NONE: break;
		case TRL_SMOKE:
			if(proj->extra() >= 0.075f) {
				proj->extra() = 0.0f;
				SpawnEntity(ENT_SMOKE,0,proj->GetPosition(),CVec(0,0),0,NULL);
			}
			break;
		case TRL_CHEMSMOKE:
			if(proj->extra() >= 0.075f) {
				proj->extra() = 0.0;
				SpawnEntity(ENT_CHEMSMOKE,0,proj->GetPosition(),CVec(0,0),0,NULL);
			}
			break;
		case TRL_DOOMSDAY:
			if(proj->extra() >= 0.05f) {
				proj->extra() = 0.0;
				SpawnEntity(ENT_DOOMSDAY,0,proj->GetPosition(),proj->GetVelocity(),0,NULL);
			}
			break;
		case TRL_EXPLOSIVE:
			if(proj->extra() >= 0.05f) {
				proj->extra() = 0.0;
				SpawnEntity(ENT_EXPLOSION,10,proj->GetPosition(),CVec(0,0),0,NULL);
			}
			break;
		case TRL_PROJECTILE: // Projectile trail
			if(currentTime > proj->lastTrailProj()) {
				proj->lastTrailProj() = currentTime + pi->Trail.Delay / (float)cClient->getGameLobby()->features[FT_GameSpeed];

				*projspawn = true;
			}
			break;
		case __TRL_LBOUND: case __TRL_UBOUND: errors << "simulateProjectile_LowLevel: hit __TRL_BOUND" << endl;
		}
		return res;
	}


	void simulateProjectile(const AbsTime currentTime, CProjectile* const prj) {
		const TimeDiff orig_dt = TimeDiff(0.01f);
		const TimeDiff dt = orig_dt * (float)cClient->getGameLobby()->features[FT_GameSpeed];

		// TODO: all the event-handling in here (the game logic) should be moved, it does not belong to physics

	simulateProjectileStart:
		if(prj->fLastSimulationTime + orig_dt > currentTime) return;
		prj->fLastSimulationTime += orig_dt;

		Proj_DoActionInfo doActionInfo;
		const proj_t *pi = prj->GetProjInfo();


		// Check if the timer is up
		float f = prj->getTimeVarRandom();
		if(pi->Timer.Time > 0 && (pi->Timer.Time + pi->Timer.TimeVar * f) < prj->getLife()) {
			// HINT: all the following actions will delete this projectile after

			// Run the end timer function
			switch (pi->Timer.Type) {
				case PJ_NOTHING: break;
					
				case PJ_BOUNCE:
				case PJ_INJURE:
				case PJ_DISAPPEAR:
					// TODO: do anything?
					break;

				// Explode
				case PJ_EXPLODE:
					doActionInfo.explode = true;
					doActionInfo.timer = true;
	
					if(pi->Timer.Projectiles)
						doActionInfo.spawnprojectiles = true;
					if(pi->Timer.Shake > doActionInfo.shake)
						doActionInfo.shake = pi->Timer.Shake;
					break;
	
				// Create some dirt
				case PJ_DIRT:
					doActionInfo.dirt = true;
					if(pi->Timer.Projectiles)
						doActionInfo.spawnprojectiles = true;
					if(pi->Timer.Shake > doActionInfo.shake)
						doActionInfo.shake = pi->Timer.Shake;
					break;
	
				// Create some green dirt
				case PJ_GREENDIRT:
					doActionInfo.grndirt = true;
					if(pi->Timer.Projectiles)
						doActionInfo.spawnprojectiles = true;
					if(pi->Timer.Shake > doActionInfo.shake)
						doActionInfo.shake = pi->Timer.Shake;
					break;
	
				// Carve
				case PJ_CARVE:  {
					int d = cClient->getMap()->CarveHole( pi->Timer.Damage, prj->GetPosition() );
					doActionInfo.deleteAfter = true;
	
					if(pi->Timer.Projectiles)
						doActionInfo.spawnprojectiles = true;
	
					// Increment the dirt count
					if(prj->hasOwner())
						cClient->getRemoteWorms()[prj->GetOwner()].incrementDirtCount( d );
					break;
				}
					
				case __PJ_LBOUND: case __PJ_UBOUND: errors << "simulateProjectile: hit __PJ_BOUND" << endl;
			}
			
			if(doActionInfo.spawnprojectiles)
				doActionInfo.spawnInfo = &pi->Timer.Proj;
		}

		// Simulate the projectile
		ProjCollisionType result = simulateProjectile_LowLevel( prj->fLastSimulationTime, dt.seconds(), prj, cClient->getRemoteWorms(), &doActionInfo.trailprojspawn, &doActionInfo.deleteAfter );

		/*
		===================
		Terrain Collision
		===================
		*/
		if( !result.withWorm && (result.colMask & PJC_TERRAIN) ) {			
			pi->Hit.applyTo(&result, dt, prj, &doActionInfo);
		}

		/*
		===================
		Worm Collision
		===================
		*/
		if( result.withWorm && !doActionInfo.explode) {
			bool preventSelfShooting = ((int)result.wormId == prj->GetOwner());
			preventSelfShooting &= (prj->getIgnoreWormCollBeforeTime() > prj->fLastSimulationTime); // if the simulation is too early, ignore this worm col
			if( !preventSelfShooting || NewNet::Active() ) {
				pi->PlyHit.applyTo(&result, dt, prj, &doActionInfo);
			}
		}

		doActionInfo.execute(prj, currentTime);
		if(doActionInfo.deleteAfter) return;
		
		goto simulateProjectileStart;
	}

	virtual void simulateProjectiles(Iterator<CProjectile*>::Ref projs, AbsTime currentTime) {
		const float orig_dt = 0.01f;

		// TODO: all the event-handling in here (the game logic) should be moved, it does not belong to physics

	simulateProjectilesStart:
		if(cClient->fLastSimulationTime + orig_dt > currentTime) return;

		for(Iterator<CProjectile*>::Ref i = projs; i->isValid(); i->next()) {
			CProjectile* p = i->get();
			simulateProjectile( cClient->fLastSimulationTime, p );
		}

		cClient->fLastSimulationTime += TimeDiff(orig_dt);
		goto simulateProjectilesStart;
	}

	void simulateNinjarope(float dt, CWorm* owner, CWorm *worms) {
		CNinjaRope* rope = owner->getNinjaRope();
		CVec playerpos = owner->getPos();

		rope->updateOldHookPos();

		if(!rope->isReleased())
			return;

		float length2;
		bool firsthit = !rope->isAttached();
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
		}

		bool outsideMap = false;

		// Hack to see if the hook went out of the cClient->getMap()
		if(!rope->isPlayerAttached())
		if(
				rope->hookPos().x <= 0 || rope->hookPos().y <= 0 ||
				rope->hookPos().x >= cClient->getMap()->GetWidth()-1 ||
				rope->hookPos().y >= cClient->getMap()->GetHeight()-1) {
			rope->setShooting( false );
			rope->setAttached( true );
			rope->setPlayerAttached( false );

			// Make the hook stay at an edge
			rope->hookPos().x = ( MAX((float)0, rope->hookPos().x) );
			rope->hookPos().y = ( MAX((float)0, rope->hookPos().y) );

			rope->hookPos().x = ( MIN(cClient->getMap()->GetWidth()-(float)1, rope->hookPos().x) );
			rope->hookPos().y = ( MIN(cClient->getMap()->GetHeight()-(float)1, rope->hookPos().y) );

			outsideMap = true;
		}


		// Check if the hook has hit anything on the cClient->getMap()
		if(!rope->isPlayerAttached()) {
			rope->setAttached( false );

			LOCK_OR_QUIT(cClient->getMap()->GetImage());
			uchar px = outsideMap ? PX_ROCK : cClient->getMap()->GetPixelFlag((int)rope->hookPos().x, (int)rope->hookPos().y);
			if((px & PX_ROCK || px & PX_DIRT || outsideMap)) {
				rope->setShooting( false );
				rope->setAttached( true );
				rope->setPlayerAttached( false );
				rope->hookVelocity() = CVec(0,0);

				if((px & PX_DIRT) && firsthit) {
					Uint32 col = GetPixel(cClient->getMap()->GetImage().get(), (int)rope->hookPos().x, (int)rope->hookPos().y);
					for( short i=0; i<5; i++ )
						SpawnEntity(ENT_PARTICLE,0, rope->hookPos() + CVec(0,2), CVec(GetRandomNum()*40,GetRandomNum()*40),col,NULL);
				}
			}
			UnlockSurface(cClient->getMap()->GetImage());
		}

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
				if(!worms[i].isVisible(owner))
					continue;

				if( ( worms[i].getPos() - rope->hookPos() ).GetLength2() < 25 ) {
					rope->setShooting( false );
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

			// If the worm has been killed, or dropped, or became invisible in H&S - drop the hook
			if(	!rope->getAttachedPlayer() || 
				!rope->getAttachedPlayer()->isUsed() || 
				!rope->getAttachedPlayer()->getAlive() ||
				!rope->getAttachedPlayer()->isVisible(owner) ) 
			{
				if(!rope->getAttachedPlayer())
					warnings("WARNING: the rope is attached to a non-existant player!\n");						
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

	void simulateBonus(CBonus* bonus) {
		const float orig_dt = 0.01f;
		const float dt = orig_dt * (float)cClient->getGameLobby()->features[FT_GameSpeed];

	simulateBonusStart:
		if(bonus->fLastSimulationTime + orig_dt > tLX->currentTime) return;
		bonus->fLastSimulationTime += TimeDiff(orig_dt);

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


		mw = cClient->getMap()->GetWidth();
		mh = cClient->getMap()->GetHeight();

		for(y=py-2; y<=py+2; y++) {

			// Clipping
			if(y<0)
				continue;
			if(y>=mh) {
				colideBonus(bonus, x,y);
				return;
			}

			const uchar *pf = cClient->getMap()->GetPixelFlags() + y*mw + px-2;

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
		if(!cClient->getGameLobby()->bBonusesOn)
			return;

		if(!cClient->getMap()) return;

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
