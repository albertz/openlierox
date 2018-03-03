////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Client class - Game routines
// Created 21/7/02
// Jason Boettcher

#include <algorithm>

#include "LieroX.h"
#include "Clipboard.h"
#include "AuxLib.h"
#include "MathLib.h"
#include "CClient.h"
#include "CServer.h"
#include "CServerConnection.h"
#include "CBonus.h"
#include "console.h"
#include "GfxPrimitives.h"
#include "DeprecatedGUI/Graphics.h"
#include "StringUtils.h"
#include "CWorm.h"
#include "Entity.h"
#include "Protocol.h"
#include "Physics.h"
#include "CClient.h"
#include "CClientNetEngine.h"
#include "ProfileSystem.h"
#include "Debug.h"
#include "CGameMode.h"
#include "CHideAndSeek.h"
#include "ProjectileDesc.h"
#include "WeaponDesc.h"
#include "Touchscreen.h"


CClient		*cClient = NULL;


bool CClient::shouldDoProjectileSimulation() {
	if(bDedicated && tLX->iGameType != GME_JOIN && !tLXOptions->doProjectileSimulationInDedicated) return false;
	return true;
}



///////////////////
// Simulation
void CClient::Simulation()
{
	short i;
    CWorm *w;

	// Don't simulate if the physics engine is not ready
	if (!PhysicsEngine::Get()->isInitialised())  {
		errors << "WARNING: trying to simulate with non-initialized physics engine!" << endl;
		return;
	}


	// If we're in a menu & a local game, don't do simulation
	if (tLX->iGameType == GME_LOCAL)  {
		if( bGameOver || bGameMenu || bViewportMgr || Con_IsVisible() ) {

			// Clear the input of the local worms
			clearLocalWormInputs();
			
			// gameover case will be checked below
			if(!bGameOver) {
				// skip simulations for this frame
				for(int i = 0; i < MAX_WORMS; i++) {
					CWorm* w = &cRemoteWorms[i];
					if(!w->isUsed()) continue;
					if(!w->getAlive()) continue;
					PhysicsEngine::Get()->skipWorm(w);
				}
				PhysicsEngine::Get()->skipProjectiles(cProjectiles.begin());
				PhysicsEngine::Get()->skipBonuses(cBonuses, MAX_BONUSES);				
			}
		}
	}

    // We stop a few seconds after the actual game over
    if(bGameOver && (tLX->currentTime - fGameOverTime).seconds() > GAMEOVER_WAIT)
        return;


	// TODO: does it work also, if we
	// 1. simulate all worms
	// 2. check collisions with bonuses
	// (at the moment, we are doing these 2 things at once in the loop, I want to have 2 loops)
	// TODO: make it working
	// TODO: create a function simulateWorms() in PhysicsEngine which does all worms-simulation

	// Player simulation
	w = cRemoteWorms;
	for(i = 0; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;
		
		if(w->getAlive()) {

			// Simulate the worm
			// TODO: move this to a simulateWorms() in PhysicsEngine
			PhysicsEngine::Get()->simulateWorm( w, cRemoteWorms, w->getLocal() );

			if(bGameOver)
				// TODO: why continue and not break?
                continue;


			// Check if this worm picked up a bonus

			CBonus *b = cBonuses;
			if (tGameInfo.bBonusesOn)  {
				for(short n = 0; n < MAX_BONUSES; n++, b++) {
					if(!b->getUsed())
						continue;

					if(w->CheckBonusCollision(b)) {
						if(w->getLocal() || (iNumWorms > 0 && cLocalWorms[0]->getID() == 0 && tLXOptions->tGameInfo.bServerSideHealth)) {

							if( w->GiveBonus(b) ) {

								// Pickup noise
								PlaySoundSample(sfxGame.smpPickup);

								DestroyBonus(n, w->getLocal(), w->getID());

								bShouldRepaintInfo = true;
							}
						}
					}
				}
			}
		}

        if(bGameOver)
			// TODO: why continue and not break?
            continue;


		// TODO: move this to physics as it should also be FPS independent
		// Is this worm shooting?
		if(w->getAlive()) {

			// Shoot
			if(w->getWormState()->bShoot) {
				// This handles only client-side weapons, like jetpack and for beam drawing
				// It doesn't process the shot itself.
				// The shot-info will be sent to the server which sends it back and
				// we handle it in CClient::ProcessShot in the end.
				PlayerShoot(w);
			}

			// If the worm is using a weapon with a laser sight, spawn a laser sight
			if (w->getCurWeapon()->Weapon)
				if(w->getCurWeapon()->Weapon->LaserSight && !w->getCurWeapon()->Reloading)
					LaserSight(w, w->getAngle());
			
			// Show vision cone of seeker worm
			if( getGameLobby()->gameMode == GameMode(GM_HIDEANDSEEK) &&
				w->getTeam() == HIDEANDSEEK_SEEKER  )
			{
				int Angle = tGameInfo.features[FT_HS_SeekerVisionAngle];
				if( Angle < 360 )
				{
					LaserSight(w, w->getAngle() + Angle/2, false );
					LaserSight(w, w->getAngle() - Angle/2, false );
				}
			}
		}


		// In a timed game increment the tagged worms time
		if(getGeneralGameType() == GMT_TIME && w->getTagIT())  {
			w->incrementTagTime(tLX->fRealDeltaTime);

			// Log
			log_worm_t *l = GetLogWorm(w->getID());
			if (l)
				l->fTagTime += tLX->fRealDeltaTime;
		}
	}

	// Entities
	// only some gfx effects, therefore it doesn't belong to PhysicsEngine
	if(!bDedicated)
		SimulateEntities(tLX->fDeltaTime);

	// Projectiles
	if(shouldDoProjectileSimulation())
		PhysicsEngine::Get()->simulateProjectiles(cProjectiles.begin());

	// Bonuses
	PhysicsEngine::Get()->simulateBonuses(cBonuses, MAX_BONUSES);

}


///////////////////
// Explosion
void CClient::Explosion(CVec pos, float damage, int shake, int owner)
{
	int		x,y,px;
	ushort i;
	Color	Colour = cMap->GetTheme()->iDefaultColour;
    bool    gotDirt = false;

	// Go through until we find dirt to throw around
	y = MIN((uint)pos.y,cMap->GetHeight()-1);

	px = (uint)pos.x;

	LOCK_OR_QUIT(cMap->GetImage());
	for(x=px-2; x<px+2; x++) {
		// Clipping
		if(x < 0)	continue;
		if(x >= (int)cMap->GetWidth())	break;

		if(cMap->GetPixelFlag(x,y) & PX_DIRT) {
			Colour = Color(cMap->GetImage()->format, GetPixel(cMap->GetImage().get(),x,y));
            gotDirt = true;
			break;
		}
	}
	UnlockSurface(cMap->GetImage());

	// Go through bonuses. If any were next to an explosion, destroy the bonus explosivly
	if (tGameInfo.bBonusesOn && !getGameLobby()->features[FT_IndestructibleBonuses]) {
		CBonus *b = cBonuses;
		for(i=0; i < MAX_BONUSES; i++,b++) {
			if(!b->getUsed())
				continue;

			if( fabs(b->getPosition().x - pos.x) > 15 )
				continue;
			if( fabs(b->getPosition().y - pos.y) > 15 )
				continue;

			b->setUsed(false);
			Explosion(pos,15,5,owner);
		}
	}


    // Go through projectiles. If any were next to an explosion, set the projectile's explode event to true
	/*  CProjectile *prj = cProjectiles;
    for( i=0; i<MAX_PROJECTILES; i++, prj++ ) {
        if( !prj->isUsed() )
            continue;

        if( fabs(prj->GetPosition().x - pos.x) > 15 )
            continue;
        if( fabs(prj->GetPosition().y - pos.y) > 15 )
            continue;

        prj->setExplode( tLX->currentTime + CalculateDistance(prj->GetPosition(),pos) / 500.0f, true);
    }*/


	// Particles
    if(gotDirt) {
	    for(x=0;x<2;x++)
		    SpawnEntity(ENT_PARTICLE,0,pos,CVec(GetRandomNum()*30,GetRandomNum()*10),Colour,NULL);
    }


	int expsize = 8;
	if(damage > 5)
		expsize = (int)damage;

	// Explosion
	SpawnEntity(ENT_EXPLOSION, expsize, pos, CVec(0,0),Color(),NULL);

	int d = cMap->CarveHole((int)damage,pos, getGameLobby()->features[FT_InfiniteMap]);

    // Increment the dirt count
	if(owner >= 0 && owner < MAX_WORMS)
		cRemoteWorms[owner].incrementDirtCount( d );

	// If this is within a viewport, shake the viewport
	for(i=0; i<NUM_VIEWPORTS; i++) {
		if(!cViewports[i].getUsed())
            continue;

		if(shake) {
			if(cViewports[i].inView(pos))
				cViewports[i].Shake(shake);
		}
	}


	// Check if any worm is near the explosion, for both my worms and remote worms
	for(i=0;i<MAX_WORMS;i++) {
		CWorm* w = & cRemoteWorms[i];

		if( !w->isUsed() || !w->getAlive())
			continue;

		if((pos - w->getPos()).GetLength2() <= 25) {

			// Injure him
			InjureWorm(w, damage,owner);
		}
	}
}


///////////////////
// Injure a worm
void CClient::InjureWorm(CWorm *w, float damage, int owner)
{
	if (!w->getAlive())  // Injuring a dead worm makes no sense
		return;

	CWorm* ownerWorm = NULL;
	if(owner >= 0 && owner < MAX_WORMS) {
		ownerWorm = &this->getRemoteWorms()[owner];
		if(!ownerWorm->isUsed())
			ownerWorm = NULL;
	}

	bool me = ownerWorm && ownerWorm->getID() == w->getID();
	if(ownerWorm)
		damage *= ownerWorm->damageFactor();

	if(w->shieldFactor() == 0) {
		if(damage > 0)
			damage = w->getHealth() + 1;
		else if(damage < 0)
			damage = MIN(w->getHealth() - 100, 0.0f);
	}
	else
		damage /= w->shieldFactor();
	
	ushort i;
	
	if(damage > 0 && ownerWorm && this->isTeamGame() && !this->getGameLobby()->features[FT_TeamInjure] && !me && w->getTeam() == ownerWorm->getTeam())
		return;
	
	if(damage > 0 && ownerWorm && !this->getGameLobby()->features[FT_SelfInjure] && me)
		return;
	
	if (w->getLocal())  // Health change
		bShouldRepaintInfo = true;

	bool someOwnWorm = false;
	for(i=0; i < iNumWorms; i++) {
		if(cLocalWorms[i]->getID() == w->getID()) {
			someOwnWorm = true;
			break;
		}
	}

	CServerConnection *client = NULL;
	if( tLX->iGameType == GME_HOST || tLX->iGameType == GME_LOCAL )
		client = cServer->getClient(w->getID());
	Version clientver;
	if (client)
		clientver = client->getClientVersion();

	float realdamage = MIN(w->getHealth(), damage);

	// Send REPORTDAMAGE to server (also calculate & send it for pre-Beta9 clients, when we're hosting)
	if( realdamage > 0 && getServerVersion() >= OLXBetaVersion(0,58,1) && 
		( someOwnWorm || 
		( tLX->iGameType == GME_HOST && clientver < OLXBetaVersion(0,58,1) ) ) )
		getNetEngine()->QueueReportDamage( w->getID(), realdamage, owner );

	// Set damage report for local worm for Beta9 server - server won't send it back to us
	// Set it also for remote worms on pre-Beta9 server
	if( getServerVersion() < OLXBetaVersion(0,58,1) || 
		( getServerVersion() >= OLXBetaVersion(0,58,1) && someOwnWorm ) )
	{
		// TODO: fix this
		if(ownerWorm) {
			w->getDamageReport()[owner].damage += realdamage;
			w->getDamageReport()[owner].lastTime = GetPhysicsTime();
		}
	}

	// Update our scoreboard for local worms
	if( ! (	tLX->iGameType == GME_JOIN && getServerVersion() < OLXBetaVersion(0,58,1) ) ) // Do not update scoreboard for pre-Beta9 servers
		// TODO: fix this
		if(ownerWorm)
			getRemoteWorms()[owner].addDamage( realdamage, w, tGameInfo ); // Update client scoreboard

	// Do not injure remote worms when playing on Beta9 - server will report us their correct health with REPORTDAMAGE packets
	if( getServerVersion() < OLXBetaVersion(0,58,1) || 
		( getServerVersion() >= OLXBetaVersion(0,58,1) && someOwnWorm ) ||
		( tLX->iGameType == GME_HOST && clientver < OLXBetaVersion(0,58,1) ) ) // We're hosting, calculate health for pre-Beta9 clients
	{
		if(w->injure(damage)) {
			// His dead Jim

			w->setAlreadyKilled(true);

			// Kill someOwnWorm
			// TODO: why is localworm[0] == 0 checked here?
			if(someOwnWorm ||
				(iNumWorms > 0 && cLocalWorms[0]->getID() == 0 && tLXOptions->tGameInfo.bServerSideHealth) ) {

				w->setAlive(false);
				w->Kill();
				w->clearInput();

				cNetEngine->SendDeath(w->getID(), owner); // Let the server know that i am dead
			}
		}
	}
	// If we are hosting then synchronise the serverside worms with the clientside ones
	if(tLX->iGameType == GME_HOST && cServer ) {
		CWorm *sw = cServer->getWorms() + w->getID();
		sw->setHealth(w->getHealth());
	}

	// Spawn some blood
	if( damage > 0 )	// Don't spawn blood if we hit with healing weapon (breaks old behavior)
	{
		float amount = ((float)tLXOptions->iBloodAmount / 100.0f);
		float sp;
		for(i=0;i<amount;i++) {
			sp = GetRandomNum()*50;
			SpawnEntity(ENT_BLOODDROPPER,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp*4),Color(128,0,0),NULL);
			SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),Color(128,0,0),NULL);
			SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),Color(200,0,0),NULL);
		}
	}
}

///////////////////
// Send a carve to the server
void CClient::SendCarve(CVec pos)
{
	int x,y,n,px;
	Color Colour = cMap->GetTheme()->iDefaultColour;

	// Go through until we find dirt to throw around
	y = MIN((int)pos.y,cMap->GetHeight()-1);
	y = MAX(y,0);
	px = (int)pos.x;

	LOCK_OR_QUIT(cMap->GetImage());
	for(x=px-2; x<=px+2; x++) {
		// Clipping
		if(x<0)	continue;
		if((uint)x>=cMap->GetWidth())	break;

		if(cMap->GetPixelFlag(x,y) & PX_DIRT) {
			Colour = Color(cMap->GetImage()->format, GetPixel(cMap->GetImage().get(),x,y));
			for(n=0;n<3;n++)
				SpawnEntity(ENT_PARTICLE,0,pos,CVec(GetRandomNum()*30,GetRandomNum()*10),Colour,NULL);
			break;
		}
	}
	UnlockSurface(cMap->GetImage());

	/*for(x=0;x<3;x++)
		SpawnEntity(ENT_PARTICLE,0,pos,CVec(GetRandomNum()*30,GetRandomNum()*10),Colour);*/

	// Just carve a hole for the moment
	cMap->CarveHole(4,pos, (bool)getGameLobby()->features[FT_InfiniteMap]);
}


///////////////////
// Player shoot
// This is the function which is called very first. It does not much as most shoots aren't handled yet
// but later by the server. The server itself checks for w->tState.bShoot, so we don't care about that here.
void CClient::PlayerShoot(CWorm *w)
{
	wpnslot_t *Slot = w->getCurWeapon();

	if(!Slot->Enabled)
		return;

	if(!Slot->Weapon) {
		errors << "PlayerShoot: Slot->Weapon not set. Guilty worm: " << itoa(w->getID()) << " with name " << w->getName() << endl;
		return;
	}

	if(Slot->Weapon->Type == WPN_BEAM) {
		// Beam weapons get processed differently
		// Beam weapons have no rate of fire, they fire continuously until out of ammo,
		// but we don't know when they are out of ammo, because the server won't tell us,
		// so we'll draw it for some short time after each 'shoot' packet from the server
		if (Slot->LastFire>0) {
			DrawBeam(w);
			Slot->LastFire -= tLX->fDeltaTime.seconds();
		}
		return;
	}

	if(Slot->Reloading)
		return;

	if(Slot->LastFire>0) {
		return;
	}

	// Safe the ROF time (Rate of Fire). That's the pause time after each shot.
	// In simulateWormWeapon, we decrease this by deltatime and
	// the next shot is allowed if lastfire<=0.
	Slot->LastFire = Slot->Weapon->ROF;

	// Special weapons get processed differently
	if(Slot->Weapon->Type == WPN_SPECIAL) {
		ShootSpecial(w);
		return;
	}


	// Shots are now handled by the server. Server directly checks the bShoot state
	// and server handles it in GameServer::WormShoot.
	// Server sends us later a shootlist. We are handling each shot in this shootlist in
	// CClient::ProcessShot().
	return;

}


///////////////////
// Shoot a special weapon
void CClient::ShootSpecial(CWorm *w)
{
	wpnslot_t *Slot = w->getCurWeapon();
	TimeDiff dt = tLX->fDeltaTime;

	// Safety
	if(!Slot->Weapon)
	{
		errors << "ShootSpecial: Slot->Weapon was not set! Guilty worm: " << itoa(w->getID()) << " with name " << w->getName() << endl;
		return;
	}

	switch(Slot->Weapon->Special) {
		case SPC_NONE: break;

		// Jetpack
		case SPC_JETPACK: {
			w->velocity().y += -50.0f * ((float)Slot->Weapon->tSpecial.Thrust * (float)dt.seconds());

			Color blue = Color(80,150,200);
			CVec s = CVec(15,0) * GetRandomNum();
			SpawnEntity(ENT_JETPACKSPRAY, 0, w->getPos(), s + CVec(0,1) * (float)Slot->Weapon->tSpecial.Thrust, blue, NULL);
			break;
		}
			
		case __SPC_LBOUND: case __SPC_UBOUND: errors << "ShootSpecial: hit __SPC_BOUND" << endl;
	}


	// Drain the Weapon charge
	Slot->Charge -= Slot->Weapon->Drain / 100;
	if(Slot->Charge < 0) {
		Slot->Charge = 0;
		Slot->Reloading = true;
	}
}


///////////////////
// Shoot a beam
void CClient::DrawBeam(CWorm *w)
{
	//return;

	wpnslot_t *Slot = w->getCurWeapon();

	// Safety
	if(!Slot->Weapon)
	{
		errors << "DrawBeam: Slot->Weapon was not set! Guilty worm: " << itoa(w->getID()) << " with name " << w->getName() << endl;
		return;
	}

	if(Slot->Reloading)
		return;

	// Get the direction angle
	float Angle = w->getAngle();
	if(w->getFaceDirectionSide() == DIR_LEFT)
		Angle=180-Angle;


	// Trace a line from the worm to length or until it hits something
	CVec pos = w->getPos();
	CVec dir;
	GetVecsFromAngle(Angle,&dir,NULL);

	CVec orth_dir = dir.orthogonal();
	
	int i;
	int width = 0;
	std::vector<bool> goodWidthParts;
	bool drawBeam = false;
	// Don't draw the beam if it is 255,0,255
	Color col = Slot->Weapon->Bm.Colour;
	// HINT: We have to check the visibility for everybody as we don't have entities for specific teams/worms.
	// If you want to make that better, you would have to give the CViewport to DrawBeam.
	if(col != tLX->clPink && w->isVisibleForEverybody())
		drawBeam = true;

	std::list<CWorm*> worms;
	{
		CWorm* w2 = cRemoteWorms;
		for(short n=0;n<MAX_WORMS;n++,w2++) {
			if(!w2->isUsed() || !w2->getAlive())
				continue;
			
			// Don't check against someOwnWorm
			if(w2->getID() == w->getID())
				continue;
			
			worms.push_back(w2);
		}
	}
	
	for(i=0; i<Slot->Weapon->Bm.Length; ++i) {
		{
			int newWidth = int(float(Slot->Weapon->Bm.InitWidth) + Slot->Weapon->Bm.WidthIncrease * i);
			newWidth = MIN( newWidth, MIN( cMap->GetWidth(), cMap->GetHeight() ) );
			if(newWidth != width) {
				goodWidthParts.resize(newWidth);
				for(int j = width; j < newWidth; ++j) {
					int old = MAX(j - 2, 0);
					goodWidthParts[j] = (width > 0) ? goodWidthParts[old] : true;
				}
				width = newWidth;
			}
			if(width <= 0) break;
		}
		
		{
			bool stopbeam = true;
			for(uint j = 0; j < goodWidthParts.size(); ++j) {
				if(!goodWidthParts[j]) continue;

				int o = (j % 2 == 0) ? j/2 : - int((j + 1)/2);
				CVec p = pos + orth_dir * o;
				uchar px = (p.x <= 0 || p.y <= 0) ? PX_ROCK : cMap->GetPixelFlag( (int)p.x, (int)p.y );

				if((px & PX_DIRT) || (px & PX_ROCK)) {
					// Don't draw explosion when damage is -1
					if (Slot->Weapon->Bm.Damage != -1) {
						if(width <= 2) SpawnEntity(ENT_EXPLOSION, 5, p, CVec(0,0), Color(), NULL);
						int damage = Slot->Weapon->Bm.Damage;
						if(Slot->Weapon->Bm.DistributeDamageOverWidth) { damage /= width; if(damage == 0) damage = SIGN(Slot->Weapon->Bm.Damage); }
						int d = cMap->CarveHole(damage, p, getGameLobby()->features[FT_InfiniteMap]);
						w->incrementDirtCount(d);
					}
					
					goodWidthParts[j] = false;
					
					continue;
				}

				for(std::list<CWorm*>::iterator w2 = worms.begin(); w2 != worms.end(); ++w2) {					
					static const float wormsize = 5;
					if((p - (*w2)->getPos()).GetLength2() < wormsize*wormsize) {
						if (Slot->Weapon->Bm.Damage != -1)
							SpawnEntity(ENT_EXPLOSION, 3, p+CVec(1,1), CVec(0,0), Color(), NULL);

						goodWidthParts[j] = false;
						worms.erase(w2);
						break;
					}
				}
								
				stopbeam = false;								
			}
			
			if(stopbeam) break;
		}
		
		pos += dir;
	}

	// Spawn a beam entity
	if(drawBeam) {
		if(Slot->Weapon->Bm.InitWidth != 1 || Slot->Weapon->Bm.WidthIncrease != 0) {
			Line startLine;
			startLine.start = w->getPos() - orth_dir * Slot->Weapon->Bm.InitWidth / 2;
			startLine.end = w->getPos() + orth_dir * Slot->Weapon->Bm.InitWidth / 2;
			Line endLine;
			endLine.start = pos + orth_dir * width / 2;
			endLine.end = pos - orth_dir * width / 2;			
			SetWormBeamEntity(w->getID(), col, startLine, endLine);
		} else
			SetWormBeamEntity(w->getID(), col, w->getPos(), pos);
	}
}


///////////////////
// Spawn a projectile
void CClient::SpawnProjectile(CVec pos, CVec vel, int rot, int owner, proj_t *_proj, int _random, AbsTime remotetime, AbsTime ignoreWormCollBeforeTime)
{
	CProjectile* proj = cProjectiles.getNewObj();

	if(proj == NULL) {
		// Warning: Out of space for a projectile
		//warnings << "Warning: Out of space for SpawnProjectile" << endl;
		return;
	}

    // Safety
    _random %= 255;

	proj->Spawn(_proj,pos,vel,rot,owner,_random,remotetime, ignoreWormCollBeforeTime);
}

struct ScoreCompare {
	CClient* cl;
	ScoreCompare(CClient* c) : cl(c) {}
	bool operator()(int i, int j) const {
		CWorm *w1 = &cl->getRemoteWorms()[i], *w2 = &cl->getRemoteWorms()[j];
		if(cl->getGameLobby()->gameMode) return cl->getGameLobby()->gameMode->CompareWormsScore(w1, w2) > 0;
		return GameMode(GM_DEATHMATCH)->CompareWormsScore(w1, w2) > 0;
	}
};


///////////////////
// Update the scoreboard
void CClient::UpdateScoreboard()
{
	// Should be called ONLY in game
	if(!bGameReady)
		return;
	
	bUpdateScore = true;

	CWorm *w = cRemoteWorms;
	int s;
	short i,p,j;

	// Clear the team scores
	for(i=0;i<4;i++) {
		iTeamList[i]=i;
		if(getServerVersion() < OLXBetaVersion(0,58,1))
			iTeamScores[i]=0;
	}

	// Add the worms to the list
	iScorePlayers = 0;
	for(p=0; p<MAX_WORMS; p++,w++) {
		if(!w->isUsed())
			continue;
		
		iScoreboard[iScorePlayers++] = p;
		
		// in other cases, we got the scores from the server
		if(getServerVersion() < OLXBetaVersion(0,58,1)) {
			// Add to the team score
			if(getGeneralGameType() == GMT_TEAMS) {
				int team = w->getTeam();
				if (team < 0 || team >= 4)  {  // prevents crashing sometimes
					w->setTeam(0);
					team = 0;
				}
				// Make the score at least zero to say we have
				iTeamScores[team] = MAX(0,iTeamScores[team]);
				
				if(w->getLives() != WRM_OUT && w->getLives() != WRM_UNLIM)
					iTeamScores[team] += w->getLives();
			}				
		}
	}
		
	
	// Sort the team lists
	if(getGeneralGameType() == GMT_TEAMS) {
		for(i=0;i<4;i++) {
			for(j=0;j<3-i;j++) {
				if(iTeamScores[iTeamList[j]] < iTeamScores[iTeamList[j+1]]) {

					// Swap the team list entries
					s = iTeamList[j];
					iTeamList[j] = iTeamList[j+1];
					iTeamList[j+1] = s;
				}
			}
		}
	}


	if(iScorePlayers < 2)
		return;

	/*
	 // Just some test code for correct ordering. Should be removed later.
	for(int i = 0; i < iScorePlayers; ++i)
		for(int j = i; j < iScorePlayers; ++j) {
			ScoreCompare comp(this);
			if(comp(iScoreboard[i],iScoreboard[j]) && comp(iScoreboard[j],iScoreboard[i])) {
				errors << "1: worm " << iScoreboard[i] << " and " << iScoreboard[j] << " dont have correct order" << endl;
				return;
			}
			if(comp(iScoreboard[i],iScoreboard[j]) && cRemoteWorms[iScoreboard[i]].getKills() < cRemoteWorms[iScoreboard[j]].getKills()) {
				errors << "2: worm " << iScoreboard[i] << " and " << iScoreboard[j] << " dont have correct order" << endl;
				return;
			}
		} */
	std::sort(&iScoreboard[0], &iScoreboard[iScorePlayers], ScoreCompare(this));

	bUpdateScore = true;
}


///////////////////
// Destroy a bonus (not explosively)
void CClient::DestroyBonus(int id, bool local, int wormid)
{
	cBonuses[id].setUsed(false);


	// Tell the server the bonus is gone if we grabbed it
	if(local) {
		cNetEngine->SendGrabBonus(id, wormid);
	}
}



///////////////////
// Show a laser sight
void CClient::LaserSight(CWorm *w, float Angle, bool highlightCrosshair)
{

	// Get the direction angle
	if(w->getFaceDirectionSide() == DIR_LEFT)
		Angle=180-Angle;

	while( Angle > 180.0f ) // Normalize it
		Angle -= 360.0f;
	while( Angle < -180.0f )
		Angle += 360.0f;

	// Trace a line from the worm to length or until it hits something
	CVec pos = w->getPos();
	CVec dir;
	GetVecsFromAngle(Angle,&dir,NULL);

	int divisions = 3;			// How many pixels we go through each check (less = slower)

	int stopbeam = false;

	short i;
	uchar px;
	for(i=0; i<9999; i+=divisions) {
		px = cMap->GetPixelFlag( (int)pos.x, (int)pos.y );

		if((px & (PX_DIRT|PX_ROCK)))
			break;

		// Check if it has hit any of the worms
		CWorm *w2 = cRemoteWorms;
		for(short n=0;n<MAX_WORMS;n++,w2++) {
			if(!w2->isUsed() || !w2->getAlive())
				continue;

			// Don't check against me
			if(w2->getID() == w->getID())
				continue;

			static const float wormsize = 5;
			if((pos - w2->getPos()).GetLength2() < wormsize*wormsize) {
				stopbeam = true;

				// We have a target
				if( highlightCrosshair )
					w->setTarget(true);
				break;
			}
		}

		if(stopbeam)
			break;

		pos = pos + dir*(float)divisions;
	}

	// Spawn a laser sight entity (only if the worm is visible though, else we'd tell the worm position)
	// HINT: We have to check the visibility for everybody as we don't have entities for specific teams/worms.
	// If you want to make that better, you would have to give the CViewport to LaserSight.
	if (w->isVisibleForEverybody())
		SpawnEntity(ENT_LASERSIGHT, i, w->getPos(), dir, Color(), NULL);
}


///////////////////
// Process the shots
void CClient::ProcessServerShotList()
{
	int num = cShootList.getNumShots();

	// fServerTime is the time we calculated for the server,
	// shot->fTime was the fServerTime of the server when it added the shot
	// HINT: Though we are not using these because these times are not synchronised and sometimes
	// shot->fTime > fServerTime.
	// We are estimating the time with iMyPing. We divide it by 2 as iMyPing represents
	// the time of both ways (ping+pong).
	AbsTime fSpawnTime = tLX->currentTime - TimeDiff(((float)iMyPing / 1000.0f) / 2.0f);

	for(int i=0; i<num; i++) {
		shoot_t *sh = cShootList.getShot(i);

		AbsTime time = fSpawnTime;
		// HINT: Since Beta8 though, we have a good synchronisation of fServertime and we can actually use the provided sh->fTime
		if(cServerVersion >= OLXBetaVersion(8))
			if(sh->fTime <= fServertime) // just a security check
				time = tLX->currentTime - (fServertime - sh->fTime);
		
		// handle all shots not given by me
		if(sh)
			ProcessShot(sh, time);
	}
}

// IMPORTANT HINT: this function is currently not used
// It was intended for client side shooting but we
// decided for good reasons to not use it.
// The main reason is to keep the code clean and
// to have better forward-compatibility support.
// See discussion in OLX mailinglist for further details.
void CClient::DoLocalShot( float fTime, float fSpeed, int nAngle, CWorm *pcWorm ) {
	assert(false); // to ensure you not use this if you are not really sure

	shoot_t shot;

	shot.cPos = pcWorm->getPos();
	shot.cWormVel = pcWorm->getVelocity();
	shot.fTime = fTime;
	shot.nAngle = nAngle;
	shot.nRandom = GetRandomInt(255);
	shot.nSpeed = (int)( fSpeed*100 );
	shot.nWeapon = pcWorm->getCurWeapon()->Weapon->ID;
	shot.nWormID = pcWorm->getID();

	ProcessShot( &shot, tLX->currentTime );
}

///////////////////
// Process a shot
void CClient::ProcessShot(shoot_t *shot, AbsTime fSpawnTime)
{
	if(shot->nWormID >= 0 && shot->nWormID < MAX_WORMS) {
		CWorm *w = &cRemoteWorms[shot->nWormID];

		if(!w->isUsed())  {
			notes << "ProcessShot: unused worm was shooting" << endl;
			return;
		}
		
		// If this worm is dead, ignore the shot (original LX56 behaviour)
		if(!w->getAlive()) {
			//warnings << "WARNING: dead worm was shooting" << endl; // Occurs pretty often, don't spam console
			return;
		}
	}

	// Weird
	if (!cGameScript.get()->GetWeapons()) {
		warnings << "ProcessShot: weapons not loaded while a client was shooting" << endl;
		return;
	}

	// Safety check
	if (shot->nWeapon < 0 || shot->nWeapon >= cGameScript.get()->GetNumWeapons())  {
		warnings << "ProcessShot: weapon ID invalid. Guilty worm: " << shot->nWormID << endl;
		return;
	}
	
	const weapon_t *wpn = cGameScript.get()->GetWeapons() + shot->nWeapon;

	if(shot->release) { // the shot key was released
		wpn->FinalProj.apply(shot, fSpawnTime);
		return;
	}
	
	// Process beam weapons differently
	if(wpn->Type == WPN_BEAM) {
		ProcessShot_Beam(shot);
		return;
	}

	if(wpn->Proj.Proj == NULL) {
		errors << "ProcessShot: weapon " << wpn->Name << " has no projectile set" << endl;
		return;
	}
	

	// Play the weapon's sound
	if(wpn->UseSound) {
		CWorm* me = cViewports[0].getTarget();
		if(shot->nWormID >= 0 && shot->nWormID < MAX_WORMS) {
			StartSound(wpn->smpSample, shot->cPos, cRemoteWorms[shot->nWormID].getLocal(), 100, me);
		} else
			StartSound(wpn->smpSample, shot->cPos, false, 100, me);
	}
	
	if(shot->nWormID >= 0 && shot->nWormID < MAX_WORMS) {
		CWorm *w = &cRemoteWorms[shot->nWormID];

		// Add the recoil
		const CVec dir = GetVecFromAngle(float(shot->nAngle));
		w->velocity() -= dir*(float)wpn->Recoil;

		// Draw the muzzle flash
		w->setDrawMuzzle(true);
	}

	
	wpn->Proj.apply(shot, fSpawnTime);
}


///////////////////
// Process a shot - Beam
void CClient::ProcessShot_Beam(shoot_t *shot)
{
	if(shot->nWormID >= 0 && shot->nWormID < MAX_WORMS) {
		CWorm *w = &cRemoteWorms[shot->nWormID];
		if(!w->isUsed()) return;
		// Selected weapon for a worm is sent in an unreliable packet,
		// it may be different from the actual weapon the worm is using.
		// Since we received the shot data in a reliable packet, use it to select the right weapon.
		if (w->getCurWeapon()->Weapon != cGameScript.get()->GetWeapons() + shot->nWeapon) {
			for (int i = 0; i < w->getNumWeaponSlots(); i++) {
				if (w->getWeapon(i)->Weapon == cGameScript.get()->GetWeapons() + shot->nWeapon) {
					w->setCurrentWeapon(i);
					break;
				}
			}
		}
		// Draw the beam for 0.3 seconds after receiving the packet
		w->getCurWeapon()->LastFire = 0.3;
	}
	const weapon_t *wpn = cGameScript.get()->GetWeapons() + shot->nWeapon;

	// Trace a line from the worm to length or until it hits something
	CVec dir;
	GetVecsFromAngle((float)shot->nAngle,&dir,NULL);
	CVec pos = shot->cPos;

	CVec orth_dir = dir.orthogonal();
	std::vector<bool> goodWidthParts;
	int width = 0;

	std::list<CWorm*> worms;
	{
		CWorm* w2 = cRemoteWorms;
		for(short n=0;n<MAX_WORMS;n++,w2++) {
			if(!w2->isUsed() || !w2->getAlive())
				continue;
			
			// Don't check against someOwnWorm
			if(w2->getID() == shot->nWormID)
				continue;
			
			worms.push_back(w2);
		}
	}
	
	for(int i=0; i<wpn->Bm.Length; ++i) {
		{
			int newWidth = int(float(wpn->Bm.InitWidth) + wpn->Bm.WidthIncrease * i);
			newWidth = MIN( newWidth, MIN( cMap->GetWidth(), cMap->GetHeight() ) );
			if(newWidth != width) {
				goodWidthParts.resize(newWidth);
				for(int j = width; j < newWidth; ++j) {
					int old = MAX(j - 2, 0);
					goodWidthParts[j] = (width > 0) ? goodWidthParts[old] : true;
				}
				width = newWidth;
			}
			if(width <= 0) break;
		}
		
		{
			bool stopbeam = true;
			for(uint j = 0; j < goodWidthParts.size(); ++j) {
				if(!goodWidthParts[j]) continue;
				
				int o = (j % 2 == 0) ? j/2 : - int((j + 1)/2);
				CVec p = pos + orth_dir * o;
				uchar px = (p.x <= 0 || p.y <= 0) ? PX_ROCK : cMap->GetPixelFlag( (int)p.x, (int)p.y );
			
				// Check bonus colision and destroy the bonus, if damage isn't -1
				if (wpn->Bm.Damage != -1)  {
					CBonus *b = cBonuses;
					for(int n=0;n<MAX_BONUSES;n++,b++) {
						if(!b->getUsed())
							continue;

						const float bonussize = 3;
						if((p - b->getPosition()).GetLength() < bonussize) {
							Explosion(p,0,5,shot->nWormID); // Destroy the bonus by an explosion
							goodWidthParts[j] = false;
							break;
						}
					}
					
					if(!goodWidthParts[j]) continue;
				}

				if((px & (PX_DIRT|PX_ROCK))) {
					// No explosion if damage is -1
					if(wpn->Bm.Damage != -1) {
						//SpawnEntity(ENT_EXPLOSION, 5, pos, CVec(0,0), 0, NULL);
						int damage = wpn->Bm.Damage;
						if(wpn->Bm.DistributeDamageOverWidth) { damage /= width; if(damage == 0) damage = SIGN(wpn->Bm.Damage); }
						int d = cMap->CarveHole(damage, p, getGameLobby()->features[FT_InfiniteMap]);
						if(shot->nWormID >= 0 && shot->nWormID < MAX_WORMS)
							cRemoteWorms[shot->nWormID].incrementDirtCount(d);						
					}

					// Stop the beam regardless of explosion being shown
					goodWidthParts[j] = false;
					continue;
				}

				// Check if it has hit any of the worms
				for(std::list<CWorm*>::iterator w2 = worms.begin(); w2 != worms.end(); ++w2) {						
					static const float wormsize = 5;
					if((p - (*w2)->getPos()).GetLength2() < wormsize*wormsize) {
						int damage = wpn->Bm.PlyDamage;
						if(wpn->Bm.DistributeDamageOverWidth) { damage /= width; if(damage == 0) damage = SIGN(wpn->Bm.PlyDamage); }
						InjureWorm(*w2, (float)damage, shot->nWormID);

						goodWidthParts[j] = false;
						worms.erase(w2);
						break;
					}
				}
				
				if(!goodWidthParts[j]) continue;
				
				stopbeam = false;
			}
			
			if(stopbeam) break;
		}
		
		pos += dir;
	}

	// Spawn a beam entity and don't draw 255,0,255 (pink) beams
	/*if(wpn->Bm.Colour[0] != 255 || wpn->Bm.Colour[1] != 0 || wpn->Bm.Colour[2] != 255) {
		Color col = Color(wpn->Bm.Colour[0], wpn->Bm.Colour[1], wpn->Bm.Colour[2]);
		SpawnEntity(ENT_BEAM, i, w->getPos(), dir, col, NULL);
	}*/
}


void CClient::clearLocalWormInputs() {
	CWorm* w = cRemoteWorms;
	for(int i = 0; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;

		if( w->getLocal() )
			w->clearInput();
	}
}

void CClient::clearHumanWormInputs() {
	for (uint j=0;j<iNumWorms;j++)
		if (cLocalWorms[j]->getType() == PRF_HUMAN)
			cLocalWorms[j]->clearInput();
}


///////////////////
// Process any chatter
void CClient::processChatter()
{
	if (tLX->iGameType == GME_LOCAL)
		return;

    keyboard_t *kb = GetKeyboard();

	// If we're currently typing a message, add any keys to it
	if(bChat_Typing) {

		fChat_BlinkTime += tLX->fDeltaTime;
		if (fChat_BlinkTime.seconds() > 0.5f)  {
			bChat_CursorVisible = !bChat_CursorVisible;
			fChat_BlinkTime = 0;
		}

        // Go through the keyboard queue
        for(short i=0; i<kb->queueLength; i++) {

			if(kb->keyQueue[i].down && kb->keyQueue[i].sym == SDLK_ESCAPE) {
				// Stop typing
				bChat_Typing = false;
				sChat_Text = "";
				clearHumanWormInputs();

				// TODO: why are these needed? is this still up-to-date?
				kb->keys[SDLK_ESCAPE] = false;
				kb->KeyDown[SDLK_ESCAPE] = false;
				kb->KeyUp[SDLK_ESCAPE] = false;

				if(iNumWorms > 0 && cLocalWorms[0]->getType() != PRF_COMPUTER)
					cNetEngine->SendAFK( cLocalWorms[0]->getID(), AFK_BACK_ONLINE );

				break;
			}

			bChat_CursorVisible = true;
            processChatCharacter(kb->keyQueue[i]);
        }

        return;
    }

	if (GetTouchscreenTextInputShown()) {
		std::string text;
		if (ProcessTouchscreenTextInput(&text)) {
			CWorm* firstLocalWorm = cClient->getWorm(0);
			if(!firstLocalWorm || !firstLocalWorm->isUsed()) return;
			cNetEngine->SendText(text, firstLocalWorm->getName());
		}
		return;
	}

	// Check if we have hit the chat key and we're in a network game
	if( cChat_Input.wasDown() || cTeamChat_Input.wasDown() ) {

		// Initialize the chatter
		fChat_BlinkTime = 0;
		bChat_CursorVisible = true;
		bChat_Typing = true;
		iChat_Pos = 0;
		sChat_Text = "";
		iChat_Lastchar = 0;
		bChat_Holding = false;
		fChat_TimePushed = AbsTime();
		bTeamChat = false;
		if( cTeamChat_Input.wasDown() )
			bTeamChat = true;

		// Clear the input
		clearHumanWormInputs();
		
		if(iNumWorms > 0 && cLocalWorms[0]->getType() != PRF_COMPUTER)
			cNetEngine->SendAFK( cLocalWorms[0]->getID(), AFK_TYPING_CHAT );

#ifdef __ANDROID__
		bChat_Typing = false; // Always use Android native text input box instead of in-game input
#endif
		if (!GetTouchscreenTextInputShown()) {
			ShowTouchscreenTextInput();
		}

		return;
	}

    // Start typing on any keypress, if we can
	if (!tLXOptions->bAutoTyping)
		return;

	bTeamChat = false;

    for(short i=0; i<kb->queueLength; i++) {
    	const KeyboardEvent& input = kb->keyQueue[i];

        // Was it an up keypress?
        if(!input.down) continue;

		if (!bChat_Typing)  {

			if (input.ch<32) continue;

			// Check, if we can start typing
			bool controls =
				cChat_Input.isDown() ||
				cShowScore.isDown() ||
				cShowHealth.isDown() ||
				cShowSettings.isDown() ||
				cToggleTopBar.isDown() ||
				(input.sym == SDLK_BACKQUOTE) ||
				tLX->isAnyControlKeyDown();

			if(controls) continue;

			for(ushort j=0; j < iNumWorms; j++)  {
				if (cLocalWorms[j]->getType() == PRF_HUMAN)  {

					// Can we type?
					if (cLocalWorms[j]->getLives() == WRM_OUT && !cLocalWorms[j]->getAlive())  { // We're spectating
						if (cSpectatorViewportKeys.Down.isDown() ||
							cSpectatorViewportKeys.Left.isDown() ||
							cSpectatorViewportKeys.Right.isDown() ||
							cSpectatorViewportKeys.Up.isDown() ||
							cSpectatorViewportKeys.V1Type.isDown() ||
							cSpectatorViewportKeys.V2Toggle.isDown() ||
							cSpectatorViewportKeys.V2Type.isDown())
							return;
					} else if (!cLocalWorms[j]->CanType() && cLocalWorms[j]->isUsed()) // Playing
						return;
				}
			}

			// Clear the input
			clearHumanWormInputs();

			// Initialize the chatter
			fChat_BlinkTime = 0;
			bChat_CursorVisible = true;
			bChat_Typing = true;
			iChat_Pos = 0;
			sChat_Text = "";
			iChat_Lastchar = 0;
			bChat_Holding = false;
			fChat_TimePushed = AbsTime();

			if(iNumWorms > 0 && cLocalWorms[0]->getType() != PRF_COMPUTER)
				cNetEngine->SendAFK( cLocalWorms[0]->getID(), AFK_TYPING_CHAT );

		}

        processChatCharacter(input);
    }
}




std::string CClient::getChatterCommand() {
	if(strSeemsLikeChatCommand(sChat_Text))
		return sChat_Text.substr(1);
	else
		return "";
}



///////////////////
// Process a single character chat
void CClient::processChatCharacter(const KeyboardEvent& input)
{

    // Up?
    if(!input.down) {
        bChat_Holding = false;
        return;
    }

    // Must be down

    if(bChat_Holding) {
        if(iChat_Lastchar != input.ch)
            bChat_Holding = false;
        else {
            if((tLX->currentTime - fChat_TimePushed).seconds() < 0.4f)
                return;
        }
    }

    if(!bChat_Holding) {
        bChat_Holding = true;
        fChat_TimePushed = tLX->currentTime;
    }

    iChat_Lastchar = input.ch;

    // Backspace
    if(input.sym == SDLK_BACKSPACE) {
		if(iChat_Pos > 0)  {
			Utf8Erase(sChat_Text, --iChat_Pos, 1);
		}
        return;
    }

	// Delete
	if (input.sym == SDLK_DELETE)  {
		Utf8Erase(sChat_Text, iChat_Pos, 1);
		return;
	}

	// Home
	if (input.sym == SDLK_HOME)  {
		iChat_Pos = 0;
		return;
	}

	// End
	if (input.sym == SDLK_END)  {
		iChat_Pos = Utf8StringSize(sChat_Text);
		return;
	}

	// Left arrow
	if (input.sym == SDLK_LEFT) {
		if (iChat_Pos > 0)
			iChat_Pos--;
		else
			iChat_Pos = 0;
		return;
	}

	// Right arrow
	if (input.sym == SDLK_RIGHT) {
		if (iChat_Pos < Utf8StringSize(sChat_Text))
			iChat_Pos++;
		else
			iChat_Pos = Utf8StringSize(sChat_Text);
		return;
	}

    // Enter
    if(input.ch == '\r') {
        bChat_Typing = false;
		clearHumanWormInputs();

		if(iNumWorms > 0 && cLocalWorms[0]->getType() != PRF_COMPUTER)
			cNetEngine->SendAFK( cLocalWorms[0]->getID(), AFK_BACK_ONLINE );

        // Send chat message to the server
		if(sChat_Text != "") {
			if( bTeamChat )	// No "/me" macro in teamchat - server won't recognize such command
				cNetEngine->SendText("/teamchat \"" + sChat_Text + "\"", cLocalWorms[0]->getName() );
			else
				cNetEngine->SendText(sChat_Text, cLocalWorms[0]->getName() );
		}
		sChat_Text = "";
        return;
    }

	// Paste
	if (input.ch == 22 ||
		( input.sym == SDLK_v && ( input.state.bCtrl || input.state.bMeta ) ) || 
		( input.sym == SDLK_INSERT && input.state.bShift )) {
		
		size_t text_len = Utf8StringSize(sChat_Text);

		// Safety
		if (iChat_Pos > text_len)
			iChat_Pos = text_len;

		// Get the text
		std::string buf = copy_from_clipboard();
		replace(buf, "\r", " ");
		replace(buf, "\n", " ");
		replace(buf, "\t", " ");

		// Paste
		Utf8Insert(sChat_Text, iChat_Pos, buf);
		iChat_Pos += Utf8StringSize(buf);
		return;
	}

	// Tab key
	if(input.ch == SDLK_TAB) {
		if(strSeemsLikeChatCommand(sChat_Text)) {
			cNetEngine->SendChatCommandCompletionRequest(getChatterCommand());
			return;
		}
		// handle the key like a normal key
	}

    // Normal key
    if(tLX->cFont.CanDisplayCharacter(input.ch) ) {
    	InsertUnicodeChar(sChat_Text, iChat_Pos, input.ch);
		iChat_Pos++;
    }
}

///////////////////
// Find a nearest spot to the current one
CVec CClient::FindNearestSpot(CWorm *w)
{
    int     x, y;

	// Are we in the map?
	bool bInMap = true;
	bInMap = (w->getPos().x <= cMap->GetWidth()) && (w->getPos().x >= 0);
	bInMap = bInMap && (w->getPos().y <= cMap->GetHeight()) && (w->getPos().y >= 0);

	// Is the current spot good?
	x = (int) (w->getPos().x-4.5f);
	y = (int) (w->getPos().y-4.0f);
	int RockPixels = 0;

	ushort i,j;
	for(i=0; i<9 && (uint)x < cMap->GetWidth(); i++,x++)  {
		for (j=0; j<8 && (uint)y<cMap->GetHeight(); j++,y++)  {
			if(cMap->GetPixelFlag(x,y) & PX_ROCK)  {
				//cMap->PutImagePixel(x,y,tLX->clPink);
				RockPixels++;
			}
		}
		y = y-8;
	}

	if (RockPixels < 9 && bInMap)
		return w->getPos();

	// If it isn't, look in the nearest cells

    // Calculate our current cell
	int     px, py;
    uint     gw = cMap->getGridWidth();
    uint     gh = cMap->getGridHeight();

    px = (int) fabs((w->getPos().x)/gw);
	py = (int) fabs((w->getPos().y)/gh);

	if (bInMap)  {
		// Check the closest cells
		int tmp_x = px-1;
		int tmp_y = py-1;
		uchar tmp_pf = PX_ROCK;
		for (i=0; i<3; i++, tmp_y++)  {
			for (short j=0; j<3; j++, tmp_x++)  {

				// Skip our current cell
				if (tmp_x == px && tmp_y == py)
					continue;

				tmp_pf = *(cMap->getGridFlags() + tmp_y*cMap->getGridCols() +tmp_x);
				if (!(tmp_pf & PX_ROCK))
					return CVec((float)tmp_x*gw+gw/2, (float)tmp_y*gh+gh/2);
			}
			tmp_x -= 3;
		}

	}


    // Just find some spot
    bool    first = true;
    int     cols = cMap->getGridCols()-1;       // Note: -1 because the grid is slightly larger than the
    int     rows = cMap->getGridRows()-1;       // level size

    // Set our current cell as default
	if (bInMap)  {
		px = (uint) fabs((w->getPos().x)/gw);
		py = (uint) fabs((w->getPos().y)/gh);
	}
	else  {
		px = 0;
		py = 0;
	}

    x = px; y = py;

    // Start from the cell and go through until we get to an empty cell
    uchar pf;
    while(true) {
        while(true) {
            // If we're on the original starting cell, and it's not the first move we have checked all cells
            // and should leave
            if(!first) {
                if(px == x && py == y) {
                    return CVec((float)x*gw+gw/2, (float)y*gh+gh/2);
                }
	            first = false;
            }

            pf = *(cMap->getGridFlags() + y*cMap->getGridCols() + x);
            if(!(pf & PX_ROCK))
                return CVec((float)x*gw+gw/2, (float)y*gh+gh/2);

            if(++x >= cols) {
                x=0;
                break;
            }
        }

        if(++y >= rows) {
            y=0;
            x=0;
        }
    }

    // Can't get here
    return CVec((float)x, (float)y);

}
