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
//	int		jump = false; // TODO: not used
	int		weap = false;
	int		RightOnce = false;
	int		move = false;

	worm_state_t *ws = &tState;
//	gs_worm_t *wd = cGameScript->getWorm();  // TODO: not used

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
        fAngleSpeed = MIN(fAngleSpeed,(float)100);
        fAngleSpeed = MAX(fAngleSpeed,(float)-100);
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

	// Calculate dir
	dir.x=( (float)cos(fAngle * (PI/180)) );
	dir.y=( (float)sin(fAngle * (PI/180)) );
	if(iDirection==DIR_LEFT)
		dir.x=(-dir.x);

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
void CWorm::Simulate(CMap *map, CWorm *worms, int local, float dt)
{
//	int move = false;  // TODO: not used
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
	dir.x=( (float)cos(fAngle * (PI/180)) );
	dir.y=( (float)sin(fAngle * (PI/180)) );
	if(iDirection==DIR_LEFT)
		dir.x=(-dir.x);

	if(iOnGround)
		speed = wd->GroundSpeed;
	else
		speed = wd->AirSpeed;


	// Process the ninja rope
	if(cNinjaRope.isReleased()) {
		cNinjaRope.Simulate(dt,map,vPos,worms,iID);
		vVelocity += cNinjaRope.GetForce(vPos)*dt;
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


	// Process the jump
	if(ws->iJump) {
		if(CheckOnGround(map)) {
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


	if((int)fFrame>2)
		fFrame=0;
	if(!ws->iMove)
		fFrame=0;


	// Gravity
	vVelocity.y += wd->Gravity*dt;


	// Check collisions

	// TODO: Use a projectile style collision system based on velocity speed

	//int x,y;

	vOldPos = vPos;

	//CVec newpos = vPos + vVelocity*dt;
	vPos += vVelocity * dt;

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
			if(fabs(vVelocity.x) > 160)
				vVelocity = vVelocity * CVec(-0.4f,1);
			else
				vVelocity.x=(0);
		}
	}

	tLX->debug_int = coll;*/

	/*x = newpos.x+4;
	y = newpos.y;
	int clip = 0;

	if(y >= 0 && y < map->GetHeight()) {
		for(x=-3;x<4;x++) {
			// Optimize: pixelflag++

			// Clipping
			if(newpos.x+x < 0 || newpos.x+x >= map->GetWidth())
				continue;

			if(!(map->GetPixelFlag(newpos.x+x,y) & PX_EMPTY)) {
				if(fabs(vVelocity.x) > 40)
					vVelocity = vVelocity * CVec(-0.4f,1);
				else
					vVelocity.x=(0);

				int width = 4;
				if(x<0) {
					clip |= 0x01;
					vPos.x=( newpos.x+x+width );
				}
				else {
					clip |= 0x02;
					vPos.x=( newpos.x+x-width );
				}
				break;
			}
		}
	}

	iOnGround = false;

	int hit = false;
	x = newpos.x;

	if(x >= 0 && x < map->GetWidth()) {
		for(y=4;y>-5;y--) {
			// Optimize: pixelflag + Width

			// Clipping
			if(newpos.y+y < 0 || newpos.y+y >= map->GetHeight())
				continue;

			if(!(map->GetPixelFlag(x,newpos.y+y) & PX_EMPTY)) {
				if(fabs(vVelocity.y) > 40 && !hit && !jump)
					vVelocity = vVelocity * CVec(1,-0.4f);
				else if(!jump)
					vVelocity.y=(0);

				hit = true;
				iOnGround = true;

				int height = 5;
				if(y<0) {
					clip |= 0x04;
					vPos.y=( newpos.y+y+height );
				}
				else {
					clip |= 0x08;
					vPos.y=( newpos.y+y-height );
				}

				//if(y>3 && !jump) {
					//vVelocity.y=(-10);
					//Velocity.y=(0);
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
int CWorm::CheckWormCollision( float dt, CMap *map, CVec pos, CVec *vel, int jump )
{
	int x,y;
	static const int maxspeed2 = 20;

	// If the worm is going too fast, divide the speed by 2 and perform 2 collision checks
	if( (*vel*dt).GetLength2() > maxspeed2) {
		dt /= 2;
		if(CheckWormCollision(dt,map,pos,vel,jump)) return true;
		return CheckWormCollision(dt,map,vPos,vel,jump);
	}

	pos += *vel*dt;
	vPos = pos;


	x = (int)pos.x;
	y = (int)pos.y;
	int clip = 0;
	int coll = false;
	bool check_needed = false;

	const uchar* gridflags = map->getAbsoluteGridFlags();
	int grid_w = map->getGridWidth();
	int grid_h = map->getGridHeight();
	int grid_cols = map->getGridCols();
	if(y-4 < 0 || y+5 > map->GetHeight()-1
	|| x-3 < 0 || x+3 > map->GetWidth()-1)
		check_needed = true; // we will check later, what to do here
	else if(grid_w < 7 || grid_h < 10 // this ensures, that this check is safe
	|| gridflags[((y-4)/grid_h)*grid_cols + (x-3)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((y+5)/grid_h)*grid_cols + (x-3)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((y-4)/grid_h)*grid_cols + (x+3)/grid_w] & (PX_ROCK|PX_DIRT)
	|| gridflags[((y+5)/grid_h)*grid_cols + (x+3)/grid_w] & (PX_ROCK|PX_DIRT))
		check_needed = true;

	if(check_needed && y >= 0 && y < map->GetHeight()) {
		for(x=-3;x<4;x++) {
			// Optimize: pixelflag++

			// Left side clipping
			if(pos.x+x <= 2) {
				vPos.x=( 5 );
				coll = true;
				if(fabs(vel->x) > 40)
					vel->x *=  -0.4f;
				else
					vel->x=(0);
				break;
			}

			// Right side clipping
			if(pos.x+x >= map->GetWidth()) {
				vPos.x=( (float)map->GetWidth() - 5 );
				coll = true;
				if(fabs(vel->x) > 40)
					vel->x *= -0.4f;
				else
					vel->x=(0);
				break;
			}


			if(!(map->GetPixelFlag((int)pos.x+x,y) & PX_EMPTY)) {

				coll = true;

				// Bounce
				if(fabs(vel->x) > 30)
					vel->x *= -0.4f;
				else
					vel->x=(0);

				int width = 4;
				if(x<0) {
					clip |= 0x01;
					vPos.x=( pos.x+x+width );
				}
				else {
					clip |= 0x02;
					vPos.x=( pos.x+x-width );
				}
				break;
			}
		}
	}

	iOnGround = false;

	int hit = false;
	x = (int)pos.x;

	if(check_needed && x >= 0 && x < map->GetWidth()) {
		for(y=5;y>-5;y--) {
			// Optimize: pixelflag + Width

			// Top side clipping
			if(pos.y+y <= 1) {
				vPos.y=( 6 );
				coll = true;
				if(fabs(vel->y) > 40)
					vel->y *= -0.4f;
				break;
			}

			// Bottom side clipping
			if(pos.y+y >= map->GetHeight()) {
				vPos.y=( (float)map->GetHeight() - 5 );
				coll = true;
                iOnGround = true;
				if(fabs(vel->y) > 40)
					vel->y *= -0.4f;
				else
					vel->y=(0);
				break;
			}


			if(!(map->GetPixelFlag(x,(int)pos.y+y) & PX_EMPTY)) {
				coll = true;

                if(!hit && !jump) {
				    if(fabs(vel->y) > 40 && ((vel->y > 0 && y>0) || (vel->y < 0 && y<0)))
					    vel->y *= -0.4f;
				    else
					    vel->y=(0);
                }

				hit = true;
				iOnGround = true;

				int height = 5;
				if(y<0) {
					clip |= 0x04;
					vPos.y=( pos.y+y+height );
				}
				else {
					clip |= 0x08;
					vPos.y=( pos.y+y-height );
				}

				//if(y>3 && !jump) {
					//vVelocity.y=(-10);
					//Velocity.y=(0);
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
//	float	dt = tLX->fDeltaTime;  // TODO: not used
	CVec	dir;
//	int		jump = false;  // TODO: not used
	int		weap = false;
	int		RightOnce = false;
	int		move = false;

	worm_state_t *ws = &tState;
//	gs_worm_t *wd = cGameScript->getWorm();  // TODO: not used

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
