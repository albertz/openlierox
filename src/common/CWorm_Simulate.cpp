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
#include "CServerConnection.h"
#include "Utils.h"


using namespace std;


///////////////////
// Get the input from a human worm
void CWorm::getInput()
{
	// HINT: we are calling this from simulateWorm
	const float	dt = tLX->fCurTime - fLastInputTime;
	fLastInputTime = tLX->fCurTime;

	CVec	dir;
	int		weap = false;

	mouse_t *ms = GetMouse();

	// do it here to ensure that it is called exactly once in a frame (needed because of intern handling)
	bool leftOnce = cLeft.isDownOnce();
	bool rightOnce = cRight.isDownOnce();

	worm_state_t *ws = &tState;

	// Init the ws
	ws->bCarve = false;
	ws->bMove = false;
	ws->bShoot = false;
	ws->bJump = false;

	const bool mouseControl = 
			tLXOptions->bMouseAiming &&
			( cClient->isHostAllowingMouse() || tGameInfo.iGameType == GME_LOCAL) &&
			ApplicationHasFocus(); // if app has no focus, don't use mouseaiming, the mousewarping is pretty annoying then
	const float mouseSensity = (float)tLXOptions->iMouseSensity; // how sensitive is the mouse in X/Y-dir

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
		static const float joystickCoeff = 150.0f/65536.0f;
		static const float joystickShift = 15; // 15 degrees

		// Joystick up
		if (cDown.isJoystickThrottle())  {
			fAngleSpeed = 0;
			fAngle = CLAMP((float)cUp.getJoystickValue() * joystickCoeff - joystickShift, -90.0f, 60.0f);
		}

		// Joystick down
		if (cDown.isJoystickThrottle())  {
			fAngleSpeed = 0;
			fAngle = CLAMP((float)cUp.getJoystickValue() * joystickCoeff - joystickShift, -90.0f, 60.0f);
		}

		// Up
		if(cUp.isDown() && !cUp.isJoystickThrottle()) {
			// HINT: 500 is the original value here (rev 1)
			fAngleSpeed -= 500 * dt;
		} else if(cDown.isDown() && !cDown.isJoystickThrottle()) { // Down
			// HINT: 500 is the original value here (rev 1)
			fAngleSpeed += 500 * dt;
		} else {
			if(!mouseControl) {
				// HINT: this is the original order and code (before mouse patch - rev 1007)
				CLAMP_DIRECT(fAngleSpeed, -100.0f, 100.0f);
				REDUCE_CONST(fAngleSpeed, 200*dt);
				RESET_SMALL(fAngleSpeed, 5.0f);

			} else { // mouseControl for angle
				// HINT: to behave more like keyboard, we should use CLAMP(..500) here
				float diff = mouse_dy * mouseSensity;
				CLAMP_DIRECT(diff, -500.0f, 500.0f); // same limit as keyboard
				fAngleSpeed += diff * dt;

				// this tries to be like keyboard where this code is only applied if up/down is not pressed
				if(abs(mouse_dy) < 5) {
					CLAMP_DIRECT(fAngleSpeed, -100.0f, 100.0f);
					REDUCE_CONST(fAngleSpeed, 200*dt);
					RESET_SMALL(fAngleSpeed, 5.0f);
				}
			}
		}

		fAngle += fAngleSpeed * dt;
		if(CLAMP_DIRECT(fAngle, -90.0f, 60.0f) != 0)
			fAngleSpeed = 0;

		// Calculate dir
		dir.x=( (float)cos(fAngle * (PI/180)) );
		dir.y=( (float)sin(fAngle * (PI/180)) );
		if( iMoveDirection == DIR_LEFT ) // Fix: Ninja rope shoots backwards when you strafing or mouse-aiming
			dir.x=(-dir.x);

	} // end angle section


	// basic mouse control (moving)
	if(mouseControl) {
		// no dt here, it's like the keyboard; and the value will be limited by dt later
		fMoveSpeedX += mouse_dx * mouseSensity * 0.01f;

		REDUCE_CONST(fMoveSpeedX, 1000*dt);
		//RESET_SMALL(fMoveSpeedX, 5.0f);
		CLAMP_DIRECT(fMoveSpeedX, -500.0f, 500.0f);

		if(fabs(fMoveSpeedX) > 50) {
			if(fMoveSpeedX > 0) {
				iMoveDirection = DIR_RIGHT;
				if(mouse_dx < 0) lastMoveTime = tLX->fCurTime;
			} else {
				iMoveDirection = DIR_LEFT;
				if(mouse_dx > 0) lastMoveTime = tLX->fCurTime;
			}
			ws->bMove = true;
			if(!cClient->isHostAllowingStrafing() || !cStrafe.isDown()) iDirection = iMoveDirection;

		} else {
			ws->bMove = false;
		}

	}


	if(mouseControl) { // set shooting, ninja and jumping, weapon selection for mousecontrol
		// like Jason did it
		ws->bShoot = (ms->Down & SDL_BUTTON(1)) ? true : false;
		ws->bJump = (ms->Down & SDL_BUTTON(3)) ? true : false;
		if(ws->bJump) {
			if(cNinjaRope.isReleased())
				cNinjaRope.Release();
		}
		else if(ms->FirstDown & SDL_BUTTON(2)) {
			// TODO: this is bad. why isn't there a ws->iNinjaShoot ?
			cNinjaRope.Shoot(vPos, dir);
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

/*		// this is a bit unfair to keyboard players
		if(mouseControl) { // mouseControl
			if(fabs(fMoveSpeedX) > 200) {
				ws->iCarve = true;
			}
		} */

		const float carveDelay = 0.2f;

		if((mouseControl && ws->bMove && iMoveDirection == DIR_LEFT)
			|| ((( cLeft.isJoystick() && cLeft.isDown()) || (cLeft.isKeyboard() && leftOnce)) && !cSelWeapon.isDown())) {

			if(tLX->fCurTime - fLastCarve >= carveDelay) {
				ws->bCarve = true;
				ws->bMove = true;
				fLastCarve = tLX->fCurTime;
			}
		}

		if((mouseControl && ws->bMove && iMoveDirection == DIR_RIGHT)
			|| ((( cRight.isJoystick() && cRight.isDown()) || (cRight.isKeyboard() && rightOnce)) && !cSelWeapon.isDown())) {

			if(tLX->fCurTime - fLastCarve >= carveDelay) {
				ws->bCarve = true;
				ws->bMove = true;
				fLastCarve = tLX->fCurTime;
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
		int change = (rightOnce ? 1 : 0) - (leftOnce ? 1 : 0);
		iCurrentWeapon += change;
		MOD(iCurrentWeapon, iNumWeaponSlots);

		// Joystick: if the button is pressed, change the weapon (it is annoying to move the axis for weapon changing)
		if (cSelWeapon.isJoystick() && change == 0 && cSelWeapon.isDownOnce())  {
			iCurrentWeapon++;
			MOD(iCurrentWeapon, iNumWeaponSlots);
		}
	}

	// Process weapon quick-selection keys
	for(size_t i = 0; i < sizeof(cWeapons) / sizeof(cWeapons[0]); i++ )
	{
		if( cWeapons[i].isDown() )
		{
			iCurrentWeapon = i;
			// Let the weapon name show up for a short moment
			bForceWeapon_Name = true;
			fForceWeapon_Time = tLX->fCurTime + 0.75f;
		}
	}


	// Safety: clamp the current weapon
	iCurrentWeapon = CLAMP(iCurrentWeapon, 0, iNumWeaponSlots-1);


	ws->bShoot = cShoot.isDown();

	if(!cSelWeapon.isDown()) {
		if(cLeft.isDown()) {
			ws->bMove = true;
			lastMoveTime = tLX->fCurTime;

			if(!cRight.isDown()) {
				if(!cClient->isHostAllowingStrafing() || !cStrafe.isDown()) iDirection = DIR_LEFT;
				iMoveDirection = DIR_LEFT;
			}

			if(rightOnce) {
				ws->bCarve = true;
				fLastCarve = tLX->fCurTime;
			}
		}

		if(cRight.isDown()) {
			ws->bMove = true;
			lastMoveTime = tLX->fCurTime;

			if(!cLeft.isDown()) {
				if(!cClient->isHostAllowingStrafing() || !cStrafe.isDown()) iDirection = DIR_RIGHT;
				iMoveDirection = DIR_RIGHT;
			}

			if(leftOnce) {
				ws->bCarve = true;
				fLastCarve = tLX->fCurTime;
			}
		}

		// inform player about disallowed strafing
		if(!cClient->isHostAllowingStrafing() && cStrafe.isDownOnce())
			// TODO: perhaps in chat?
			printf("HINT: strafing is not allowed on this server.\n");
	}


	bool oldskool = tLXOptions->bOldSkoolRope;

	bool jumpdownonce = cJump.isDownOnce();

	// Jump
	if(jumpdownonce) {
		if( !(oldskool && cSelWeapon.isDown()) )  {
			ws->bJump = true;

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


	cUp.reset();
	cDown.reset();
	cLeft.reset();
	cRight.reset();
	cShoot.reset();
	cJump.reset();
	cSelWeapon.reset();
	cInpRope.reset();
	cStrafe.reset();
	for( size_t i = 0; i < sizeof(cWeapons) / sizeof(cWeapons[0]) ; i++  )
		cWeapons[i].reset();
}


///////////////////
// Clear the input
void CWorm::clearInput(void)
{
	fLastInputTime = tLX->fCurTime;

	// Clear the state
	tState.bCarve = false;
	tState.bMove  = false;
	tState.bShoot = false;
	tState.bJump  = false;

	// clear inputs
	cUp.reset();
	cDown.reset();
	cLeft.reset();
	cRight.reset();
	cShoot.reset();
	cJump.reset();
	cSelWeapon.reset();
	cInpRope.reset();
	cStrafe.reset();
	for( size_t i = 0; i < sizeof(cWeapons) / sizeof(cWeapons[0]) ; i++  )
		cWeapons[i].reset();
}





///////////////////
// Setup the inputs
void CWorm::SetupInputs(const controls_t& Inputs)
{
	bUsesMouse = false;
	for (byte i=0;i<Inputs.ControlCount(); i++)
		if (Inputs[i].find("ms"))  {
			bUsesMouse = true;
			break;
		}

	cUp.Setup(		Inputs[SIN_UP] );
	cDown.Setup(	Inputs[SIN_DOWN] );
	cLeft.Setup(	Inputs[SIN_LEFT] );
	cRight.Setup(	Inputs[SIN_RIGHT] );

	cShoot.Setup(	Inputs[SIN_SHOOT] );
	cJump.Setup(	Inputs[SIN_JUMP] );
	cSelWeapon.Setup(Inputs[SIN_SELWEAP] );
	cInpRope.Setup(	Inputs[SIN_ROPE] );

	cStrafe.Setup( Inputs[SIN_STRAFE] );

	for( size_t i = 0; i < sizeof(cWeapons) / sizeof(cWeapons[0]) ; i++  )
		cWeapons[i].Setup(Inputs[SIN_WEAPON1 + i]);
}


void CWorm::InitInputSystem() {
	cUp.setResetEachFrame( false );
	cDown.setResetEachFrame( false );
	cLeft.setResetEachFrame( false );
	cRight.setResetEachFrame( false );
	cShoot.setResetEachFrame( false );
	cJump.setResetEachFrame( false );
	cSelWeapon.setResetEachFrame( false );
	cInpRope.setResetEachFrame( false );
	cStrafe.setResetEachFrame( false );
	for( size_t i = 0; i < sizeof(cWeapons) / sizeof(cWeapons[0]) ; i++  )
		cWeapons[i].setResetEachFrame( false );
}

void CWorm::StopInputSystem() {
	cUp.setResetEachFrame( true );
	cDown.setResetEachFrame( true );
	cLeft.setResetEachFrame( true );
	cRight.setResetEachFrame( true );
	cShoot.setResetEachFrame( true );
	cJump.setResetEachFrame( true );
	cSelWeapon.setResetEachFrame( true );
	cInpRope.setResetEachFrame( true );
	cStrafe.setResetEachFrame( true );
	for( size_t i = 0; i < sizeof(cWeapons) / sizeof(cWeapons[0]) ; i++  )
		cWeapons[i].setResetEachFrame( true );
}
