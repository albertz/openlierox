/*
	OpenLieroX

	the LX56 physics

	code under LGPL
	created on 9/2/2008
*/

#include <iostream>

#include "LieroX.h"

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

using std::cout; using std::endl;


class PhysicsLX56 : public PhysicsEngine {
public:
	CMap* m_map;
	CClient* m_client;
	bool m_inited;

// ---------
	PhysicsLX56() : m_map(NULL), m_inited(false) {}
	virtual ~PhysicsLX56() { uninitGame(); }

	virtual std::string name() { return "LX56 physics"; }

	virtual void initGame( CMap* m, CClient* c ) { m_map = m; m_client = c; m_inited = true; }
	virtual void uninitGame() { m_map = NULL; m_client = NULL; m_inited = false; }
	virtual bool isInitialised() { return m_inited; }


// -----------------------------
// ------ worm -----------------

	// Check collisions with the level
	// HINT: it directly manipulates vPos!
	bool moveAndCheckWormCollision(float dt, CWorm* worm, CVec pos, CVec *vel, CVec vOldPos, int jump ) {
		static const int maxspeed2 = 10; // this should not be too high as we could run out of the m_map without checking else

		// Can happen when starting a game
		if (!m_map)
			return false;

		// check if the vel is really too high (or infinity), in this case just ignore
		if( (*vel*dt).GetLength2() > (float)m_map->GetWidth() * (float)m_map->GetHeight() )
			return true;

		// If the worm is going too fast, divide the speed by 2 and perform 2 collision checks
		// TODO: is this still needed? we call this function with a fixed dt
		// though perhaps it is as with higher speed the way we have to check is longer
		if( (*vel*dt).GetLength2() > maxspeed2 && dt > 0.001f ) {
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

		const uchar* gridflags = m_map->getAbsoluteGridFlags();
		uint grid_w = m_map->getGridWidth();
		uint grid_h = m_map->getGridHeight();
		uint grid_cols = m_map->getGridCols();
		if(y-4 < 0 || (uint)y+5 >= m_map->GetHeight()
		|| x-3 < 0 || (uint)x+3 >= m_map->GetWidth())
			check_needed = true; // we will check later, what to do here
		else if(grid_w < 7 || grid_h < 10 // this ensures, that this check is safe
		|| (gridflags[((y-4)/grid_h)*grid_cols + (x-3)/grid_w] & (PX_ROCK|PX_DIRT))
		|| (gridflags[((y+5)/grid_h)*grid_cols + (x-3)/grid_w] & (PX_ROCK|PX_DIRT))
		|| (gridflags[((y-4)/grid_h)*grid_cols + (x+3)/grid_w] & (PX_ROCK|PX_DIRT))
		|| (gridflags[((y+5)/grid_h)*grid_cols + (x+3)/grid_w] & (PX_ROCK|PX_DIRT)))
			check_needed = true;

		if(check_needed && y >= 0 && (uint)y < m_map->GetHeight()) {
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
				if(pos.x+x >= m_map->GetWidth()) {
					worm->pos().x=( (float)m_map->GetWidth() - 5 );
					coll = true;
					clip |= 0x02;
					if(fabs(vel->x) > 40)
						vel->x *= -0.4f;
					else
						vel->x=(0);
					continue;
				}


				if(!(m_map->GetPixelFlag((int)pos.x+x,y) & PX_EMPTY)) {
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

		if(check_needed && x >= 0 && (uint)x < m_map->GetWidth()) {
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
				if(pos.y+y >= m_map->GetHeight()) {
					worm->pos().y=( (float)m_map->GetHeight() - 5 );
					clip |= 0x08;
					coll = true;
					worm->setOnGround( true );
					if(fabs(vel->y) > 40)
						vel->y *= -0.4f;
					else
						vel->y=(0);
					continue;
				}


				if(!(m_map->GetPixelFlag(x,(int)pos.y+y) & PX_EMPTY)) {
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
		if(coll) {
			if( fabs(vel->x) > 30 && (clip & 0x01 || clip & 0x02) )
				StartSound( sfxGame.smpBump, worm->pos(), worm->getLocal(), -1, worm );
			else if( fabs(vel->y) > 30 && (clip & 0x04 || clip & 0x08) )
				StartSound( sfxGame.smpBump, worm->pos(), worm->getLocal(), -1, worm );
		}

		return coll;
	}


	virtual void simulateWorm(CWorm* worm, CWorm* worms, bool local) {
		const float orig_dt = 0.01f;
		const float dt = orig_dt * tGameInfo.fGameSpeed;		
		if(worm->fLastSimulationTime + orig_dt > tLX->fCurTime) return;

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

		if(m_client && local && !m_client->isGameMenu() && !m_client->isChatTyping() && !m_client->isGameOver() && !Con_IsUsed() && worm->getWeaponsReady()) {
			int old_weapon = worm->getCurrentWeapon();

			// TODO: use one getInput for both
			if(worm->getType() == PRF_HUMAN)
				worm->getInput();
			else
				// TODO: why is the first parameter not enough?
				worm->AI_GetInput(m_client->getGameType(),
					m_client->getGameType() == GMT_TEAMDEATH,
					m_client->getGameType() == GMT_TAG,
					m_client->getGameType() == GMT_VIP,
					m_client->getGameType() == GMT_CTF,
					m_client->getGameType() == GMT_TEAMCTF);

			if (worm->isShooting() || old_weapon != worm->getCurrentWeapon())  // The weapon bar is changing
				m_client->shouldRepaintInfo() = true;
		}

		const gs_worm_t *wd = worm->getGameScript()->getWorm();
		worm_state_t *ws = worm->getWormState();


	simulateWormStart:
	
		if(worm->fLastSimulationTime + orig_dt > tLX->fCurTime) return;
		worm->fLastSimulationTime += orig_dt;


		float speed;
		float	fFrameRate = 7.5f;

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


		// Process the carving
		if(ws->bCarve) {
			// Calculate dir
			CVec dir;
			dir.x=( (float)cos(worm->getAngle() * (PI/180)) );
			dir.y=( (float)sin(worm->getAngle() * (PI/180)) );
			if(worm->getMoveDirection()==DIR_LEFT)
				dir.x=(-dir.x);

			worm->incrementDirtCount( CarveHole(worm->getMap(), worm->getPos() + dir*4) );
			//cClient->SendCarve(vPos + dir*4);
		}

		if(ws->bMove)
			worm->frame() += fFrameRate * dt;

		if(worm->frame() >= 3.0f || !ws->bMove)
			worm->frame() = (0);

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
		if(ws->bJump && worm->CheckOnGround()) {
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
		moveAndCheckWormCollision( dt, worm, worm->getPos(), worm->getVelocity(), worm->getPos(), ws->bJump );


		// Ultimate in friction
		if(worm->isOnGround()) {
			worm->velocity().x *= 0.9f;

			//vVelocity = vVelocity * CVec(/*wd->GroundFriction*/ 0.9f,1);        // Hack until new game script is done

			// Too slow, just stop
			if(fabs(worm->getVelocity()->x) < 5 && !ws->bMove)
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

		if(!Slot->Weapon) return;

		if(Slot->LastFire > 0)
			Slot->LastFire -= dt;

		if(Slot->Reloading) {

			if(worm->getLoadingTime() == 0)
				Slot->Charge = 1;
			else
				Slot->Charge += dt * (Slot->Weapon->Recharge * (1.0f/worm->getLoadingTime()));

			if(Slot->Charge >= 1) {
				Slot->Charge = 1;
				Slot->Reloading = false;
			}
		}
	}

	int simulateProjectile_LowLevel(float fCurTime, float dt, CProjectile* proj, CWorm *worms, int *wormid, bool* projspawn, bool* deleteAfter) {
		int res = PJC_NONE;

		// If this is a remote projectile, we have already set the correct fLastSimulationTime
		//proj->setRemote( false );

		// Check for collisions
		// ATENTION: dt will manipulated directly here!
		// TODO: use a more general CheckCollision here
		int colret = proj->CheckCollision(dt, m_map, worms, &dt);
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
		if( proj->explode() && fCurTime > proj->explodeTime()) {
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
		switch(pi->Trail) {
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
			if(fCurTime > proj->lastTrailProj()) {
				proj->lastTrailProj() = fCurTime + pi->PrjTrl_Delay / tGameInfo.fGameSpeed;

				*projspawn = true;
			}
		}

		return res;
	}

	void projectile_doExplode(CProjectile* const prj, int shake) {
		const proj_t *pi = prj->GetProjInfo();
		// Explosion
		int damage = pi->Hit_Damage;
		if(pi->PlyHit_Type == PJ_EXPLODE)
			damage = pi->PlyHit_Damage;

		if(damage != -1)
			m_client->Explosion(prj->GetPosition(), damage, shake, prj->GetOwner());
	}

	void projectile_doTimerExplode(CProjectile* const prj, int shake) {
		const proj_t *pi = prj->GetProjInfo();
		// Explosion
		int damage = pi->Timer_Damage;
		if(pi->PlyHit_Type == PJ_EXPLODE)
			damage = pi->PlyHit_Damage;

		if(damage != -1)
			m_client->Explosion(prj->GetPosition(), damage, shake, prj->GetOwner());
	}

	void projectile_doProjSpawn(CProjectile* const prj, float fSpawnTime) {
		const proj_t *pi = prj->GetProjInfo();

		CVec sprd;
		if(pi->PrjTrl_UsePrjVelocity) {
			sprd = prj->GetVelocity();
			float l = NormalizeVector(&sprd);
			sprd *= (l*0.3f);		// Slow it down a bit.
									// It can be sped up by the speed variable in the script
		}

		for(int i=0; i < pi->PrjTrl_Amount; i++) {
			if(!pi->PrjTrl_UsePrjVelocity)
				GetAngles((int)((float)pi->PrjTrl_Spread * prj->getRandomFloat()),&sprd,NULL);

			CVec v = sprd*(float)pi->PrjTrl_Speed + CVec(1,1)*(float)pi->PrjTrl_SpeedVar*prj->getRandomFloat();

			m_client->SpawnProjectile(prj->GetPosition(), v, 0, prj->GetOwner(), pi->PrjTrl_Proj, prj->getRandomIndex()+1, fSpawnTime, prj->getIgnoreWormCollBeforeTime());
		}
	}

	void projectile_doSpawnOthers(CProjectile* const prj, float fSpawnTime) {
		const proj_t *pi = prj->GetProjInfo();
		CVec v = prj->GetVelocity();
		NormalizeVector(&v);

		// Calculate the angle of the direction the projectile is heading
		float heading = 0;
		if(pi->ProjUseangle) {
			heading = (float)( -atan2(v.x,v.y) * (180.0f/PI) );
			heading+=90;
			FMOD(heading, 360.0f);
		}

		CVec sprd;
		for(int i=0;i<pi->ProjAmount;i++) {
			int a = (int)( (float)pi->ProjAngle + heading + prj->getRandomFloat()*(float)pi->ProjSpread );
			GetAngles(a,&sprd,NULL);

			float speed = (float)pi->ProjSpeed + (float)pi->ProjSpeedVar * prj->getRandomFloat();

			m_client->SpawnProjectile(prj->GetPosition(), sprd*speed, 0, prj->GetOwner(), pi->Projectile, prj->getRandomIndex()+1, fSpawnTime, prj->getIgnoreWormCollBeforeTime());
		}
	}

	void projectile_doMakeDirt(CProjectile* const prj) {
		int damage = 5;
		int d = 0;
		d += m_map->PlaceDirt(damage,prj->GetPosition()-CVec(6,6));
		d += m_map->PlaceDirt(damage,prj->GetPosition()+CVec(6,-6));
		d += m_map->PlaceDirt(damage,prj->GetPosition()+CVec(0,6));

		// Remove the dirt count on the worm
		m_client->getRemoteWorms()[prj->GetOwner()].incrementDirtCount( -d );
	}

	void projectile_doMakeGreenDirt(CProjectile* const prj) {
		int d = m_map->PlaceGreenDirt(prj->GetPosition());

		// Remove the dirt count on the worm
		m_client->getRemoteWorms()[prj->GetOwner()].incrementDirtCount( -d );
	}

	void simulateProjectile(const float fCurTime, CProjectile* const prj) {
		const float orig_dt = 0.01f;
		const float dt = orig_dt * tGameInfo.fGameSpeed;		

		// TODO: all the event-handling in here (the game logic) should be moved, it does not belong to physics

	simulateProjectileStart:
		if(prj->fLastSimulationTime + orig_dt > fCurTime) return;
		prj->fLastSimulationTime += orig_dt;


		bool explode = false;
		bool timer = false;
		int shake = 0;
		bool dirt = false;
		bool grndirt = false;
		bool deleteAfter = false;
		bool trailprojspawn = false;
		int result = 0;
		int wormid = -1;

		bool spawnprojectiles = false;
		const proj_t *pi = prj->GetProjInfo();
		float f;


		// Check if the timer is up
		f = prj->getTimeVarRandom();
		if(pi->Timer_Time > 0 && (pi->Timer_Time + pi->Timer_TimeVar * f) < prj->getLife()) {
			// HINT: all the following actions will delete this projectile after

			// Run the end timer function
			switch (pi->Timer_Type) {

			// Explode
			case PJ_EXPLODE:
				explode = true;
				timer = true;

				if(pi->Timer_Projectiles)
					spawnprojectiles = true;
				if(pi->Timer_Shake > shake)
					shake = pi->Timer_Shake;
			break;

			// Create some dirt
			case PJ_DIRT:
				dirt = true;
				if(pi->Timer_Projectiles)
					spawnprojectiles = true;
				if(pi->Timer_Shake > shake)
					shake = pi->Timer_Shake;
			break;

			// Create some green dirt
			case PJ_GREENDIRT:
				grndirt = true;
				if(pi->Timer_Projectiles)
					spawnprojectiles = true;
				if(pi->Timer_Shake > shake)
					shake = pi->Timer_Shake;
			break;

			// Carve
			case PJ_CARVE:  {
				int d = m_map->CarveHole(	pi->Timer_Damage, prj->GetPosition() );
				deleteAfter = true;

				if(pi->Timer_Projectiles)
					spawnprojectiles = true;

				// Increment the dirt count
				m_client->getRemoteWorms()[prj->GetOwner()].incrementDirtCount( d );

				m_client->CheckDemolitionsGame();
			}
			break;
			}
		}

		// Simulate the projectile
		wormid = -1;
		result = simulateProjectile_LowLevel( prj->fLastSimulationTime, dt, prj, m_client->getRemoteWorms(), &wormid, &trailprojspawn, &deleteAfter );

		/*
		===================
		Terrain Collision
		===================
		*/
		if( result & PJC_TERRAIN ) {

			// Explosion
			switch (pi->Hit_Type)  {
			case PJ_EXPLODE:
				explode = true;

				if(pi->Hit_Shake > shake)
					shake = pi->Hit_Shake;

				// Play the hit sound
				if(pi->Hit_UseSound)
					PlaySoundSample(pi->smpSample);
			break;

			// Bounce
			case PJ_BOUNCE:
				prj->Bounce(pi->Hit_BounceCoeff);

				// Do we do a bounce-explosion (bouncy larpa uses this)
				if(pi->Hit_BounceExplode > 0)
					m_client->Explosion(prj->GetPosition(), pi->Hit_BounceExplode, false, prj->GetOwner());
			break;

			// Carve
			case PJ_CARVE:  {
				int d = m_map->CarveHole(
					pi->Hit_Damage, prj->GetPosition());
				deleteAfter = true;

				// Increment the dirt count
				m_client->getRemoteWorms()[MIN(prj->GetOwner(),MAX_WORMS - 1)].incrementDirtCount( d );

				m_client->CheckDemolitionsGame();
			}
			break;

			// Dirt
			case PJ_DIRT:
				dirt = true;
			break;

			// Green Dirt
			case PJ_GREENDIRT:
				grndirt = true;
			break;
			}


			if(pi->Hit_Projectiles)
				spawnprojectiles = true;
		}



		/*
		===================
		Worm Collision
		===================
		*/
		if( (result & PJC_WORM) && wormid >= 0 && !explode) {
			bool preventSelfShooting = (wormid == prj->GetOwner());
			preventSelfShooting &= (prj->getIgnoreWormCollBeforeTime() > prj->fLastSimulationTime); // if the simulation is too early, ignore this worm col
			if( !preventSelfShooting )  {
				bool push_worm = true;

				switch (pi->PlyHit_Type)  {

				// Explode
				case PJ_EXPLODE:
					explode = true;
				break;

				// Injure
				case PJ_INJURE:

					// Add damage to the worm
					m_client->InjureWorm(&m_client->getRemoteWorms()[wormid], pi->PlyHit_Damage, prj->GetOwner());
					deleteAfter = true;
				break;

				// Bounce
				case PJ_BOUNCE:
					push_worm = false;
					prj->Bounce(pi->PlyHit_BounceCoeff);
				break;

				// Dirt
				case PJ_DIRT:
					dirt = true;
				break;

				// Green Dirt
				case PJ_GREENDIRT:
					grndirt = true;
				break;

				case PJ_NOTHING:
					push_worm = false;
				break;
				}

				// Push the worm back
				if(push_worm) {
					CVec d = prj->GetVelocity();
					NormalizeVector(&d);
					CVec *v = m_client->getRemoteWorms()[wormid].getVelocity();
					*v += (d*100)*dt;
				}

				if(pi->PlyHit_Projectiles)
					spawnprojectiles = true;
			}
		}


		// Explode?
		if(explode) {
			if(!timer)
				projectile_doExplode(prj, shake);
			else
				projectile_doTimerExplode(prj, shake);
			deleteAfter = true;
		}

		// Dirt
		if(dirt) {
			projectile_doMakeDirt(prj);
			deleteAfter = true;
		}

		// Green dirt
		if(grndirt) {
			projectile_doMakeGreenDirt(prj);
			deleteAfter = true;
		}

		if(trailprojspawn) {
			// we use prj->fLastSimulationTime here to simulate the spawing at the current simulation time of this projectile
			projectile_doProjSpawn( prj, prj->fLastSimulationTime );
		}

		// Spawn any projectiles?
		if(spawnprojectiles) {
			// we use fCurTime (= the simulation time of the m_client) to simulate the spawing at this time
			// because the spawing is caused probably by conditions of the environment like collision with worm/m_map
			projectile_doSpawnOthers(prj, fCurTime);
		}

		// HINT: delete "junk projectiles" - projectiles that have no action assigned and are therefore never destroyed
		// Some bad-written mods contain those projectiles and they make the game more and more laggy (because new and new
		// projectiles are spawned and never destroyed) and prevent more important projectiles from spawning.
		// These conditions test for those projectiles and remove them
		if (pi->Hit_Type == PJ_NOTHING && pi->PlyHit_Type == PJ_NOTHING && (pi->Timer_Type == PJ_NOTHING || pi->Timer_Time <= 0)) // Isn't destroyed by any event
			if (!pi->Animating || (pi->Animating && (pi->AnimType != ANI_ONCE || pi->bmpImage == NULL))) // Isn't destroyed after animation ends
				if (!pi->Hit_Projectiles && !pi->PlyHit_Projectiles && !pi->Timer_Projectiles)  // Doesn't spawn any projectiles
					deleteAfter = true;

		if(deleteAfter) {
			prj->setUnused();
			return;
		}

		goto simulateProjectileStart;
	}

	virtual void simulateProjectiles(Iterator<CProjectile>::Ref projs) {
		const float orig_dt = 0.01f;

		// TODO: all the event-handling in here (the game logic) should be moved, it does not belong to physics

	simulateProjectilesStart:
		if(m_client->fLastSimulationTime + orig_dt > tLX->fCurTime) return;

		for(Iterator<CProjectile>::Ref i = projs; i->isValid(); i->next()) {
			CProjectile* p = &i->get();
			simulateProjectile( m_client->fLastSimulationTime, p );
		}

		m_client->fLastSimulationTime += orig_dt;
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

		bool outsidem_map = false;

		// Hack to see if the hook went out of the m_map
		if(!rope->isPlayerAttached())
		if(
				rope->hookPos().x <= 0 || rope->hookPos().y <= 0 ||
				rope->hookPos().x >= m_map->GetWidth()-1 ||
				rope->hookPos().y >= m_map->GetHeight()-1) {
			rope->setShooting( false );
			rope->setAttached( true );
			rope->setPlayerAttached( false );

			// Make the hook stay at an edge
			rope->hookPos().x = ( MAX((float)0, rope->hookPos().x) );
			rope->hookPos().y = ( MAX((float)0, rope->hookPos().y) );

			rope->hookPos().x = ( MIN(m_map->GetWidth()-(float)1, rope->hookPos().x) );
			rope->hookPos().y = ( MIN(m_map->GetHeight()-(float)1, rope->hookPos().y) );

			outsidem_map = true;
		}


		// Check if the hook has hit anything on the m_map
		if(!rope->isPlayerAttached()) {
			rope->setAttached( false );

			LOCK_OR_QUIT(m_map->GetImage());
			uchar px = outsidem_map ? PX_ROCK : m_map->GetPixelFlag((int)rope->hookPos().x, (int)rope->hookPos().y);
			if((px & PX_ROCK || px & PX_DIRT || outsidem_map)) {
				rope->setShooting( false );
				rope->setAttached( true );
				rope->setPlayerAttached( false );
				rope->hookVelocity() = CVec(0,0);

				if((px & PX_DIRT) && firsthit) {
					Uint32 col = GetPixel(m_map->GetImage().get(), (int)rope->hookPos().x, (int)rope->hookPos().y);
					for( short i=0; i<5; i++ )
						SpawnEntity(ENT_PARTICLE,0, rope->hookPos() + CVec(0,2), CVec(GetRandomNum()*40,GetRandomNum()*40),col,NULL);
				}
			}
			UnlockSurface(m_map->GetImage());
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

			// If the worm has been killed, or dropped, drop the hook
			if(!rope->getAttachedPlayer() || !rope->getAttachedPlayer()->isUsed() || !rope->getAttachedPlayer()->getAlive()) {
				if(!rope->getAttachedPlayer())
					printf("WARNING: the rope is attached to a non-existant player!\n");						
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
		const float dt = orig_dt * tGameInfo.fGameSpeed;		

	simulateBonusStart:
		if(bonus->fLastSimulationTime + orig_dt > tLX->fCurTime) return;
		bonus->fLastSimulationTime += orig_dt;

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


		mw = m_map->GetWidth();
		mh = m_map->GetHeight();

		for(y=py-2; y<=py+2; y++) {

			// Clipping
			if(y<0)
				continue;
			if(y>=mh) {
				colideBonus(bonus, x,y);
				return;
			}

			const uchar *pf = m_map->GetPixelFlags() + y*mw + px-2;

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

		if(!m_map) return;

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
