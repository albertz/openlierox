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
#include "PhysicsLX56.h"


// defined in PhysicsLX56_Projectiles
void LX56_simulateProjectiles(Iterator<CProjectile*>::Ref projs);

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
	bool moveAndCheckWormCollision(AbsTime currentTime, float dt, CWorm* worm, CVec pos, CVec *vel, CVec vOldPos, bool jump ) {
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

		bool wrapAround = cClient->getGameLobby()->features[FT_InfiniteMap];
		CVec prevWormPos = worm->pos();
		pos += *vel * dt * worm->speedFactor();
		if(wrapAround) {
			FMOD(pos.x, (float)cClient->getMap()->GetWidth());
			FMOD(pos.y, (float)cClient->getMap()->GetHeight());
		}
		worm->pos() = pos;
		
		
		int x = (int)pos.x;
		int y = (int)pos.y;
		short clip = 0; // 0x1=left, 0x2=right, 0x4=top, 0x8=bottom
		bool coll = false;
		bool check_needed = cClient->getMap()->GetCollisionFlag(x, y, wrapAround) != 0;

		if(check_needed && y >= 0 && (uint)y < cClient->getMap()->GetHeight()) {
			for(x=-3;x<4;x++) {
				// Optimize: pixelflag++
				
				// Left side clipping
				if(!wrapAround && (pos.x+x <= 2)) {
					clip |= 0x01;
					worm->pos().x=( 5 );
					coll = true;
					if(fabs(vel->x) > 40)
						vel->x *=  -0.4f;
					else
						vel->x=(0);
					continue; // Note: This was break in LX56, but continue is really better here
				}

				// Right side clipping
				if(!wrapAround && (pos.x+x >= cClient->getMap()->GetWidth())) {
					worm->pos().x=( (float)cClient->getMap()->GetWidth() - 5 );
					coll = true;
					clip |= 0x02;
					if(fabs(vel->x) > 40)
						vel->x *= -0.4f;
					else
						vel->x=(0);
					continue; // Note: This was break in LX56, but continue is really better here
				}

				int posx = (int)pos.x + x; if(wrapAround) { posx %= (int)cClient->getMap()->GetWidth(); if(posx < 0) posx += cClient->getMap()->GetWidth(); }
				if(!(cClient->getMap()->GetPixelFlag(posx,y) & PX_EMPTY)) {
					coll = true;

					// NOTE: Be carefull that you don't do any float->int->float conversions here.
					// This has some *huge* effect. People reported it as high ceil friction.
					if(x<0) {
						clip |= 0x01;
						worm->pos().x = pos.x + x + 4;
					}
					else {
						clip |= 0x02;
						worm->pos().x = pos.x + x - 4;
					}

					// Bounce
					if(fabs(vel->x) > 40)
						vel->x *= -0.4f;
					else
						vel->x=(0);
				}
			}
		}

		// In case of this, it could be that we need to do a FMOD. Just do it to be sure.
		if(check_needed && wrapAround) {
			FMOD(pos.x, (float)cClient->getMap()->GetWidth());
		}
		
		worm->setOnGround( false );

		bool hit = false;
		x = (int)pos.x;

		if(check_needed && x >= 0 && (uint)x < cClient->getMap()->GetWidth()) {
			for(y=5;y>-5;y--) {
				// Optimize: pixelflag + Width

				// Top side clipping
				if(!wrapAround && (pos.y+y <= 1)) {
					worm->pos().y=( 6 );
					coll = true;
					clip |= 0x04;
					if(fabs(vel->y) > 40)
						vel->y *= -0.4f;
					else
						vel->y = (0);
					
					// it was also break in LX56. this makes a huge difference when bouncing against the bottom.
					break;
				}

				// Bottom side clipping
				if(!wrapAround && (pos.y+y >= cClient->getMap()->GetHeight())) {
					worm->pos().y=( (float)cClient->getMap()->GetHeight() - 5 );
					clip |= 0x08;
					coll = true;
					worm->setOnGround( true );
					if(fabs(vel->y) > 40)
						vel->y *= -0.4f;
					else
						vel->y=(0);
					continue; // Note: This was break in LX56, but continue is really better here
				}

				int posy = (int)pos.y + y; if(wrapAround) { posy %= (int)cClient->getMap()->GetHeight(); if(posy < 0) posy += cClient->getMap()->GetHeight(); }
				if(!(cClient->getMap()->GetPixelFlag(x,posy) & PX_EMPTY)) {
					coll = true;

					if(!hit && !jump) {
						if(fabs(vel->y) > 40 && ((vel->y > 0 && y>0) || (vel->y < 0 && y<0)))
							vel->y *= -0.4f;
						else
							vel->y=(0);
					}

					hit = true;
					worm->setOnGround( true );

					// NOTE: Be carefull that you don't do any float->int->float conversions here.
					// This has some *huge* effect. People reported it as high ceil friction.
					if(y<0) {
						clip |= 0x04;
						worm->pos().y = pos.y + y + 5;
					}
					else {
						clip |= 0x08;
						worm->pos().y = pos.y + y - 5;
					}

					//if(y>3 && !jump) {
						//vVelocity.y=(-10);
						//Velocity.y=(0);
					//	break;
					//}
				}
			}
		}
		
		// In case of this, it could be that we need to do a FMOD. Just do it to be sure.
		if(check_needed && wrapAround) {
			FMOD(pos.y, (float)cClient->getMap()->GetHeight());			
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

		if (wrapAround && worm->getNinjaRope()->isReleased()) {
			if (worm->pos().x > prevWormPos.x + cClient->getMap()->GetWidth() / 2)
				worm->getNinjaRope()->hookPos().x += cClient->getMap()->GetWidth();
			if (worm->pos().x < prevWormPos.x - cClient->getMap()->GetWidth() / 2)
				worm->getNinjaRope()->hookPos().x -= cClient->getMap()->GetWidth();
			if (worm->pos().y > prevWormPos.y + cClient->getMap()->GetHeight() / 2)
				worm->getNinjaRope()->hookPos().y += cClient->getMap()->GetHeight();
			if (worm->pos().y < prevWormPos.y - cClient->getMap()->GetHeight() / 2)
				worm->getNinjaRope()->hookPos().y -= cClient->getMap()->GetHeight();
		}

		return coll;
	}

// TODO: this function does not behave exactly as the above one (still some bugs probably), fix this in beta 10 as it is faster than the above
/*bool moveAndCheckWormCollision(AbsTime currentTime, float dt, CWorm* worm, CVec pos, CVec *vel, CVec vOldPos, int jump ) {
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

		// Handle infinite map
		const bool wrapAround = cClient->getGameLobby()->features[FT_InfiniteMap];
		pos += *vel * dt * worm->speedFactor();
		if(wrapAround) {
			FMOD(pos.x, (float)cClient->getMap()->GetWidth());
			FMOD(pos.y, (float)cClient->getMap()->GetHeight());
		}
		CMap *map = cClient->getMap();
		worm->pos() = pos;
		
		worm->setOnGround(false);

		// Check for collision
		CMap::CollisionInfo coll = map->StaticCollisionCheck(pos, 7, 7, wrapAround);

		// Left or right
		if (coll.left || coll.right)  {
			if (fabs(vel->x) > 40)
				vel->x *= -0.4f;
			else
				vel->x = 0;

			// If we collided with the ground and we were going pretty fast, make a bump sound
			if (fabs(vel->x) > 30)
				StartSound(sfxGame.smpBump, worm->pos(), worm->getLocal(), -1, worm);

			// Adjust the position
			worm->pos().x = (float)coll.moveToX;
		}

		// Top or bottom
		if (coll.top || coll.bottom)  {
			worm->setOnGround(true);

			if (fabs(vel->y) > 40)
				vel->y *= -0.4f;
			else
				vel->y = 0;

			// If we collided with the ground and we were going pretty fast, make a bump sound
			if (fabs(vel->y) > 30)
				StartSound(sfxGame.smpBump, worm->pos(), worm->getLocal(), -1, worm);

			// Adjust the position
			worm->pos().y = (float)coll.moveToY;
		}
			
		// If we are stuck in left & right or top & bottom, just don't move in that direction
		if (coll.left && coll.right)
			worm->pos().x = vOldPos.x;

		// HINT: when stucked horizontal we move slower - it's more like original LX
		if (coll.top && coll.bottom)  {
			worm->pos().y = vOldPos.y;
			if (!worm->getWormState()->bJump)  // HINT: this is almost exact as old LX
				worm->pos().x = vOldPos.x;
		}
		
		return coll.occured;
	}*/


	virtual void simulateWorm(CWorm* worm, CWorm* worms, bool local) {
		AbsTime simulationTime = GetPhysicsTime();
		warpSimulationTimeForDeltaTimeCap(worm->fLastSimulationTime, tLX->fDeltaTime, tLX->fRealDeltaTime);
		static const TimeDiff orig_dt = LX56PhysicsDT;
		const float dt = (bool)cClient->getGameLobby()->features[FT_GameSpeedOnlyForProjs] ? orig_dt.seconds() : (orig_dt.seconds() * (float)cClient->getGameLobby()->features[FT_GameSpeed]);
		const TimeDiff wpnDT = orig_dt * (float)cClient->getGameLobby()->features[FT_GameSpeed]; // wpnDT could be different from dt
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


		// If we're seriously injured (below 15% health) and visible, bleed
		// HINT: We have to check the visibility for everybody as we don't have entities for specific teams/worms.
		// If you want to make that better, you would have to give the CViewport to simulateWorm (but that would be really stupid).
		if(worm->getHealth() < 15 && worm->isVisibleForEverybody()) {
			if(simulationTime > worm->getLastBlood() + 2.0f) {
				worm->setLastBlood( worm->fLastSimulationTime );

				const float amount = ((float)tLXOptions->iBloodAmount / 100.0f) * 10;
				for(short i=0;i<amount;i++) {
					const CVec v = CVec(GetRandomNum(), GetRandomNum()) * 30;
					SpawnEntity(ENT_BLOOD,0,worm->getPos(),v,Color(200,0,0),NULL);
					SpawnEntity(ENT_BLOOD,0,worm->getPos(),v,Color(180,0,0),NULL);
				}
			}
		}


		// Process the carving
		if(ws->bCarve) {
			const CVec dir = worm->getMoveDirection();
			worm->incrementDirtCount( CarveHole(worm->getPos() + dir*4) );
			//cClient->SendCarve(vPos + dir*4);
		}

		static const float	fFrameRate = 7.5f;
		if(ws->bMove)
			worm->frame() += fFrameRate * dt;

		if(worm->frame() >= 3.0f || !ws->bMove)
			worm->frame() = 0.0f;
		if(worm->frame() < 0)
			worm->frame() = 2.99f;

		// Process the ninja rope
		if(worm->getNinjaRope()->isReleased() && worms) {
			simulateNinjarope( dt, worm, worms );
			// TODO: move 'getForce' here?
			worm->velocity() += worm->getNinjaRope()->GetForce(worm->getPos()) * dt;
		}

		// Process the moving
		if(ws->bMove) {
			const float speed = worm->isOnGround() ? wd->GroundSpeed : wd->AirSpeed;
			if(worm->getMoveDirectionSide() == DIR_RIGHT) {
				// Right
				if(worm->velocity().x < 30)
					worm->velocity().x += speed * dt * 90.0f;
			} else {
				// Left
				if(worm->velocity().x > -30)
					worm->velocity().x -= speed * dt * 90.0f;
			}
		}


		// Process the jump
		bool jumped = false;
		{
			const bool onGround = worm->CheckOnGround();
			if( onGround )
				worm->setLastAirJumpTime(AbsTime());
			if(ws->bJump && ( onGround || worm->canAirJump() ||
				( bool(cClient->getGameLobby()->features[FT_RelativeAirJump]) && GetPhysicsTime() > 
					worm->getLastAirJumpTime() + float( cClient->getGameLobby()->features[FT_RelativeAirJumpDelay] ) ) )) 
			{
				if( onGround )
					worm->velocity().y = wd->JumpForce;
				else {
					// GFX effect, as in TeeWorlds (we'll change velocity after that)
					SpawnEntity(ENT_SPARKLE, 10, worm->getPos() + CVec( 0, 4 ), worm->velocity() + CVec( 0, 40 ), Color(), NULL );
					SpawnEntity(ENT_SPARKLE, 10, worm->getPos() + CVec( 2, 4 ), worm->velocity() + CVec( 20, 40 ), Color(), NULL );
					SpawnEntity(ENT_SPARKLE, 10, worm->getPos() + CVec( -2, 4 ), worm->velocity() + CVec( -20, 40 ), Color(), NULL );

					if( worm->canAirJump() && worm->velocity().y > wd->JumpForce ) // Negative Y coord = moving up
						worm->velocity().y = wd->JumpForce; // Absolute velocity - instant air jump
					else
						worm->velocity().y += wd->JumpForce; // Relative velocity - relative air jump
				}
				worm->setLastAirJumpTime(GetPhysicsTime());
				worm->setOnGround( false );
				jumped = true;
			}
		}

		{
			// Air drag (Mainly to dampen the ninja rope)
			const float Drag = wd->AirFriction;

			if(!worm->isOnGround())	{
				worm->velocity().x -= SQR(worm->velocity().x) * SIGN(worm->velocity().x) * Drag * dt;
				worm->velocity().y += -SQR(worm->velocity().y) * SIGN(worm->velocity().y) * Drag * dt;
			}
		}
				
		// Gravity
		worm->velocity().y += wd->Gravity*dt;

		{
			float friction = cClient->getGameLobby()->features[FT_WormFriction];
			if(friction > 0) {
				static const float wormSize = 5.0f;
				static const float wormMass = (wormSize/2) * (wormSize/2) * (float)PI;
				static const float wormDragCoeff = 0.1f; // Note: Never ever change this! (Or we have to make this configureable)
				applyFriction(worm->velocity(), dt, wormSize, wormMass, wormDragCoeff, friction);
			}
		}
		
		//resetFollow(); // reset follow here, projectiles will maybe re-enable it...

		// Check collisions and move
		moveAndCheckWormCollision( simulationTime, dt, worm, worm->getPos(), &worm->velocity(), worm->getPos(), jumped );


		// Ultimate in friction
		if(worm->isOnGround()) {
			worm->velocity().x *= 1.0f - float(cClient->getGameLobby()->features[FT_WormGroundFriction]);

			//vVelocity = vVelocity * CVec(/*wd->GroundFriction*/ 0.9f,1);        // Hack until new game script is done

			// Too slow, just stop
			if(fabs(worm->velocity().x) < 5 && !ws->bMove)
				worm->velocity().x = 0;
		}

		simulateWormWeapon(wpnDT, worm);


		// Fill in the info for sending
		if(local) {
			ws->iAngle = (int)worm->getAngle();
			ws->iFaceDirectionSide = worm->getFaceDirectionSide();
			ws->iX = (int)worm->getPos().x;
			ws->iY = (int)worm->getPos().y;
		}

		goto simulateWormStart;
	}

	virtual void simulateWormWeapon(CWorm* worm) {
		// we use deltatime here and not realdeltatime because we want to have it the same way as the call from simulateWorm
		TimeDiff dt = tLX->fDeltaTime * (float)cClient->getGameLobby()->features[FT_GameSpeed];
		simulateWormWeapon(dt, worm);
	}

	void simulateWormWeapon(TimeDiff dt, CWorm* worm) {
		// Weird
		if (worm->getCurrentWeapon() < 0 || worm->getCurrentWeapon() >= 5) {
			warnings("WARNING: SimulateWeapon: iCurrentWeapon is bad\n");
			return;
		}

		wpnslot_t *Slot = worm->getWeapon(worm->getCurrentWeapon());

		if(!Slot->Weapon) return;

		// Slot should still do the reloading even without enabled wpn, thus uncommented this check.
		//if(!Slot->Enabled) return;
		
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

	virtual void simulateProjectiles(Iterator<CProjectile*>::Ref projs) {
		LX56_simulateProjectiles(projs);
	}

	void simulateNinjarope(float dt, CWorm* owner, CWorm *worms) {
		CNinjaRope* rope = owner->getNinjaRope();
		CVec playerpos = owner->getPos();

		const bool wrapAround = cClient->getGameLobby()->features[FT_InfiniteMap];

		rope->updateOldHookPos();

		if(!rope->isReleased())
			return;

		const bool firsthit = !rope->isAttached();
		CVec force = rope->isShooting() ? CVec(0,100) : CVec(0,150);

		// dt is fixed, but for very high speed, this could be inaccurate.
		// We use the limit 5 here to have it very unpropable to shoot through a wall.
		// In most cases, dt his halfed once, so this simulateNinjarope is
		// like in LX56 with 200FPS.
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

			const float length2 = (playerpos - rope->hookPos()) . GetLength2();

			// Check if it's too long
			if(length2 > rope->getMaxLength() * rope->getMaxLength()) {
				rope->hookVelocity() = CVec(0,0);
				rope->setShooting( false );
			}
		}
		// Failing
		else if(!rope->isShooting() && !rope->isAttached()) {

			// Going towards the player
			const float length2 = (playerpos - rope->hookPos()) . GetLength2();
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
		if(!rope->isPlayerAttached() && !wrapAround &&
			(	rope->hookPos().x <= 0 || rope->hookPos().y <= 0 ||
				rope->hookPos().x >= cClient->getMap()->GetWidth()-1 ||
				rope->hookPos().y >= cClient->getMap()->GetHeight()-1)) {
			rope->setShooting( false );
			rope->setAttached( true );

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

			VectorD2<long> wrappedHookPos = rope->hookPos();
			MOD(wrappedHookPos.x, (long)cClient->getMap()->GetWidth());
			MOD(wrappedHookPos.y, (long)cClient->getMap()->GetHeight());
			
			LOCK_OR_QUIT(cClient->getMap()->GetImage());
			uchar px = outsideMap ? PX_ROCK : cClient->getMap()->GetPixelFlag(wrappedHookPos.x, wrappedHookPos.y);
			if((px & PX_ROCK || px & PX_DIRT || outsideMap)) {
				rope->setShooting( false );
				rope->setAttached( true );
				rope->hookVelocity() = CVec(0,0);

				if((px & PX_DIRT) && firsthit) {
					Color col = Color(cClient->getMap()->GetImage()->format, GetPixel(cClient->getMap()->GetImage().get(), wrappedHookPos.x, wrappedHookPos.y));
					for( short i=0; i<5; i++ )
						SpawnEntity(ENT_PARTICLE,0, rope->hookPos() + CVec(0,2), CVec(GetRandomNum()*40,GetRandomNum()*40),col,NULL);
				}
			}
			UnlockSurface(cClient->getMap()->GetImage());
		}

		// Check if the hook has hit another worm
		if(!rope->isAttached() && !rope->isPlayerAttached()) {

			for(short i=0; i<MAX_WORMS; i++) {
				// Don't check against the worm if they aren't used, dead, a flag or the ninja rope was shot by the worm
				if(!worms[i].isUsed())
					continue;
				if(!worms[i].getAlive())
					continue;
				if(worms[i].getID() == owner->getID())
					continue;
				if(!worms[i].isVisible(owner))
					continue;

				CVec dist = worms[i].getPos() - rope->hookPos();
				if( wrapAround ) {
					FMOD(dist.x, (float)cClient->getMap()->GetWidth());
					FMOD(dist.y, (float)cClient->getMap()->GetHeight());
				}
				if( dist.GetLength2() < 25 ) {
					rope->AttachToPlayer(&worms[i], owner);
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
				rope->UnAttachPlayer();
			} else {
				CVec prevHookPos = rope->hookPos();
				rope->hookPos() = rope->getAttachedPlayer()->getPos();
				if( wrapAround ) {
					//printf("worm %d rope %5.2f %5.2f -> %5.2f %5.2f width %d\n", owner->getID(), prevHookPos.x, prevHookPos.y, rope->hookPos().x, rope->hookPos().y, cClient->getMap()->GetWidth());
					while (rope->hookPos().x > prevHookPos.x + cClient->getMap()->GetWidth() / 2)
						rope->hookPos().x -= cClient->getMap()->GetWidth();
					while (rope->hookPos().x < prevHookPos.x - cClient->getMap()->GetWidth() / 2)
						rope->hookPos().x += cClient->getMap()->GetWidth();
					while (rope->hookPos().y > prevHookPos.y + cClient->getMap()->GetHeight() / 2)
						rope->hookPos().y -= cClient->getMap()->GetHeight();
					while (rope->hookPos().y < prevHookPos.y - cClient->getMap()->GetHeight() / 2)
						rope->hookPos().y += cClient->getMap()->GetHeight();
				}
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
		AbsTime simulationTime = GetPhysicsTime();	
		warpSimulationTimeForDeltaTimeCap(bonus->fLastSimulationTime, tLX->fDeltaTime, tLX->fRealDeltaTime);
		static const float orig_dt = LX56PhysicsDT.seconds();
		const float dt = (bool)cClient->getGameLobby()->features[FT_GameSpeedOnlyForProjs] ? orig_dt : (orig_dt * (float)cClient->getGameLobby()->features[FT_GameSpeed]);

	simulateBonusStart:
		if(bonus->fLastSimulationTime + orig_dt > simulationTime) return;
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
				if( cClient->getGameLobby()->features[FT_InfiniteMap] )
					bonus->pos().y -= (float)mh;
				else
					colideBonus(bonus, x,y);
				return;
			}
			
			// TODO: do we need to lock pixel flags here?
			const uchar *pf = cClient->getMap()->GetPixelFlags() + y*mw + px-2;

			for(x=px-2; x<=px+2; x++) {

				// Clipping
				if(x<0) {
					pf++;
					continue;
				}
				if(x>=mw) {
					colideBonus(bonus, x,y); // TODO: should not happen, remove that (bonus doesn't move horizontally). And why checking only right map border, not left one?
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
