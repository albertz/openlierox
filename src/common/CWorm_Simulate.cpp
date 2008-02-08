/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Worm class - Simulation
// Created 2/8/02
// Jason Boettcher


#include "LieroX.h"
#include "GfxPrimitives.h"
#include "InputEvents.h"
#include "CWorm.h"
#include "MathLib.h"
#include "Entity.h"
#include "CClient.h"
#include "Utils.h"


///////////////////
// Get the input from a human worm
void CWorm::getInput()
{
	float	dt = tLX->fDeltaTime;
	CVec	dir;
	int		weap = false;
	int		RightOnce = false;

	mouse_t *ms = GetMouse();
	
	// Backup the direction so that the worm can strafe
	iStrafeDirection = iDirection;

	worm_state_t *ws = &tState;

	// Init the ws
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;

	bool mouseControl = tLXOptions->bMouseAiming && ( cOwner->getHostAllowsMouse() || tGameInfo.iGameType == GME_LOCAL);

	// TODO: here are width/height of the window hardcoded
	int mouse_dx = ms->X - 640/2;
	int mouse_dy = ms->Y - 480/2;
	if(mouseControl) SDL_WarpMouse(640/2, 480/2);
	
	{
/*		// only some debug output for checking the values		
		if(mouseControl && (mouse_dx != 0 || mouse_dy != 0))
			printf("mousepos changed: %i, %i\n", mouse_dx, mouse_dy),
			printf("anglespeed: %f\n", fAngleSpeed),
			printf("movespeed: %f\n", fMoveSpeedX),
			printf("dt: %f\n", dt); */
	}

	// angle section
	{	
		// Up
		if(cUp.isDown()) {
			fAngleSpeed -= 500 * dt;
			//fAngle -= wd->AngleSpeed * dt;
		} else if(cDown.isDown()) { // Down
			fAngleSpeed += 500 * dt;
			//fAngle += wd->AngleSpeed * dt;
		} else {
			if(!mouseControl) {
				// HINT: this is the original order and code (before mouse patch - rev 1007)
				CLAMP_DIRECT(fAngleSpeed, -100.0f, 100.0f);
				REDUCE_CONST(fAngleSpeed, 200*dt);
				RESET_SMALL(fAngleSpeed, 5.0f);
			
			} else { // mouseControl for angle
				static const float mult_Y = 4; // how sensitive is the mouse in Y-dir
				fAngleSpeed += mouse_dy * mult_Y;
				
				CLAMP_DIRECT(fAngleSpeed, -100.0f, 100.0f);
				REDUCE_CONST(fAngleSpeed, 200*dt);
				RESET_SMALL(fAngleSpeed, 5.0f);
				
/*				REDUCE_CONST(fAngleSpeed, 200*dt);
				RESET_SMALL(fAngleSpeed, 5.0f);
				CLAMP_DIRECT(fAngleSpeed, -500.0f, 500.0f); */
			}
		}

		fAngle += fAngleSpeed * dt;
		if(CLAMP_DIRECT(fAngle, -90.0f, 60.0f) != 0)
			fAngleSpeed = 0;

		// Calculate dir
		dir.x=( (float)cos(fAngle * (PI/180)) );
		dir.y=( (float)sin(fAngle * (PI/180)) );
		if( iStrafeDirection == DIR_LEFT ) // Fix: Ninja rope shoots backwards when you strafing or mouse-aiming
			dir.x=(-dir.x);

	} // end angle section
		
	
	// basic mouse control (moving)
	if(mouseControl) {
						
		static const float mult_X = 2; // how sensitive is the mouse in X-dir
		fMoveSpeedX += mouse_dx * mult_X;		
		
		REDUCE_CONST(fMoveSpeedX, 1000*dt);
		//RESET_SMALL(fMoveSpeedX, 5.0f);
		CLAMP_DIRECT(fMoveSpeedX, -500.0f, 500.0f);
				
		if(fabs(fMoveSpeedX) > 50) {
			if(fMoveSpeedX > 0) {
				iDirection = DIR_RIGHT;
				if(mouse_dx > 0) lastMoveTime = tLX->fCurTime;
			} else {
				iDirection = DIR_LEFT;
				if(mouse_dx > 0) lastMoveTime = tLX->fCurTime;
			}
			ws->iMove = true;
			
		} else {
			ws->iMove = false;
		}
					
	}
	
	
	if(mouseControl) { // set shooting, ninja and jumping, weapon selection for mousecontrol
		// like Jason did it
		ws->iShoot = (ms->Down & SDL_BUTTON(1)) ? true : false;
		ws->iJump = (ms->Down & SDL_BUTTON(3)) ? true : false;
		if(ws->iJump) {
			if(cNinjaRope.isReleased())
				cNinjaRope.Release();
		}
		else if(ms->FirstDown & SDL_BUTTON(2)) {
			// TODO: this is bad. why isn't there a ws->iNinjaShoot ?
			cNinjaRope.Shoot(vPos,dir);
			PlaySoundSample(sfxGame.smpNinja);
		}
	
		if( ms->WheelScrollUp || ms->WheelScrollDown ) {
			bForceWeapon_Name = true;
			fForceWeapon_Time = tLX->fCurTime + 0.75f;
			if( ms->WheelScrollUp ) 
				iCurrentWeapon ++;
			else 
				iCurrentWeapon --;
			if(iCurrentWeapon >= iNumWeaponSlots) 
				iCurrentWeapon=0;
			if(iCurrentWeapon < 0) 
				iCurrentWeapon=iNumWeaponSlots-1;
		}
	}
		


	{ // set carving
	
		if(!cRight.isDown())
			iCarving &= ~1;
		if(!cLeft.isDown())
			iCarving &= ~2;
	
	
		// Carving hack
		RightOnce = cRight.isDownOnce();
		if(cLeft.isDown() && RightOnce && iDirection == DIR_LEFT)  {
			iCarving |= 2;
		}
		
/*		// this is a bit unfair to keyboard players
		if(mouseControl) { // mouseControl
			if(fabs(fMoveSpeedX) > 200) {
				ws->iCarve = true;
				iCarving |= (fMoveSpeedX > 0) ? 1 : 2;
			}
		} */
		
		static const float movetimed_min = 0.08f, movetimed_max = 0.2f;
		
		if((mouseControl && ws->iMove && iDirection == DIR_LEFT)
		|| (/*cLeft.isJoystick() &&*/ cLeft.isDown())) {
			float movetimed = tLX->fCurTime - lastMoveTime;
			//printf("movetimed: %f\n", movetimed);
			if(movetimed_min < movetimed && movetimed < movetimed_max) {
				ws->iCarve = true;
				ws->iMove = true;
				iCarving |= 2;
			}
		}
	
		if((mouseControl && ws->iMove && iDirection == DIR_RIGHT)
		|| (/*cRight.isJoystick() &&*/ cRight.isDown())) {
			float movetimed = tLX->fCurTime - lastMoveTime;
			//printf("movetimed: %f\n", movetimed);
			if(movetimed_min < movetimed && movetimed < movetimed_max) {
				ws->iCarve = true;
				ws->iMove = true;
				iCarving |= 1;
			}
		}
	}
	
    //
    // Weapon changing
	//
	if(cSelWeapon.isDown()) {
		// TODO: was is the intention of this var? if weapon change, then it's wrong
		// if cSelWeapon.isDown(), then we don't need it
		weap = true;
		
		// we don't want keyrepeats here, so only count the first down-event
		int change = (RightOnce ? 1 : 0) - (cLeft.isDownOnce() ? 1 : 0);		
		iCurrentWeapon += change;
		MOD(iCurrentWeapon, iNumWeaponSlots);
	}


	// Use keys 1-5 and 6-0 for fast weapon changing
	// TODO: in network game iID == 0 only for server owner, so these keys won't work
	if( tLXOptions->bUseNumericKeysToSwitchWeapons )
    switch (iID) {

	// If this is the first worm, let the user use the 1-5 keys for weapon shortcuts
	// Also use mouse wheel to switch weapons and aim with mouse if this option is selected
	case 0:  {
			keyboard_t *kb = GetKeyboard();
			for(int i = SDLK_1; i <= SDLK_5; i++ ) {
				if( kb->KeyDown[i] ) {

					iCurrentWeapon = i - SDLK_1;

					// Let the weapon name show up for a short moment
					bForceWeapon_Name = true;
					fForceWeapon_Time = tLX->fCurTime + 0.75f;
				}
			}
			
		}
		break;

	// If this is the second worm, let the user use the 6-0 keys for weapon shortcuts
	case 1:  {
			keyboard_t *kb = GetKeyboard();
			for(int i = SDLK_6; i <= SDLK_9; i++ ) {
				if( kb->KeyDown[i] ) {

					iCurrentWeapon = i - SDLK_6;

					// Let the weapon name show up for a short moment
					bForceWeapon_Name = true;
					fForceWeapon_Time = tLX->fCurTime + 0.75f;
				}
			}

			// 0 has to be separate, its keysym is not SDLK_9 + 1
			if (kb->KeyDown[SDLK_0])  {
				iCurrentWeapon = 5;

				// Let the weapon name show up for a short moment
				bForceWeapon_Name = true;
				fForceWeapon_Time = tLX->fCurTime + 0.75f;
			}

		}
		break;
	}

	// Safety: clamp the current weapon
	iCurrentWeapon = CLAMP(iCurrentWeapon, 0, iNumWeaponSlots-1);


	if(cShoot.isDown())  {
		ws->iShoot = true;
	}



	// Right
	if((cRight.isDown() && !(iCarving & 2) && !weap)  || (iType == PRF_COMPUTER && !(iCarving & 2))) {

		// Check if we dig a small hole
		if(cLeft.isDownOnce() && iDirection == DIR_RIGHT) {
			ws->iCarve = true;

			//cClient->SendCarve(vPos + dir*4);
			//map->CarveHole(3,Pos + dir*4);
			iCarving |= 1;
		}

		if(!cLeft.isDown() || iDirection == DIR_RIGHT) {
			iDirection = DIR_RIGHT;
			ws->iMove = true;
			lastMoveTime = tLX->fCurTime;
			
			//if(vVelocity.x<75)
			//	vVelocity = vVelocity + CVec(speed,0);
			//fFrame+=5*dt;
		}
	}

	// Left
	if((cLeft.isDown() && !(iCarving & 1) && !weap) || (iType == PRF_COMPUTER && !(iCarving & 1))) {

		// Check if we dig a small hole
		if(RightOnce && iDirection == DIR_LEFT) {
			ws->iCarve = true;

			//cClient->SendCarve(vPos + dir*4);
			//map->CarveHole(3,Pos + dir*4);
			iCarving |= 2;
		}

		iDirection = DIR_LEFT;
		ws->iMove = true;
		lastMoveTime = tLX->fCurTime;
		
		//if(vVelocity.x>-75)
		//	vVelocity = vVelocity + CVec(-speed,0);
		//fFrame+=5*dt;
	}


	bool oldskool = tLXOptions->bOldSkoolRope;

	bool jumpdownonce = cJump.isDownOnce();

	// Jump
	if(jumpdownonce) {
		if( !(oldskool && cSelWeapon.isDown()) )  {
			ws->iJump = true;

			if(cNinjaRope.isReleased())
				cNinjaRope.Release();
		}
	}

	// Ninja Rope
	if(oldskool) {
		// Old skool style rope throwing
		// Change-weapon & jump

		if(!cSelWeapon.isDown() || !cJump.isDown())  {
			bRopeDown = false;
		}

		if(cSelWeapon.isDown() && cJump.isDown() && !bRopeDown) {

			bRopeDownOnce = true;
			bRopeDown = true;
		}

		// Down
		if(bRopeDownOnce) {
			bRopeDownOnce = false;

			cNinjaRope.Shoot(vPos,dir);

			// Throw sound
			PlaySoundSample(sfxGame.smpNinja);
		}


	} else {
		// Newer style rope throwing
		// Seperate dedicated button for throwing the rope
		if(cInpRope.isDownOnce()) {

			cNinjaRope.Shoot(vPos,dir);
			// Throw sound
			PlaySoundSample(sfxGame.smpNinja);
		}
	}

	ws->iAngle = (int)fAngle;
	ws->iX = (int)vPos.x;
	ws->iY = (int)vPos.y;
		

}


///////////////////
// Clear the input
void CWorm::clearInput(void)
{
	// Clear the state
	tState.iCarve = false;
	tState.iMove  = false;
	tState.iShoot = false;
	tState.iJump  = false;
}

///////////////////
// Simulate the worm
void CWorm::Simulate(CWorm *worms, int local, float dt)
{
	// If the delta time is too big, divide it and run the simulation twice
	if( dt > 0.25f ) {
		dt /= 2;
		Simulate(worms,local,dt);
		Simulate(worms,local,dt);
		return;
	}

	float speed;
	CVec dir;
	const gs_worm_t *wd = cGameScript->getWorm();

	float	fFrameRate = 7.5f;

	worm_state_t *ws = &tState;

	// Simulate the viewport
	//cViewport.Process(vPos,map->GetWidth(),map->GetHeight());


	// Special things for remote worms
	if(!local) {
		fAngle = (float)ws->iAngle;
		iDirection = ws->iDirection;
	}


	// If we're IT, spawn some sparkles
	if(bTagIT && tGameInfo.iGameMode == GMT_TAG) {
		if(tLX->fCurTime - fLastSparkle > 0.15f) {
			fLastSparkle = tLX->fCurTime;
			CVec p = vPos + CVec(GetRandomNum()*3, GetRandomNum()*3);

			SpawnEntity(ENT_SPARKLE,0,p, CVec(0,0), 0,NULL);
		}
	}

	// If we're seriously injured (below 15% health) bleed
	if(iHealth < 15) {
		if(tLX->fCurTime - fLastBlood > 2) {
			fLastBlood = tLX->fCurTime;

			float amount = ((float)tLXOptions->iBloodAmount / 100.0f) * 10;
			CVec v;
			for(short i=0;i<amount;i++) {
				v = CVec(GetRandomNum(), GetRandomNum()) * 30;
				SpawnEntity(ENT_BLOOD,0,vPos,v,MakeColour(200,0,0),NULL);
				SpawnEntity(ENT_BLOOD,0,vPos,v,MakeColour(180,0,0),NULL);
			}
		}
	}

	// Calculate dir
	dir.x=( (float)cos(fAngle * (PI/180)) );
	dir.y=( (float)sin(fAngle * (PI/180)) );
	if(iDirection==DIR_LEFT)
		dir.x=(-dir.x);

	// Process the carving
	if(ws->iCarve) {
		iDirtCount += CarveHole(pcMap, vPos + dir*4);
		//cClient->SendCarve(vPos + dir*4);
	}

	if(ws->iMove)
		fFrame += fFrameRate * dt;

	if(fFrame >= 3.0f || !ws->iMove)
		fFrame=0;

	speed = bOnGround ? wd->GroundSpeed : wd->AirSpeed;

	// Process the ninja rope
	if(cNinjaRope.isReleased() && worms) {
		cNinjaRope.Simulate(dt,pcMap,vPos,worms,iID);
		vVelocity += cNinjaRope.GetForce(vPos)*dt;
	}

	// Process the moving
	if(ws->iMove) {
		if(iDirection == DIR_RIGHT) {
			// Right
			if(vVelocity.x < 30)
				vVelocity.x += speed * dt * 90.0f;
		} else {
			// Left
			if(vVelocity.x > -30)
				vVelocity.x -= speed * dt * 90.0f;
		}
	}

	if(cStrafe.isDown())
		iDirection = iStrafeDirection;


	// Process the jump
	if(ws->iJump && CheckOnGround()) {
		vVelocity.y = wd->JumpForce;
		bOnGround = false;
	}


	// Air drag (Mainly to dampen the ninja rope)
	float Drag = wd->AirFriction;

	if(!bOnGround)	{
		vVelocity.x -= SQR(vVelocity.x) * SIGN(vVelocity.x) * Drag * dt;
		vVelocity.y += -SQR(vVelocity.y) * SIGN(vVelocity.y) * Drag * dt;
	}


	// Gravity
	vVelocity.y += wd->Gravity*dt;


	//resetFollow(); // reset follow here, projectiles will maybe re-enable it...

	// Check collisions and move
	MoveAndCheckWormCollision( dt, vPos, &vVelocity, vPos, ws->iJump );


	// Ultimate in friction
	if(bOnGround) {
//		vVelocity.x *= pow(0.9f, dt * 100.0f); // wrong

		// TODO: if you manage to make it this way but exponential, it will be perfect :)
		/*float old_x = vVelocity.x;
		vVelocity.x -= SIGN(vVelocity.x) * 0.9f * (dt * 100.0f); // TODO: this is the bug described in
		if (SIGN(old_x) != SIGN(vVelocity.x))  // Make sure we don't dampen to opposite direction
			vVelocity.x = 0.0f;*/

		// Even though this is highly FPS dependent, it seems to work quite good (this is how old lx did it)
		vVelocity.x *= 0.9f;

		//vVelocity = vVelocity * CVec(/*wd->GroundFriction*/ 0.9f,1);        // Hack until new game script is done

		// Too slow, just stop
		if(fabs(vVelocity.x) < 5 && !ws->iMove)
			vVelocity.x = 0;
	}


	// Weapon
	SimulateWeapon( dt );

	// Fill in the info for sending
	if(local) {
		ws->iAngle = (int)fAngle;
		ws->iDirection = iDirection;
		ws->iX = (int)vPos.x;
		ws->iY = (int)vPos.y;
	}
}


///////////////////
// Simulates the weapon
void CWorm::SimulateWeapon( float dt )
{
	// Weird
	if (iCurrentWeapon < 0 || iCurrentWeapon >= 5) {
		printf("WARNING: SimulateWeapon: iCurrentWeapon is bad\n");
		return;
	}

	wpnslot_t *Slot = &tWeapons[iCurrentWeapon];

	if(Slot->LastFire>0)
		Slot->LastFire-=dt;

	if(Slot->Reloading) {

		// Prevent a div by zero error
		if(fLoadingTime == 0)
			fLoadingTime = 0.00001f;

		Slot->Charge += dt * (Slot->Weapon->Recharge * (1.0f/fLoadingTime));
		if(Slot->Charge > 1) {
			Slot->Charge = 1;
			Slot->Reloading = false;
		}
	}
}


///////////////////
// Check collisions with the level
// HINT: it directly manipulates vPos!
bool CWorm::MoveAndCheckWormCollision( float dt, CVec pos, CVec *vel, CVec vOldPos, int jump )
{
	static const int maxspeed2 = 10;
	
	// If the worm is going too fast, divide the speed by 2 and perform 2 collision checks
	if( (*vel*dt).GetLength2() > maxspeed2) {
		dt /= 2;
		if(MoveAndCheckWormCollision(dt,pos,vel,vOldPos,jump)) return true;
		return MoveAndCheckWormCollision(dt,vPos,vel,vOldPos,jump);
	}

	pos += *vel * dt;
	vPos = pos;


	int x,y;
	x = (int)pos.x;
	y = (int)pos.y;
	short clip = 0; // 0x1=left, 0x2=right, 0x4=top, 0x8=bottom
	bool coll = false;
	bool check_needed = false;

	const uchar* gridflags = pcMap->getAbsoluteGridFlags();
	uint grid_w = pcMap->getGridWidth();
	uint grid_h = pcMap->getGridHeight();
	uint grid_cols = pcMap->getGridCols();
	if(y-4 < 0 || (uint)y+5 > pcMap->GetHeight()-1
	|| x-3 < 0 || (uint)x+3 > pcMap->GetWidth()-1)
		check_needed = true; // we will check later, what to do here
	else if(grid_w < 7 || grid_h < 10 // this ensures, that this check is safe
	|| (gridflags[((y-4)/grid_h)*grid_cols + (x-3)/grid_w] & (PX_ROCK|PX_DIRT))
	|| (gridflags[((y+5)/grid_h)*grid_cols + (x-3)/grid_w] & (PX_ROCK|PX_DIRT))
	|| (gridflags[((y-4)/grid_h)*grid_cols + (x+3)/grid_w] & (PX_ROCK|PX_DIRT))
	|| (gridflags[((y+5)/grid_h)*grid_cols + (x+3)/grid_w] & (PX_ROCK|PX_DIRT)))
		check_needed = true;

	if(check_needed && y >= 0 && (uint)y < pcMap->GetHeight()) {
		for(x=-3;x<4;x++) {
			// Optimize: pixelflag++

			// Left side clipping
			if(pos.x+x <= 2) {
				clip |= 0x01;
				vPos.x=( 5 );
				coll = true;
				if(fabs(vel->x) > 40)
					vel->x *=  -0.4f;
				else
					vel->x=(0);
				continue;
			}

			// Right side clipping
			if(pos.x+x >= pcMap->GetWidth()) {
				vPos.x=( (float)pcMap->GetWidth() - 5 );
				coll = true;
				clip |= 0x02;
				if(fabs(vel->x) > 40)
					vel->x *= -0.4f;
				else
					vel->x=(0);
				continue;
			}


			if(!(pcMap->GetPixelFlag((int)pos.x+x,y) & PX_EMPTY)) {
				coll = true;
				
				if(x<0) {
					clip |= 0x01;
					vPos.x=( pos.x+x+4 );
				}
				else {
					clip |= 0x02;
					vPos.x=( pos.x+x-4 );
				}

				// Bounce
				if(fabs(vel->x) > 30)
					vel->x *= -0.4f;
				else
					vel->x=(0);
			}
		}
	}

	bOnGround = false;

	bool hit = false;
	x = (int)pos.x;

	if(check_needed && (uint)x < pcMap->GetWidth()) {
		for(y=5;y>-5;y--) {
			// Optimize: pixelflag + Width

			// Top side clipping
			if(pos.y+y <= 1) {
				vPos.y=( 6 );
				coll = true;
				clip |= 0x04;
				if(fabs(vel->y) > 40)
					vel->y *= -0.4f;
				continue;
			}

			// Bottom side clipping
			if(pos.y+y >= pcMap->GetHeight()) {
				vPos.y=( (float)pcMap->GetHeight() - 5 );
				clip |= 0x08;
				coll = true;
                bOnGround = true;
				if(fabs(vel->y) > 40)
					vel->y *= -0.4f;
				else
					vel->y=(0);
				continue;
			}


			if(!(pcMap->GetPixelFlag(x,(int)pos.y+y) & PX_EMPTY)) {
				coll = true;

                if(!hit && !jump) {
				    if(fabs(vel->y) > 40 && ((vel->y > 0 && y>0) || (vel->y < 0 && y<0)))
					    vel->y *= -0.4f;
				    else
					    vel->y=(0);
                }

				hit = true;
				bOnGround = true;

				if(y<0) {
					clip |= 0x04;
					vPos.y=( pos.y+y+5 );
				}
				else {
					clip |= 0x08;
					vPos.y=( pos.y+y-5 );
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
		vPos.x = vOldPos.x;

	// HINT: when stucked horizontal we move slower - it's more like original LX
	if ((clip & 0x04) && (clip & 0x08))  {
		vPos.y = vOldPos.y;
		vPos.x -= (vPos.x - vOldPos.x) / 2;
	}

	// If we collided with the ground and we were going pretty fast, make a bump sound
	if(coll) {
		if( fabs(vel->x) > 30 && (clip & 0x01 || clip & 0x02) )
			StartSound( sfxGame.smpBump, vPos, getLocal(), -1, this );
		else if( fabs(vel->y) > 30 && (clip & 0x04 || clip & 0x08) )
			StartSound( sfxGame.smpBump, vPos, getLocal(), -1, this );
	}

	return coll;
}






int MouseX = -1, MouseY = -1;


///////////////////
// Use a mouse for worm input
void CWorm::getMouseInput(void)
{
	if (!bUsesMouse)
		return;

	CVec	dir;
	int		weap = false;
	int		RightOnce = false;
	int		move = false;

	worm_state_t *ws = &tState;

	// Init the ws
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;


	// Get mouse delta's
	mouse_t *Mouse = GetMouse();
	int		DeltaX = 0;
	int		DeltaY = 0;

	if(MouseX >= 0)
		DeltaX = Mouse->X - 320;
	MouseX = Mouse->X;

	if(MouseY >= 0)
		DeltaY = Mouse->Y - 200;
	MouseY = Mouse->Y;

	// Change angle
	fAngle += DeltaY;
	fAngle = MAX(fAngle,(float)-90);
	fAngle = MIN(fAngle,(float)60);

	int Right = DeltaX > 0;
	int Left = DeltaX < 0;
	int Shoot = Mouse->Down & SDL_BUTTON(1);
	int Rope = Mouse->Down & SDL_BUTTON(3);
	int Jump = Mouse->Down & SDL_BUTTON(2);

	static int RopeDown = false;

	// Restore mouse to the middle
	SDL_WarpMouse(320,200);


	if(!Right)
		iCarving &= ~1;
	if(!Left)
		iCarving &= ~2;


	// Carving hack
	/*RightOnce = cRight.isDownOnce();
	if(cLeft.isDown() && RightOnce && iDirection == DIR_LEFT)
		iCarving |= 2;*/


	// Weapon changing

	if(cSelWeapon.isDown()) {
		weap = true;

		if(RightOnce) {
			iCurrentWeapon++;
			if(iCurrentWeapon >= iNumWeaponSlots)
				iCurrentWeapon=0;
		}
		if(cLeft.isDownOnce()) {
			iCurrentWeapon--;
			if(iCurrentWeapon < 0)
				iCurrentWeapon=iNumWeaponSlots-1;
		}
	}


	ws->iShoot = false;
	if(Shoot)
		ws->iShoot = true;



	// Right
	if(Right && !(iCarving & 2) && !weap) {

		// Check if we dig a small hole
		if(Left && iDirection == DIR_RIGHT) {
			ws->iCarve = true;

			//cClient->SendCarve(vPos + dir*4);
			//map->CarveHole(3,Pos + dir*4);
			iCarving |= 1;
		}

		if(!Left || iDirection == DIR_RIGHT) {
			iDirection = DIR_RIGHT;
			ws->iMove = true;

			//if(vVelocity.x<75)
			//	vVelocity = vVelocity + CVec(speed,0);
			//fFrame+=5*dt;
			move = true;
		}
	}

	// Left
	if(Left && !(iCarving & 1) && !weap) {

		// Check if we dig a small hole
		if(Right && iDirection == DIR_LEFT) {
			ws->iCarve = true;

			//cClient->SendCarve(vPos + dir*4);
			//map->CarveHole(3,Pos + dir*4);
			iCarving |= 2;
		}

		iDirection = DIR_LEFT;
		ws->iMove = true;

		//if(vVelocity.x>-75)
		//	vVelocity = vVelocity + CVec(-speed,0);
		//fFrame+=5*dt;
		move = true;
	}

	// Calculate dir
	dir.x=( (float)cos(fAngle * (PI/180)) );
	dir.y=( (float)sin(fAngle * (PI/180)) );
	if(iDirection==DIR_LEFT)
		dir.x=(-dir.x);

	if(Jump) {
		ws->iJump = true;
		cNinjaRope.Release();
	}


	if(Rope && !RopeDown) {
		cNinjaRope.Shoot(vPos,dir);
		// Throw sound
		PlaySoundSample(sfxGame.smpNinja);
		RopeDown = true;
	}

	if(!Rope)
		RopeDown = false;


	ws->iAngle = (int)fAngle;
	ws->iX = (int)vPos.x;
	ws->iY = (int)vPos.y;
}
