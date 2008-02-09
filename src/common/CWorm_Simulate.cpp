/////////////////////////////////////////
//
//             OpenLieroX
//
//    Worm class - input handling
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// TODO: rename this file (only input handling here)

// Created 2/8/02
// Jason Boettcher

#include <iostream>

#include "LieroX.h"
#include "GfxPrimitives.h"
#include "InputEvents.h"
#include "CWorm.h"
#include "MathLib.h"
#include "Entity.h"
#include "CClient.h"
#include "Utils.h"


using namespace std;


///////////////////
// Get the input from a human worm
void CWorm::getInput()
{
	// TODO: it seems that iCarving isn't used at all. if this is the case, please remove. else, something has to be fixed

	// HINT: we are calling this from simulateWorm and we use this fixed dt there
	const static float	dt = 0.01f;
	
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
	
	/*
		if(!cRight.isDown())
			iCarving &= ~1; // disable right carving
		if(!cLeft.isDown())
			iCarving &= ~2; // disable left carving
	
		// Carving hack
		RightOnce = cRight.isDownOnce();
		if(cLeft.isDown() && RightOnce && iDirection == DIR_LEFT)  {
			iCarving |= 2; // carve left
		}
	*/
		
/*		// this is a bit unfair to keyboard players
		if(mouseControl) { // mouseControl
			if(fabs(fMoveSpeedX) > 200) {
				ws->iCarve = true;
				iCarving |= (fMoveSpeedX > 0) ? 1 : 2;
			}
		} */
		
		static const float movetimed_min = 0.08f, movetimed_max = 0.2f;
		
		if((mouseControl && ws->iMove && iDirection == DIR_LEFT)
		|| (cLeft.isJoystick() && cLeft.isDown())) {
			float movetimed = tLX->fCurTime - lastMoveTime;
			//printf("movetimed: %f\n", movetimed);
			if(movetimed_min < movetimed && movetimed < movetimed_max) {
				ws->iCarve = true;
				ws->iMove = true;
				iCarving |= 2; // carve left
			}
		}
	
		if((mouseControl && ws->iMove && iDirection == DIR_RIGHT)
		|| (cRight.isJoystick() && cRight.isDown())) {
			float movetimed = tLX->fCurTime - lastMoveTime;
			//printf("movetimed: %f\n", movetimed);
			if(movetimed_min < movetimed && movetimed < movetimed_max) {
				ws->iCarve = true;
				ws->iMove = true;
				iCarving |= 1; // carve right
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

	// do it here to ensure that it is called exactly once in a frame (needed because of intern handling)
	bool leftOnce = cLeft.isDownOnce();
	bool rightOnce = cRight.isDownOnce();
	
	if(cLeft.isDown()) {
		ws->iMove = true;
		
		if(!cRight.isDown()) iDirection = DIR_LEFT;
		
		if(rightOnce) {
			ws->iCarve = true;
			iCarving |= 2; // carve left
		}
	}
	
	if(cRight.isDown()) {
		ws->iMove = true;
		
		if(!cLeft.isDown()) iDirection = DIR_RIGHT;
		
		if(leftOnce) {
			ws->iCarve = true;
			iCarving |= 1; // carve right
		}
	}
	

/*

	// Right
	if((cRight.isDown() && !(iCarving & 2) && !weap)  || (iType == PRF_COMPUTER && !(iCarving & 2))) {

		// Check if we dig a small hole
		if(cLeft.isDownOnce() && iDirection == DIR_RIGHT) {
			ws->iCarve = true;
			iCarving |= 1;
		}

		if(!cLeft.isDown() || iDirection == DIR_RIGHT) {
			iDirection = DIR_RIGHT;
			ws->iMove = true;
			lastMoveTime = tLX->fCurTime;
		}
	}

	// Left
	if((cLeft.isDown() && !(iCarving & 1) && !weap) || (iType == PRF_COMPUTER && !(iCarving & 1))) {

		// Check if we dig a small hole
		if(RightOnce && iDirection == DIR_LEFT) {
			ws->iCarve = true;
			iCarving |= 2;
		}

		iDirection = DIR_LEFT;
		ws->iMove = true;
		lastMoveTime = tLX->fCurTime;
	}

*/

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






int MouseX = -1, MouseY = -1;


///////////////////
// Use a mouse for worm input
// HINT: we don't use this function
// it is the original function by JasonB
// atm it is just kept to compare with current implementation
// TODO: remove this function later
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
