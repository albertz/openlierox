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



#include "LieroX.h"
#include "GfxPrimitives.h"
#include "InputEvents.h"
#include "CWorm.h"
#include "MathLib.h"
#include "Entity.h"
#include "CClient.h"
#include "CServerConnection.h"
#include "CWormHuman.h"
#include "ProfileSystem.h"
#include "CGameScript.h"
#include "Debug.h"
#include "CGameMode.h"
#include "Physics.h"
#include "WeaponDesc.h"
#include "AuxLib.h" // for doActionInMainThread
#include "game/Game.h"
#include "gusanos/player_input.h"
#include "sound/SoundsBase.h"

#ifdef __MINGW32_VERSION
// TODO: ugly hack, fix it - mingw stdlib seems to be broken
#define powf(x,y) ((float)pow((double)x,(double)y))
#endif

///////////////////
// Get the input from a human worm
void CWormHumanInputHandler::getInput() {		
	// HINT: we are calling this from simulateWorm
	TimeDiff dt = GetPhysicsTime() - m_worm->fLastInputTime;
	m_worm->fLastInputTime = GetPhysicsTime();

	int		weap = false;

	mouse_t *ms = GetMouse();

	// do it here to ensure that it is called exactly once in a frame (needed because of intern handling)
	bool leftOnce = cLeft.isDownOnce();
	bool rightOnce = cRight.isDownOnce();

	worm_state_t *ws = &m_worm->tState;

	// Init the ws
	ws->bCarve = false;
	ws->bMove = false;
	ws->bShoot = false;
	if(!(bool)cClient->getGameLobby()[FT_GusanosWormPhysics]) // Gusanos worm physics behaves slightly different for jumping input
		ws->bJump = false;

	const bool mouseControl = 
			tLXOptions->bMouseAiming &&
			ApplicationHasFocus(); // if app has no focus, don't use mouseaiming, the mousewarping is pretty annoying then
	const float mouseSensity = (float)tLXOptions->iMouseSensity; // how sensitive is the mouse in X/Y-dir

	// TODO: here are width/height of the window hardcoded
	//int mouse_dx = ms->X - 640/2;
	//int mouse_dy = ms->Y - 480/2;
	int mouse_dx = 0, mouse_dy = 0;
	SDL_GetRelativeMouseState(&mouse_dx, &mouse_dy); // Won't lose mouse movement and skip frames, also it doesn't call libX11 funcs, so it's safe to call not from video thread
	
	if(mouseControl)
	{
		struct CenterMouse: public Action
		{
			int handle()
			{
				SDL_WarpMouse(640/2, 480/2); // Should be called from main thread, or you'll get race condition with libX11
				return 0;
			} 
		};
		doActionInMainThread( new CenterMouse() );
	}

	{
/*		// only some debug output for checking the values
		if(mouseControl && (mouse_dx != 0 || mouse_dy != 0))
			notes("mousepos changed: %i, %i\n", mouse_dx, mouse_dy),
			notes("anglespeed: %f\n", fAngleSpeed),
			notes("movespeed: %f\n", fMoveSpeedX),
			notes("dt: %f\n", dt); */
	}

	// angle section
	{
		static const float joystickCoeff = 150.0f/65536.0f;
		static const float joystickShift = 15; // 15 degrees

		// Joystick up
		if (cDown.isJoystickThrottle())  {
			m_worm->fAngleSpeed = 0;
			m_worm->fAngle = CLAMP((float)cUp.getJoystickValue() * joystickCoeff - joystickShift, -90.0f, 60.0f);
		}

		// Joystick down
		if (cDown.isJoystickThrottle())  {
			m_worm->fAngleSpeed = 0;
			m_worm->fAngle = CLAMP((float)cUp.getJoystickValue() * joystickCoeff - joystickShift, -90.0f, 60.0f);
		}

		float aimMaxSpeed = MAX((float)fabs(tLXOptions->fAimMaxSpeed), 20.0f); // Note: 100 was LX56 max aimspeed
		float aimAccel = MAX((float)fabs(tLXOptions->fAimAcceleration), 100.0f); // HINT: 500 is the LX56 value here (rev 1)
		float aimFriction = CLAMP(tLXOptions->fAimFriction, 0.0f, 1.0f); // we didn't had that in LX56; it behaves more natural
		bool aimLikeLX56 = tLXOptions->bAimLikeLX56;
		if(cClient->getGameLobby()[FT_ForceLX56Aim] || cClient->getServerVersion() < OLXRcVersion(0,58,3)) {
			aimMaxSpeed = 100;
			aimAccel = 500;
			aimLikeLX56 = true;
		}
		
		if(cUp.isDown() && !cUp.isJoystickThrottle()) { // Up
			m_worm->fAngleSpeed -= aimAccel * dt.seconds();
			if(!aimLikeLX56) CLAMP_DIRECT(m_worm->fAngleSpeed, -aimMaxSpeed, aimMaxSpeed);
		} else if(cDown.isDown() && !cDown.isJoystickThrottle()) { // Down
			m_worm->fAngleSpeed += aimAccel * dt.seconds();
			if(!aimLikeLX56) CLAMP_DIRECT(m_worm->fAngleSpeed, -aimMaxSpeed, aimMaxSpeed);
		} else {			
			if(!mouseControl) {
				// HINT: this is the original order and code (before mouse patch - rev 1007)
				CLAMP_DIRECT(m_worm->fAngleSpeed, -aimMaxSpeed, aimMaxSpeed);
				if(aimLikeLX56) REDUCE_CONST(m_worm->fAngleSpeed, 200*dt.seconds());
				else m_worm->fAngleSpeed *= powf(aimFriction, dt.seconds() * 100.0f);
				RESET_SMALL(m_worm->fAngleSpeed, 5.0f);

			} else { // mouseControl for angle
				// HINT: to behave more like keyboard, we use CLAMP(..aimAccel) here
				float diff = mouse_dy * mouseSensity;
				CLAMP_DIRECT(diff, -aimAccel, aimAccel); // same limit as keyboard
				m_worm->fAngleSpeed += diff * dt.seconds();

				// this tries to be like keyboard where this code is only applied if up/down is not pressed
				if(abs(mouse_dy) < 5) {
					CLAMP_DIRECT(m_worm->fAngleSpeed, -aimMaxSpeed, aimMaxSpeed);
					if(aimLikeLX56) REDUCE_CONST(m_worm->fAngleSpeed, 200*dt.seconds());
					else m_worm->fAngleSpeed *= powf(aimFriction, dt.seconds() * 100.0f);
					RESET_SMALL(m_worm->fAngleSpeed, 5.0f);
				}
			}
		}

		m_worm->fAngle += m_worm->fAngleSpeed * dt.seconds();
		if(CLAMP_DIRECT(m_worm->fAngle, -90.0f, cClient->getGameLobby()[FT_FullAimAngle] ? 90.0f : 60.0f) != 0)
			m_worm->fAngleSpeed = 0;

	} // end angle section

	const CVec ninjaShootDir = m_worm->getFaceDirection();

	// basic mouse control (moving)
	if(mouseControl) {
		// no dt here, it's like the keyboard; and the value will be limited by dt later
		m_worm->fMoveSpeedX += mouse_dx * mouseSensity * 0.01f;

		REDUCE_CONST(m_worm->fMoveSpeedX, 1000*dt.seconds());
		//RESET_SMALL(m_worm->fMoveSpeedX, 5.0f);
		CLAMP_DIRECT(m_worm->fMoveSpeedX, -500.0f, 500.0f);

		if(fabs(m_worm->fMoveSpeedX) > 50) {
			if(m_worm->fMoveSpeedX > 0) {
				m_worm->iMoveDirectionSide = DIR_RIGHT;
				if(mouse_dx < 0) m_worm->lastMoveTime = tLX->currentTime;
			} else {
				m_worm->iMoveDirectionSide = DIR_LEFT;
				if(mouse_dx > 0) m_worm->lastMoveTime = tLX->currentTime;
			}
			ws->bMove = true;
			if(!cClient->isHostAllowingStrafing() || !cStrafe.isDown())
				m_worm->iFaceDirectionSide = m_worm->iMoveDirectionSide;

		} else {
			ws->bMove = false;
		}

	}


	if(mouseControl) { // set shooting, ninja and jumping, weapon selection for mousecontrol
		// like Jason did it
		ws->bShoot = (ms->Down & SDL_BUTTON(1)) ? true : false;
		ws->bJump = (ms->Down & SDL_BUTTON(3)) ? true : false;
		if(ws->bJump) {
			if(m_worm->cNinjaRope.isReleased())
				m_worm->cNinjaRope.Release();
		}
		else if(ms->FirstDown & SDL_BUTTON(2)) {
			// TODO: this is bad. why isn't there a ws->iNinjaShoot ?
			m_worm->cNinjaRope.Shoot(m_worm, m_worm->vPos, ninjaShootDir);
			PlaySoundSample(sfxGame.smpNinja);
		}

		if( ms->WheelScrollUp || ms->WheelScrollDown ) {
			m_worm->bForceWeapon_Name = true;
			m_worm->fForceWeapon_Time = tLX->currentTime + 0.75f;
			if( ms->WheelScrollUp )
				m_worm->iCurrentWeapon ++;
			else
				m_worm->iCurrentWeapon --;
			if(m_worm->iCurrentWeapon >= m_worm->iNumWeaponSlots)
				m_worm->iCurrentWeapon=0;
			if(m_worm->iCurrentWeapon < 0)
				m_worm->iCurrentWeapon=m_worm->iNumWeaponSlots-1;
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

		if(		(mouseControl && ws->bMove && m_worm->iMoveDirectionSide == DIR_LEFT)
			||	( ( (cLeft.isJoystick() && cLeft.isDown()) /*|| (cLeft.isKeyboard() && leftOnce)*/ ) && !cSelWeapon.isDown())
			) {

			if(tLX->currentTime - m_worm->fLastCarve >= carveDelay) {
				ws->bCarve = true;
				ws->bMove = true;
				m_worm->fLastCarve = tLX->currentTime;
			}
		}

		if(		(mouseControl && ws->bMove && m_worm->iMoveDirectionSide == DIR_RIGHT)
			||	( ( (cRight.isJoystick() && cRight.isDown()) /*|| (cRight.isKeyboard() && rightOnce)*/ ) && !cSelWeapon.isDown())
			) {

			if(tLX->currentTime - m_worm->fLastCarve >= carveDelay) {
				ws->bCarve = true;
				ws->bMove = true;
				m_worm->fLastCarve = tLX->currentTime;
			}
		}
	}

	
	bool allocombo = cClient->getGameLobby()[FT_WeaponCombos];
	
	ws->bShoot = cShoot.isDown();

	if(!ws->bShoot || allocombo) {
		//
		// Weapon changing
		//
		if(cSelWeapon.isDown()) {
			// TODO: was is the intention of this var? if weapon change, then it's wrong
			// if cSelWeapon.isDown(), then we don't need it
			weap = true;

			// we don't want keyrepeats here, so only count the first down-event
			int change = (rightOnce ? 1 : 0) - (leftOnce ? 1 : 0);
			m_worm->iCurrentWeapon += change;
			MOD(m_worm->iCurrentWeapon, m_worm->iNumWeaponSlots);

			// Joystick: if the button is pressed, change the weapon (it is annoying to move the axis for weapon changing)
			if (cSelWeapon.isJoystick() && change == 0 && cSelWeapon.isDownOnce())  {
				m_worm->iCurrentWeapon++;
				MOD(m_worm->iCurrentWeapon, m_worm->iNumWeaponSlots);
			}
		}

		// Process weapon quick-selection keys
		for(size_t i = 0; i < sizeof(cWeapons) / sizeof(cWeapons[0]); i++ )
		{
			if( cWeapons[i].isDown() )
			{
				m_worm->iCurrentWeapon = i;
				// Let the weapon name show up for a short moment
				m_worm->bForceWeapon_Name = true;
				m_worm->fForceWeapon_Time = tLX->currentTime + 0.75f;
			}
		}

	}

	// Safety: clamp the current weapon
	m_worm->iCurrentWeapon = CLAMP(m_worm->iCurrentWeapon, 0, m_worm->iNumWeaponSlots-1);



	if(!cSelWeapon.isDown()) {
		if(cLeft.isDown()) {
			ws->bMove = true;
			m_worm->lastMoveTime = tLX->currentTime;

			if(!cRight.isDown()) {
				if(!cClient->isHostAllowingStrafing() || !cStrafe.isDown()) m_worm->iFaceDirectionSide = DIR_LEFT;
				m_worm->iMoveDirectionSide = DIR_LEFT;
			}

			if(rightOnce) {
				ws->bCarve = true;
				m_worm->fLastCarve = tLX->currentTime;
			}
		}

		if(cRight.isDown()) {
			ws->bMove = true;
			m_worm->lastMoveTime = tLX->currentTime;

			if(!cLeft.isDown()) {
				if(!cClient->isHostAllowingStrafing() || !cStrafe.isDown()) m_worm->iFaceDirectionSide = DIR_RIGHT;
				m_worm->iMoveDirectionSide = DIR_RIGHT;
			}

			if(leftOnce) {
				ws->bCarve = true;
				m_worm->fLastCarve = tLX->currentTime;
			}
		}

		// inform player about disallowed strafing
		if(!cClient->isHostAllowingStrafing() && cStrafe.isDownOnce())
			// TODO: perhaps in chat?
			hints << "strafing is not allowed on this server." << endl;
	}


	const bool oldskool = tLXOptions->bOldSkoolRope;

	const bool jump = cJump.isDownOnce();

	// Jump
	if( !(oldskool && cSelWeapon.isDown()) )  {
		ws->bJump |= jump;

		if(jump && m_worm->cNinjaRope.isReleased())
			m_worm->cNinjaRope.Release();
	}
	
	// Ninja Rope
	if(oldskool) {
		// Old skool style rope throwing
		// Change-weapon & jump

		if(!cSelWeapon.isDown() || !cJump.isDown())  {
			m_worm->bRopeDown = false;
		}

		if(cSelWeapon.isDown() && cJump.isDown() && !m_worm->bRopeDown) {

			m_worm->bRopeDownOnce = true;
			m_worm->bRopeDown = true;
		}

		// Down
		if(m_worm->bRopeDownOnce) {
			m_worm->bRopeDownOnce = false;

			m_worm->cNinjaRope.Shoot(m_worm, m_worm->vPos, ninjaShootDir);

			// Throw sound
			PlaySoundSample(sfxGame.smpNinja);
		}


	} else {
		// Newer style rope throwing
		// Seperate dedicated button for throwing the rope
		if(cInpRope.isDownOnce()) {

			m_worm->cNinjaRope.Shoot(m_worm, m_worm->vPos, ninjaShootDir);
			// Throw sound
			PlaySoundSample(sfxGame.smpNinja);
		}
	}

	ws->iAngle = (int)m_worm->fAngle;
	ws->iX = (int)m_worm->vPos.x;
	ws->iY = (int)m_worm->vPos.y;


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

// This function will get the keypresses for new net engine from different input like mouse/joystic or bot AI
// by comparing worm state variables before and after getInput() call
NewNet::KeyState_t CWorm::NewNet_GetKeys()
{
	CWorm oldState;
	oldState.NewNet_CopyWormState(*this);
	getInput();
	NewNet::KeyState_t ret;

	if( tState.bJump )
		ret.keys[NewNet::K_JUMP] = true;

	if( tState.bShoot )
		ret.keys[NewNet::K_SHOOT] = true;

	if( tState.bCarve )
	{
		ret.keys[NewNet::K_LEFT] = true;
		ret.keys[NewNet::K_RIGHT] = true;
	}
	
	if( tState.bMove )
	{
		if( iMoveDirectionSide == DIR_LEFT )
			ret.keys[NewNet::K_LEFT] = true;
		else
			ret.keys[NewNet::K_RIGHT] = true;
		if( iMoveDirectionSide != iFaceDirectionSide )
			ret.keys[NewNet::K_STRAFE] = true;
	}
	
	if( oldState.fAngle > fAngle )
		ret.keys[NewNet::K_UP] = true;
	if( oldState.fAngle < fAngle )
		ret.keys[NewNet::K_DOWN] = true;

	if( oldState.iCurrentWeapon != iCurrentWeapon )
	{
		// TODO: ignores fast weapon selection keys, they will just not work
		// I'll probably remove K_SELWEAP and add K_SELWEAP_1 - K_SELWEAP_5 "buttons"
		ret.keys[NewNet::K_SELWEAP] = true;
		int WeaponLeft = iCurrentWeapon + 1;
		MOD(WeaponLeft, iNumWeaponSlots);
		if( WeaponLeft == oldState.iCurrentWeapon )
			ret.keys[NewNet::K_LEFT] = true;
		int WeaponRight = iCurrentWeapon - 1;
		MOD(WeaponRight, iNumWeaponSlots);
		if( WeaponRight == oldState.iCurrentWeapon )
			ret.keys[NewNet::K_RIGHT] = true;
	};
	
	return ret;
};

// Synthetic input from new net engine - should modify worm state in the same as CWormHumanInputHandler::getInput()
void CWorm::NewNet_SimulateWorm( NewNet::KeyState_t keys, NewNet::KeyState_t keysChanged ) 
{
	TimeDiff dt ( (int)NewNet::TICK_TIME );

	if( GetPhysicsTime() > fLastInputTime )
		dt = GetPhysicsTime() - fLastInputTime;
	fLastInputTime = GetPhysicsTime();
	
	// do it here to ensure that it is called exactly once in a frame (needed because of intern handling)
	bool leftOnce = keys.keys[NewNet::K_LEFT] && keysChanged.keys[NewNet::K_LEFT];
	bool rightOnce = keys.keys[NewNet::K_RIGHT] && keysChanged.keys[NewNet::K_RIGHT];
	
	worm_state_t *ws = &tState;

	// Init the ws
	ws->bCarve = false;
	ws->bMove = false;
	ws->bShoot = false;
	ws->bJump = false;

	{
		if( keys.keys[NewNet::K_UP] && keys.keys[NewNet::K_DOWN] )
		{
			// Do not change angle speed (precise aiming as in original Liero)
		}
		else if(keys.keys[NewNet::K_UP]) {
			// HINT: 500 is the original value here (rev 1)
			fAngleSpeed -= 500 * dt.seconds();
		} else if(keys.keys[NewNet::K_DOWN]) { // Down
			// HINT: 500 is the original value here (rev 1)
			fAngleSpeed += 500 * dt.seconds();
		} else {
				// HINT: this is the original order and code (before mouse patch - rev 1007)
				CLAMP_DIRECT(fAngleSpeed, -100.0f, 100.0f);
				REDUCE_CONST(fAngleSpeed, 200*dt.seconds());
				RESET_SMALL(fAngleSpeed, 5.0f);

		}

		fAngle += fAngleSpeed * dt.seconds();
		if(CLAMP_DIRECT(fAngle, -90.0f, 60.0f) != 0)
			fAngleSpeed = 0;

	} // end angle section

	{ // set carving

		const float carveDelay = 0.2f;

		if(leftOnce && !keys.keys[NewNet::K_SELWEAP]) {

			if(GetPhysicsTime() >= fLastCarve + carveDelay ) {
				ws->bCarve = true;
				ws->bMove = true;
				fLastCarve = GetPhysicsTime();
			}
		}

		if(rightOnce && !keys.keys[NewNet::K_SELWEAP]) {

			if(GetPhysicsTime() >= fLastCarve + carveDelay ) {
				ws->bCarve = true;
				ws->bMove = true;
				fLastCarve = GetPhysicsTime();
			}
		}
	}

    //
    // Weapon changing
	//
	if(keys.keys[NewNet::K_SELWEAP]) {
		// we don't want keyrepeats here, so only count the first down-event
		int change = (rightOnce ? 1 : 0) - (leftOnce ? 1 : 0);
		iCurrentWeapon += change;
		MOD(iCurrentWeapon, iNumWeaponSlots);
	}

	// Safety: clamp the current weapon
	iCurrentWeapon = CLAMP(iCurrentWeapon, 0, iNumWeaponSlots-1);

	ws->bShoot = keys.keys[NewNet::K_SHOOT];

	if(!keys.keys[NewNet::K_SELWEAP]) {
		if(keys.keys[NewNet::K_LEFT]) {
			ws->bMove = true;
			lastMoveTime = GetPhysicsTime();

			if(!keys.keys[NewNet::K_RIGHT]) {
				if(!cClient->isHostAllowingStrafing() || !keys.keys[NewNet::K_STRAFE]) 
					iFaceDirectionSide = DIR_LEFT;
				iMoveDirectionSide = DIR_LEFT;
			}

			if(rightOnce) {
				ws->bCarve = true;
				fLastCarve = GetPhysicsTime();
			}
		}

		if(keys.keys[NewNet::K_RIGHT]) {
			ws->bMove = true;
			lastMoveTime = GetPhysicsTime();

			if(!keys.keys[NewNet::K_LEFT]) {
				if(!cClient->isHostAllowingStrafing() || !keys.keys[NewNet::K_STRAFE]) 
					iFaceDirectionSide = DIR_RIGHT;
				iMoveDirectionSide = DIR_RIGHT;
			}

			if(leftOnce) {
				ws->bCarve = true;
				fLastCarve = GetPhysicsTime();
			}
		}
	}


	bool jumpdownonce = keys.keys[NewNet::K_JUMP] && keysChanged.keys[NewNet::K_JUMP];

	// Jump
	if(jumpdownonce) {
			ws->bJump = true;

			if(cNinjaRope.isReleased())
				cNinjaRope.Release();
	}

	// Newer style rope throwing
	// Seperate dedicated button for throwing the rope
	if( keys.keys[NewNet::K_ROPE] && keysChanged.keys[NewNet::K_ROPE] ) {
		cNinjaRope.Shoot(this, vPos, getFaceDirection());
		// Throw sound
		if( NewNet::CanPlaySound(getID()) )
			PlaySoundSample(sfxGame.smpNinja);
	}

	ws->iAngle = (int)fAngle;
	ws->iX = (int)vPos.x;
	ws->iY = (int)vPos.y;

	// Clean up expired damage report values
	if( tLXOptions->bColorizeDamageByWorm )
	{
		for( std::map<int, DamageReport> ::iterator it = cDamageReport.begin(); it != cDamageReport.end(); )
		{
			if( GetPhysicsTime() > it->second.lastTime + 1.5f )
			{
				cDamageReport.erase(it);
				it = cDamageReport.begin();
			}
			else
				it++;
		}
	}
	else
	{
		std::map< AbsTime, int > DamageReportDrawOrder;
		for( std::map<int, DamageReport> ::iterator it = cDamageReport.begin(); it != cDamageReport.end(); it++ )
				DamageReportDrawOrder[it->second.lastTime] = it->first;
		if( ! DamageReportDrawOrder.empty() && GetPhysicsTime() > DamageReportDrawOrder.begin()->first + 1.5f )
				cDamageReport.clear();
	}
}


///////////////////
// Clear the input
void CWormHumanInputHandler::clearInput() {
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



struct HumanWormType : WormType {
	virtual CWormInputHandler* createInputHandler(CWorm* w) { return new CWormHumanInputHandler(w); }
	int toInt() { return 0; }
} PRF_HUMAN_instance;
WormType* PRF_HUMAN = &PRF_HUMAN_instance;

CWormHumanInputHandler::CWormHumanInputHandler(CWorm* w) : CWormInputHandler(w) {	
	// we use the normal init system first after the weapons are selected and we are ready
	stopInputSystem();

	gusInit();
	
	game.onNewPlayer( this );
	game.onNewHumanPlayer( this );
	game.onNewHumanPlayer_Lua( this );
	game.onNewPlayer_Lua(this);
}

CWormHumanInputHandler::~CWormHumanInputHandler() {
	gusShutdown();
}

void CWormHumanInputHandler::startGame() {
	initInputSystem();
}



///////////////////
// Setup the inputs
void CWormHumanInputHandler::setupInputs(const PlyControls& Inputs)
{
	//bUsesMouse = false;
	for (byte i=0;i<Inputs.ControlCount(); i++)
		if (Inputs[i].find("ms"))  {
			//bUsesMouse = true;
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


void CWormHumanInputHandler::initInputSystem() {
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

void CWormHumanInputHandler::stopInputSystem() {
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





///////////////////
// Initialize the weapon selection screen
void CWormHumanInputHandler::initWeaponSelection() {
	// the way we handle the inputs in wpn selection is different
	stopInputSystem();
	
	// This is used for the menu screen as well
	m_worm->iCurrentWeapon = 0;
	
	m_worm->bWeaponsReady = false;
	
	m_worm->iNumWeaponSlots = 5;
	
	m_worm->clearInput();
	
	// Safety
	if (!m_worm->tProfile)  {
		errors << "initWeaponSelection called and tProfile is not set" << endl;
		return;
	}
	
	// Load previous settings from profile
	short i;
	for(i=0;i<m_worm->iNumWeaponSlots;i++) {
		
		m_worm->tWeapons[i].Weapon = game.gameScript()->FindWeapon( m_worm->tProfile->sWeaponSlots[i] );
		
        // If this weapon is not enabled in the restrictions, find another weapon that is enabled
        if( !m_worm->tWeapons[i].Weapon || !game.weaponRestrictions()->isEnabled( m_worm->tWeapons[i].Weapon->Name ) ) {
			m_worm->tWeapons[i].Weapon = game.gameScript()->FindWeapon( game.weaponRestrictions()->findEnabledWeapon( game.gameScript()->GetWeaponList() ) );
        }
		
		m_worm->tWeapons[i].Enabled = m_worm->tWeapons[i].Weapon != NULL;
	}
	
	
	for(short n=0;n<m_worm->iNumWeaponSlots;n++) {
		m_worm->tWeapons[n].Charge = 1;
		m_worm->tWeapons[n].Reloading = false;
		m_worm->tWeapons[n].SlotNum = n;
		m_worm->tWeapons[n].LastFire = 0;
	}
	
	// Skip the dialog if there's only one weapon available
	int enabledWeaponsAmount = 0;
	for( int f = 0; f < game.gameScript()->GetNumWeapons(); f++ )
		if( game.weaponRestrictions()->isEnabled( game.gameScript()->GetWeapons()[f].Name ) )
			enabledWeaponsAmount++;
	
	if( enabledWeaponsAmount <= 1 ) // server can ban ALL weapons, noone will be able to shoot then
		m_worm->bWeaponsReady = true;
}


///////////////////
// Draw/Process the weapon selection screen
void CWormHumanInputHandler::doWeaponSelectionFrame(SDL_Surface * bmpDest, CViewport *v)
{
	// TODO: this should also be used for selecting the weapons for the bot (but this in CWorm_AI then)
	// TODO: reduce local variables in this function
	// TODO: make this function shorter
	// TODO: give better names to local variables
		
	if(bDedicated) {
		warnings << "doWeaponSelectionFrame: we have a local human input in our dedicated server" << endl; 
		return; // just for safty; atm this function only handles non-bot players
	}

	// do that check here instead of initWeaponSelection() because at that time,
	// not all params of the gamemode are set
	if(cClient->getGameLobby()[FT_GameMode].as<GameModeInfo>()->mode && !cClient->getGameLobby()[FT_GameMode].as<GameModeInfo>()->mode->Shoot(m_worm)) {
		// just skip weapon selection in game modes where shooting is not possible (e.g. hidenseek)
		m_worm->bWeaponsReady = true;
		return;
	}
	
	int l = 0;
	int t = 0;
	short i;
	int centrex = 320; // TODO: hardcoded screen width here
	
    if( v ) {
        if( v->getUsed() ) {
            l = v->GetLeft();
	        t = v->GetTop();
            centrex = v->GetLeft() + v->GetVirtW()/2;
        }
		
		DrawRectFill(bmpDest, l, t, l + v->GetVirtW(), t + v->GetVirtH(), Color(0,0,0,100));
    }
		
	tLX->cFont.DrawCentre(bmpDest, centrex, t+30, tLX->clWeaponSelectionTitle, "~ Weapons Selection ~");
		
	tLX->cFont.DrawCentre(bmpDest, centrex, t+48, tLX->clWeaponSelectionTitle, "(Use up/down and left/right for selection.)");
	tLX->cFont.DrawCentre(bmpDest, centrex, t+66, tLX->clWeaponSelectionTitle, "(Go to 'Done' and press shoot then.)");
	//tLX->cOutlineFont.DrawCentre(bmpDest, centrex, t+30, tLX->clWeaponSelectionTitle, "Weapons Selection");
	//tLX->cOutlineFont.DrawCentre(bmpDest, centrex, t+30, tLX->clWeaponSelectionTitle, "Weapons Selection");
	
	bool bChat_Typing = cClient->isTyping();
	
	int y = t + 100;
	for(i=0;i<m_worm->iNumWeaponSlots;i++) {
		
		std::string slotDesc;
		if(m_worm->tWeapons[i].Weapon) slotDesc = m_worm->tWeapons[i].Weapon->Name;
		else slotDesc = "* INVALID WEAPON *";
		Color col = tLX->clWeaponSelectionActive;		
		if(m_worm->iCurrentWeapon != i) col = tLX->clWeaponSelectionDefault;
		tLX->cOutlineFont.Draw(bmpDest, centrex-70, y, col,  slotDesc);
		
		if (bChat_Typing)  {
			y += 18;
			continue;
		}
		
		// Changing weapon
		if(m_worm->iCurrentWeapon == i && !bChat_Typing) {
			int change = cRight.wasDown() - cLeft.wasDown();
			if(cSelWeapon.isDown()) change *= 6; // jump with multiple speed if selWeapon is pressed
			int id = m_worm->tWeapons[i].Weapon ? m_worm->tWeapons[i].Weapon->ID : 0;
			if(change > 0) while(change) {
				id++; MOD(id, game.gameScript()->GetNumWeapons());
				if( game.weaponRestrictions()->isEnabled( game.gameScript()->GetWeapons()[id].Name ) )
					change--;
				if(!m_worm->tWeapons[i].Weapon && id == 0)
					break;
				if(m_worm->tWeapons[i].Weapon && id == m_worm->tWeapons[i].Weapon->ID) // back where we were before
					break;
			} else
				if(change < 0) while(change) {
					id--; MOD(id, game.gameScript()->GetNumWeapons());
					if( game.weaponRestrictions()->isEnabled( game.gameScript()->GetWeapons()[id].Name ) )
						change++;
					if(!m_worm->tWeapons[i].Weapon && id == 0)
						break;
					if(m_worm->tWeapons[i].Weapon && id == m_worm->tWeapons[i].Weapon->ID) // back where we were before
						break;
				}
			m_worm->tWeapons[i].Weapon = &game.gameScript()->GetWeapons()[id];
			m_worm->tWeapons[i].Enabled = true;
		}
		
		y += 18;
	}
	
	for(i=0;i<5;i++)
		m_worm->tProfile->sWeaponSlots[i] = m_worm->tWeapons[i].Weapon ? m_worm->tWeapons[i].Weapon->Name : "";
	
    // Note: The extra weapon weapon is the 'random' button
    if(m_worm->iCurrentWeapon == m_worm->iNumWeaponSlots) {
		
		// Fire on the random button?
		if((cShoot.isDownOnce()) && !bChat_Typing) {
			m_worm->GetRandomWeapons();
		}
	}
	
	
	// Note: The extra weapon slot is the 'done' button
	if(m_worm->iCurrentWeapon == m_worm->iNumWeaponSlots+1) {
		
		// Fire on the done button?
		// we have to check isUp() here because if we continue while it is still down, we will fire after in the game
		if((cShoot.isUp()) && !bChat_Typing) {
			// we are ready with manual human weapon selection
			m_worm->bWeaponsReady = true;
			m_worm->iCurrentWeapon = 0;
			
			// Set our profile to the weapons (so we can save it later)
			for(byte i=0;i<5;i++)
				m_worm->tProfile->sWeaponSlots[i] = m_worm->tWeapons[i].Weapon ? m_worm->tWeapons[i].Weapon->Name : "";
		}
	}
	
	
	
    y+=5;
	if(m_worm->iCurrentWeapon == m_worm->iNumWeaponSlots)
		tLX->cOutlineFont.DrawCentre(bmpDest, centrex, y, tLX->clWeaponSelectionActive, "Random");
	else
		tLX->cOutlineFont.DrawCentre(bmpDest, centrex, y, tLX->clWeaponSelectionDefault, "Random");
	
    y+=18;
	
	if(m_worm->iCurrentWeapon == m_worm->iNumWeaponSlots+1)
		tLX->cOutlineFont.DrawCentre(bmpDest, centrex, y, tLX->clWeaponSelectionActive, "Done");
	else
		tLX->cOutlineFont.DrawCentre(bmpDest, centrex, y, tLX->clWeaponSelectionDefault, "Done");
	
	
	// list current key settings
	// TODO: move this out here
	y += 20;
	tLX->cFont.DrawCentre(bmpDest, centrex, y += 15, tLX->clWeaponSelectionTitle, "~ Key settings ~");
	tLX->cFont.Draw(bmpDest, centrex - 150, y += 15, tLX->clWeaponSelectionTitle, "up/down: " + cUp.getEventName() + "/" + cDown.getEventName());
	tLX->cFont.Draw(bmpDest, centrex - 150, y += 15, tLX->clWeaponSelectionTitle, "left/right: " + cLeft.getEventName() + "/" + cRight.getEventName());
	tLX->cFont.Draw(bmpDest, centrex - 150, y += 15, tLX->clWeaponSelectionTitle, "shoot: " + cShoot.getEventName());
	y -= 45;
	tLX->cFont.Draw(bmpDest, centrex, y += 15, tLX->clWeaponSelectionTitle, "jump/ninja: " + cJump.getEventName() + "/" + cInpRope.getEventName());
	tLX->cFont.Draw(bmpDest, centrex, y += 15, tLX->clWeaponSelectionTitle, "select weapon: " + cSelWeapon.getEventName());
	tLX->cFont.Draw(bmpDest, centrex, y += 15, tLX->clWeaponSelectionTitle, "strafe: " + cStrafe.getEventName());
	tLX->cFont.Draw(bmpDest, centrex, y += 15, tLX->clWeaponSelectionTitle, "quick select weapon: " + cWeapons[0].getEventName() + " " + cWeapons[1].getEventName() + " " + cWeapons[2].getEventName() + " " + cWeapons[3].getEventName() + " " + cWeapons[4].getEventName() );
	
	
	if(!bChat_Typing) {
		// move selection up or down
		if (cDown.isJoystickThrottle() || cUp.isJoystickThrottle())  {
			m_worm->iCurrentWeapon = (cUp.getJoystickValue() + 32768) * (m_worm->iNumWeaponSlots + 2) / 65536; // We have 7 rows and 65536 throttle states
			
		} else {
			int change = cDown.wasDown() - cUp.wasDown();
			m_worm->iCurrentWeapon += change;
			m_worm->iCurrentWeapon %= m_worm->iNumWeaponSlots + 2;
			if(m_worm->iCurrentWeapon < 0) m_worm->iCurrentWeapon += m_worm->iNumWeaponSlots + 2;
		}
	}
}




#include "gusanos/player_options.h"
#include "gusanos/worm.h"
#ifndef DEDICATED_ONLY
#include "CViewport.h"
#endif
#include "gusanos/ninjarope.h"

//#include "gusanos/allegro.h"

using namespace std;

void CWormHumanInputHandler::gusInit()
{
	aimingUp=(false);
	aimingDown=(false);
	changing=(false);
	jumping=(false);
	walkingLeft=(false);
	walkingRight=(false);
	m_viewport=(0);
}

void CWormHumanInputHandler::gusShutdown()
{
#ifndef DEDICATED_ONLY
	//delete m_viewport; m_viewport = NULL;
#endif
}

#ifndef DEDICATED_ONLY
void CWormHumanInputHandler::assignViewport(CViewport* viewport)
{
	m_viewport = viewport;
}
#endif

void CWormHumanInputHandler::subThink()
{
	if ( m_worm ) {
#ifndef DEDICATED_ONLY
	//	if ( m_viewport )
	//		m_viewport->interpolateTo(m_worm->getRenderPos(), m_options->viewportFollowFactor);
#endif

		if(changing && m_worm->getNinjaRopeObj()->active) {
			if(aimingUp) {
				m_worm->addRopeLength(-m_options->ropeAdjustSpeed);
			}
			if(aimingDown) {
				m_worm->addRopeLength(m_options->ropeAdjustSpeed);
			}
		} else {
			const double aimMaxSpeed = MAX((float)fabs(tLXOptions->fAimMaxSpeed), 20.0f) * 0.01;
			const double aimAccel = MAX((float)fabs(tLXOptions->fAimAcceleration), 100.0f) * 0.0001;
			
			if (aimingUp && m_worm->aimSpeed > -AngleDiff(aimMaxSpeed)) {
				m_worm->addAimSpeed(-AngleDiff(aimAccel));
			}
			// No "else if" since we want to support precision aiming
			if (aimingDown && m_worm->aimSpeed < AngleDiff(aimMaxSpeed)) {
				m_worm->addAimSpeed(AngleDiff(aimAccel));
			}
		}

		if(!aimingDown && !aimingUp) {
			// I placed this here since CWorm doesn't have access to aiming flags
			const double aimFriction = CLAMP((double)tLXOptions->fAimFriction, 0.0, 1.0);
			m_worm->aimSpeed *= aimFriction;
		}
	}
}

#ifndef DEDICATED_ONLY
void CWormHumanInputHandler::render()
{
	if ( m_viewport ) {
		m_viewport->gusRender();
	}
}
#endif

void CWormHumanInputHandler::actionStart ( Actions action )
{
	switch (action) {
			case LEFT: {
				if ( m_worm ) {
					if(changing) {
						m_worm->changeWeaponTo( m_worm->getWeaponIndexOffset(-1) );
					} else {
						CWormInputHandler::baseActionStart(CWormInputHandler::LEFT);
						walkingLeft = true;
						if ( walkingRight )
							CWormInputHandler::baseActionStart(CWormInputHandler::DIG);
					}
				}
			}
			break;

			case RIGHT: {
				if ( m_worm ) {
					if(changing) {
						m_worm->changeWeaponTo( m_worm->getWeaponIndexOffset(1) );
					} else {
						CWormInputHandler::baseActionStart(CWormInputHandler::RIGHT);
						walkingRight = true;
						if ( walkingLeft )
							CWormInputHandler::baseActionStart(CWormInputHandler::DIG);
					}
				}
			}
			break;

			case FIRE: {
				if ( m_worm ) {
					if(!changing)
						CWormInputHandler::baseActionStart(CWormInputHandler::FIRE);
				}
			}
			break;

			case JUMP: {
				if ( m_worm ) {
					if ( m_worm->isActive() ) {
						if (tLXOptions->bOldSkoolRope && changing) {
							CWormInputHandler::baseActionStart(CWormInputHandler::NINJAROPE);
						} else {
							CWormInputHandler::baseActionStart(CWormInputHandler::JUMP);
							CWormInputHandler::baseActionStop(CWormInputHandler::NINJAROPE);
						}

						jumping = true;
					} else {
						CWormInputHandler::baseActionStart(CWormInputHandler::RESPAWN);
					}
				}
			}
			break;

			case UP: {
				if ( m_worm ) {
					aimingUp = true;
				}
			}
			break;

			case DOWN: {
				if ( m_worm ) {
					aimingDown = true;
				}
			}
			break;

			case CHANGE: {
				if ( m_worm ) {
					m_worm->actionStart(Worm::CHANGEWEAPON);

					if (tLXOptions->bOldSkoolRope && jumping) {
						CWormInputHandler::baseActionStart(CWormInputHandler::NINJAROPE);
						jumping = false;
					} else {
						m_worm->actionStop(Worm::FIRE); //TODO: Stop secondary fire also

						// Stop any movement
						m_worm->actionStop(Worm::MOVELEFT);
						m_worm->actionStop(Worm::MOVERIGHT);

					}

					changing = true;
				}
			}
			break;
			
			case NINJAROPE:
				CWormInputHandler::baseActionStart(CWormInputHandler::NINJAROPE);
				break;
			
			case ACTION_COUNT:
			break;
	}
}

void CWormHumanInputHandler::actionStop ( Actions action )
{
	switch (action) {
			case LEFT: {
				if ( m_worm ) {
					CWormInputHandler::baseActionStop(CWormInputHandler::LEFT);
					walkingLeft = false;
				}
			}
			break;

			case RIGHT: {
				if ( m_worm ) {
					CWormInputHandler::baseActionStop(CWormInputHandler::RIGHT);
					walkingRight = false;
				}
			}
			break;

			case FIRE: {
				if ( m_worm ) {
					CWormInputHandler::baseActionStop(CWormInputHandler::FIRE);
				}
			}
			break;

			case JUMP: {
				if ( m_worm ) {
					CWormInputHandler::baseActionStop(CWormInputHandler::JUMP);
					jumping = false;
				}
			}
			break;

			case UP: {
				if ( m_worm ) {
					aimingUp = false;
				}
			}
			break;

			case DOWN: {
				if ( m_worm ) {
					aimingDown = false;
				}
			}
			break;

			case CHANGE: {
				if ( m_worm ) {
					m_worm->actionStop(Worm::CHANGEWEAPON);

					changing = false;
				}
			}
			break;

			case NINJAROPE:
			break;
			
			case ACTION_COUNT:
			break;
	}
}




void CWormHumanInputHandler::OlxInputToGusEvents()
{	
	// change + jump -> ninja

	size_t i = 0;
	for(; i < game.localPlayers.size(); ++i)
		if(game.localPlayers[i] == this) break;

	if(i >= game.localPlayers.size()) {
		errors << "CWormHumanInputHandler::OlxInputToGusEvents: local player unknown" << endl;
		return;
	}
	
	//LEFT
	if(cLeft.wasDown()) eventStart(i, CWormHumanInputHandler::LEFT);
	if(cLeft.wasUp()) eventStop(i, CWormHumanInputHandler::LEFT);
	
 	//RIGHT
	if(cRight.wasDown()) eventStart(i, CWormHumanInputHandler::RIGHT);
	if(cRight.wasUp()) eventStop(i, CWormHumanInputHandler::RIGHT);
 	
	//UP
	if(cUp.wasDown()) eventStart(i, CWormHumanInputHandler::UP);
	if(cUp.wasUp()) eventStop(i, CWormHumanInputHandler::UP);
	
	//DOWN
	if(cDown.wasDown()) eventStart(i, CWormHumanInputHandler::DOWN);
	if(cDown.wasUp()) eventStop(i, CWormHumanInputHandler::DOWN);
	
	//FIRE
	if(cShoot.wasDown()) eventStart(i, CWormHumanInputHandler::FIRE);
	if(cShoot.wasUp()) eventStop(i, CWormHumanInputHandler::FIRE);
	
	//JUMP
	if(cJump.wasDown()) eventStart(i, CWormHumanInputHandler::JUMP);
	if(cJump.wasUp()) eventStop(i, CWormHumanInputHandler::JUMP);
	
	//CHANGE
	if(cSelWeapon.wasDown()) eventStart(i, CWormHumanInputHandler::CHANGE);
	if(cSelWeapon.wasUp()) eventStop(i, CWormHumanInputHandler::CHANGE);
	
	if(!tLXOptions->bOldSkoolRope) {
		if(cInpRope.isDownOnce()) eventStart(i, CWormHumanInputHandler::NINJAROPE);
		if(cInpRope.wasUp()) eventStop(i, CWormHumanInputHandler::NINJAROPE);
	}

	
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

