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


#include <cassert>

#include "LieroX.h"
#include "ProfileSystem.h"
#include "CClient.h"
#include "CServer.h"
#include "DeprecatedGUI/Graphics.h"
#include "GfxPrimitives.h"
#include "CWorm.h"
#include "DeprecatedGUI/CBar.h"
#include "MathLib.h"
#include "CServerConnection.h"
#include "CWormHuman.h"
#include "Debug.h"



///////////////////
// Clear the worm details
void CWorm::Clear(void)
{
	bUsed = false;
	bIsPrepared = false;
	iID = 0;
	iTeam = 0;
	bLocal = false;
	m_type = PRF_HUMAN;
	iRanking = 0;
	iClientID = 0;
	iClientWormID = 0;
	cOwner = NULL;
	bSpectating = false;
	iAFK = AFK_BACK_ONLINE;
	sAFKMessage = "";

	iKills = 0;
	iDeaths = 0;
	iSuicides = 0;
	iDamage = 0;

	iKillsInRow = 0;
	iDeathsInRow = 0;

	iHealth = 100;
	iLives = 10;
	bAlive = false;
	iDirection = DIR_RIGHT;
	fAngle = 0;
    fAngleSpeed = 0;
    fMoveSpeedX = 0;
	fFrame = 0;
	bDrawMuzzle = false;

	bOnGround = false;
	vPos = CVec(0,0);
	vVelocity = CVec(0,0);
	vFollowPos = CVec(0,0);
	bFollowOverride = false;
	fLastUpdateWritten = -9999;
	fCollisionTime = 0;
	vCollisionVelocity = CVec(0, 0);
	bCollidedLastFrame = false;
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

	//bUsesMouse = false;
	fLastInputTime = tLX->fCurTime;

	//pcViewport = NULL;//.Setup(0,0,640,480);
	tProfile = NULL;

	cGameScript = NULL;
	short i;
	for(i=0;i<NUM_FRAMES;i++)
		fFrameTimes[i] = -99999.0f;

	for(i=0; i<5; i++)
		tWeapons[i].Weapon = NULL;

    cWeaponRest = NULL;
	
	

	bmpGibs = NULL;

	// Lobby
	tLobbyState.bHost = false;
	tLobbyState.bReady = false;
	tLobbyState.iType = LBY_OPEN;
    tLobbyState.iTeam = 0;

    bForceWeapon_Name = false;

	

	
	// Graphics
	cHealthBar = DeprecatedGUI::CBar(LoadGameImage("data/frontend/worm_health.png", true), 0, 0, 0, 0, DeprecatedGUI::BAR_LEFTTORIGHT);
	cHealthBar.SetLabelVisible(false);

	bAlreadyKilled = false;

	bNoShooting = false;
	bFlag = false;

	fLastSimulationTime = tLX->fCurTime;
	pcMap = NULL;
	
	
	if(m_inputHandler) {
		delete m_inputHandler;
		m_inputHandler = NULL;
	}
	
	for( i=0; i<MAX_WORMS; i++ )
	{
		cDamageReport[i].damage = 0;
		cDamageReport[i].lastTime = 0;
	}
	
	// bmpOldGibs.clear(); // Do NOT clear old gibs - worms array is deleted anyway when client cleans up
}


///////////////////
// Initialize the worm
void CWorm::Init(void)
{
	// TODO: is this needed?
	// WARNING: this works only because it does not contain any classes
	tState = worm_state_t();
}


///////////////////
// Shutdown the worm
void CWorm::Shutdown(void)
{
	Unprepare();
	FreeGraphics();
}


///////////////////
// Free the graphics
void CWorm::FreeGraphics(void)
{
	bmpGibs = NULL;
}




///////////////////
// Prepare the worm for the game
void CWorm::Prepare(CMap *pcMap)
{
	assert(cGameScript);

	if(bIsPrepared) {
		warnings << "worm was already prepared! ";
		if(this->pcMap != pcMap) warnings << "AND pcMap differs!";
		warnings << endl;
	}
	
	this->pcMap = pcMap;

	// Setup the rope
	cNinjaRope.Setup(cGameScript);

	iCurrentWeapon = 0;

	if(m_inputHandler) {
		warnings << "WARNING: worm " << getName() << " has already the following input handler set: "; warnings.flush();
		warnings << m_inputHandler->name(); warnings << endl;
		delete m_inputHandler;
	}

	if(bLocal) {
		m_inputHandler = m_type->createInputHandler(this);
	}
	
	bIsPrepared = true;
}

void CWorm::Unprepare() {
	setGameReady(false);
	setTagIT(false);
	setTagTime(0);
	
	if(m_inputHandler) {
		if(!bLocal) {
			warnings << "WARNING: the following input handler was set for the non-local worm " << getName() << ": "; warnings.flush();
			warnings << m_inputHandler->name() << endl;
		}
		delete m_inputHandler;
		m_inputHandler = NULL;
	}
		
	bIsPrepared = false;
}

void CWorm::StartGame() {
	fTimeofDeath = tLX->fCurTime;
	m_inputHandler->startGame();
}


WormType* WormType::fromInt(int type) {
	switch(type) {
		case 0: return PRF_HUMAN;
		case 1: return PRF_COMPUTER;
		default: return NULL;
	}
}


void CWorm::getInput() {
	if(!bLocal) {
		warnings << "WARNING: called getInput() on non-local worm " << getName() << endl;
		return;
	}
	
	if(!m_inputHandler) {
		warnings << "WARNING: input handler not set for worm " << getName() << ", cannot get input" << endl;
		return;
	}
	
	m_inputHandler->getInput();
}

void CWorm::clearInput() {
	fLastInputTime = tLX->fCurTime;
	
	// Clear the state
	tState.bCarve = false;
	tState.bMove  = false;
	tState.bShoot = false;
	tState.bJump  = false;
	
	if(bLocal && m_inputHandler)
		m_inputHandler->clearInput();
}

void CWorm::initWeaponSelection() {
	if(!bLocal) {
		warnings << "WARNING: called initWeaponSelection() on non-local worm " << getName() << endl;
		return;
	}
	
	if(!m_inputHandler) {
		warnings << "WARNING: input handler not set for worm " << getName() << ", cannot init weapon selection" << endl;
		return;
	}
	
	if(this->shouldDoOwnWeaponSelection())
		m_inputHandler->initWeaponSelection();

	if(this->isHostWorm() && tLXOptions->tGameInfo.bSameWeaponsAsHostWorm && tLXOptions->tGameInfo.bForceRandomWeapons) {
		this->GetRandomWeapons();
		this->bWeaponsReady = true;		
	}

	if(this->isHostWorm() && this->bWeaponsReady && tLXOptions->tGameInfo.bSameWeaponsAsHostWorm)
		cServer->cloneWeaponsToAllWorms(this);

}

void CWorm::doWeaponSelectionFrame(SDL_Surface * bmpDest, CViewport *v) {
	if(!bLocal) {
		warnings << "WARNING: called doWeaponSelectionFrame() on non-local worm " << getName() << endl;
		return;
	}
	
	if(!m_inputHandler) {
		warnings << "WARNING: input handler not set for worm " << getName() << ", cannot do weapon selection" << endl;
		return;
	}
	
	if(bWeaponsReady) {
		warnings << "WARNING: doWeaponSelectionFrame: weapons already selected" << endl;
		return;
	}
	
	if(!this->shouldDoOwnWeaponSelection()) {
		int t = 0;
		int centrex = 320; // TODO: hardcoded screen width here
		
		if( v ) {
			if( v->getUsed() ) {
				t = v->GetTop();
				centrex = v->GetLeft() + v->GetVirtW()/2;
			}
		}
		
		tLX->cFont.DrawCentre(bmpDest, centrex, t+48, tLX->clWeaponSelectionTitle, "... Waiting for server selection ...");
		return;
	}
	
	m_inputHandler->doWeaponSelectionFrame(bmpDest, v);	
		   
	if(this->bWeaponsReady && this->isHostWorm() && tLXOptions->tGameInfo.bSameWeaponsAsHostWorm) {
		cServer->cloneWeaponsToAllWorms(this);
	}

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
	// No spawn if spectating
	if (bSpectating)
		return;

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

	if(bLocal) {
		clearInput();
		m_inputHandler->onRespawn();
	}

	for( int i=0; i<MAX_WORMS; i++ )
	{
		cDamageReport[i].damage = 0;
		cDamageReport[i].lastTime = 0;
	}
}


///////////////////
// Load the graphics
bool CWorm::ChangeGraphics(int gametype)
{
	// TODO: create some good way to allow custom colors

	bool team = false;
    Uint8 r=0,g=0,b=0;

	// Destroy any previous graphics
	FreeGraphics();

	Uint32 colour = cSkin.getDefaultColor();
	// If we are in a team game, use the team colours
    if(gametype == GMT_TEAMDEATH || gametype == GMT_VIP) {
		team = true;
		colour = tLX->clTeamColors[iTeam];
	}

    // Use the colours set on the network
    // Profile or team colours will override this
	GetColour3(colour, getMainPixelFormat(), &r, &g, &b);

    // Colourise the giblets
    bmpOldGibs.push_back(bmpGibs); // If client leaves and another joins the gib pic is recalculated - save it 'till the end of the game, 'cause it used in SpawnEntity()
	bmpGibs = ChangeGraphics("data/gfx/giblets.png", team);

    // Colourise the skin
	cSkin.Colorize(colour);

	return bmpGibs.get() != NULL;
}

///////////////////
// Change the graphics of an image
SmartPointer<SDL_Surface> CWorm::ChangeGraphics(const std::string& filename, int team)
{
	SmartPointer<SDL_Surface> img;
	SmartPointer<SDL_Surface> loaded;

	// Load the image
	loaded = LoadGameImage(filename);
	if(loaded.get() == NULL) {
		// Error: Couldn't load image
		errors << "CWorm::ChangeGraphics: Could not load image " << filename << endl;
		return NULL;
	}

	img = gfxCreateSurface(loaded.get()->w,loaded.get()->h);
	if (img.get() == NULL)  {
		errors << "CWorm::ChangeGraphics: Not enough of memory." << endl;
		return NULL;
	}
	DrawImage(img.get(),loaded,0,0); // Blit to the new surface

	SetColorKey(img.get());


	// Set the colour of the img
	int x,y;
	Uint8 r,g,b;
	Uint32 pixel;

	Uint32 colour = cSkin.getColor();
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

	for(y = 0; y < img.get()->h; y++) {
		for(x = 0; x < img.get()->w; x++) {

			pixel = GetPixel(img.get(),x,y);
			GetColour3(pixel,img.get()->format,&r,&g,&b);

			// Ignore pink & gun colours
			if(IsTransparent(img.get(),pixel))
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

			PutPixel(img.get(),x,y, MakeColour((int)r2, (int)g2, (int)b2));
		}
	}

	UnlockSurface(img);

	return img;
}



///////////////////
// Randomize the weapons
void CWorm::GetRandomWeapons(void)
{
	int lastenabled = 0;
	int num,n;

	for(short i=0; i<5; i++) {
		num = MAX(1, GetRandomInt(cGameScript->GetNumWeapons()-1)); // HINT: num must be >= 1 or else we'll loop forever in the ongoing loop

        // Cycle through weapons starting from the random one until we get an enabled weapon
        n=num;
		lastenabled = 0;
		while(true) {
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

// the first host worm (there can only be one such worm in a game)
bool CWorm::isHostWorm() {
	return tLX->iGameType != GME_JOIN && cServer->getClients()[0].getNumWorms() > 0 && cServer->getClients()[0].getWorm(0)->getID() == iID;
}

bool CWorm::shouldDoOwnWeaponSelection() {
	return !cClient->serverChoosesWeapons() || (this->isHostWorm() && tLXOptions->tGameInfo.bSameWeaponsAsHostWorm);
}

void CWorm::CloneWeaponsFrom(CWorm* w) {
	for(int wp = 0; wp < 5; wp++)
		this->getWeapon(wp)->Weapon = w->getWeapon(wp)->Weapon;
}




// Muzzle flash positions for different angles
int	RightMuzzle[14] = {2,3, 5,3, 4,0, 5,-8, 3,-9, 2,-13, -2,-12};
int	LeftMuzzle[14] =  {4,-12, -1,-12, -1,-9, -3,-8, -2,0, -2,4, 1,3};

// TODO: what is this??
void DrawWormName(SDL_Surface * dest, const std::string& name, Uint32 x, Uint32 y) {
}


void CWorm::UpdateDrawPos() {
	if( tLXOptions->bAntilagMovementPrediction && !cClient->OwnsWorm(this->getID()) ) {
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
/*		SmartPointer<SDL_Surface> bmpDestDebug = pcMap->GetDebugImage();
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
void CWorm::Draw(SDL_Surface * bmpDest, CViewport *v)
{
    if( !v )
        return;

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

	// Draw the damage amount that worm received
	// Even the worm is dead draw damage for some time anyway (looks pretty)
	if( tLXOptions->bDamagePopups )
	{
		// Sort them first
		std::map< float, int > DamageReportDrawOrder;
		int i;
		for( i=0; i<MAX_WORMS; i++ )
			if( cDamageReport[i].damage != 0 )
				DamageReportDrawOrder[cDamageReport[i].lastTime] = i;
		// Draw
		if( ! DamageReportDrawOrder.empty() )
		{
			int damageDrawPos = WormNameY + tLX->cFont.GetHeight(); 
			// Make it float up a bit when time passes
			damageDrawPos += (int)(( tLX->fCurTime - DamageReportDrawOrder.begin()->first ) * 30);

			int damageSum = 0;
			std::map< float, int > :: const_iterator it;
			for( it = DamageReportDrawOrder.begin(); it != DamageReportDrawOrder.end(); it++ )
				damageSum += cDamageReport[it->second].damage;
			Uint32 damageColor = damageSum >= 0 ? Color( 0xff, 0x80, 0x40 ).get() : Color( 0x40, 0xff, 0 ).get() ; // Red or green
			
			for( it = DamageReportDrawOrder.begin(); it != DamageReportDrawOrder.end(); it++ )
			{
				int id = it->second;
				//if( !cClient->getRemoteWorms()[id].isUsed() )
				//	continue;
				if( tLXOptions->bColorizeDamageByWorm )
				{
					damageSum = cDamageReport[id].damage;
					damageColor = cClient->getRemoteWorms()[id].getGameColour();
				}
				std::string damageStr = itoa( damageSum );
				if( damageSum < 0 )
					damageStr[0] = '+';	// Negative damage = healing
				if( getClientVersion() < OLXBetaVersion(9) )
					damageStr = "? " + damageStr; // + "\xC2\xBF"; // Inverted question mark in UTF-8
				tLX->cOutlineFont.DrawCentre(bmpDest, x + l, y + t - damageDrawPos, damageColor, damageStr);
				//printf("Print damage for worm %i = %s at %i %i\n", getID(), damageStr.c_str(), x, y-damageDrawPos);
				damageDrawPos += tLX->cFont.GetHeight();

				if( ! tLXOptions->bColorizeDamageByWorm )
					break;
			};
			// Clean up expired damage values

			if( tLXOptions->bColorizeDamageByWorm )
			{
				for( i=0; i<MAX_WORMS; i++ )
					if( tLX->fCurTime - cDamageReport[i].lastTime > 1.5f )
						cDamageReport[i].damage = 0;
			}
			else
			{
				if( tLX->fCurTime - DamageReportDrawOrder.begin()->first > 1.5f )
					for( i=0; i<MAX_WORMS; i++ )
						cDamageReport[i].damage = 0;
			}
		}
	}

	// Do not draw further if we're not alive
	if( !getAlive() )
		return;

	//
	// Draw the ninja rope
	//
	cNinjaRope.Draw(bmpDest,v,vDrawPos);

	if (tLXOptions->bShowHealth)  {
		if (!bLocal || m_type != PRF_HUMAN)  {
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
		DrawImageAdv(bmpDest, DeprecatedGUI::gfxGame.bmpCrosshair, x, 0, cx - 2, cy - 2, 6, 6);

	//
	// Draw the worm
	//
	x = (int) ( (vDrawPos.x - v->GetWorldX()) * 2 + l );
	y = (int) ( (vDrawPos.y - v->GetWorldY()) * 2 + t );

	// Find the right pic
	f = ((int)fFrame*7);
	ang = (int)( (fAngle+90)/151 * 7 );
	f += ang;


	// Snap the position to a slighter bigger pixel grid (2x2)
	x -= x % 2;
	y -= y % 2;


	// Draw the worm
	cSkin.Draw(bmpDest, x - SKIN_WIDTH/2, y - SKIN_HEIGHT/2, f, false, iDirection == DIR_LEFT);
    /*FillSurfaceTransparent(bmpShadowPic.get());
	if(iDirection == DIR_RIGHT)
        CopySurface(bmpShadowPic.get(), bmpWormRight, f,0, 6,0, 32,18);
	else
        CopySurface(bmpShadowPic.get(), bmpWormLeft, bmpWormLeft.get()->w-f-32,0, 0,0, 32,18);

    DrawImage(bmpDest, bmpShadowPic, x-18,y-10);*/




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
			DrawImageStretch2Key(bmpDest, DeprecatedGUI::gfxGame.bmpMuzzle, f, 0,
				(x-12)+RightMuzzle[ang*2],
				(y-10)+RightMuzzle[ang*2+1],
				16,16);
			break;

		case DIR_LEFT:
			ang = (int)( (fAngle+90)/151 * 7 );
			f = (ang+7)*16;
			DrawImageStretch2Key(bmpDest, DeprecatedGUI::gfxGame.bmpMuzzle, f, 0,
				(x-21)+LeftMuzzle[ang*2],
				(y-10)+LeftMuzzle[ang*2+1],
				16,16);
			break;

		}  // switch
	} // if
	bDrawMuzzle = false;


	wpnslot_t *Slot = &tWeapons[iCurrentWeapon];

	// Draw the weapon name
    if(bLocal && m_type == PRF_HUMAN) {
        if(bForceWeapon_Name || ((CWormHumanInputHandler*)m_inputHandler)->getInputWeapon().isDown()) {
		    tLX->cOutlineFont.DrawCentre(bmpDest,x,y-30,tLX->clPlayerName,Slot->Weapon->Name);

            if( tLX->fCurTime > fForceWeapon_Time )
                bForceWeapon_Name = false;
        }
    }

	// Draw the worm's name
	std::string WormName = sName;
	if( sAFKMessage != "" )
		WormName += " " + sAFKMessage;
	if(!bLocal || (bLocal && m_type != PRF_HUMAN)) {
		if ((cClient->getGameLobby()->iGameMode == GMT_TEAMDEATH || cClient->getGameLobby()->iGameMode == GMT_VIP) && tLXOptions->bColorizeNicks)  {
			Uint32 col = tLX->clTeamColors[iTeam];
			tLX->cOutlineFont.DrawCentre(bmpDest,x,y-WormNameY,col,WormName);
		} // if
		else
			tLX->cOutlineFont.DrawCentre(bmpDest,x,y-WormNameY,tLX->clPlayerName,WormName);
	} else { // local human worm
		if(iAFK != AFK_BACK_ONLINE) {
			tLX->cOutlineFont.DrawCentre(bmpDest,x,y-WormNameY,tLX->clPlayerName,".. " + sAFKMessage + " ..");
		}
	}

}


///////////////////
// Draw the worm's shadow
void CWorm::DrawShadow(SDL_Surface * bmpDest, CViewport *v)
{
	if( tLXOptions->bShadows && v )  {
    	static const int drop = 4;

		// Copied from ::Draw
		// TODO: a separate function for this
		int f = ((int)fFrame*7);
		int ang = (int)( (fAngle+90)/151 * 7 );
		f += ang;

		// Draw the shadow
		
		// NOTE: the cSkin.DrawShadow function draws a shadow over solid objects
		// Later we should render the world layer by layer so this trouble will be gone
		// The CMap::DrawObjectShadow function is slow and also logically incorrect - why should a map know about other
		// objects?
		//cSkin.DrawShadow(bmpDest, x, y, f, iDirection == DIR_LEFT);
		if (iDirection == DIR_RIGHT)
			pcMap->DrawObjectShadow(bmpDest, cSkin.getRightImage().get(), f * 32 + 4, 0, SKIN_WIDTH, SKIN_HEIGHT, v, (int)vPos.x - SKIN_WIDTH/2 + drop, (int)vPos.y - SKIN_HEIGHT/2 + drop);
		else
			pcMap->DrawObjectShadow(bmpDest, cSkin.getLeftImage().get(), cSkin.getLeftImage()->w - (f * 32 + 24), 0, SKIN_WIDTH, SKIN_HEIGHT, v, (int)vPos.x - SKIN_WIDTH/2 + drop, (int)vPos.y - SKIN_HEIGHT/2 + drop);
	}
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
	if(tLX->iGameType == GME_HOST && cServer) {
		// If playing CTF and I am a flag don't injure me
		if(cClient->getGameLobby()->iGameMode == GMT_CTF && getFlag())
			return false;

		// If playing teams CTF and I am a flag don't injure me
		if(cClient->getGameLobby()->iGameMode == GMT_TEAMCTF && getFlag())
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
//	notes << "our worm " << iID << " died" << endl;

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

		// If our health is at 100 or higher, don't pick it up
		if(iHealth >= 100) // Some mods have healing weapons, so the check is >= instead of == (breaks old behavior)
			return false;

		// Health between 10% - 50%
		int health = GetRandomInt(40)+10;

		// Route call to CClient::InjureWorm() so it will send ReportDamage packet, to track valid worm healthbar on server
		// Clamp it
		health = MIN(health, 100 - getHealth());
		cClient->InjureWorm(this, -health, getID());

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
	CServerConnection *cl = cServer->getClient(getID());
	if (!cl)
		return 0;
	return cl->getPing();
}

///////////////////
// Resturns true, if we can start (auto-)typing
bool CWorm::CanType(void)
{
	if(!bAlive && iLives == WRM_OUT)
		return true; // a worm can always type if out of game (whereby further checks for not-worm related stuff could be done elsewhere)

	if(m_type == PRF_HUMAN) {
		CWormHumanInputHandler* handler = (CWormHumanInputHandler*)m_inputHandler;
		return handler->canType();
	} else
		return true;
}

bool CWormHumanInputHandler::canType() {
	keyboard_t* kb = GetKeyboard();
	for (int i = 0; i < kb->queueLength; i++)  {
		if (cUp.getData() == kb->keyQueue[i].sym ||
			cDown.getData() == kb->keyQueue[i].sym ||
			cLeft.getData() == kb->keyQueue[i].sym ||
			cRight.getData() == kb->keyQueue[i].sym ||
			cShoot.getData() == kb->keyQueue[i].sym ||
			cJump.getData() == kb->keyQueue[i].sym ||
			cSelWeapon.getData() == kb->keyQueue[i].sym ||
			cInpRope.getData() == kb->keyQueue[i].sym ||
			cStrafe.getData() == kb->keyQueue[i].sym ||
			cWeapons[0].getData() == kb->keyQueue[i].sym ||
			cWeapons[1].getData() == kb->keyQueue[i].sym ||
			cWeapons[2].getData() == kb->keyQueue[i].sym ||
			cWeapons[3].getData() == kb->keyQueue[i].sym ||
			cWeapons[4].getData() == kb->keyQueue[i].sym)
			return false;
	}
	return true;
}
	

void CWorm::setUsed(bool _u)
{ 
	bUsed = _u; 
	if( ! _u ) 
		return;
	fLastSimulationTime = tLX->fCurTime; 
	iTotalWins = iTotalLosses =	iTotalKills = iTotalDeaths = iTotalSuicides = 0;
}

void CWorm::setAFK(AFK_TYPE afkType, const std::string & msg)
{
	iAFK = afkType;
	sAFKMessage = msg;
}

Uint32 CWorm::getGameColour(void)
{
	switch(cClient->getGameLobby()->iGameMode) {
		case GMT_TEAMDEATH:
		case GMT_VIP:
		case GMT_TEAMCTF:
			return tLX->clTeamColors[iTeam];
		default:
			return cSkin.getDefaultColor();
	}
}

void CWorm::addDamage(int damage, CWorm* victim, const GameOptions::GameInfo & settings)
{
	if( damage < 0 )	// Do not count when we heal ourselves or someone else, only damage counts
		return;

	if( getID() == victim->getID() )
	{
		if( tLXOptions->tGameInfo.features[FT_SuicideDecreasesScore] )
			setDamage( getDamage() - damage );	// Decrease damage from score if injured yourself
	}
	else if( (tLXOptions->tGameInfo.iGameMode == GMT_TEAMDEATH || tLXOptions->tGameInfo.iGameMode == GMT_VIP) && 
				getTeam() == victim->getTeam() ) 
	{
		if( tLXOptions->tGameInfo.features[FT_SuicideDecreasesScore] )
			setDamage( getDamage() - damage );	// Decrease damage from score if injured teammate
		else if( tLXOptions->tGameInfo.features[FT_CountTeamkills] )
			setDamage( getDamage() + damage );	// Count team damage along with teamkills
	}
	else
		setDamage( getDamage() + damage );
};
