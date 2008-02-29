/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Worm class
// Created 28/6/02
// Jason Boettcher

#include <iostream>
#include <assert.h>

#include "LieroX.h"
#include "CClient.h"
#include "CServer.h"
#include "Graphics.h"
#include "GfxPrimitives.h"
#include "CWorm.h"
#include "CBar.h"
#include "MathLib.h"


///////////////////
// Clear the worm details
void CWorm::Clear(void)
{
	bUsed = false;
	iID = 0;
	iTeam = 0;
	bLocal = false;
	iType = PRF_HUMAN;
	iRanking = 0;
	iClientID = 0;
	iClientWormID = 0;
    szSkin = "";
	cOwner = NULL;
    
	iKills = 0;
	iDeaths = 0;
	iSuicides = 0;

	iKillsInRow = 0;
	iDeathsInRow = 0;

	iHealth = 100;
	iLives = 10;
	bAlive = false;
	iDirection = DIR_RIGHT;
	fAngle = 0;
    fAngleSpeed = 0;
    fMoveSpeedX = 0;
	iCarving = 0;
	fFrame = 0;
	bDrawMuzzle = false;

	bOnGround = false;
	vPos = CVec(0,0);
	vVelocity = CVec(0,0);
	vFollowPos = CVec(0,0);
	bFollowOverride = false;
	fLastUpdateWritten = -9999;
	tLastState = worm_state_t();
	fLastAngle = -1;
	iLastCharge = 255;
	iLastCurWeapon = 255;

	cNinjaRope.Clear();
	fRopeTime = -9999;
	bRopeDownOnce = false;
	bRopeDown = false;

	bWeaponsReady = false;
	iNumWeaponSlots = 2;
	iCurrentWeapon = 0;
	bGameReady = false;

	bGotTarget = false;
	bHooked = false;
	pcHookWorm = NULL;

	bTagIT = false;
	fTagTime = 0;
	fLastSparkle = -99999;
    iDirtCount = 0;

	fLastBlood = -9999;

	fPreLastPosUpdate = fLastPosUpdate = -9999;

	bUsesMouse = false;
	fLastInputTime = tLX->fCurTime;

	//pcViewport = NULL;//.Setup(0,0,640,480);
	tProfile = NULL;

	cGameScript = NULL;
	short i;
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
	pathSearcher = NULL;

	bmpWormLeft = NULL;
	bmpWormRight = NULL;
	bmpGibs = NULL;
	bmpPic = NULL;
    bmpShadowPic = NULL;

	// Lobby
	tLobbyState.bHost = false;
	tLobbyState.bReady = false;
	tLobbyState.iType = LBY_OPEN;
    tLobbyState.iTeam = 0;

    bForceWeapon_Name = false;

	fLastFace = 0;
	fBadAimTime = 0;

	fLastShoot = 0; // for AI
	fLastJump = 999999;
	fLastWeaponChange = 0;
	fLastCompleting = -9999;

	// Graphics
	cHealthBar = CBar(LoadImage("data/frontend/worm_health.png", true), 0, 0, 0, 0, BAR_LEFTTORIGHT);
	cHealthBar.SetLabelVisible(false);
	
	ProfileGraphics = true;
	bAlreadyKilled = false;

	bNoShooting = false;
	bFlag = false;
	lastMoveTime = 0;
	
	fLastSimulationTime = tLX->fCurTime;
}


///////////////////
// Initialize the worm
void CWorm::Init(void)
{
	// TODO: is this needed?
	tState = worm_state_t();
}


///////////////////
// Shutdown the worm
void CWorm::Shutdown(void)
{
	FreeGraphics();

    // Shutdown the AI
    if(iType == PRF_COMPUTER && bLocal)
        AI_Shutdown();
}


///////////////////
// Free the graphics
void CWorm::FreeGraphics(void)
{
	gfxFreeSurface(bmpWormLeft);
	bmpWormLeft = NULL;

	gfxFreeSurface(bmpWormRight);
	bmpWormRight = NULL;
	
	gfxFreeSurface(bmpPic);
	bmpPic = NULL;

    gfxFreeSurface(bmpShadowPic);
    bmpShadowPic = NULL;

	gfxFreeSurface(bmpGibs);
	bmpGibs = NULL;
}




///////////////////
// Prepare the worm for the game
void CWorm::Prepare(CMap *pcMap)
{
	assert(cGameScript);

	this->pcMap = pcMap;

	// Setup the rope
	cNinjaRope.Setup(cGameScript);

	iCurrentWeapon = 0;

    // If this is an AI worm, initialize the AI stuff
    if(iType == PRF_COMPUTER && bLocal)
        AI_Initialize();
       
    // we use the normal init system first after the weapons are selected and we are ready
	StopInputSystem();
}


void CWorm::StartGame() {
	InitInputSystem();
}


///////////////////
// Setup the lobby details
void CWorm::setupLobby(void)
{
	tLobbyState.bHost = false;
	tLobbyState.iType = LBY_USED;
	tLobbyState.bReady = false;
    tLobbyState.iTeam = 0;
}


///////////////////
// Spawn this worm
void CWorm::Spawn(CVec position) {
    bAlive = true;
	bAlreadyKilled = false;
	fAngle = 0;
    fAngleSpeed = 0;
    fMoveSpeedX = 0;
	iHealth = 100;
	iDirection = DIR_RIGHT;
	iMoveDirection = DIR_RIGHT;
	fLastInputTime = tLX->fCurTime;
	vPos = vDrawPos = vLastPos = vPreOldPosOfLastPaket = vOldPosOfLastPaket = position;
	vPreLastEstimatedVel = vLastEstimatedVel = vVelocity = CVec(0,0);
	cNinjaRope.Clear();
    nAIState = AI_THINK;
	fLastShoot = 0;
	
	iCarving = 0;
	fFrame = 0;
	bDrawMuzzle = false;
	bHooked = false;
    bForceWeapon_Name = false;

	bOnGround = false;
    iNumWeaponSlots = 5;

	// Reset the weapons
	for(ushort n = 0; n < iNumWeaponSlots; n++) {
		tWeapons[n].Charge = 1;
		tWeapons[n].Reloading = false;
		tWeapons[n].SlotNum = n;
		tWeapons[n].LastFire = 0;
	}

	fSpawnTime = fPreLastPosUpdate = fLastPosUpdate = fLastSimulationTime = tLX->fCurTime;

    if(iType == PRF_COMPUTER && bLocal)
		AI_Respawn();
	else if(cClient->OwnsWorm(this))
		clearInput();
}


///////////////////
// Respawn this worm
// TODO: what is the difference between Respawn and Spawn?
void CWorm::Respawn(CVec position) {
	vPos = vDrawPos = vLastPos = vPreOldPosOfLastPaket = vOldPosOfLastPaket = position;
    nAIState = AI_THINK;
	
	iCarving = 0;
	fFrame = 0;
	bDrawMuzzle = false;
	bHooked = false;
    bForceWeapon_Name = false;
	vPreLastEstimatedVel =vLastEstimatedVel = vVelocity = CVec(0,0);

	bOnGround = false;

	fLastSimulationTime = fSpawnTime = fPreLastPosUpdate = fLastPosUpdate = tLX->fCurTime;

    if(iType == PRF_COMPUTER && bLocal)
		AI_Respawn();
	else if(cClient->OwnsWorm(this))
		clearInput();
}


///////////////////
// Load the graphics
bool CWorm::LoadGraphics(int gametype)
{
	// TODO: create some good way to allow custom colors
	
	bool team = false;
    Uint8 r=0,g=0,b=0;
    
	// Destroy any previous graphics
	FreeGraphics();

	// Only load the profile graphics once
	if(ProfileGraphics) {
		LoadProfileGraphics();
		ProfileGraphics = false;
	}
		
	Uint32 colour = iColour;
	// If we are in a team game, use the team colours
    if(gametype == GMT_TEAMDEATH || gametype == GMT_VIP) {
		team = true;
		colour = tLX->clTeamColors[iTeam];
	}

    // Use the colours set on the network
    // Profile or team colours will override this
	GetColour3(colour, getMainPixelFormat(), &r, &g, &b);

    // Colourise the giblets
	bmpGibs = ChangeGraphics("data/gfx/giblets.png", team);

    // Load the skin
    bmpWormRight = LoadSkin(szSkin, r,g,b);	
	if (!bmpWormRight)  {
		// HINT: should not happen because the default skin should be *always* available (else the game doesn't start)
		bmpWormLeft = NULL;
		bmpPic = NULL;
		bmpShadowPic = NULL;
		return false;
	}
	bmpWormLeft = GetMirroredImage(bmpWormRight);
    
    // Create the minipic
    bmpPic = gfxCreateSurface(18,16);
    SetColorKey(bmpPic);
    FillSurfaceTransparent(bmpPic);
    CopySurface(bmpPic, bmpWormRight, 134,2,0,0, 18,16);

	
    // Shadow buffer
    bmpShadowPic = gfxCreateSurface(32,18);
    SetColorKey(bmpShadowPic);

	return bmpWormRight != NULL && bmpWormLeft != NULL && 
			bmpGibs != NULL && bmpPic != NULL && bmpShadowPic != NULL;
}

///////////////////
// Load the graphics
void CWorm::LoadProfileGraphics() {
    if(tProfile) {
		iColour = MakeColour(tProfile->R, tProfile->G, tProfile->B);
        szSkin = tProfile->szSkin;
    }/* else
    	printf("WARNING: LoadProfileGraphics: tProfile isn't set\n");*/
}

///////////////////
// Change the graphics of an image
SDL_Surface *CWorm::ChangeGraphics(const std::string& filename, int team)
{
	SDL_Surface *img;
	SDL_Surface *loaded;

	// Load the image
	loaded = LoadImage(filename);
	if(loaded == NULL) {
		// Error: Couldn't load image
		printf("CWorm::ChangeGraphics: Error: Could not load image %s\n", filename.c_str());
		return NULL;
	}

	img = gfxCreateSurface(loaded->w,loaded->h);
	if (img == NULL)  {
		printf("CWorm::ChangeGraphics: Not enough of memory.");
		return NULL;
	}
	DrawImage(img,loaded,0,0); // Blit to the new surface

	SetColorKey(img);


	// Set the colour of the img
	int x,y;
	Uint8 r,g,b;
	Uint32 pixel;
	
	Uint32 colour = iColour;
	if (team)
		colour = tLX->clTeamColors[iTeam];

	GetColour3(colour,getMainPixelFormat(),&r,&g,&b);

	int ColR = r;
	int ColG = g;
	int ColB = b;
	float r2,g2,b2;

	float dr, dg, db;
	const Uint32 gun1 = MakeColour(216,216,216);
	const Uint32 gun2 = MakeColour(180,180,180);
	const Uint32 gun3 = MakeColour(144,144,144);

	if (!LockSurface(img))
		return img;

	for(y = 0; y < img->h; y++) {
		for(x = 0; x < img->w; x++) {

			pixel = GetPixel(img,x,y);
			GetColour3(pixel,img->format,&r,&g,&b);

			// Ignore pink & gun colours
			if(IsTransparent(img,pixel))
				continue;
			else if(pixel == gun1)
				continue;
			else if(pixel == gun2)
				continue;
			else if(pixel == gun3)
				continue;

			dr = (float)r / 96.0f;
			dg = (float)g / 156.0f;
			db = (float)b / 252.0f;

			r2 = (float)ColR * dr;
			g2 = (float)ColG * dg;
			b2 = (float)ColB * db;

			r2 = MIN(255.0f,r2);
			g2 = MIN(255.0f,g2);
			b2 = MIN(255.0f,b2);

			// Make sure it isn't exactly 'magic pink'
			if(MakeColour((int)r2, (int)g2, (int)b2) == tLX->clPink) {
				r2=240;
				b2=240;
			}
			
			PutPixel(img,x,y, MakeColour((int)r2, (int)g2, (int)b2));
		}
	}

	UnlockSurface(img);

	return img;
}

///////////////////
// Initialize the weapon selection screen
void CWorm::InitWeaponSelection(void)
{
	// This is used for the menu screen as well
	iCurrentWeapon = 0;

	bWeaponsReady = false;
	
	iNumWeaponSlots = 5;

	// Load previous settings from profile
	short i;
	for(i=0;i<iNumWeaponSlots;i++) {

		tWeapons[i].Weapon = cGameScript->FindWeapon( tProfile->sWeaponSlots[i] );

        // If this weapon is not enabled in the restrictions, find another weapon that is enabled
        if( !cWeaponRest->isEnabled( tWeapons[i].Weapon->Name ) ) {

            tWeapons[i].Weapon = cGameScript->FindWeapon( cWeaponRest->findEnabledWeapon( cGameScript ) );
        }
	}


	// If this is an AI worm, lets give him a preset or random arsenal
	if(iType == PRF_COMPUTER && bLocal) {

		// TODO: move this to CWorm_AI
		bool bRandomWeaps = true;
		// Combo (rifle)
		if ((tGameInfo.iLoadingTimes > 15 && tGameInfo.iLoadingTimes < 26) && (tGameInfo.sModName.find("Classic") != std::string::npos || tGameInfo.sModName.find("Liero v1.0") != std::string::npos ))  {
			if (cWeaponRest->isEnabled("Rifle"))  {
				for (i=0; i<5; i++)
					tWeapons[i].Weapon = cGameScript->FindWeapon("Rifle");  // set all weapons to Rifle
				bRandomWeaps = false;
				AI_SetGameType(GAM_RIFLES);
			}
		}
		// 100 lt
		else if ((tGameInfo.sModName.find("Liero") != std::string::npos || tGameInfo.sModName.find("Classic") != std::string::npos) && tGameInfo.iLoadingTimes == 100)  {
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
		else if ((tGameInfo.sModName.find("MW 1.0") != std::string::npos || tGameInfo.sModName.find("Modern Warfare1.0") != std::string::npos) && tGameInfo.iLoadingTimes < 50)  {
			if (cWeaponRest->isEnabled("Mortar Launcher"))  {
				for (i=0; i<5; i++)
					tWeapons[i].Weapon = cGameScript->FindWeapon("Mortar Launcher");  // set all weapons to Mortar
				bRandomWeaps = false;
				AI_SetGameType(GAM_MORTARS);
			}
		}

		// Random
		if (bRandomWeaps) {
			GetRandomWeapons();
			AI_SetGameType(GAM_OTHER);
		}

		setWeaponsReady(true);
	}

	
	for(short n=0;n<iNumWeaponSlots;n++) {
		tWeapons[n].Charge = 1;
		tWeapons[n].Reloading = false;
		tWeapons[n].SlotNum = n;
		tWeapons[n].LastFire = 0;
	}
	// Skip weapon selection dialog if we're spectating
	if( getClient()->getSpectate() )
		bWeaponsReady = true;
}

///////////////////
// Randomize the weapons
void CWorm::GetRandomWeapons(void)
{
	int lastenabled = 0;
	int num,n;

	for(short i=0; i<5; i++) {
		num = GetRandomInt(cGameScript->GetNumWeapons()-1);

		// Safety hack
		if (!num)
			num = 1;
		
        // Cycle through weapons starting from the random one until we get an enabled weapon
        n=num;
		lastenabled = 0;
		while(1) {
			// Wrap around
			if(n >= cGameScript->GetNumWeapons())  
 			   n = 0;

			// Have we already got this weapon?
			bool bSelected = false;
			for(int k=0; k<i; k++) {
				if((cGameScript->GetWeapons()+n)->ID == tWeapons[k].Weapon->ID) {
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
		tWeapons[i].Weapon = cGameScript->GetWeapons()+n;
	}

}

///////////////////
// Draw/Process the weapon selection screen
void CWorm::SelectWeapons(SDL_Surface *bmpDest, CViewport *v)
{
	// TODO: this should also be used for selecting the weapons for the bot (but this in CWorm_AI then)
	// TODO: reduce local variables in this function
	// TODO: make this function shorter
	// TODO: give better names to local variables
	
	if(bDedicated) return; // just for safty; atm this function only handles non-bot players
	
	int l = 0;
	int t = 0;
	short i;
	int centrex = 320; // TODO: hardcoded width here

    if( v ) {
        if( v->getUsed() ) {
            l = v->GetLeft();
	        t = v->GetTop();
            centrex = v->GetLeft() + v->GetVirtW()/2;
        }
    }
	
	tLX->cFont.DrawCentre(bmpDest, centrex, t+30, tLX->clWeaponSelectionTitle, "~ Weapons Selection ~");
	tLX->cFont.DrawCentre(bmpDest, centrex, t+48, tLX->clWeaponSelectionTitle, "(Use up/down and left/right for selection.)");
	tLX->cFont.DrawCentre(bmpDest, centrex, t+66, tLX->clWeaponSelectionTitle, "(Go to 'Done' and press shoot then.)");
	//tLX->cOutlineFont.DrawCentre(bmpDest, centrex, t+30, tLX->clWeaponSelectionTitle, "Weapons Selection");
	//tLX->cOutlineFont.DrawCentre(bmpDest, centrex, t+30, tLX->clWeaponSelectionTitle, "Weapons Selection");

	bool bChat_Typing = cClient->isTyping();

	int y = t + 100;
	for(i=0;i<iNumWeaponSlots;i++) {
		
		//tLX->cFont.Draw(bmpDest, centrex-69, y+1, 0,"%s", tWeapons[i].Weapon->Name.c_str());
		if(iCurrentWeapon == i)
			tLX->cOutlineFont.Draw(bmpDest, centrex-70, y, tLX->clWeaponSelectionActive,  tWeapons[i].Weapon->Name);
		else
			tLX->cOutlineFont.Draw(bmpDest, centrex-70, y, tLX->clWeaponSelectionDefault,  tWeapons[i].Weapon->Name);

		if (bChat_Typing)  {
			y += 18;
			continue;
		}

		// Changing weapon
		if(iCurrentWeapon == i && !bChat_Typing) {
			int change = cRight.wasDown() - cLeft.wasDown();
			if(cSelWeapon.isDown()) change *= 6; // jump with multiple speed if selWeapon is pressed
			int id = tWeapons[i].Weapon->ID;
			if(change > 0) while(change) {
				id++; MOD(id, cGameScript->GetNumWeapons());
				if( cWeaponRest->isEnabled( cGameScript->GetWeapons()[id].Name ) )
					change--;
				if(id == tWeapons[i].Weapon->ID) // back where we were before
					break;
			} else
			if(change < 0) while(change) {
				id--; MOD(id, cGameScript->GetNumWeapons());
				if( cWeaponRest->isEnabled( cGameScript->GetWeapons()[id].Name ) )
					change++;
				if(id == tWeapons[i].Weapon->ID) // back where we were before
					break;
			}
			tWeapons[i].Weapon = &cGameScript->GetWeapons()[id];
		}
		
		y += 18;
	}

	for(i=0;i<5;i++)
		tProfile->sWeaponSlots[i] = tWeapons[i].Weapon->Name;

    // Note: The extra weapon weapon is the 'random' button
    if(iCurrentWeapon == iNumWeaponSlots) {

		// Fire on the random button?
		if((cShoot.wasDown()) && !bChat_Typing) {
			GetRandomWeapons();
		}
	}


	// Note: The extra weapon slot is the 'done' button
	if(iCurrentWeapon == iNumWeaponSlots+1) {

		// Fire on the done button?
		// we have to check isUp() here because if we continue while it is still down, we will fire after in the game
		if((cShoot.isUp()) && !bChat_Typing) {
			bWeaponsReady = true;
			iCurrentWeapon = 0;

			// Set our profile to the weapons (so we can save it later)
			for(byte i=0;i<5;i++)
				tProfile->sWeaponSlots[i] = tWeapons[i].Weapon->Name;
		}
	}

    

	// AI Worms select their weapons automatically
	if(iType == PRF_COMPUTER && bLocal) {
		bWeaponsReady = true;
		iCurrentWeapon = 0;
	}


    y+=5;
	if(iCurrentWeapon == iNumWeaponSlots)
		tLX->cOutlineFont.DrawCentre(bmpDest, centrex, y, tLX->clWeaponSelectionActive, "Random");
	else
		tLX->cOutlineFont.DrawCentre(bmpDest, centrex, y, tLX->clWeaponSelectionDefault, "Random");

    y+=18;

	if(iCurrentWeapon == iNumWeaponSlots+1)
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

		
	if(!bChat_Typing) {
		// move selection up or down
		int change = cDown.wasDown() - cUp.wasDown();
		iCurrentWeapon += change;
		iCurrentWeapon %= iNumWeaponSlots + 2;
		if(iCurrentWeapon < 0) iCurrentWeapon += iNumWeaponSlots + 2;
	}	
}


// Muzzle flash positions for different angles
int	RightMuzzle[14] = {2,3, 5,3, 4,0, 5,-8, 3,-9, 2,-13, -2,-12};
int	LeftMuzzle[14] =  {4,-12, -1,-12, -1,-9, -3,-8, -2,0, -2,4, 1,3};

// TODO: what is this??
void DrawWormName(SDL_Surface* dest, const std::string& name, Uint32 x, Uint32 y) {
}


void CWorm::UpdateDrawPos() {
	if( tLXOptions->bAntilagMovementPrediction && !cClient->OwnsWorm(this) ) {
		//if(fLastPosUpdate > tLX->fCurTime) return; // something is wrong, we probably have not gotten any update yet

		// tmp hack
		vDrawPos = vPos;
 
		// update drawing position
		CVec vDif = vPos - vDrawPos;
		float dif = vDif.GetLength();
		if(dif > 0) {
/*			if(dif < 1)
				vDrawPos = vPos;
			else
				vDrawPos += vDif * (1/dif)
					* MAX(10.0f, MIN(dif * dif * (1.0f / 50.0f) * 40.0f, 200.0f)) * tLX->fDeltaTime;
*/		
		}
		
	
#ifdef _AI_DEBUG
/*		SDL_Surface *bmpDestDebug = pcMap->GetDebugImage();
		if (bmpDestDebug) {
			int node_x = (int)vPos.x*2, node_y = (int)vPos.y*2;
			
			if(node_x-4 >= 0 && node_y-4 >= 0 && node_x+4 < bmpDestDebug->w && node_y+4 < bmpDestDebug->h) {
				// Draw the new pos
				DrawRectFill(bmpDestDebug,node_x-4,node_y-4,node_x+4,node_y+4, MakeColour(0,255,0));
			}
		} */
#endif	
	
	} else {
		// no antilag movement prediction
		vDrawPos = vPos;
	}
}


///////////////////
// Draw the worm
void CWorm::Draw(SDL_Surface *bmpDest, CViewport *v)
{
    if( !v )
        return;
			
	//
	// Draw the ninja rope
	//		
	cNinjaRope.Draw(bmpDest,v,vDrawPos);


	int x,y,f,ang;
	int l = v->GetLeft();
	int t = v->GetTop();

	x = (int)vDrawPos.x - v->GetWorldX();
	y = (int)vDrawPos.y - v->GetWorldY();
	x*=2;
	y*=2;

	// Are we inside the viewport?
	if(x+l+10 < l || x-10 > v->GetVirtW()
	|| y+t+10 < t || y-10 > v->GetVirtH()) {
		return;
	}


	int a = (int)fAngle;
	if(iDirection == DIR_LEFT)
		a=180-a;


	int WormNameY = tLX->cFont.GetHeight()+12; // Font height + worm height/2 + some space


	if (tLXOptions->bShowHealth)  {
		if (!bLocal || iType != PRF_HUMAN)  {
			int hx = x + l;
			int hy = y + t - 9; // -8 = worm height/2

			if (cHealthBar.IsProperlyLoaded())  {

				cHealthBar.SetX(hx - cHealthBar.GetWidth() / 2);
				cHealthBar.SetY(hy - cHealthBar.GetHeight() - 1);
				cHealthBar.Draw( bmpDest );
				cHealthBar.SetPosition(getHealth());
				WormNameY += cHealthBar.GetHeight()+2; // Leave some space

			} else {  // Old style healthbar
				hy -= 7;

				// Draw the "grid"
				{
					Uint32 BorderColor = MakeColour(0x49,0x50,0x65);
					DrawRect(bmpDest, hx-10,hy-1,hx+15,hy+5,BorderColor);
					DrawVLine(bmpDest, hy, hy+4, hx-5,BorderColor);
					DrawVLine(bmpDest, hy, hy+4, hx,BorderColor);
					DrawVLine(bmpDest, hy, hy+4, hx+5,BorderColor);
					DrawVLine(bmpDest, hy, hy+4, hx+10,BorderColor);
				}
											// Red			Orange				Yellow		   Light Green		  Green	
				static const Uint8 HealthColors[15] = {0xE3,0x04,0x04,  0xFE,0x85,0x03,  0xFE,0xE9,0x03,  0xA8,0xFE,0x03,  0x21,0xFE,0x03};

				// Clamp it
				int iShowHealth = Round((float)((getHealth()+15)/20));
				if (iShowHealth > 5)
					iShowHealth = 5;

				Uint32 CurColor;
				for (short i=0; i<iShowHealth; i++) {
					CurColor = MakeColour(HealthColors[i*3],HealthColors[i*3+1],HealthColors[i*3+2]);
					DrawRectFill(bmpDest,hx-10+(i*5+1),hy,hx-10+(i*5+1)+4,hy+5,CurColor);
				}

				WormNameY += 7;

			}

		}
	}


	//
	// Draw the crosshair
	//
	CVec forw;
	GetAngles(a, &forw, NULL);
	forw *= 16.0f;

	int cx = (int)forw.x + (int)vDrawPos.x;
	int cy = (int)forw.y + (int)vDrawPos.y;

	cx = (cx - v->GetWorldX()) * 2 + l;
	cy = (cy - v->GetWorldY()) * 2 + t;

	// Snap the position to a slighter bigger pixel grid (2x2)
	cx -= cx % 2;
	cy -= cy % 2;

	// Show a green crosshair if we have a target
	x = 0;
	if (bGotTarget) {
		x = 6;
		bGotTarget = false;
	}
	
	if(bLocal)
		DrawImageAdv(bmpDest, gfxGame.bmpCrosshair, x, 0, cx - 2, cy - 2, 6, 6);

	//
	// Draw the worm
	//
	x = (int) ( (vDrawPos.x - v->GetWorldX()) * 2 + l );
	y = (int) ( (vDrawPos.y - v->GetWorldY()) * 2 + t );

	// Find the right pic
	f = ((int)fFrame*7)*32;
	ang = (int)( (fAngle+90)/151 * 7 );
	//f+=ang*16;
	f+=ang*32;


	// Snap the position to a slighter bigger pixel grid (2x2)
	x -= x % 2;
	y -= y % 2;

    
	// Draw the worm
    FillSurfaceTransparent(bmpShadowPic);
	if(iDirection == DIR_RIGHT)
        CopySurface(bmpShadowPic, bmpWormRight, f,0, 6,0, 32,18);
	else
        CopySurface(bmpShadowPic, bmpWormLeft, bmpWormLeft->w-f-32,0, 0,0, 32,18);

    DrawImage(bmpDest, bmpShadowPic, x-18,y-10);

    

	
	// Debug: Show the actual worm pos
	/*x = (int)( (vPos.x-wx)*2+l );
	y = (int)( (vPos.y-wy)*2+t );
	
	// Snap the position to a slighter bigger pixel grid (2x2)
	x -= x % 2;
	y -= y % 2;*/

    /*x = (int)( (tLX->debug_pos.x-wx)*2+l );
	y = (int)( (tLX->debug_pos.y-wy)*2+t );
    DrawRectFill(bmpDest, x-5,y-5,x+5,y+5,tLX->clBlack);*/

	
	//
	// Draw the muzzle flash
	//
	if (bDrawMuzzle)  {
		switch(iDirection) {

		case DIR_RIGHT:
			ang = (int)( (fAngle+90)/151 * 7 );
			ang = 6-ang;
			f = ang*16;
			DrawImageStretch2Key(bmpDest,gfxGame.bmpMuzzle,f,0,
				(x-12)+RightMuzzle[ang*2],
				(y-10)+RightMuzzle[ang*2+1],
				16,16);
			break;

		case DIR_LEFT:
			ang = (int)( (fAngle+90)/151 * 7 );
			f = (ang+7)*16;
			DrawImageStretch2Key(bmpDest,gfxGame.bmpMuzzle,f,0,
				(x-21)+LeftMuzzle[ang*2],
				(y-10)+LeftMuzzle[ang*2+1],
				16,16);
			break;

		}  // switch
	} // if
	bDrawMuzzle = false;
	

	wpnslot_t *Slot = &tWeapons[iCurrentWeapon];

	// Draw the weapon name
    if(bLocal && iType == PRF_HUMAN) {
        if(bForceWeapon_Name || cSelWeapon.isDown()) {
		    tLX->cOutlineFont.DrawCentre(bmpDest,x,y-30,tLX->clPlayerName,Slot->Weapon->Name);

            if( tLX->fCurTime > fForceWeapon_Time )
                bForceWeapon_Name = false;
        }
    }

	// Draw the worm's name
	if(!bLocal || (bLocal && iType != PRF_HUMAN)) {
		//tLX->cFont.DrawCentre(bmpDest,x+1,y-29,0,sName);
		if ((tGameInfo.iGameMode == GMT_TEAMDEATH || tGameInfo.iGameMode == GMT_VIP) && tLXOptions->bColorizeNicks)  {
			Uint32 col = tLX->clTeamColors[iTeam];
			tLX->cOutlineFont.DrawCentre(bmpDest,x,y-WormNameY,col,sName);
		} // if
		else
			tLX->cOutlineFont.DrawCentre(bmpDest,x,y-WormNameY,tLX->clPlayerName,sName);
	}
}


///////////////////
// Draw the worm's shadow
void CWorm::DrawShadow(SDL_Surface *bmpDest, CViewport *v)
{
    if( tLXOptions->bShadows && v )
    	// HINT: the move by (-9,-5) is needed as it seems that the shadowpic has the worm on this position
    	// TODO: is the above hint correct?
    	// HINT: we don't use vDrawPos but vPos here to give a hint where the player is really atm (if interpolation is too slow)
    	// TODO: if we just use vDrawPos here, we also have to fix the drawing of rifle and perhaps also of other ninjaropes
		// FIXME: just tested it, I was on a high-ping server and it was horrible! The shadow should be always synced with the worm image!
/*        if( tLXOptions->bAntilagMovementPrediction && !cClient->OwnsWorm(this) )
			// just don't draw it as a real shadow but show the real position instead (not the interpolated)
			pcMap->DrawObjectShadow(bmpDest, bmpShadowPic, 0,0, 32,18, v, (int) vOldPosOfLastPaket.x-9 - SHADOW_DROP, (int) vOldPosOfLastPaket.y-5 - SHADOW_DROP);
        else */
			pcMap->DrawObjectShadow(bmpDest, bmpShadowPic, 0,0, 32,18, v, (int) vPos.x-9,(int) vPos.y-5);
}


///////////////////
// Quickly check if we are on the ground
bool CWorm::CheckOnGround()
{
	int px = (int)vPos.x;
	int py = (int)vPos.y;

	for(short y = 6; y > 0; y--) {

		// Optimize: pixelflag + Width
		if(!(pcMap->GetPixelFlag(px - 2, py + y) & PX_EMPTY))
			return true;
		if(!(pcMap->GetPixelFlag(px + 2, py + y) & PX_EMPTY))
			return true;
	}

	return false;
}


///////////////////
// Injure me
// Returns true if i was killed by this injury
bool CWorm::Injure(int damage)
{
	if(tGameInfo.iGameType == GME_HOST && cServer) {
		// If playing CTF and I am a flag don't injure me
		if(cServer->getLobby()->nGameMode == GMT_CTF && getFlag())
			return false;
	
		// If playing teams CTF and I am a flag don't injure me
		if(cServer->getLobby()->nGameMode == GMT_TEAMCTF && getFlag())
			return false;
	}

	iHealth -= damage;

	if(iHealth < 0) {
		iHealth = 0;
		return true;
	}

	return false;
}


///////////////////
// Kill me
// Returns true if we are out of the game
bool CWorm::Kill(void)
{
	std::cout << "our worm " << iID << " died" << std::endl;
	
	bAlive = false;
	fTimeofDeath = tLX->fCurTime;

	// -2 means there is no lives starting value
	if(iLives == WRM_UNLIM)
		return false;

	iLives--;
	return iLives == WRM_OUT;
}


///////////////////
// Check if we have collided with a bonus
bool CWorm::CheckBonusCollision(CBonus *b)
{
	CVec diff = vPos - b->getPosition();

	return (fabs(diff.x) < 7) && (fabs(diff.y) < 7);
}


///////////////////
// Give me a bonus
// Returns true if we picked it up
bool CWorm::GiveBonus(CBonus *b)
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
bool CWorm::CanType(void)
{
	return
		!cUp.wasDown() &&
		!cDown.wasDown() &&
		!cLeft.wasDown() &&
		!cRight.wasDown() &&
		!cShoot.wasDown() &&
		!cJump.wasDown() &&
		!cSelWeapon.wasDown() &&
		!cInpRope.wasDown() &&
		!cStrafe.wasDown();
}
