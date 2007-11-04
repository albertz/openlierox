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
void CWorm::getInput(/*worm_state_t *ws*/)
{
	float	dt = tLX->fDeltaTime;
	CVec	dir;
	int		weap = false;
	int		RightOnce = false;
	int		move = false;

	mouse_t *ms = GetMouse();
	
	// Backup the direction so that the worm can strafe
	iStrafeDirection = iDirection;

	worm_state_t *ws = &tState;

	// Temp thing
	// TODO: Try out mouse input for a 3rd worm. Not needed no more? We got mouse for player 1.
	//getMouseInput();
	//return;


	// Init the ws
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;

	bool mouseControl = tLXOptions->bMouseAiming && ( cOwner->getHostAllowsMouse() || tGameInfo.iGameType == GME_LOCAL);

	if (!mouseControl) {
		// Up
		if(cUp.isDown()) {
			fAngleSpeed -= 500 * dt;
			//fAngle -= wd->AngleSpeed * dt;
		} else
			
			// Down
			if(cDown.isDown()) {
				fAngleSpeed += 500 * dt;
				//fAngle += wd->AngleSpeed * dt;
			} else {
				CLAMP_DIRECT(fAngleSpeed, -100.0f, 100.0f);
				REDUCE_CONST(fAngleSpeed, 200*dt);
				RESET_SMALL(fAngleSpeed, 5.0f);
			}
			
	}
	else { //	if(mouseControl )	
		// TODO: btw, have you seen JasonBs CWorm::getMouseInput in this file?
		
		/*
		// HINT: this is another possibility which only count the mousemoving; but very confusing for fullscreen
		float mdt = tLX->fCurTime - lastMouseMoveTime;
		if(mdt > 0.0 && mdt < 0.3) { // else ignore it
			float dy = (float)(ms->Y - lastMousePosY) / mdt;
			fAngleSpeed += CLAMP(2.0 * dy, -500.0, 500.0) * mdt;
		}*/
			
		// this is better for fullscreen and also good for windowmode
		// TODO: the windows-width/height is hardcoded here! that is bad
		fAngleSpeed += (ms->Y - 480/2);
		fMoveSpeedX += (ms->X - 640/2);
		
		// TODO: here are again the hardcoded width/height of the window
		SDL_WarpMouse(640/2, 480/2);
		
		REDUCE_CONST(fMoveSpeedX, 1000*dt);
		RESET_SMALL(fMoveSpeedX, 5.0f);
		CLAMP_DIRECT(fMoveSpeedX, -500.0f, 500.0f);
		
		REDUCE_CONST(fAngleSpeed, 200*dt);
		RESET_SMALL(fAngleSpeed, 5.0f);
		CLAMP_DIRECT(fAngleSpeed, -500.0f, 500.0f);
		
		// also moving via mouse
		if(fabs(fMoveSpeedX) > 50) {
			iDirection = (fMoveSpeedX > 0) ? DIR_RIGHT : DIR_LEFT;
			ws->iMove = true;
			move = true;
		} else {
			ws->iMove = false;
			move = false;
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
	
	if(mouseControl) {
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
	
	
	
	if(lastMousePosX != ms->X || lastMousePosY != ms->Y) {
		lastMouseMoveTime = tLX->fCurTime;
		lastMousePosX = ms->X;
		lastMousePosY = ms->Y;
	}

	if(!mouseControl) {
		if(!cRight.isDown())
			iCarving &= ~1;
		if(!cLeft.isDown())
			iCarving &= ~2;
	
	
		// Carving hack
		RightOnce = cRight.isDownOnce();
		if(cLeft.isDown() && RightOnce && iDirection == DIR_LEFT)  {
			iCarving |= 2;
		}
	}
	else { // mouseControl
		if(fabs(fMoveSpeedX) > 200) {
			ws->iCarve = true;
			iCarving |= (fMoveSpeedX > 0) ? 1 : 2;
		}
	}

    //
    // Weapon changing
	//
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


	// Use keys 1-5 and 6-0 for fast weapon changing
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

	// TODO: why is this needed?
	// Clamp the current weapon
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

			//if(vVelocity.x<75)
			//	vVelocity = vVelocity + CVec(speed,0);
			//fFrame+=5*dt;
			move = true;
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

		//if(vVelocity.x>-75)
		//	vVelocity = vVelocity + CVec(-speed,0);
		//fFrame+=5*dt;
		move = true;
	}


	int oldskool = tLXOptions->iOldSkoolRope;

	int jumpdownonce = cJump.isDownOnce();

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
			iRopeDown = false;
		}

		if(cSelWeapon.isDown() && cJump.isDown() && !iRopeDown) {

			iRopeDownOnce = true;
			iRopeDown = true;
		}

		// Down
		if(iRopeDownOnce) {
			iRopeDownOnce = false;

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
	bool jump = false;
	gs_worm_t *wd = cGameScript->getWorm();

	float	fFrameRate = 7.5f;

	worm_state_t *ws = &tState;

	// Simulate the viewport
	//cViewport.Process(vPos,map->GetWidth(),map->GetHeight());


	if(!local) {
		fAngle = (float)ws->iAngle;
		iDirection = ws->iDirection;
		//vPos = CVec((float)ws->iX, (float)ws->iY);
	}


	// If we're IT, spawn some sparkles
	if(iTagIT && tGameInfo.iGameMode == GMT_TAG) {
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

	if(iOnGround)
		speed = wd->GroundSpeed;
	else
		speed = wd->AirSpeed;


	// Process the ninja rope
	if(cNinjaRope.isReleased() && worms) {
		cNinjaRope.Simulate(dt,pcMap,vPos,worms,iID);
		vVelocity += cNinjaRope.GetForce(vPos)*dt;
	}


	// If we are hooked, we need to be pulled in the direction of the other worm
	if(iHooked && pcHookWorm) {
		// TODO: please be more precice: was is wrong here? and why is this not changed?
		// RE: I didn't add it here, i thought it was you...
		// FIXME: This isn't 'right'
		//vVelocity = vVelocity + cNinjaRope.CalculateForce(vPos,pcHookWorm->getPos())*dt;
	}



	// Process the carving
	if(ws->iCarve) {
		iDirtCount += CarveHole(pcMap, vPos + dir*4);
		//cClient->SendCarve(vPos + dir*4);
	}


	// Process the moving
	if(ws->iMove) {
		if(iDirection == DIR_RIGHT) {

			// Right
			if(vVelocity.x<30)
				vVelocity.x += speed;
			fFrame += fFrameRate * dt;
		} else {

			// Left
			if(vVelocity.x>-30)
				vVelocity.x -= speed;
			fFrame += fFrameRate * dt;
		}
	}

	if(cStrafe.isDown())
		iDirection = iStrafeDirection;


	// Process the jump
	if(ws->iJump) {
		if(CheckOnGround()) {
			//vVelocity.x=(0);
			vVelocity.y = wd->JumpForce;
			iOnGround = false;
			jump = true;
		}
	}


	// Air drag (Mainly to dampen the ninja rope)
	float Drag = wd->AirFriction;

	if(!iOnGround)	{
		vVelocity.x -= SQR(vVelocity.x) * SIGN(vVelocity.x) * Drag * dt;
		vVelocity.y += -SQR(vVelocity.y) * SIGN(vVelocity.y) * Drag * dt;
	}


	if(fFrame>=3.0f || !ws->iMove)
		fFrame=0;


	// Gravity
	vVelocity.y += wd->Gravity*dt;


	// Check collisions

	// TODO: Use a projectile style collision system based on velocity speed
	vOldPos = vPos;
	vPos += vVelocity * dt;

	//resetFollow(); // reset follow here, projectiles will maybe re-enable it...

	CheckWormCollision( dt, vOldPos, &vVelocity, jump );



	// Ultimate in friction
	if(iOnGround) {
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
bool CWorm::CheckWormCollision( float dt, CVec pos, CVec *vel, int jump )
{
	static const int maxspeed2 = 10;

	// If the worm is going too fast, divide the speed by 2 and perform 2 collision checks
	if( (*vel*dt).GetLength2() > maxspeed2) {
		dt /= 2;
		if(CheckWormCollision(dt,pos,vel,jump)) return true;
		return CheckWormCollision(dt,vPos,vel,jump);
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

	iOnGround = false;

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
                iOnGround = true;
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
				iOnGround = true;

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
