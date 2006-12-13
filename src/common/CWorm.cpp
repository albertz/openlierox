/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Worm class
// Created 28/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


///////////////////
// Clear the worm details
void CWorm::Clear(void)
{
	iUsed = false;
	iID = 0;
	iTeam = 0;
	iLocal = false;
	iType = PRF_HUMAN;
	iRanking = 0;
	iClientID = 0;
	iClientWormID = 0;
    szSkin[0] = '\0';
    
	iKills = 0;
	iDeaths = 0;
	iSuicides = 0;

	iHealth = 100;
	iLives = 10;
	iAlive = false;
	iDirection = DIR_RIGHT;
	fAngle = 0;
    fAngleSpeed = 0;
	iCarving = false;
	fFrame = 0;
	iDrawMuzzle = false;

	iOnGround = false;
	vPos = CVec(0,0);
	vVelocity = CVec(0,0);

	cNinjaRope.Clear();
	fRopeTime = -9999;
	iRopeDownOnce = false;
	iRopeDown = false;

	iWeaponsReady = false;
	iNumWeaponSlots = 2;
	iCurrentWeapon = 0;
	iGameReady = false;

	iGotTarget = false;
	iHooked = false;

	iTagIT = false;
	fTagTime = 0;
	fLastSparkle = -99999;
    iDirtCount = 0;

	// Have to initialize it here, VC++ doesn't allow const in class declaration
	nodesGridWidth = 10;

	fLastBlood = -9999;

	//pcViewport = NULL;//.Setup(0,0,640,480);
	tProfile = NULL;

	int i;
	for(i=0;i<NUM_FRAMES;i++)
		fFrameTimes[i] = -99999.0f;

	for(i=0; i<5; i++)
		tWeapons[i].Weapon = NULL;

    psPath = NULL;
    cWeaponRest = NULL;
    nAIState = AI_THINK;
	fLastCarve = -9999;
    pnOpenCloseGrid = NULL;
    //fLastWeaponSwitch = -9999;
	NEW_psPath = NULL;
	NEW_psCurrentNode = NULL;
	NEW_psLastNode = NULL;

	bmpWorm = NULL;
	bmpGibs = NULL;
	bmpPic = NULL;
    bmpShadowPic = NULL;

	// Lobby
	tLobbyState.iHost = false;
	tLobbyState.iReady = false;
	tLobbyState.iType = LBY_OPEN;
    tLobbyState.iTeam = 0;

    bForceWeapon_Name = false;

	fLastFace = 0;
	fBadAimTime = 0;

	fLastShoot = 0; // for AI
	fLastJump = 999999;
	fLastWeaponChange = 0;
}


///////////////////
// Initialize the worm
void CWorm::Init(void)
{
	memset(&tState,0,sizeof(worm_state_t));

}


///////////////////
// Shutdown the worm
void CWorm::Shutdown(void)
{
	FreeGraphics();

    // Shutdown the AI
    if(iType == PRF_COMPUTER)
        AI_Shutdown();
}


///////////////////
// Free the graphics
void CWorm::FreeGraphics(void)
{
	if(bmpWorm) {
		SDL_FreeSurface(bmpWorm);
		bmpWorm = NULL;
	}

	if(bmpGibs) {
		SDL_FreeSurface(bmpGibs);
		bmpGibs = NULL;
	}

	if(bmpPic) {
		SDL_FreeSurface(bmpPic);
		bmpPic = NULL;
	}

    if(bmpShadowPic) {
        SDL_FreeSurface(bmpShadowPic);
        bmpShadowPic = NULL;
    }

}


///////////////////
// Setup the inputs
void CWorm::SetupInputs(char Inputs[32][8])
{
	cUp.Setup(		Inputs[SIN_UP] );
	cDown.Setup(	Inputs[SIN_DOWN] );
	cLeft.Setup(	Inputs[SIN_LEFT] );
	cRight.Setup(	Inputs[SIN_RIGHT] );

	cShoot.Setup(	Inputs[SIN_SHOOT] );
	cJump.Setup(	Inputs[SIN_JUMP] );
	cSelWeapon.Setup(Inputs[SIN_SELWEAP] );
	cInpRope.Setup(	Inputs[SIN_ROPE] );
}


///////////////////
// Prepare the worm for the game
void CWorm::Prepare(CMap *pcMap)
{
	assert(cGameScript);

	// Setup the rope
	cNinjaRope.Setup(cGameScript);

	iCurrentWeapon = 0;

    // If this is an AI worm, initialize the AI stuff
    if(iType == PRF_COMPUTER)
        AI_Initialize(pcMap);
}


///////////////////
// Setup the lobby details
void CWorm::setupLobby(void)
{
	tLobbyState.iHost = false;
	tLobbyState.iType = LBY_USED;
	tLobbyState.iReady = false;
    tLobbyState.iTeam = 0;
}


///////////////////
// Spawn this worm
void CWorm::Spawn(CVec position)
{
    iAlive = true;
	fAngle = 0;
    fAngleSpeed = 0;
	iHealth = 100;
	iDirection = DIR_RIGHT;
	vPos = position;
	vVelocity = CVec(0,0);
	cNinjaRope.Clear();
    nAIState = AI_THINK;
	fLastShoot = 0;
	
	iCarving = false;
	fFrame = 0;
	iDrawMuzzle = false;
	iHooked = false;
    bForceWeapon_Name = false;

	iOnGround = false;
    iNumWeaponSlots = 5;

	// Reset the weapons
	for(int n=0;n<iNumWeaponSlots;n++) {
		tWeapons[n].Charge = 1;
		tWeapons[n].Reloading = false;
		tWeapons[n].SlotNum = n;
		tWeapons[n].LastFire = 0;
	}

	fSpawnTime = tLX->fCurTime;
}


///////////////////
// Load the graphics
int CWorm::LoadGraphics(int gametype)
{
	int team = false;
    Uint8 teamcolours[] = {102,153,255,  255,51,0,  51,153,0,  255,255,0};
    Uint8 r=0,g=0,b=0;
    
	// Destroy any previous graphics
	FreeGraphics();

    // Use the colours set on the network
    // Profile or team colours will override this
    r = iColComps[0];
    g = iColComps[1];
    b = iColComps[2];


    if(tProfile) {
		iColour = MakeColour(tProfile->R, tProfile->G, tProfile->B);
        r = tProfile->R;
        g = tProfile->G;
        b = tProfile->B;
        strcpy(szSkin, tProfile->szSkin);
    }


	// If we are in a team game, use the team colours
    if(gametype == GMT_TEAMDEATH) {
		team = true;
		r = teamcolours[iTeam*3];
		g = teamcolours[iTeam*3+1];
		b = teamcolours[iTeam*3+2];
		iColour = MakeColour(r,g,b);
	}

	
    // Colourise the giblets
	bmpGibs = ChangeGraphics("data/gfx/giblets.png", team);

    // Load the skin
    bmpWorm = LoadSkin(szSkin, r,g,b);
    
    // Create the minipic
    bmpPic = gfxCreateSurface(18,16);
    SDL_SetColorKey(bmpPic, SDL_SRCCOLORKEY, MakeColour(255,0,255));
    DrawRectFill(bmpPic, 0,0,bmpPic->w,bmpPic->h, MakeColour(255,0,255));
    DrawImageAdv(bmpPic, bmpWorm, 134,2,0,0, 18,16);

	
    // Shadow buffer
    bmpShadowPic = gfxCreateSurface(32,18);
    SDL_SetColorKey(bmpShadowPic, SDL_SRCCOLORKEY, SDL_MapRGB(bmpShadowPic->format,255,0,255));

	if(bmpWorm && bmpGibs && bmpPic && bmpShadowPic)
		return true;

	return false;
}


///////////////////
// Change the graphics of an image
SDL_Surface *CWorm::ChangeGraphics(char *filename, int team)
{
	SDL_Surface *tmp, *img;

	// Load the image
	tmp = _LoadImage(filename);
	if(tmp == NULL) {
		// Error: Couldn't load image
		printf("CWorm::ChangeGraphics: Error: Could not load image %s\n", filename);
		return NULL;
	}

	// Convert the image to the screen's colour depth
	img = SDL_CreateRGBSurface(SDL_SWSURFACE, tmp->w,tmp->h,16, 0,0,0,0);
	if(img == NULL) {
		// Error: Our of memory
		return NULL;
	}

	// Blit it onto the new surface
	SDL_BlitSurface(tmp,NULL,img,NULL);
	SDL_FreeSurface(tmp);


	// Set the colour key
	SDL_SetColorKey(img, SDL_SRCCOLORKEY, SDL_MapRGB(img->format,255,0,255));


	// Set the colour of the img
	int x,y;
	Uint8 r,g,b,a;
	Uint32 pixel;
	
	GetColour4(iColour,SDL_GetVideoSurface(),&r,&g,&b,&a);

	// Team graphics
	Uint8 teamcolours[] = {102,153,255,  255,51,0,  51,153,0,  255,255,0};

	if(team) {
		r = teamcolours[iTeam*3];
		g = teamcolours[iTeam*3+1];
		b = teamcolours[iTeam*3+2];
		iColour = MakeColour(r,g,b);
	}

	int ColR = r;
	int ColG = g;
	int ColB = b;
	float r2,g2,b2;

	
	for(y=0; y<img->h; y++) {
		for(x=0; x<img->w; x++) {

			pixel = GetPixel(img,x,y);
			GetColour4(pixel,img,&r,&g,&b,&a);

			// Ignore pink & gun colours
			if(pixel == MakeColour(255,0,255))
				continue;
			if(pixel == MakeColour(216,216,216))
				continue;
			if(pixel == MakeColour(180,180,180))
				continue;
			if(pixel == MakeColour(144,144,144))
				continue;

			float dr, dg, db;

			dr = (float)r / 96.0f;
			dg = (float)g / 156.0f;
			db = (float)b / 252.0f;

			r2 = (float)ColR * dr;
			g2 = (float)ColG * dg;
			b2 = (float)ColB * db;

			r2 = MIN((float)255,r2);
			g2 = MIN((float)255,g2);
			b2 = MIN((float)255,b2);

			// Make sure it isn't exactly 'magic pink'
			if(MakeColour((int)r2, (int)g2, (int)b2) == MakeColour(255,0,255)) {
				r2=240;
				b2=240;
			}
			
			PutPixel(img,x,y, MakeColour((int)r2, (int)g2, (int)b2));
		}
	}

	return img;
}

///////////////////
// Initialize the weapon selection screen
void CWorm::InitWeaponSelection(void)
{
	// This is used for the menu screen as well
	iCurrentWeapon = 0;

	iWeaponsReady = false;
	
	iNumWeaponSlots = 5;

	// Load previous settings from profile
	int i;
	for(i=0;i<iNumWeaponSlots;i++) {

		tWeapons[i].Weapon = cGameScript->FindWeapon( tProfile->sWeaponSlots[i] );

        // If this weapon is not enabled in the restrictions, find another weapon that is enabled
        if( !cWeaponRest->isEnabled( tWeapons[i].Weapon->Name ) ) {

            tWeapons[i].Weapon = cGameScript->FindWeapon( cWeaponRest->findEnabledWeapon( cGameScript ) );
        }
	}


	// If this is an AI worm, lets give him a preset or random arsenal
	if(iType == PRF_COMPUTER) {

		bool bRandomWeaps = true;
		// Combo (rifle)
		if ((tGameInfo.iLoadingTimes > 15 && tGameInfo.iLoadingTimes < 26) && (strstr(&tGameInfo.sModName[0],"Classic") || strstr(&tGameInfo.sModName[0],"Liero v1.0")))  {
			if (cWeaponRest->isEnabled("Rifle"))  {
				for (i=0; i<5; i++)
					tWeapons[i].Weapon = cGameScript->FindWeapon("Rifle");  // set all weapons to Rifle
				bRandomWeaps = false;
				AI_SetGameType(GAM_RIFLES);
			}
		}
		// 100 lt
		else if ((strstr(&tGameInfo.sModName[0],"Liero") || strstr(&tGameInfo.sModName[0],"Classic")) && tGameInfo.iLoadingTimes == 100)  {
			int MyWeaps = cWeaponRest->isEnabled("Super Shotgun") + cWeaponRest->isEnabled("Napalm") + cWeaponRest->isEnabled("Cannon") + cWeaponRest->isEnabled("Doomsday") + cWeaponRest->isEnabled("Chaingun");
			if (MyWeaps == 5)  {
				// Set our weapons
				tWeapons[0].Weapon = cGameScript->FindWeapon("Super Shotgun");
				tWeapons[1].Weapon = cGameScript->FindWeapon("Napalm");
				tWeapons[2].Weapon = cGameScript->FindWeapon("Cannon");
				tWeapons[3].Weapon = cGameScript->FindWeapon("Doomsday");
				tWeapons[4].Weapon = cGameScript->FindWeapon("Chaingun");
				bRandomWeaps = false;
				AI_SetGameType(GAM_100LT);
			}
		}
		// Mortar game
		else if ((strstr(&tGameInfo.sModName[0],"MW 1.0") || strstr(&tGameInfo.sModName[0],"Modern Warfare1.0")) && tGameInfo.iLoadingTimes < 50)  {
			if (cWeaponRest->isEnabled("Mortar Launcher"))  {
				for (i=0; i<5; i++)
					tWeapons[i].Weapon = cGameScript->FindWeapon("Mortar Launcher");  // set all weapons to Mortar
				bRandomWeaps = false;
				AI_SetGameType(GAM_MORTARS);
			}
		}

		// Random
		if (bRandomWeaps) {
			int tRandomWeapons[MAX_WEAPONSLOTS];
			GetRandomWeapons(tRandomWeapons);
			for (i=0;i<5; i++)  
				tWeapons[i].Weapon = cGameScript->GetWeapons()+tRandomWeapons[i];

			AI_SetGameType(GAM_OTHER);
		}
	}

	
	for(int n=0;n<iNumWeaponSlots;n++) {
		tWeapons[n].Charge = 1;
		tWeapons[n].Reloading = false;
		tWeapons[n].SlotNum = n;
		tWeapons[n].LastFire = 0;
	}
}

///////////////////
// Randomize the weapons
void CWorm::GetRandomWeapons(int Result[MAX_WEAPONSLOTS])
{
	wpnslot_t tRandWeaps[MAX_WEAPONSLOTS];
	int lastenabled = 0;

	for(int i=0; i<5; i++) {
		int num = GetRandomInt(cGameScript->GetNumWeapons()-1);

		// Safety hack
		if (num == 0)
			num = 1;
		
        // Cycle through weapons starting from the random one until we get an enabled weapon
        int n=num;
		lastenabled = 0;
		while(1) {
			// Wrap around
			if(n >= cGameScript->GetNumWeapons())  
 			   n = 0;

			// Have we already got this weapon?
			bool bSelected = false;
			for(int k=0; k<i; k++) {
				if((cGameScript->GetWeapons()+n)->ID == tRandWeaps[k].Weapon->ID) {
					bSelected = true;
					break;
				}
			}

			// If this weapon is enabled AND we have not selected it already, then exit the loop
			if(cWeaponRest->isEnabled( (cGameScript->GetWeapons()+n)->Name ))  {
				if (!bSelected)
				  break;
				lastenabled = n;
			}

			n++;

			// We made a whole loop
			if(n == num) {
			   n = lastenabled;
			   break;
			}	
											
		}  // while
		tRandWeaps[i].Weapon = cGameScript->GetWeapons()+n;
		Result[i] = n;
	}

}

///////////////////
// Draw/Process the weapon selection screen
void CWorm::SelectWeapons(SDL_Surface *bmpDest, CViewport *v)
{
	int l = 0;
	int t = 0;
	int i,id;
	int centrex = 320;

    if( v ) {
        if( v->getUsed() ) {
            l = v->GetLeft();
	        t = v->GetTop();
            centrex = v->GetLeft() + v->GetVirtW()/2;
        }
    }
	
	//tLX->cFont.DrawCentre(bmpDest, centrex+2, t+82, 0,"%s", "Weapons Selection");
	tLX->cOutlineFont.DrawCentre(bmpDest, centrex, t+30, 0xffff,"%s", "Weapons Selection");

	int iChat_Typing = false;
	if (getClient())
		iChat_Typing = getClient()->isTyping();

	int y = t + 60;
	for(i=0;i<iNumWeaponSlots;i++) {
		
		//tLX->cFont.Draw(bmpDest, centrex-69, y+1, 0,"%s", tWeapons[i].Weapon->Name);
		if(iCurrentWeapon == i)
			tLX->cOutlineFont.Draw(bmpDest, centrex-70, y, 0xffff, "%s", tWeapons[i].Weapon->Name);
		else
			tLX->cOutlineFontGrey.Draw(bmpDest, centrex-70, y, 0xffff, "%s", tWeapons[i].Weapon->Name);

		if (iChat_Typing)  {
			y += 18;
			continue;
		}

		// Changing weapon
		if(cRight.isUp() && iCurrentWeapon == i) {
			int orig = tWeapons[i].Weapon->ID;
            id = orig;

            // If select is held down, we will advance by 5 weapons at a time
            if(cSelWeapon.isDown())
                id+=5;
            
            // Check if this weapon is enabled. If not, go to the next weapon in the list and check. and so on
            while(1) {
                id++;
                if(id >= cGameScript->GetNumWeapons())
				    id=0;
                tWeapons[i].Weapon = cGameScript->GetWeapons()+id;

                // Check the weapon
                if( cWeaponRest->isEnabled(tWeapons[i].Weapon->Name) )
                    break;

                // If we are back to the original weapon (ie, all disabled/bonus)
                // then just choose the first weapon
                if( id == orig ) {
                    tWeapons[i].Weapon = cGameScript->GetWeapons();
                    break;
                }
            }
						
		}

		if(cLeft.isUp() && iCurrentWeapon == i) {
            int orig = tWeapons[i].Weapon->ID;
            id = orig;

            // If select is held down, we will go back by 5 weapons at a time
            if(cSelWeapon.isDown())
                id-=5;
            
            // Check if this weapon is enabled. If not, go to the next weapon in the list and check. and so on
            while(1) {
                id--;
                if(id < 0)
				    id = cGameScript->GetNumWeapons()-1;
                tWeapons[i].Weapon = cGameScript->GetWeapons()+id;

                // Check the weapon
                if( cWeaponRest->isEnabled(tWeapons[i].Weapon->Name) )
                    break;

                // If we are back to the original weapon (ie, all disabled/bonus)
                // then just choose the first weapon
                if( id == orig ) {
                    tWeapons[i].Weapon = cGameScript->GetWeapons();
                    break;
                }
            }
		}
		

		y += 18;
	}

	for(i=0;i<5;i++)
		strcpy( tProfile->sWeaponSlots[i], tWeapons[i].Weapon->Name );

    // Note: The extra weapon weapon is the 'random' button
    if(iCurrentWeapon == iNumWeaponSlots) {

		// Fire on the random button?
		if(cShoot.isUp() && !iChat_Typing) {
			int tRandomWeapons[MAX_WEAPONSLOTS];
			GetRandomWeapons(tRandomWeapons);
			for (i=0;i<5; i++)  
				tWeapons[i].Weapon = cGameScript->GetWeapons()+tRandomWeapons[i];
		}
	}


	// Note: The extra weapon slot is the 'done' button
	if(iCurrentWeapon == iNumWeaponSlots+1) {

		// Fire on the done button?
		if(cShoot.isUp() && !iChat_Typing) {
			iWeaponsReady = true;
			iCurrentWeapon = 0;

			// Set our profile to the weapons (so we can save it later)
			for(int i=0;i<5;i++)
				strcpy( tProfile->sWeaponSlots[i], tWeapons[i].Weapon->Name );
		}
	}

    

	// AI Worms select their weapons automatically
	if(iType == PRF_COMPUTER) {
		iWeaponsReady = true;
		iCurrentWeapon = 0;
	}


    y+=5;
	if(iCurrentWeapon == iNumWeaponSlots)
		tLX->cOutlineFont.DrawCentre(bmpDest, centrex, y, 0xffff,"%s", "Random");
	else
		tLX->cOutlineFontGrey.DrawCentre(bmpDest, centrex, y, 0xffff,"%s", "Random");

    y+=18;

	if(iCurrentWeapon == iNumWeaponSlots+1)
		tLX->cOutlineFont.DrawCentre(bmpDest, centrex, y, 0xffff,"%s", "Done");
	else
		tLX->cOutlineFontGrey.DrawCentre(bmpDest, centrex, y, 0xffff,"%s", "Done");


	if(iChat_Typing)
		return;

	if(cUp.isUp()) {
		iCurrentWeapon--;
		if(iCurrentWeapon<0)
			iCurrentWeapon = iNumWeaponSlots+1;
	}

	if(cDown.isUp()) {
		iCurrentWeapon++;
		if(iCurrentWeapon > iNumWeaponSlots+1)
			iCurrentWeapon = 0;
	}
}


// Muzzle flash positions for different angles
int	RightMuzzle[14] = {2,3, 5,3, 4,0, 5,-8, 3,-9, 2,-13, -2,-12};
int	LeftMuzzle[14] =  {4,-12, -1,-12, -1,-9, -3,-8, -2,0, -2,4, 1,3};

///////////////////
// Draw the worm
void CWorm::Draw(SDL_Surface *bmpDest, CMap *map, CViewport *v)
{
	int x,y,f,ang;

    if( !v )
        return;

	int wx = v->GetWorldX();
	int wy = v->GetWorldY();
	int l = v->GetLeft();
	int t = v->GetTop();

	
	//
	// Draw the ninja rope
	//		
	if(cNinjaRope.isReleased())
		cNinjaRope.Draw(bmpDest,v,vPos);



	// Are we inside the viewport?
	x = (int)vPos.x - wx;
	y = (int)vPos.y - wy;
	x*=2;
	y*=2;

	if(x+l+10 < l || x-10 > v->GetVirtW())
		return;
	if(y+t+10 < t || y-10 > v->GetVirtH())
		return;



	Uint16 PlayerPink = (Uint16)MakeColour(255,0,255);


	int a = (int)fAngle;
	if(iDirection == DIR_LEFT)
		a=180-a;

	// Draw the health
	//DrawRectFill(bmpDest,x-31,y-31,x+41,y-22,0);  // border

	int WormNameY = 30; // If health is displayed, worm name has another position


	if (tLXOptions->iShowHealth)  {
		if (!iLocal || iType != PRF_HUMAN)  {
			Uint32 BorderColor = MakeColour(0x49,0x50,0x65);
			int iShowHealth = Round((float)((getHealth()+15)/20));
			//DrawRectFill(bmpDest,x-31,y-26,x-27,y-21,MakeColour(127,127,127));
			/*DrawLine(bmpDest,x-1,y-26,x+21,y-26,0);
			DrawLine(bmpDest,x-1,y-21,x+21,y-21,0);
			DrawLine(bmpDest,x-1,y-26,x-1,y-21,0);
			DrawLine(bmpDest,x+21,y-26,x+21,y-21,0);*/
			DrawRect(bmpDest,x-10,y-20,x+15,y-15,BorderColor);
			DrawVLine(bmpDest,y-19,y-16,x-5,BorderColor);
			DrawVLine(bmpDest,y-19,y-16,x,BorderColor);
			DrawVLine(bmpDest,y-19,y-16,x+5,BorderColor);
			DrawVLine(bmpDest,y-19,y-16,x+10,BorderColor);

										// Red			Orange				Yellow		   Light Green		  Green	
			Uint8 HealthColors[15] = {0xE3,0x04,0x04,  0xFE,0x85,0x03,  0xFE,0xE9,0x03,  0xA8,0xFE,0x03,  0x21,0xFE,0x03};

			// For health hacks
			if (iShowHealth > 5)
				iShowHealth = 5;

			for (int i=0; i<iShowHealth; i++) {
				Uint32 CurColor = MakeColour(HealthColors[i*3],HealthColors[i*3+1],HealthColors[i*3+2]);
				DrawRectFill(bmpDest,x-10+(i*5+1),y-19,x-10+(i*5+1)+4,y-15,CurColor);
			}

			WormNameY = 35;
			
			//int iHealthProgress = (int) (20*getHealth()/100);
			//DrawRectFill(bmpDest,x-30,y-25,x-30+iHealthProgress,y-18,MakeColour(0,220,0));  // health
		}
	}


	//
	// Draw the crosshair
	//
	CVec forw;
	GetAngles(a,&forw,NULL);
	forw = forw*16.0f;

	int cx = (int)forw.x + (int)vPos.x;
	int cy = (int)forw.y + (int)vPos.y;

	cx = (cx-wx)*2+l;
	cy = (cy-wy)*2+t;

	// Snap the position to a slighter bigger pixel grid (2x2)
	cx -= cx % 2;
	cy -= cy % 2;

	// Show a green crosshair if we have a target
	x = 0;
	if(iGotTarget) {
		x = 6;
		iGotTarget = false;
	}
	
	if(iLocal)
		DrawImageAdv(bmpDest,gfxGame.bmpCrosshair,x,0,cx-2,cy-2,6,6);

	//
	// Draw the worm
	//
	x = (int) ( (vPos.x-wx)*2+l );
	y = (int) ( (vPos.y-wy)*2+t );

	// Find the right pic
	f = ((int)fFrame*7)*32;
	ang = (int)( (fAngle+90)/151 * 7 );
	//f+=ang*16;
	f+=ang*32;

	// Snap the position to a slighter bigger pixel grid (2x2)
	x -= x % 2;
	y -= y % 2;

    
	// Draw the worm
    DrawRectFill(bmpShadowPic,0,0,32,18,MakeColour(255,0,255));
	if(iDirection == DIR_RIGHT)
        DrawImageAdv(bmpShadowPic, bmpWorm, f,0, 6,0, 32,18);
		//DrawImageAdv(bmpDest, bmpWorm, f,0, x-12,y-10, 32,18);
		//DrawImageStretch2Key(bmpDest,bmpWorm,f,0,x-12,y-10,16,9,PlayerPink);
	else
        DrawImageAdv_Mirror(bmpShadowPic, bmpWorm, f,0, 0,0, 32,18);
		//DrawImageAdv_Mirror(bmpDest, bmpWorm, f,0, x-18,y-10, 32,18);
		//DrawImageStretchMirrorKey(bmpDest,bmpWorm,f,0,x-18,y-10,16,9,PlayerPink);

    DrawImage(bmpDest, bmpShadowPic, x-18,y-10);

    

	
	// Debug: Show the actual worm pos
	/*x = (int)( (vPos.x-wx)*2+l );
	y = (int)( (vPos.y-wy)*2+t );
	
	// Snap the position to a slighter bigger pixel grid (2x2)
	x -= x % 2;
	y -= y % 2;*/

    /*x = (int)( (tLX->debug_pos.x-wx)*2+l );
	y = (int)( (tLX->debug_pos.y-wy)*2+t );
    DrawRectFill(bmpDest, x-5,y-5,x+5,y+5,0);*/


	
	//
	// Draw the muzzle flash
	//
	if (iDrawMuzzle)  {
		switch(iDirection) {

		case DIR_RIGHT:  {
			ang = (int)( (fAngle+90)/151 * 7 );
			ang = 6-ang;
			f = ang*16;
			DrawImageStretch2Key(bmpDest,gfxGame.bmpMuzzle,f,0,(x-12)+RightMuzzle[ang*2],
															   (y-10)+RightMuzzle[ang*2+1],
															   16,16,PlayerPink);
		}
		break;

		case DIR_LEFT: {
			ang = (int)( (fAngle+90)/151 * 7 );
			f = (ang+7)*16;
			
			
			DrawImageStretch2Key(bmpDest,gfxGame.bmpMuzzle,f,0,(x-21)+LeftMuzzle[ang*2],
															   (y-10)+LeftMuzzle[ang*2+1],
															   16,16,PlayerPink);
		}
		break;

		}  // switch
	} // if
	iDrawMuzzle = false;
	




	/*wpnslot_t *Slot = &tWeapons[CurrentWeapon];

	// Draw the player's weapon info
	DrawRectFill(bmpDest,l,t+3,l+Health,t+8,MakeColour(64,255,64));
	DrawRectFill(bmpDest,l,t+10,l+(Slot->Charge * 100.0f),t+15,MakeColour(64,64,255));

	if(Slot->Reloading)
		GfxGui.Font.Draw(bmpDest,l,t+5,0xffff,"Reloading...");*/

	wpnslot_t *Slot = &tWeapons[iCurrentWeapon];

	// Draw the weapon name
    if(iLocal && iType == PRF_HUMAN) {
        if(bForceWeapon_Name || cSelWeapon.isDown()) {
		    tLX->cOutlineFont.DrawCentre(bmpDest,x,y-30,0xffffff,"%s",Slot->Weapon->Name);

            if( tLX->fCurTime > fForceWeapon_Time )
                bForceWeapon_Name = false;
        }
    }

	// Draw the worm's name
	if(!iLocal || (iLocal && iType != PRF_HUMAN)) {
		//tLX->cFont.DrawCentre(bmpDest,x+1,y-29,0,"%s",sName);
		if (tGameInfo.iGameMode == GMT_TEAMDEATH && tLXOptions->iColorizeNicks)  {
									// Blue				Red				Green			Yellow
			Uint8 teamcolours[] = {0x02,0xB8,0xFC,  0xFF,0x02,0x02,  0x20,0xFD,0x00,  0xFD,0xF4,0x00};
			Uint8 clR = teamcolours[iTeam*3];
			Uint8 clG = teamcolours[iTeam*3+1];
			Uint8 clB = teamcolours[iTeam*3+2];
			Uint32 iColor = MakeColour(clR,clG,clB);
			tLX->cOutlineFont.DrawCentre(bmpDest,x,y-WormNameY,iColor,"%s",sName);
		} // if
		else
		  tLX->cOutlineFont.DrawCentre(bmpDest,x,y-WormNameY,0xffff,"%s",sName);

		//if(iTagIT)
		//	tLX->cFont.DrawCentre(bmpDest, x,y+20, 0xffff,"%s", "IT");
	}
}


///////////////////
// Draw the worm's shadow
void CWorm::DrawShadow(SDL_Surface *bmpDest, CMap *map, CViewport *v)
{
    if( !v )
        return;

    if( tLXOptions->iShadows )
        map->DrawObjectShadow(bmpDest, bmpShadowPic, 0,0, 32,18, v, (int) vPos.x-9,(int) vPos.y-5);
}


///////////////////
// Quickly check if we are on the ground
int CWorm::CheckOnGround(CMap *map)
{
	int px = (int)vPos.x;
	int py = (int)vPos.y;

	for(int y=6;y>0;y--) {

		// Optimize: pixelflag + Width
		if(!(map->GetPixelFlag(px-2,py+y) & PX_EMPTY))
			return true;
		if(!(map->GetPixelFlag(px+2,py+y) & PX_EMPTY))
			return true;
	}

	return false;
}


///////////////////
// Injure me
// Returns true if i was killed by this injury
int CWorm::Injure(int damage)
{
	iHealth-=damage;

	if(iHealth<0) {
		iHealth = 0;
		return true;
	}

	return false;
}


///////////////////
// Kill me
// Returns true if we are out of the game
int CWorm::Kill(void)
{
	iAlive = false;
	fTimeofDeath = tLX->fCurTime;

	// -2 means there is no lives starting value
	if(iLives == -2)
		return false;

	iLives--;
	if(iLives == -1)
		return true;

	return false;
}


///////////////////
// Check if we have collided with a bonus
int CWorm::CheckBonusCollision(CBonus *b)
{
	CVec diff = vPos - b->getPosition();

	if(fabs(diff.x) < 7 &&
	   fabs(diff.y) < 7)
		return true;

	return false;
}


///////////////////
// Give me a bonus
// Returns true if we picked it up
int CWorm::GiveBonus(CBonus *b)
{
	// Weapon
	if(b->getType() == BNS_WEAPON) {

		// Replace our current weapon
		tWeapons[iCurrentWeapon].Weapon = cGameScript->GetWeapons() + b->getWeapon();
		tWeapons[iCurrentWeapon].Charge = 1;
		tWeapons[iCurrentWeapon].Reloading = false;

		return true;
	}


	// Health
	if(b->getType() == BNS_HEALTH) {

		// If our health is at 100, don't pick it up
		if(iHealth == 100)
			return false;


		// Health between 10% - 50%
		int health = GetRandomInt(40)+10;

		iHealth += health;

		// Clamp it
		iHealth = MIN(iHealth, 100);

		return true;
	}

	return false;
}

///////////////////
// Get the worm's ping
int CWorm::GetMyPing(void)
{
	if (!cServer)
		return 0;
	CClient *cl = cServer->getClient(getID());
	if (!cl)
		return 0;
	return cl->getPing();
}

///////////////////
// Resturns true, if we can start typing
int CWorm::CanType(void)
{
	int result = 0;
	result += cUp.isDown() + cDown.isDown() + cLeft.isDown() + cRight.isDown() + cShoot.isDown() + cJump.isDown() + cSelWeapon.isDown() + cInpRope.isDown();
	return result == 0;
}
