/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Worm class - Simulation
// Created 2/8/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


///////////////////
// Get the input from a human worm
void CWorm::getInput(/*worm_state_t *ws*/)
{
	float	dt = tLX->fDeltaTime;
	CVec	dir;
	int		jump = false;
	int		weap = false;
	int		RightOnce = false;
	int		move = false;

	worm_state_t *ws = &tState;
	gs_worm_t *wd = cGameScript->getWorm();

	// Temp thing
	// TODO: Try out mouse input for a 3rd worm
	//getMouseInput();
	//return;


	// Init the ws
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;


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
        fAngleSpeed = MIN(fAngleSpeed,100);
        fAngleSpeed = MAX(fAngleSpeed,-100);
        if( fAngleSpeed > 0 )
            fAngleSpeed -= 200*dt;
        else if( fAngleSpeed < 0 )
            fAngleSpeed += 200*dt;
        if( fabs(fAngleSpeed) < 5 )
            fAngleSpeed = 0;
    }

    fAngle += fAngleSpeed * dt;
    if(fAngle>60)
		fAngle = 60;
    if(fAngle<-90)
		fAngle = -90;
	
	if(!cRight.isDown())
		iCarving &= ~1;
	if(!cLeft.isDown())
		iCarving &= ~2;


	// Carving hack
	RightOnce = cRight.isDownOnce();
	if(cLeft.isDown() && RightOnce && iDirection == DIR_LEFT)  {
		iCarving |= 2;
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


    // If this is the first worm, let the user use the 1-5 keys for weapon shortcuts
    if( iID == 0 ) {
        keyboard_t *kb = GetKeyboard();
        for( int i=SDLK_1; i<=SDLK_5; i++ ) {
            if( kb->KeyDown[i] ) {

                iCurrentWeapon = i-SDLK_1;

                bForceWeapon_Name = true;
                fForceWeapon_Time = tLX->fCurTime+0.75f;
            }
        }

        // Clamp the current weapon
        iCurrentWeapon = MIN(iCurrentWeapon, iNumWeaponSlots-1);
        iCurrentWeapon = MAX(iCurrentWeapon, 0);
	}


	ws->iShoot = false;
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

			//if(vVelocity.GetX()<75)
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

		//if(vVelocity.GetX()>-75)
		//	vVelocity = vVelocity + CVec(-speed,0);
		//fFrame+=5*dt;
		move = true;
	}

	// Calculate dir
	dir.SetX( (float)cos(fAngle * (PI/180)) );
	dir.SetY( (float)sin(fAngle * (PI/180)) );
	if(iDirection==DIR_LEFT)
		dir.SetX(-dir.GetX());

	int oldskool = tLXOptions->iOldSkoolRope;

	int jumpdownonce = cJump.isDownOnce();
	
	// Jump
	if(jumpdownonce) {

		if( !(oldskool && cSelWeapon.isDown()) )
			ws->iJump = true;
	}


	if(jumpdownonce) {

		if(!(oldskool && cSelWeapon.isDown()) ) {
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
//TODO : sound
//			BASS_SamplePlay(sfxGame.smpNinja);
		}


	} else {
		// Newer style rope throwing
		// Seperate dedicated button for throwing the rope
		if(cInpRope.isDownOnce()) {

			cNinjaRope.Shoot(vPos,dir);
			// Throw sound
//TODO : sound
//			BASS_SamplePlay(sfxGame.smpNinja);
		}
	}


	ws->iAngle = (int)fAngle;
	ws->iX = (int)vPos.GetX();
	ws->iY = (int)vPos.GetY();
	
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
void CWorm::Simulate(CMap *map, CWorm *worms, int local, float dt)
{
	int move = false;
	float speed;
	CVec dir;
	int jump = false;
	gs_worm_t *wd = cGameScript->getWorm();

	float	fFrameRate = 7.5f;

	worm_state_t *ws = &tState;

	// If the delta time is too big, divide it and run the simulation twice
	if( dt > 0.25f ) {
		dt /= 2;
		Simulate(map,worms,local,dt);
		Simulate(map,worms,local,dt);
		return;
	}

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

	// If we're seriously dead (below 15% health) bleed
	if(iHealth < 15) {
		if(tLX->fCurTime - fLastBlood > 2) {
			fLastBlood = tLX->fCurTime;			
			
			float amount = ((float)tLXOptions->iBloodAmount / 100.0f) * 10;
			for(int i=0;i<amount;i++) {
				CVec v = CVec(GetRandomNum(), GetRandomNum()) * 30;
				SpawnEntity(ENT_BLOOD,0,vPos,v,MakeColour(200,0,0),NULL);
				SpawnEntity(ENT_BLOOD,0,vPos,v,MakeColour(180,0,0),NULL);
			}
		}
	}



	// Calculate dir
	dir.SetX( (float)cos(fAngle * (PI/180)) );
	dir.SetY( (float)sin(fAngle * (PI/180)) );
	if(iDirection==DIR_LEFT)
		dir.SetX(-dir.GetX());

	if(iOnGround)
		speed = wd->GroundSpeed;
	else
		speed = wd->AirSpeed;


	// Process the ninja rope
	if(cNinjaRope.isReleased()) {
		cNinjaRope.Simulate(dt,map,vPos,worms,iID);
		vVelocity = vVelocity + cNinjaRope.GetForce(vPos)*dt;
	}


	// If we are hooked, we need to be pulled in the direction of the other worm
	if(iHooked && pcHookWorm) {
		// FIXME: This isn't 'right'
		//vVelocity = vVelocity + cNinjaRope.CalculateForce(vPos,pcHookWorm->getPos())*dt;
	}



	// Process the carving
	if(ws->iCarve) {
		iDirtCount += CarveHole(map,vPos + dir*4);
		//cClient->SendCarve(vPos + dir*4);
	}


	// Process the moving
	if(ws->iMove) {
		if(iDirection == DIR_RIGHT) {

			// Right
			if(vVelocity.GetX()<30)
				vVelocity = vVelocity + CVec(speed,0);
			fFrame += fFrameRate * dt;
		} else {

			// Left
			if(vVelocity.GetX()>-30)
				vVelocity = vVelocity + CVec(-speed,0);
			fFrame += fFrameRate * dt;
		}
	}


	// Process the jump
	if(ws->iJump) {
		if(CheckOnGround(map)) {
			//vVelocity.SetX(0);
			vVelocity.SetY(0);
			vVelocity = vVelocity + CVec(0,wd->JumpForce);
			iOnGround = false;
			jump = true;
		}
	}
	

	// Air drag (Mainly to dampen the ninja rope)
	float Drag = wd->AirFriction;

	if(!iOnGround)	{
		vVelocity.SetX( vVelocity.GetX() - SQR(vVelocity.GetX()) * SIGN(vVelocity.GetX()) * Drag * dt );
		vVelocity.SetY( vVelocity.GetY() + (-SQR(vVelocity.GetY()) * SIGN(vVelocity.GetY()) * Drag) * dt );	
	}


	if((int)fFrame>2)
		fFrame=0;
	if(!ws->iMove)
		fFrame=0;

	
	// Gravity
	vVelocity = vVelocity + CVec(0,wd->Gravity)*dt;


	// Check collisions

	// TODO: Use a projectile style collision system based on velocity speed

	//int x,y;
	
	vOldPos = vPos;

	//CVec newpos = vPos + vVelocity*dt;
	vPos = vPos + vVelocity * dt;
	
	CheckWormCollision( dt, map, vOldPos, &vVelocity, jump );

	/*iOnGround = false;

	int coll = CheckCollision(newpos, vVelocity, 3, 3, map);

	if(coll) {
		iOnGround = true;

		// Top & Bottom
		if(coll & COL_TOP  ||  coll & COL_BOTTOM)
			vVelocity = vVelocity * CVec(0.8f,-0.5f);

		// Left & Right
		if(coll & COL_LEFT  ||  coll & COL_RIGHT) {	
			if(fabs(vVelocity.GetX()) > 160)
				vVelocity = vVelocity * CVec(-0.4f,1);
			else
				vVelocity.SetX(0);
		}
	}

	tLX->debug_int = coll;*/

	/*x = newpos.GetX()+4;
	y = newpos.GetY();
	int clip = 0;

	if(y >= 0 && y < map->GetHeight()) {
		for(x=-3;x<4;x++) {
			// Optimize: pixelflag++

			// Clipping
			if(newpos.GetX()+x < 0 || newpos.GetX()+x >= map->GetWidth())
				continue;

			if(!(map->GetPixelFlag(newpos.GetX()+x,y) & PX_EMPTY)) {
				if(fabs(vVelocity.GetX()) > 40)
					vVelocity = vVelocity * CVec(-0.4f,1);
				else
					vVelocity.SetX(0);

				int width = 4;
				if(x<0) {
					clip |= 0x01;
					vPos.SetX( newpos.GetX()+x+width );
				}
				else {
					clip |= 0x02;
					vPos.SetX( newpos.GetX()+x-width );
				}
				break;
			}
		}
	}

	iOnGround = false;
	
	int hit = false;
	x = newpos.GetX();
	
	if(x >= 0 && x < map->GetWidth()) {
		for(y=4;y>-5;y--) {
			// Optimize: pixelflag + Width

			// Clipping
			if(newpos.GetY()+y < 0 || newpos.GetY()+y >= map->GetHeight())
				continue;
		
			if(!(map->GetPixelFlag(x,newpos.GetY()+y) & PX_EMPTY)) {
				if(fabs(vVelocity.GetY()) > 40 && !hit && !jump)
					vVelocity = vVelocity * CVec(1,-0.4f);
				else if(!jump)
					vVelocity.SetY(0);

				hit = true;
				iOnGround = true;

				int height = 5;
				if(y<0) {
					clip |= 0x04;
					vPos.SetY( newpos.GetY()+y+height );
				}
				else {
					clip |= 0x08;
					vPos.SetY( newpos.GetY()+y-height );
				}

				//if(y>3 && !jump) {
					//vVelocity.SetY(-10);
					//Velocity.SetY(0);
				//	break;
				//}
			}
		}
	}

	// If we are stuck in left & right or top & bottom, just don't move
	if((clip & 0x01 && clip & 0x02) || (clip & 0x04 && clip & 0x08))
		vPos = oldpos;*/


	// Ultimate in friction
	if(iOnGround) {
		vVelocity = vVelocity * CVec(/*wd->GroundFriction*/ 0.9f,1);        // Hack until new game script is done

		// Too slow, just stop
		if(fabs(vVelocity.GetX()) < 5 && !ws->iMove)
			vVelocity = vVelocity * CVec(0,1);
	}


	// Weapon
	SimulateWeapon( dt );

	// Fill in the info for sending
	if(local) {
		ws->iAngle = (int)fAngle;
		ws->iDirection = iDirection;
		ws->iX = (int)vPos.GetX();
		ws->iY = (int)vPos.GetY();
	}
}


///////////////////
// Simulates the weapon
void CWorm::SimulateWeapon( float dt )
{
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
int CWorm::CheckWormCollision( float dt, CMap *map, CVec pos, CVec *vel, int jump )
{
	int x,y;
	int maxspeed = 50;
	
	// If the worm is going too fast, divide the speed by 2 and perform 2 collision checks
	/*if( VectorLength(*vel) > maxspeed) {
		*vel = *vel / 2;

		CheckWormCollision(dt,map,pos,vel,jump);
		//return true;

		pos = pos + *vel*dt;

		CheckWormCollision(dt,map,pos,vel,jump);
		//return true;

		return false;
	}*/

	pos = pos + *vel*dt;
	vPos = pos;


	x = (int)pos.GetX()+4;
	y = (int)pos.GetY();
	int clip = 0;
	int coll = false;

	if(y >= 0 && y < map->GetHeight()) {
		for(x=-3;x<4;x++) {
			// Optimize: pixelflag++

			// Left side clipping
			if(pos.GetX()+x <= 2) {
				vPos.SetX( 5 );
				coll = true;
				if(fabs(vel->GetX()) > 40)
					*vel = *vel * CVec(-0.4f,1);
				else
					vel->SetX(0);
				break;
			}

			// Right side clipping
			if(pos.GetX()+x >= map->GetWidth()) {
				vPos.SetX( (float)map->GetWidth() - 5 );
				coll = true;
				if(fabs(vel->GetX()) > 40)
					*vel = *vel * CVec(-0.4f,1);
				else
					vel->SetX(0);
				break;
			}


			if(!(map->GetPixelFlag((int)pos.GetX()+x,y) & PX_EMPTY)) {

				coll = true;

				// Bounce
				if(fabs(vel->GetX()) > 30)
					*vel = *vel * CVec(-0.4f,1);
				else
					vel->SetX(0);				

				int width = 4;
				if(x<0) {
					clip |= 0x01;
					vPos.SetX( pos.GetX()+x+width );
				}
				else {
					clip |= 0x02;
					vPos.SetX( pos.GetX()+x-width );
				}
				break;
			}
		}
	}

	iOnGround = false;
	
	int hit = false;
	x = (int)pos.GetX();
	
	if(x >= 0 && x < map->GetWidth()) {
		for(y=5;y>-5;y--) {
			// Optimize: pixelflag + Width

			// Top side clipping
			if(pos.GetY()+y <= 1) {
				vPos.SetY( 6 );
				coll = true;
				if(fabs(vel->GetY()) > 40)
					*vel = *vel * CVec(1,-0.4f);
				break;
			}

			// Bottom side clipping
			if(pos.GetY()+y >= map->GetHeight()) {
				vPos.SetY( (float)map->GetHeight() - 5 );
				coll = true;
                iOnGround = true;
				if(fabs(vel->GetY()) > 40)
					*vel = *vel * CVec(1,-0.4f);
				else
					vel->SetY(0);
				break;
			}

		
			if(!(map->GetPixelFlag(x,(int)pos.GetY()+y) & PX_EMPTY)) {
				coll = true;

                if(!hit && !jump) {
				    if(fabs(vel->GetY()) > 40 && ((vel->GetY() > 0 && y>0) || (vel->GetY() < 0 && y<0)))
					    *vel = *vel * CVec(1,-0.4f);
				    else
					    vel->SetY(0);
                }

				hit = true;
				iOnGround = true;

				int height = 5;
				if(y<0) {
					clip |= 0x04;
					vPos.SetY( pos.GetY()+y+height );
				}
				else {
					clip |= 0x08;
					vPos.SetY( pos.GetY()+y-height );
				}

				//if(y>3 && !jump) {
					//vVelocity.SetY(-10);
					//Velocity.SetY(0);
				//	break;
				//}
			}
		}
	}

	// If we are stuck in left & right or top & bottom, just don't move
	if((clip & 0x01 && clip & 0x02) || (clip & 0x04 && clip & 0x08))
		vPos = vOldPos;

	// If we collided with the ground and we were going pretty fast, make a bump sound
	if(coll) {
		if( fabs(vel->GetX()) > 30 && (clip & 0x01 || clip & 0x02) )
			StartSound( sfxGame.smpBump, vPos, getLocal(), -1, this );
		else if( fabs(vel->GetY()) > 30 && (clip & 0x04 || clip & 0x08) )
			StartSound( sfxGame.smpBump, vPos, getLocal(), -1, this );
	}

	return false;
}






int MouseX = -1, MouseY = -1;


///////////////////
// Use a mouse for worm input
void CWorm::getMouseInput(void)
{
	float	dt = tLX->fDeltaTime;
	CVec	dir;
	int		jump = false;
	int		weap = false;
	int		RightOnce = false;
	int		move = false;

	worm_state_t *ws = &tState;
	gs_worm_t *wd = cGameScript->getWorm();

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
	fAngle = MAX(fAngle,-90);
	fAngle = MIN(fAngle,60);

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

			//if(vVelocity.GetX()<75)
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

		//if(vVelocity.GetX()>-75)
		//	vVelocity = vVelocity + CVec(-speed,0);
		//fFrame+=5*dt;
		move = true;
	}

	// Calculate dir
	dir.SetX( (float)cos(fAngle * (PI/180)) );
	dir.SetY( (float)sin(fAngle * (PI/180)) );
	if(iDirection==DIR_LEFT)
		dir.SetX(-dir.GetX());

	if(Jump) {
		ws->iJump = true;
		cNinjaRope.Release();
	}


	if(Rope && !RopeDown) {
		cNinjaRope.Shoot(vPos,dir);
		// Throw sound
//TODO : sound
//		BASS_SamplePlay(sfxGame.smpNinja);
		RopeDown = true;
	}

	if(!Rope)
		RopeDown = false;


	ws->iAngle = (int)fAngle;
	ws->iX = (int)vPos.GetX();
	ws->iY = (int)vPos.GetY();
}
