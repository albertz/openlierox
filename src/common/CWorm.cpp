/////////////////////////////////////////
//
//			 OpenLieroX
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
#include "game/CWorm.h"
#include "DeprecatedGUI/CBar.h"
#include "MathLib.h"
#include "CServerConnection.h"
#include "CWormHuman.h"
#include "Debug.h"
#include "CGameMode.h"
#include "FlagInfo.h"
#include "Physics.h"
#include "WeaponDesc.h"
#include "Mutex.h"
#include "game/Game.h"
#include "gusanos/gusgame.h"
#include "gusanos/proxy_player.h"
#include "game/GameMode.h"
#include "CodeAttributes.h"
#include "CGameScript.h"
#include "gusanos/base_animator.h"
#include "gusanos/sprite_set.h"
#include "gusanos/animators.h"
#include "gusanos/weapon.h"
#include "game/ClassInfo.h"
#include "gusanos/luaapi/classes.h"


worm_state_t::worm_state_t() {
	thisRef.classId = LuaID<worm_state_t>::value;
	reset();
}

void worm_state_t::reset() {
	bShoot = bCarve = bMove = bJump = false;
}

uint8_t worm_state_t::asInt() const {
	uint8_t i = 0;
	if(bShoot) i |= 1;
	if(bCarve) i |= 2;
	if(bMove) i |= 4;
	if(bJump) i |= 8;
	return i;
}

void worm_state_t::fromInt(uint8_t i) {
	bShoot = i & 1;
	bCarve = i & 2;
	bMove = i & 4;
	bJump = i & 8;
}

CustomVar* worm_state_t::copy() const { return new worm_state_t(*this); }

bool worm_state_t::operator==(const CustomVar& o) const {
	const worm_state_t* os = dynamic_cast<const worm_state_t*>(&o);
	if(!os) return false;
	return asInt() == os->asInt();
}

bool worm_state_t::operator<(const CustomVar& o) const {
	const worm_state_t* os = dynamic_cast<const worm_state_t*>(&o);
	if(!os) return this < &o;
	return asInt() < os->asInt();
}

std::string worm_state_t::toString() const { return to_string<int>(asInt()); }
bool worm_state_t::fromString(const std::string& str) { fromInt(from_string<int>(str)); return true; }

void worm_state_t::copyFrom(const CustomVar& o) {
	const worm_state_t* os = dynamic_cast<const worm_state_t*>(&o);
	assert(os != NULL);
	*this = *os;
}

Result worm_state_t::toBytestream( CBytestream* bs ) const {
	bs->writeInt(asInt(), 1);
	return true;
}

Result worm_state_t::fromBytestream( CBytestream* bs ) {
	fromInt(bs->readInt(1));
	return true;
}


struct CWorm::SkinDynDrawer : DynDrawIntf {
	Mutex mutex;
	CWorm* worm;
	SkinDynDrawer(CWorm* w) : DynDrawIntf(WORM_SKIN_WIDTH,WORM_SKIN_HEIGHT), worm(w) {}
	~SkinDynDrawer() {}
	
	virtual void draw(SDL_Surface* bmpDest, int x, int y) {
		Mutex::ScopedLock lock(mutex);
		if(worm) {
			int f = ((int)worm->frame()*7);
			int ang = (int)( (worm->getAngle()+90)/151 * 7 );
			f += ang;
			
			// Draw the worm
			worm->getSkin().Draw(bmpDest, x, y, f, false, worm->getFaceDirectionSide() == DIR_LEFT);
		}
		else
			DrawCross(bmpDest, x - WORM_SKIN_WIDTH/2, y - WORM_SKIN_HEIGHT/2, WORM_SKIN_WIDTH, WORM_SKIN_HEIGHT, Color(255,0,0));
	}
};

CWorm::CWorm() :
	cNinjaRope(this), cSparkles(this),
	m_fireconeAnimator(NULL), m_animator(NULL)
{
	thisRef.classId = LuaID<CWorm>::value;

	// set all pointers to NULL
	m_inputHandler = NULL;
	cOwner = NULL;
	m_type = NULL;
	m_node = NULL;
	m_interceptor = NULL;

	bPrepared = false;
	bSpawnedOnce = false;
	bCanRespawnNow = false;
	iID = 0;
	iTeam = 0;
	bLocal = false;
	m_type = PRF_HUMAN;
	cOwner = NULL;
	bSpectating = false;

	iFaceDirectionSide = DIR_RIGHT;
	fAngle = 0;
	fAngleSpeed = 0;
	fMoveSpeedX = 0;
	fFrame = 0;
	bDrawMuzzle = false;
	bVisibleForWorm.clear();
	fVisibilityChangeTime = 0;

	bOnGround = false;
	fLastUpdateWritten = AbsTime();
	fLastAngle = -1;
	iLastCharge = 255;
	iLastCurWeapon = 255;

	cNinjaRope.Clear();
	fLastAirJumpTime = 0;

	bWeaponsReady = false;
	iNumWeaponSlots = 2;
	iCurrentWeapon = 0;

	bGotTarget = false;

	bTagIT = false;
	fTagTime = 0;
	iDirtCount = 0;

	fLastBlood = AbsTime();

	fPreLastPosUpdate = fLastPosUpdate = AbsTime();

	//bUsesMouse = false;
	fLastInputTime = tLX->currentTime;

	for(short i=0; i<MAX_WEAPONSLOTS; i++)
		tWeapons[i].Weapon = NULL;



	bmpGibs = NULL;

	bLobbyReady = false;

	bForceWeapon_Name = false;

	// Graphics
	cHealthBar = DeprecatedGUI::CBar(LoadGameImage("data/frontend/worm_health.png", true), 0, 0, 0, 0, DeprecatedGUI::BAR_LEFTTORIGHT);
	cHealthBar.SetLabelVisible(false);

	gusSkinVisble = true;

	fLastSimulationTime = tLX->currentTime;


	if(m_inputHandler) {
		m_inputHandler->deleteThis();
		m_inputHandler = NULL;
	}

	cDamageReport.clear();

	skinPreviewDrawer = skinPreviewDrawerP = new SkinDynDrawer(this);
}

CWorm::~CWorm() {
	if(bPrepared) Unprepare();
	FreeGraphics();
	
	{
		Mutex::ScopedLock lock(skinPreviewDrawerP->mutex);
		skinPreviewDrawerP->worm = NULL;
	}
}


///////////////////
// Free the graphics
void CWorm::FreeGraphics()
{
	bmpGibs = NULL;

#ifndef DEDICATED_ONLY
	if(m_animator) {
		delete m_animator;
		m_animator = 0;
	}
	if(m_fireconeAnimator) {
		delete m_fireconeAnimator;
		m_fireconeAnimator = 0;
	}
#endif

	skin = skinMask = NULL;
}




///////////////////
// Prepare the worm for the game
void CWorm::Prepare()
{
	if(game.gameScript() == NULL || !game.gameScript()->isLoaded()) {
		errors << "CWorm::Prepare " << getID() << ":" << getName() << ": gamescript not loaded" << endl;
		return;
	}

	if(bPrepared) {
		errors << "CWorm::Prepare " << getID() << ":" << getName() << ": already prepared" << endl;
		// It is safer to re-prepare the worm now because the earlier Prepare might have been the
		// wrong one and some things might be update. Re-preparing should always be safe.
		Unprepare();
	}

	tState = worm_state_t();
	bVisibleForWorm.clear();
	fVisibilityChangeTime = 0;

	bVisibleForWorm.clear();
	fVisibilityChangeTime = 0;
	gusSkinVisble = true;
	bCanRespawnNow = false;
	bRespawnRequested = false;
	
	setTeamkills(0);
	setSuicides(0);
	setDeaths(0);
	
	setCanUseNinja(true);
	setSpeedFactor(cClient->getGameLobby()[FT_WormSpeedFactor]);
	setDamageFactor(cClient->getGameLobby()[FT_WormDamageFactor]);
	setShieldFactor(cClient->getGameLobby()[FT_WormShieldFactor]);
	setCanAirJump(cClient->getGameLobby()[FT_InstantAirJump]);

	iCurrentWeapon = 0;
	for(short i=0;i<iNumWeaponSlots;i++) {
		tWeapons[i].Weapon = NULL;
		tWeapons[i].Enabled = false;
		tWeapons[i].Charge = 1;
		tWeapons[i].Reloading = false;
		tWeapons[i].SlotNum = i;
		tWeapons[i].LastFire = 0;
	}
	GetRandomWeapons();
	// weapons should be loaded properly in initWeaponSelection

	if(m_inputHandler) {
		warnings << "WARNING: worm " << getName() << " has already the following input handler set: "; warnings.flush();
		warnings << m_inputHandler->name(); warnings << endl;
		m_inputHandler->deleteThis();
		m_inputHandler = NULL;
	}

	m_isAuthority = false;

	skin = skinMask = NULL;
	aimSpeed=(AngleDiff(0.0f)); aimAngle=(Angle(90.0f)); m_lastHurt=(0);
	animate=(false); changing=(false);
	m_dir=(1);
	m_animator = m_fireconeAnimator = NULL;
	m_currentFirecone = NULL;

#ifndef DEDICATED_ONLY
	skin = spriteList.load("skin");
	skinMask = spriteList.load("skin-mask");
	m_animator = new AnimLoopRight(skin,35);

	m_fireconeTime = 0;
	m_currentFirecone = NULL;
	m_fireconeAnimator = NULL;
	m_fireconeDistance = 0;
#endif

	m_timeSinceDeath = 0;

	aimRecoilSpeed = 0;

	currentWeapon = 0;

	m_weapons.assign(gusGame.options.maxWeapons, 0 );
	m_weaponCount = 0;

	if(gusGame.weaponList.size() > 0)
		for ( size_t i = 0; i < m_weapons.size(); ++i ) {
			m_weapons[i] = new Weapon(gusGame.weaponList[rndInt(gusGame.weaponList.size())], this);
			m_weaponCount++;
		}

	cNinjaRope.gusInit();
	movingLeft = false;
	movingRight = false;
	jumping = false;
	
	if(game.isServer()) {
		// register network worm-node
		// as client, we do that in Net_cbNodeRequest_Dynamic
		NetWorm_Init(true);		
	}
		
	if(game.needToCreateOwnWormInputHandlers()) {
		if(bLocal) {
			m_inputHandler = m_type->createInputHandler(this);
			m_inputHandler->assignNetworkRole(true);
		} else if(game.needProxyWormInputHandler()) {
			m_inputHandler = new ProxyPlayer(this);
		
			Uint32 uniqueID = 0;
			do {
				uniqueID = rndgen();
			} while(!uniqueID);
			
			m_inputHandler->uniqueID = uniqueID;
			//savedScores[uniqueID] = player->stats; // TODO: merge this somehow with OLX? savedScores is from gus Server
			
			//console.addLogMsg( "* " + worm->getName() + " HAS JOINED THE GAME");
			m_inputHandler->assignNetworkRole(true);
			m_inputHandler->assignWorm(this);
		}		
	}
	
	if(game.gameScript()->gusEngineUsed()) {
		// we set this so that the OLX part sees that wpn selection is ready and it sends the ImReady packet
		// Gusanos has own weapon handling, this isn't merge yet (not sure if it even would make sense to merge)
		bWeaponsReady = true;		
	}
	
	bAlive = false; // the worm is dead at the beginning, spawn it to make it alive
	health = 0;
	posRecordings.clear();

	bPrepared = true;

	game.onPrepareWorm(this);
}

void CWorm::Unprepare() {
	if(!bPrepared) {
		errors << "CWorm::Unprepare " << getID() << ":" << getName() << ": not prepared" << endl;
		return;
	}

	bAlive = false;
	health = 0;
	bWeaponsReady = false;
	setLobbyReady( false );
	
	setTagIT(false);
	setTagTime(TimeDiff(0));

	resetAngleAndDir();

	bVisibleForWorm.clear();
	fVisibilityChangeTime = 0;
	
	if(m_inputHandler) {
		m_inputHandler->deleteThis();
		m_inputHandler = NULL;
	}
	
	for ( size_t i = 0; i < m_weapons.size(); ++i) {
		luaDelete(m_weapons[i]);
		m_weapons[i] = 0;
	}

	for(short i=0;i<iNumWeaponSlots;i++) {
		tWeapons[i].Weapon = NULL;
		tWeapons[i].Enabled = false;
		tWeapons[i].Charge = 1;
		tWeapons[i].Reloading = false;
		tWeapons[i].SlotNum = i;
		tWeapons[i].LastFire = 0;
	}

	NetWorm_Shutdown();
	FreeGraphics();

	bPrepared = false;

	game.onUnprepareWorm(this);
}

void CWorm::StartGame() {
	fTimeofDeath = GetPhysicsTime();
	setLastAirJumpTime(AbsTime());
	if(!m_inputHandler) {
		warnings << "CWorm::StartGame(): input handler not set" << endl;
	}
	else
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
		warnings << "CWorm::getInput: called getInput() on non-local worm " << getName() << endl;
		return;
	}
	
	if(!m_inputHandler) {
		warnings << "CWorm::getInput: input handler not set for worm " << getName() << ", cannot get input" << endl;
		return;
	}
	
	m_inputHandler->getInput();
}

void CWorm::clearInput() {
	fLastInputTime = GetPhysicsTime();
	
	// Clear the state
	tState.write().reset();
	
	if(bLocal && m_inputHandler)
		m_inputHandler->clearInput();
}

void CWorm::initWeaponSelection() {
	if(!bLocal) {
		warnings << "CWorm::initWeaponSelection: called initWeaponSelection() on non-local worm " << getName() << endl;
		return;
	}
	
	if(!m_inputHandler) {
		warnings << "CWorm::initWeaponSelection: input handler not set for worm " << getName() << ", cannot init weapon selection" << endl;
		return;
	}
	
	if(this->shouldDoOwnWeaponSelection())
		m_inputHandler->initWeaponSelection();

	if(this->isFirstLocalHostWorm() && gameSettings[FT_SameWeaponsAsHostWorm] && gameSettings[FT_ForceRandomWeapons]) {
		this->GetRandomWeapons();
		this->bWeaponsReady = true;
	}

	// bWeaponsReady could be true already for multiple reasons, e.g. in initWeaponSelection()
	if(this->isFirstLocalHostWorm() && this->bWeaponsReady && gameSettings[FT_SameWeaponsAsHostWorm])
		cServer->cloneWeaponsToAllWorms(this);
}

void CWorm::doWeaponSelectionFrame(SDL_Surface * bmpDest, CViewport *v) {
	if(!bLocal) {
		warnings << "CWorm::doWeaponSelectionFrame: called on non-local worm " << getName() << endl;
		return;
	}
	
	if(!m_inputHandler)
		// Note: no warning here because this can happen since the merge with Gusanos net engine
		return;
	
	if(bWeaponsReady) {
		warnings << "CWorm::doWeaponSelectionFrame: weapons already selected" << endl;
		return;
	}
	
	if(!this->shouldDoOwnWeaponSelection()) {
		if( v && v->getUsed() ) {
			int t = v->GetTop();
			int centrex = v->GetLeft() + v->GetVirtW()/2;

			tLX->cFont.DrawCentre(bmpDest, centrex, t+48, tLX->clWeaponSelectionTitle, "... Waiting for server selection ...");
		}
		
		return;
	}
	
	m_inputHandler->doWeaponSelectionFrame(bmpDest, v);	
		   
	if(this->bWeaponsReady && this->isFirstLocalHostWorm() && gameSettings[FT_SameWeaponsAsHostWorm]) {
		cServer->cloneWeaponsToAllWorms(this);
	}

}



///////////////////
// Setup the lobby details
void CWorm::setupLobby()
{
	bLobbyReady = false;
}


void CWorm::resetAngleAndDir() {
	fAngle = 0;
	fAngleSpeed = 0;
	iFaceDirectionSide = DIR_RIGHT;
}


///////////////////
// Spawn this worm
void CWorm::Spawn(CVec position) {
	// No spawn if spectating
	if (bSpectating)
		return;

	if(game.gameScript() && game.gameScript()->gusEngineUsed())
		// Gusanos will use its own spawning fct
		return;
	
	bAlive = true;
	bSpawnedOnce = true;
	bCanRespawnNow = false;
	bRespawnRequested = false;
	fVisibilityChangeTime = 0;
	resetAngleAndDir();
	fMoveSpeedX = 0;
	health = 100.0f;
	iMoveDirectionSide = DIR_RIGHT;
	fLastInputTime = GetPhysicsTime();
	vPos = vDrawPos = vLastPos = vPreOldPosOfLastPaket = vOldPosOfLastPaket = position;
	vPreLastEstimatedVel = vLastEstimatedVel = vVelocity = CVec(0,0);
	posRecordings.clear(vPos);
	cNinjaRope.Clear();
	

	fFrame = 0;
	bDrawMuzzle = false;
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

	fSpawnTime = fPreLastPosUpdate = fLastPosUpdate = fLastSimulationTime = GetPhysicsTime();

	if(bLocal) {
		if( !NewNet::Active() )
			clearInput();
		if(!m_inputHandler) {
			warnings << "CWorm::Spawn for local worm: input handler not set" << endl;
		}
		else
			m_inputHandler->onRespawn();
	}

	cDamageReport.clear();
}


///////////////////
// Load the graphics
bool CWorm::ChangeGraphics(int generalgametype)
{
	// TODO: create some good way to allow custom colors

	bool team = false;

	// Destroy any previous graphics
	FreeGraphics();

	Color colour = cSkin.get().getDefaultColor();
	// If we are in a team game, use the team colours
	if(generalgametype == GMT_TEAMS) {
		team = true;
		colour = tLX->clTeamColors[CLAMP(iTeam.get(),0,3)];
	}

	// Use the colours set on the network
	// Profile or team colours will override this

	// Colourise the giblets
	bmpGibs = ChangeGraphics("data/gfx/giblets.png", team);

	// Colourise the skin
	cSkin.write().Colorize(colour);

	return bmpGibs.get() != NULL;
}

///////////////////
// Change the graphics of an image
SmartPointer<SDL_Surface> CWorm::ChangeGraphics(const std::string& filename, bool team)
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


	// TODO: clean up this function

	// Set the colour of the img
	int x,y;
	Uint32 pixel;

	Color colour = cSkin.get().getColor();
	if (team)
		colour = tLX->clTeamColors[iTeam];

	int ColR = colour.r;
	int ColG = colour.g;
	int ColB = colour.b;
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
			Uint8 r,g,b;
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
			if(Color((int)r2, (int)g2, (int)b2) == tLX->clPink) {
				r2=240;
				b2=240;
			}

			PutPixel(img.get(),x,y, Color((int)r2, (int)g2, (int)b2).get(img.get()->format));
		}
	}

	UnlockSurface(img);

	return img;
}



///////////////////
// Randomize the weapons
void CWorm::GetRandomWeapons()
{
	if(game.weaponRestrictions() == NULL) {
		errors << "CWorm::GetRandomWeapons: cWeaponRest == NULL" << endl;
		// no break here, the function will anyway work and ignore the restrictions
	}
	
	if(game.gameScript() == NULL || game.gameScript()->GetNumWeapons() <= 0) {
		errors << "CWorm::GetRandomWeapons: gamescript is not loaded" << endl;
		for(short i = 0; i < 5; ++i) tWeapons[i].Weapon = NULL; // not sure if needed but just to be sure
		return;
	}
	
	for(short i=0; i<5; i++) {
		int num = MAX(1, GetRandomInt(game.gameScript()->GetNumWeapons()-1)); // HINT: num must be >= 1 or else we'll loop forever in the ongoing loop

		// Cycle through weapons starting from the random one until we get an enabled weapon
		int n=num;
		int lastenabled = -1;
		while(true) {
			// Wrap around
			if(n >= game.gameScript()->GetNumWeapons())
 			   n = 0;

			// Have we already got this weapon?
			bool bSelected = false;
			for(int k=0; k<i; k++) {
				if(tWeapons[k].Weapon && (game.gameScript()->GetWeapons()+n)->ID == tWeapons[k].Weapon->ID) {
					bSelected = true;
					break;
				}
			}

			// If this weapon is enabled AND we have not selected it already, then exit the loop
			if(!game.weaponRestrictions() || game.weaponRestrictions()->isEnabled( (game.gameScript()->GetWeapons()+n)->Name ))  {
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
		if(n >= 0 && n < game.gameScript()->GetNumWeapons()) {
			tWeapons[i].Weapon = game.gameScript()->GetWeapons()+n;
			tWeapons[i].Enabled = true;
		}
		else {
			tWeapons[i].Weapon = NULL;
			tWeapons[i].Enabled = false;
		}
	}

}

// the first host worm (there can only be one such worm in a game)
bool CWorm::isFirstLocalHostWorm() {
	if(game.isClient()) return false;
	if(!cServer || !cServer->isServerRunning()) return false;
	
	CServerConnection* localConn = cServer->localClientConnection();
	if(localConn == NULL) {
		errors << "CWorm::isFirstLocalHostWorm: localClient not found" << endl;
		return false;
	}
	return game.wormsOfClient(localConn)->tryGet() == this;
}

bool CWorm::isLocalHostWorm() {
	if(game.isClient()) return false;
	if(!cServer || !cServer->isServerRunning()) return false;
	
	CServerConnection* localConn = cServer->localClientConnection();
	if(localConn == NULL) {
		errors << "CWorm::isLocalHostWorm: localClient not found" << endl;
		return false;
	}
	
	for_each_iterator(CWorm*, w, game.wormsOfClient(localConn))
		if(w->get() == this)
			return true;
	
	return false;
}


bool CWorm::shouldDoOwnWeaponSelection() {
	return !cClient->serverChoosesWeapons() || (this->isFirstLocalHostWorm() && gameSettings[FT_SameWeaponsAsHostWorm]);
}

void CWorm::CloneWeaponsFrom(CWorm* w) {
	for(int i = 0; i < 5; ++i) {
		tWeapons[i].Weapon = w->getWeapon(i)->Weapon;
		tWeapons[i].Enabled = w->getWeapon(i)->Enabled;
		
		tWeapons[i].Charge = 1;
		tWeapons[i].Reloading = false;
		tWeapons[i].SlotNum = i;
		tWeapons[i].LastFire = 0;
	}
}


// Muzzle flash positions for different angles
const int	RightMuzzle[14] = {2,3, 5,3, 4,0, 5,-8, 3,-9, 2,-13, -2,-12};
const int	LeftMuzzle[14] =  {4,-12, -1,-12, -1,-9, -3,-8, -2,0, -2,4, 1,3};


void CWorm::UpdateDrawPos() {
	if( tLXOptions->bAntilagMovementPrediction && !cClient->OwnsWorm(this->getID()) && !NewNet::Active() ) {
		//if(fLastPosUpdate > tLX->currentTime) return; // something is wrong, we probably have not gotten any update yet

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
/*		SmartPointer<SDL_Surface> bmpDestDebug = game.gameMap()->GetDebugImage();
		if (bmpDestDebug) {
			int node_x = (int)vPos.x*2, node_y = (int)vPos.y*2;

			if(node_x-4 >= 0 && node_y-4 >= 0 && node_x+4 < bmpDestDebug->w && node_y+4 < bmpDestDebug->h) {
				// Draw the new pos
				DrawRectFill(bmpDestDebug,node_x-4,node_y-4,node_x+4,node_y+4, Color(0,255,0));
			}
		} */
#endif

	} else {
		// no antilag movement prediction
		vDrawPos = vPos;
	}
}


bool CWorm::isVisibleForWorm(int worm) const {
	assert(worm >= 0);
	if((size_t)worm >= bVisibleForWorm.size()) return true;
	return bVisibleForWorm[worm];
}

void CWorm::setVisibleForWorm(int worm, bool visibility) {
	assert(worm >= 0);
	if((size_t)worm >= bVisibleForWorm.size()) {
		bVisibleForWorm.resize(worm + 1, true);
	}
	bVisibleForWorm[worm] = visibility;
}

bool CWorm::isVisibleForEverybody() const {
	for(std::vector<bool>::const_iterator i = bVisibleForWorm.begin(); i != bVisibleForWorm.end(); ++i) {
		if(!*i) return false;
	}
	return true;
}

bool CWorm::isVisible(CWorm* viewerWorm) const {
	if(viewerWorm == NULL) return isVisibleForEverybody();
	return isVisibleForWorm(viewerWorm->getID());
}

bool CWorm::isVisible(const CViewport* v) const {
	return isVisible(v->getTarget());
}

static INLINE bool isWormVisible(CWorm* w, CViewport* v) {
	return w->isVisible(v);
}

Color CWorm::renderColorAt(/* relative game coordinates */ int x, int y) {
	if(!bAlive)
		return Color(0,0,0,SDL_ALPHA_TRANSPARENT);
		
	// BAD HACK: visibility depends on first local worm
	if(!isVisible( game.localWorms()->tryGet() ))
		return Color(0,0,0,SDL_ALPHA_TRANSPARENT);
	
	// Find the right pic
	int f = ((int)fFrame*7);
	int ang = MIN( (int)( (fAngle+90)/151 * 7 ), 6 ); // clamp the value because LX skins don't have the very bottom aim
	f += ang;
			
	// multiplied by 2 because skin have double res
	return cSkin.get().renderColorAt(x*2 + cSkin.get().getSkinWidth()/2, y*2 + cSkin.get().getSkinHeight()/2, f, iFaceDirectionSide == DIR_LEFT);
}

///////////////////
// Draw the worm
void CWorm::Draw(SDL_Surface * bmpDest, CViewport *v)
{
	if( !v )
		return;

	int l = v->GetLeft();
	int t = v->GetTop();

	CMap* map = game.gameMap();
	VectorD2<int> p = v->physicToReal(vDrawPos, cClient->getGameLobby()[FT_InfiniteMap], map->GetWidth(), map->GetHeight());

	int x = p.x - l;
	int y = p.y - t;

	// Draw the ninja rope
	// HINT: draw it before the clipping check because the rope might be visible even if the worm is not
	if (isWormVisible(this, v) && bAlive)
		cNinjaRope.Draw(bmpDest,v,vDrawPos);

	// Are we inside the viewport?
	if(x+l+10 < l || x-10 > v->GetVirtW()
	|| y+t+10 < t || y-10 > v->GetVirtH()) {
		return;
	}


	int a = (int)fAngle;
	if(iFaceDirectionSide == DIR_LEFT)
		a=180-a;


	int WormNameY = tLX->cFont.GetHeight()+12; // Font height + worm height/2 + some space

	// Draw the damage amount that worm received
	// Even the worm is dead draw damage for some time anyway (looks pretty)
	if( tLXOptions->bDamagePopups && isWormVisible(this, v) )
	{
		// Sort them first
		std::map< AbsTime, int > DamageReportDrawOrder;
		for( std::map<int, DamageReport> ::iterator it = cDamageReport.begin(); it != cDamageReport.end(); it++ )
				DamageReportDrawOrder[it->second.lastTime] = it->first;
		// Draw
		if( ! DamageReportDrawOrder.empty() )
		{
			int damageDrawPos = WormNameY + tLX->cFont.GetHeight(); 
			// Make it float up a bit when time passes
			if( GetPhysicsTime() > DamageReportDrawOrder.begin()->first )
				damageDrawPos += (int)(( GetPhysicsTime() - DamageReportDrawOrder.begin()->first ).seconds() * 30);

			float damageSum = 0;
			std::map< AbsTime, int > :: const_iterator it;
			for( it = DamageReportDrawOrder.begin(); it != DamageReportDrawOrder.end(); it++ )
				damageSum += cDamageReport[it->second].damage;
			Color damageColor = damageSum >= 0 ? Color( 0xff, 0x80, 0x40 ) : Color( 0x40, 0xff, 0 ) ; // Red or green
			
			for( it = DamageReportDrawOrder.begin(); it != DamageReportDrawOrder.end(); it++ )
			{
				int id = it->second;
				if( tLXOptions->bColorizeDamageByWorm )
				{
					damageSum = cDamageReport[id].damage;
					if( CWorm* w = game.wormById(id, false) )
						damageColor = w->getGameColour();
				}
				std::string damageStr = itoa( Round(damageSum) );
				if( damageSum < 0 )
					damageStr[0] = '+';	// Negative damage = healing
				if( getClientVersion() < OLXBetaVersion(0,58,1) )
					damageStr = "? " + damageStr; // + "\xC2\xBF"; // Inverted question mark in UTF-8
				if(damageSum != 0)
					tLX->cOutlineFont.DrawCentre(bmpDest, x + l, y + t - damageDrawPos, damageColor, damageStr);
				//notes << "Print damage for worm " << getID() << " = " << damageStr << " at " << x << " " << (y - damageDrawPos) << endl;
				damageDrawPos += tLX->cFont.GetHeight();

				if( ! tLXOptions->bColorizeDamageByWorm )
					break;
			}
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
				if( GetPhysicsTime() > DamageReportDrawOrder.begin()->first + 1.5f )
					cDamageReport.clear();
			}
		}
	}

	// Do not draw further if we're not alive
	if( !getAlive() )
		return;

	
	if (tLXOptions->bShowHealth && isWormVisible(this, v))  {
		if (!bLocal || m_type != PRF_HUMAN)  {
			int hx = x + l;
			int hy = y + t - 9; // -8 = worm height/2

			if (cHealthBar.IsProperlyLoaded())  {

				cHealthBar.SetX(hx - cHealthBar.GetWidth() / 2);
				cHealthBar.SetY(hy - cHealthBar.GetHeight() - 1);
				cHealthBar.Draw( bmpDest );
				cHealthBar.SetPosition((int)getHealth());
				WormNameY += cHealthBar.GetHeight()+2; // Leave some space

			} else {  // Old style healthbar
				hy -= 7;

				// Draw the "grid"
				{
					Color BorderColor = Color(0x49,0x50,0x65);
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

				for (short i=0; i<iShowHealth; i++) {
					Color CurColor = Color(HealthColors[i*3],HealthColors[i*3+1],HealthColors[i*3+2]);
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
	GetVecsFromAngle((float)a, &forw, NULL);
	forw *= tLXOptions->fCrosshairDistance;

	VectorD2<int> cp = p;
	cp.x += (int)forw.x;
	cp.y += (int)forw.y;

	// Show a green crosshair if we have a target
	x = 0;
	if (bGotTarget) {
		x = 6;
		bGotTarget = false;
	}

	if(bLocal && isWormVisible(this, v))
		DrawImageAdv(bmpDest, DeprecatedGUI::gfxGame.bmpCrosshair, x, 0, cp.x - 2, cp.y - 2, 6, 6);

	//
	// Draw the worm
	//
	x = p.x;
	y = p.y;

	// Find the right pic
	int f = ((int)fFrame*7);
	int ang = MIN( (int)( (fAngle+90)/151 * 7 ), 6 ); // clamp the value because LX skins don't have the very bottom aim
	f += ang;


	// Snap the position to a slighter bigger pixel grid (2x2)
	x -= x % 2;
	y -= y % 2;


	// Draw the worm
	if (isWormVisible(this, v)) {
		cSkin.get().Draw(bmpDest, x - cSkin.get().getSkinWidth()/2, y - cSkin.get().getSkinHeight()/2, f, false, iFaceDirectionSide == DIR_LEFT);
		cSparkles.Process();
	}
	
	/*FillSurfaceTransparent(bmpShadowPic.get());
	if(iFaceDirectionSide == DIR_RIGHT)
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
	if (bDrawMuzzle && isWormVisible(this, v))  {
		switch(iFaceDirectionSide) {

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

	if(isWormVisible(this, v))
		cClient->flagInfo()->drawWormAttachedFlag(this, bmpDest, v);

	wpnslot_t *Slot = &tWeapons[iCurrentWeapon];

	// Draw the weapon name
	if(bLocal && m_type == PRF_HUMAN && isWormVisible(this, v)) {
		bool drawWpnName = bForceWeapon_Name;
		if(CWormHumanInputHandler* h = dynamic_cast<CWormHumanInputHandler*>(m_inputHandler))
			drawWpnName |= h->getInputWeapon().isDown();
		if(drawWpnName) {
			if(Slot->Weapon)
				tLX->cOutlineFont.DrawCentre(bmpDest,x,y-30,tLX->clPlayerName,Slot->Weapon->Name);

			if( tLX->currentTime > fForceWeapon_Time )
				bForceWeapon_Name = false;
		}
	}

	// Draw the worm's name
	if (isWormVisible(this, v))  {
		std::string WormName = sName;
		if( sAFKMessage.get() != "" )
			WormName += " " + sAFKMessage.get();
		if(!bLocal || (bLocal && m_type != PRF_HUMAN)) {
			if (cClient->getGameLobby()[FT_GameMode].as<GameModeInfo>()->generalGameType == GMT_TEAMS && tLXOptions->bColorizeNicks)
				tLX->cOutlineFont.DrawCentre(bmpDest,x,y-WormNameY,tLX->clTeamColors[iTeam],WormName);
			else
				tLX->cOutlineFont.DrawCentre(bmpDest,x,y-WormNameY,tLX->clPlayerName,WormName);
		} else { // local human worm
			if(iAFK != AFK_BACK_ONLINE) {
				tLX->cOutlineFont.DrawCentre(bmpDest,x,y-WormNameY,tLX->clPlayerName,".. " + sAFKMessage.get() + " ..");
			}
		}
	}

}

///////////////////
// Draw the worm's shadow
void CWorm::DrawShadow(SDL_Surface * bmpDest, CViewport *v)
{
	if( tLXOptions->bShadows && v && isWormVisible(this, v) )  {
		// Copied from ::Draw
		// TODO: a separate function for this
		int f = ((int)fFrame*7);
		int ang = MIN( (int)( (fAngle+90)/151 * 7 ), 6 ); // clamp the value because LX skins don't have the very bottom aim
		f += ang;

		// Draw the shadow
		
		// NOTE: the cSkin.DrawShadow function draws a shadow over solid objects
		// Later we should render the world layer by layer so this trouble will be gone
		// The CMap::DrawObjectShadow function is slow and also logically incorrect - why should a map know about other
		// objects?
		cSkin.get().DrawShadowOnMap(game.gameMap(), v, bmpDest, (int)vPos.get().x, (int)vPos.get().y, f, iFaceDirectionSide == DIR_LEFT);
	}
}


///////////////////
// Quickly check if we are on the ground
bool CWorm::CheckOnGround()
{
	int px = (int)vPos.get().x;
	int py = (int)vPos.get().y;
	bool wrapAround = cClient->getGameLobby()[FT_InfiniteMap];

	for(short y = 6; y > 0; y--) {

		// Optimize: pixelflag + Width
		if(!(game.gameMap()->GetPixelFlag(px - 2, py + y, wrapAround) & PX_EMPTY))
			return true;
		if(!(game.gameMap()->GetPixelFlag(px + 2, py + y, wrapAround) & PX_EMPTY))
			return true;
	}

	return false;
}

void CWorm::incrementDirtCount(int d) {
	if(d != 0) {
		iDirtCount += d;
		
		if( game.isServer() ) {
			game.gameMode()->Carve(this, d);
		}
	}
}

///////////////////
// Injure me
// Returns true if i was killed by this injury


///////////////////
// Kill me
// Returns true if we are out of the game
void CWorm::Kill(bool serverside)
{
	bAlive = false;
	fTimeofDeath = GetPhysicsTime();
	bCanRespawnNow = false;
	bRespawnRequested = false;

	// score handling only serverside - client will get update via scoreupdate package
	if(serverside) {
		addDeath();

		if(iLives != WRM_UNLIM)
			iLives = (int)iLives - 1;
	}
}

int CWorm::getScore() const
{
	int score = getKills();
	if( (float)gameSettings[FT_DeathDecreasesScore] > 0.0f )
		score -= (int)( getDeaths() * (float)gameSettings[FT_DeathDecreasesScore] + 0.01f ); // + 0.01f to counter possible inprecise truncation
	if( gameSettings[FT_SuicideDecreasesScore] )
		score -= getSuicides();
	if( gameSettings[FT_TeamkillDecreasesScore] )
		score -= getTeamkills();
	if( gameSettings[FT_CountTeamkills] )
		score += getTeamkills();

	return score; // May be negative
}

void CWorm::addDeath()
{
	if( !gameSettings[FT_AllowNegativeScore] && getScore() <= 0 )
		return;
	iDeaths++;
}

void CWorm::addSuicide()
{
	if( !gameSettings[FT_AllowNegativeScore] && getScore() <= 0 )
		return;
	iSuicides++;
}

void CWorm::addTeamkill()
{
	if( !gameSettings[FT_AllowNegativeScore] && getScore() <= 0 )
		return;
	iTeamkills++;
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
		if(b->getWeapon() >= 0 && b->getWeapon() < game.gameScript()->GetNumWeapons()) {
			tWeapons[iCurrentWeapon].Weapon = game.gameScript()->GetWeapons() + b->getWeapon();
			tWeapons[iCurrentWeapon].Charge = 1;
			tWeapons[iCurrentWeapon].Reloading = false;
			tWeapons[iCurrentWeapon].Enabled = true;
		}
		else {
			warnings << "selected bonus has invalid weapon " << b->getWeapon() << endl;
			// do nothing
		}
		
		return true;
	}


	// Health
	if(b->getType() == BNS_HEALTH) {

		// If our health is at 100 or higher, don't pick it up
		if(health >= 100.0f) // Some mods have healing weapons, so the check is >= instead of == (breaks old behavior)
			return false;

		// Health between 10% - 50%
		float health = (float)(GetRandomInt(40)+10);

		// Route call to CClient::InjureWorm() so it will send ReportDamage packet, to track valid worm healthbar on server
		// Clamp it
		health = MIN(health, 100 - getHealth());
		cClient->InjureWorm(this, -health, getID());

		return true;
	}

	return false;
}

///////////////////
// Hide the worm, if immediate is set, no animation will be shown
void CWorm::Hide(int forworm, bool immediate)
{
	setVisibleForWorm(forworm, false);
	if (!immediate)
		fVisibilityChangeTime = tLX->currentTime;
	else
		fVisibilityChangeTime = 0;
}

//////////////////
// Show the worm, if immediate is set, no animation will be shown
void CWorm::Show(int forworm, bool immediate)
{
	setVisibleForWorm(forworm, true);
	if (!immediate)
		fVisibilityChangeTime = tLX->currentTime;
	else
		fVisibilityChangeTime = 0;
}

///////////////////
// Get the worm's ping
int CWorm::GetMyPing()
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
bool CWorm::CanType()
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
	

void CWorm::setAFK(AFK_TYPE afkType, const std::string & msg)
{
	iAFK = afkType;
	sAFKMessage = msg;
}

void CWorm::setTagIT(bool _t) 
{ 
	bTagIT = _t; 
}

Color CWorm::getGameColour()
{
	switch(cClient->getGameLobby()[FT_GameMode].as<GameModeInfo>()->generalGameType) {
		case GMT_TEAMS:
			return tLX->clTeamColors[iTeam];
		default:
			return cSkin.get().getDefaultColor();
	}
	return Color();
}
template<typename SettingsType>
static void CWorm_addDamage(CWorm* This, float damage, CWorm* victim, const SettingsType& settings, const GameModeInfo* gameModeInfo)
{
	if( damage < 0 )	// Do not count when we heal ourselves or someone else, only damage counts
		return;

	if( This->getID() == victim->getID() )
	{
		if( settings[FT_SuicideDecreasesScore] )
			This->setDamage( This->getDamage() - damage );	// Decrease damage from score if injured yourself
	}
	else if( gameModeInfo->generalGameType == GMT_TEAMS && This->getTeam() == victim->getTeam() ) 
	{
		if( settings[FT_TeamkillDecreasesScore] )
			This->setDamage( This->getDamage() - damage );	// Decrease damage from score if injured teammate
		if( settings[FT_CountTeamkills] )
			This->setDamage( This->getDamage() + damage );	// Count team damage along with teamkills
	}
	else
		This->setDamage( This->getDamage() + damage );
}

void CWorm::addDamage(float damage, CWorm* victim, bool serverside) {
	if(serverside)
		CWorm_addDamage(this, damage, victim, gameSettings, gameSettings[FT_GameMode].as<GameModeInfo>());
	else
		CWorm_addDamage(this, damage, victim, cClient->getGameLobby(), cClient->getGameLobby()[FT_GameMode].as<GameModeInfo>());		
}


void CWorm::reinitInputHandler() {
	warnings << "CWorm::reinitInputHandler not implemented right now" << endl;
	return;
	// TODO: why are we doing this here?
	// WARNING: this code is wrong, we cannot recreate the input handler because we must keep the network node
	// also, it shouldn't be needed to recreate it
	
	if(!bLocal) {
		warnings << "reinitInputHandler called for non-local worm " << getID() << endl;
		return;
	}
	
	if(m_inputHandler) {
		m_inputHandler->deleteThis();
		m_inputHandler = NULL;
	} else
		warnings << "reinitInputHandler: inputhandler was unset for worm " << getID() << endl;
	m_inputHandler = m_type->createInputHandler(this);
	
	// TODO: move this to CWormInputHandler init
	// we need to reset the inputs
	// code from CClient::SetupGameInputs()
	int humanWormNum = 0;
	for_each_iterator(CWorm*, w, game.localWorms()) {
		CWormHumanInputHandler* handler = dynamic_cast<CWormHumanInputHandler*> (w->get()->inputHandler());
		if(handler) {
			// TODO: Later, let the handler save a rev to his sPlayerControls. This would give
			// more flexibility to the player and he can have multiple player control sets.
			// Then, we would call a reloadInputs() here.
			if(w->get() == this)
				handler->setupInputs( tLXOptions->sPlayerControls[humanWormNum] );
			humanWormNum++;
		}
	}	
}

const SmartPointer<profile_t>& CWorm::getProfile() const {
	return tProfile;
}

void CWorm::setProfile(const SmartPointer<profile_t>& p) {
	tProfile = p;
}


void CWorm::NewNet_CopyWormState(const CWorm & w)
{
	// Macro to do less copypaste
	// Only the gamestate variables are copied, score is updated by server in separate packet
	#define COPY(X) X = w.X;
	COPY( fLastSimulationTime );
	COPY( tState );
	COPY( vPos );
	COPY( vVelocity );
	COPY( vLastPos );
	COPY( vDrawPos );
	COPY( bOnGround );
	COPY( fLastInputTime );
	COPY( lastMoveTime );
	COPY( fServertime );
    COPY( fLastCarve );
	COPY( health );
	// Do not copy fDamage / suicides / teamkills etc - they are managed by scoreboard routines on server
	COPY( bAlive );
	COPY( fTimeofDeath );
	COPY( iFaceDirectionSide );
	COPY( iMoveDirectionSide );
	COPY( bGotTarget );
	COPY( fAngle );
	COPY( fAngleSpeed );
	COPY( fMoveSpeedX );
	COPY( fFrame );
	COPY( cNinjaRope );
	COPY( bVisibleForWorm );
	COPY( fVisibilityChangeTime );
	COPY( fSpawnTime );
	COPY( iCurrentWeapon );
	COPY( fLastAirJumpTime );
	COPY( cDamageReport );
	
	COPY( NewNet_random );
	
	for( int i=0; i<MAX_WEAPONSLOTS; i++ )
		COPY( tWeapons[i] );
	#undef COPY
};

void CWorm::NewNet_InitWormState(int seed)
{
	NewNet_random.seed(seed);
	// These vars most probably getting reset in Spawn() but I want to be sure
	fLastSimulationTime = AbsTime();
	fLastInputTime = AbsTime();
	lastMoveTime = AbsTime();
	fServertime = TimeDiff();
	fLastCarve = AbsTime();
	fTimeofDeath = AbsTime();
	iFaceDirectionSide = DIR_LEFT;
	fSpawnTime = AbsTime();
	fLastAirJumpTime = AbsTime();
}


const Version& CWorm::getClientVersion() {
	if(game.isServer())
		return cOwner->getClientVersion();
	return cClientVersion;
}

void CWorm::setClientVersion(const Version & v) {
	cClientVersion = v;
}


REGISTER_CLASS(CWorm, LuaID<CGameObject>::value)
REGISTER_CLASS(worm_state_t, LuaID<CustomVar>::value)

