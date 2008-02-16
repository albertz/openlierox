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

#include "Clipboard.h"
#include "LieroX.h"
#include "AuxLib.h"
#include "MathLib.h"
#include "CClient.h"
#include "CServer.h"
#include "CBonus.h"
#include "console.h"
#include "GfxPrimitives.h"
#include "Graphics.h"
#include "StringUtils.h"
#include "CWorm.h"
#include "Entity.h"
#include "Protocol.h"
#include "Physics.h"
#include "CClient.h"


CClient		*cClient = NULL;


///////////////////
// Simulation
void CClient::Simulation(void)
{
	short i;
    CWorm *w;
    //bool con = Con_IsUsed();


	// If we're in a menu & a local game, don't do simulation
	if (tGameInfo.iGameType == GME_LOCAL)  {
		if( bGameOver || bGameMenu || bViewportMgr ) {

			// Clear the input of the local worms
			w = cRemoteWorms;
			for(i = 0; i < MAX_WORMS; i++, w++) {
				if(!w->isUsed())
					continue;

				if( w->getLocal() )
					w->clearInput();
			}
		}
	}

    // We stop a few seconds after the actual game over
    if(bGameOver && tLX->fCurTime - fGameOverTime > GAMEOVER_WAIT)
        return;
    if((bGameMenu || bViewportMgr) && tGameInfo.iGameType == GME_LOCAL)
        return;


	//
	// Key shortcuts
	//
	
	// Top bar toggle
	if (cToggleTopBar.isDownOnce() && !bChat_Typing)  {
		tLXOptions->tGameinfo.bTopBarVisible = !tLXOptions->tGameinfo.bTopBarVisible;

		SDL_Surface *topbar = (tGameInfo.iGameType == GME_LOCAL) ? gfxGame.bmpGameLocalTopBar : gfxGame.bmpGameNetTopBar;
			
		int toph = topbar ? (topbar->h) : (tLX->cFont.GetHeight() + 3); // Top bound of the viewports
		int top = toph;
		if (!tLXOptions->tGameinfo.bTopBarVisible)  {
			toph = -toph;
			top = 0;
		}
		
		// TODO: allow more viewports
		// Setup the viewports
		cViewports[0].SetTop(top);
		cViewports[0].SetVirtHeight(cViewports[0].GetVirtH() - toph);
		if (cViewports[1].getUsed()) {
			cViewports[1].SetTop(top);
			cViewports[1].SetVirtHeight(cViewports[1].GetVirtH() - toph);
		}

		bShouldRepaintInfo = true;
	}

	// Health bar toggle
	if (cShowHealth.isDownOnce() && !bChat_Typing)  {
		tLXOptions->bShowHealth = !tLXOptions->bShowHealth;
	}

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
			PhysicsEngine::Get()->simulateWorm( w, this, cRemoteWorms, w->getLocal() );
			
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
						if(w->getLocal() || (iNumWorms > 0 && cLocalWorms[0]->getID() == 0 && tLXOptions->bServerSideHealth)) {
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


		// Is this worm shooting?
		if(w->getAlive()) {

			// Shoot
			if(w->getWormState()->iShoot) {
				// This is only for client-side weapons, like jetpack and for beam drawing
				// It doesn't process the shot itself.
				// The shot-info will be sent to the server which sends it back and
				// we handle it in CClient::ProcessShot in the end.
				PlayerShoot(w);
			}

			// If the worm is using a weapon with a laser sight, spawn a laser sight
			if (w->getCurWeapon()->Weapon)
				if(w->getCurWeapon()->Weapon->LaserSight)
					LaserSight(w);

		}


		// In a game of tag, increment the tagged worms time
		if(iGameType == GMT_TAG && w->getTagIT())  {
			w->incrementTagTime(tLX->fRealDeltaTime);

			// Log
			log_worm_t *l = GetLogWorm(w->getID());
			if (l)
				l->fTagTime += tLX->fRealDeltaTime;
		}
	}

	// Entities
	// only some gfx effects, therefore it doesn't belong to PhysicsEngine
	SimulateEntities(tLX->fDeltaTime,cMap);

	// Weather
	// TODO: if this will be implemented once, this should be moved to the PhysicsEngine
	//cWeather.Simulate(tLX->fDeltaTime, cMap);

	// Projectiles
	PhysicsEngine::Get()->simulateProjectiles(cProjectiles, nTopProjectile, this);

	// Bonuses
	PhysicsEngine::Get()->simulateBonuses(cBonuses, MAX_BONUSES);

}



///////////////////
// Explosion
void CClient::Explosion(CVec pos, int damage, int shake, int owner)
{
	int		x,y,px;
	ushort i;
	CWorm	*w;
	Uint32	Colour = cMap->GetTheme()->iDefaultColour;
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
			Colour = GetPixel(cMap->GetImage(),x,y);
            gotDirt = true;
			break;
		}
	}
	UnlockSurface(cMap->GetImage());

	// Go through bonuses. If any were next to an explosion, destroy the bonus explosivly
	if (tGameInfo.bBonusesOn)  {
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

        prj->setExplode( tLX->fCurTime + CalculateDistance(prj->GetPosition(),pos) / 500.0f, true);
    }*/


	// Particles
    if(gotDirt) {
	    for(x=0;x<2;x++)
		    SpawnEntity(ENT_PARTICLE,0,pos,CVec(GetRandomNum()*30,GetRandomNum()*10),Colour,NULL);
    }


	int expsize = 8;
	if(damage > 5)
		expsize = damage;

	// Explosion
	SpawnEntity(ENT_EXPLOSION, expsize, pos, CVec(0,0),0,NULL);

	int d = cMap->CarveHole(damage,pos);

    // Increment the dirt count
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


	//
	// Server Side?
	//

	// Check if any of _my_ worms are near the explosion
	for(i=0;i<iNumWorms;i++) {
		w = cLocalWorms[i];

		if(!w->getAlive())
			continue;

		if((pos - w->getPos()).GetLength2() <= 25) {

			// Injure him
			InjureWorm(w, damage,owner);
		}
	}

    CheckDemolitionsGame();
}


///////////////////
// Injure a worm
void CClient::InjureWorm(CWorm *w, int damage, int owner)
{
	if (!w->getAlive())  // Injuring a dead worm makes no sense
		return;

	if (w->getLocal())  // Health change
		bShouldRepaintInfo = true;

	bool me = false;
	ushort i;
	int localid=0;


	// Make sure this is one of our worms
	for(i=0; i < iNumWorms; i++) {
		if(cLocalWorms[i]->getID() == w->getID()) {
			me=true;
			localid = i;
			break;
		}
	}


	if(w->Injure(damage)) {
		// His dead Jim

		w->setAlreadyKilled(true);

		// Kill me
		if(me || (iNumWorms > 0 && cLocalWorms[0]->getID() == 0 && tLXOptions->bServerSideHealth)) {
			w->setAlive(false);
			w->Kill();
            w->clearInput();

			// Let the server know that i am dead
			SendDeath(w->getID(), owner);
		}
	}
	// If we are hosting then synchronise the serverside worms with the clientside ones
	if(tGameInfo.iGameType == GME_HOST && cServer) {
		CWorm *sw = cServer->getWorms() + w->getID();
		sw->setHealth(w->getHealth());
	}

	// Spawn some blood
	float amount = ((float)tLXOptions->iBloodAmount / 100.0f);
	float sp;
	for(i=0;i<amount;i++) {
		sp = GetRandomNum()*50;
		SpawnEntity(ENT_BLOODDROPPER,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp*4),MakeColour(128,0,0),NULL);
		SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(128,0,0),NULL);
		SpawnEntity(ENT_BLOOD,0,w->getPos(),CVec(GetRandomNum()*sp,GetRandomNum()*sp),MakeColour(200,0,0),NULL);
	}
}

///////////////////
// Send a carve to the server
void CClient::SendCarve(CVec pos)
{
	int x,y,n,px;
	Uint32 Colour = cMap->GetTheme()->iDefaultColour;

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
			Colour = GetPixel(cMap->GetImage(),x,y);
			for(n=0;n<3;n++)
				SpawnEntity(ENT_PARTICLE,0,pos,CVec(GetRandomNum()*30,GetRandomNum()*10),Colour,NULL);
			break;
		}
	}
	UnlockSurface(cMap->GetImage());

	/*for(x=0;x<3;x++)
		SpawnEntity(ENT_PARTICLE,0,pos,CVec(GetRandomNum()*30,GetRandomNum()*10),Colour);*/

	// Just carve a hole for the moment
	cMap->CarveHole(4,pos);
}


///////////////////
// Player shoot
void CClient::PlayerShoot(CWorm *w)
{
	wpnslot_t *Slot = w->getCurWeapon();

	if(Slot->Reloading)
		return;

	if(Slot->LastFire>0)
		return;

	if(!Slot->Weapon) {
		printf("ERROR: Slot->Weapon not set\n");
		return;
	}
	
	// TODO: what is the effect of this?
	Slot->LastFire = Slot->Weapon->ROF;

	// Special weapons get processed differently
	if(Slot->Weapon->Type == WPN_SPECIAL) {
		ShootSpecial(w);
		return;
	}

	// Beam weapons get processed differently
	if(Slot->Weapon->Type == WPN_BEAM) {
		DrawBeam(w);
		return;
	}


	// Shots are now handled by the server.
	// See CClient::ProcessShot().
	return;

}


///////////////////
// Shoot a special weapon
void CClient::ShootSpecial(CWorm *w)
{
	wpnslot_t *Slot = w->getCurWeapon();
	float dt = tLX->fDeltaTime;

	// Safety
	if(!Slot->Weapon)
		return;

	switch(Slot->Weapon->Special) {

		// Jetpack
		case SPC_JETPACK:
			CVec *v = w->getVelocity();
			*v += CVec(0,-50) * ((float)Slot->Weapon->tSpecial.Thrust * dt);

			Uint32 blue = MakeColour(80,150,200);
			CVec s = CVec(15,0) * GetRandomNum();
			SpawnEntity(ENT_JETPACKSPRAY, 0, w->getPos(), s + CVec(0,1) * (float)Slot->Weapon->tSpecial.Thrust, blue, NULL);
			break;
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
		return;

	if(Slot->Reloading)
		return;

	// Get the direction angle
	float Angle = w->getAngle();
	if(w->getDirection() == DIR_LEFT)
		Angle=180-Angle;


	// Trace a line from the worm to length or until it hits something
	CVec pos = w->getPos();
	CVec dir;
	GetAngles((int)Angle,&dir,NULL);

	int divisions = 1;			// How many pixels we go through each check (more = slower)

	if( Slot->Weapon->Bm_Length < divisions)
		divisions = Slot->Weapon->Bm_Length;

	// Make sure we have at least 1 division
	divisions = MAX(divisions,1);

	int stopbeam = false;

	int i;
	CWorm *w2;
	for(i=0; i<Slot->Weapon->Bm_Length; i+=divisions) {
		uchar px = cMap->GetPixelFlag( (int)pos.x, (int)pos.y );

		// Don't draw explosion when damage is -1
		if (Slot->Weapon->Bm_Damage != -1)  {
			if ((int)pos.x <= 0)  {
				SpawnEntity(ENT_EXPLOSION, 5, pos, CVec(0,0), 0, NULL);
				stopbeam = true;
				break;
			}

			if ((int)pos.y <= 0)  {
				SpawnEntity(ENT_EXPLOSION, 5, pos, CVec(0,0), 0, NULL);
				stopbeam = true;
				break;
			}
		}

		if((px & PX_DIRT) || (px & PX_ROCK)) {
			// Don't draw explosion when damage is -1
			if (Slot->Weapon->Bm_Damage != -1)  {
				SpawnEntity(ENT_EXPLOSION, 5, pos, CVec(0,0), 0, NULL);
				int d = cMap->CarveHole(Slot->Weapon->Bm_Damage, pos);
				w->incrementDirtCount(d);
			}
			break;
		}

		w2 = cRemoteWorms;
		for(short n=0;n<MAX_WORMS;n++,w2++) {
			if(!w2->isUsed() || !w2->getAlive())
				continue;

			// Don't check against me
			if(w2->getID() == w->getID())
				continue;

			static const float wormsize = 5;
			if((pos - w2->getPos()).GetLength2() < wormsize*wormsize) {
				if (Slot->Weapon->Bm_Damage != -1)
					SpawnEntity(ENT_EXPLOSION, 3, pos+CVec(1,1), CVec(0,0), 0, NULL);
				stopbeam = true;
				break;
			}
		}

		if(stopbeam)
			break;

		pos = pos + dir*(float)divisions;
	}

	// Spawn a beam entity
	// Don't draw the beam if it is 255,0,255
	Uint32 col = MakeColour(Slot->Weapon->Bm_Colour[0], Slot->Weapon->Bm_Colour[1], Slot->Weapon->Bm_Colour[2]);
	if(col != tLX->clPink) {
		SpawnEntity(ENT_BEAM, i, w->getPos(), dir, col, NULL);
	}
}


///////////////////
// Spawn a projectile
void CClient::SpawnProjectile(CVec pos, CVec vel, int rot, int owner, proj_t *_proj, int _random, float remotetime)
{
	CProjectile *proj = cProjectiles;
	int p=0;

	for(p=0;p<MAX_PROJECTILES;p++,proj++) {
		if(!proj->isUsed())
			break;
	}

	if(p >= MAX_PROJECTILES-1) {
		// Warning: Out of space for a projectile
		return;
	}

    if( p >= nTopProjectile )
        nTopProjectile = p+1;
    nTopProjectile = MIN(nTopProjectile,MAX_PROJECTILES);

    // Safety
    _random %= 255;

	proj->Spawn(_proj,pos,vel,rot,owner,_random,remotetime);
}


///////////////////
// Update the scoreboard
void CClient::UpdateScoreboard(void)
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
		iTeamScores[i]=-1;		// -1 means no players on team
		iTeamList[i]=i;
	}

	// Add the worms to the list
	iScorePlayers = 0;
	for(p=0; p<MAX_WORMS; p++,w++) {
		if(!w->isUsed())
			continue;

		iScoreboard[iScorePlayers++] = p;

		// Add to the team score
		if(iGameType == GMT_TEAMDEATH || iGameType == GMT_VIP) {
			// Make the score at least zero to say we have
			int team = w->getTeam();
			if (team < 0)  {  // prevents crashing sometimes
				w->setTeam(0);
				team = 0;
			}
			// On VIP treat the greens and blues as one team
			if(iGameType == GMT_VIP && team == 2)
				team = 0;
			iTeamScores[team] = MAX(0,iTeamScores[team]);

			if(w->getLives() != WRM_OUT && w->getLives() != WRM_UNLIM)
				iTeamScores[team] += w->getLives();
		}
		// Add to the team score
		if(iGameType == GMT_TEAMCTF) {
			// Make the score at least zero to say we have
			int team = w->getTeam();
			if (team < 0)  {  // prevents crashing sometimes
				w->setTeam(0);
				team = 0;
			}
			iTeamScores[team] = MAX(0,iTeamScores[team]);

			if(w->getLives() != WRM_OUT && w->getLives() != WRM_UNLIM)
				iTeamScores[team] += w->getKills();
		}
	}


	// Sort the team lists
	if(iGameType == GMT_TEAMDEATH || iGameType == GMT_VIP) {
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


	// Sort the array
	for(i=0; i<iScorePlayers; i++) {
		for(j=0; j<iScorePlayers-1-i; j++) {
			if(iGameType == GMT_TAG) {

				// TAG
				if(cRemoteWorms[iScoreboard[j]].getTagTime() < cRemoteWorms[iScoreboard[j + 1]].getTagTime()) {
				   // Swap the 2 scoreboard entries
					s = iScoreboard[j];
					iScoreboard[j] = iScoreboard[j+1];
					iScoreboard[j+1] = s;
				}

            } else if( iGameType == GMT_DEMOLITION ) {

                // DEMOLITIONS
                if(cRemoteWorms[iScoreboard[j]].getDirtCount() < cRemoteWorms[iScoreboard[j + 1]].getDirtCount()) {
				    // Swap the 2 scoreboard entries
					s = iScoreboard[j];
					iScoreboard[j] = iScoreboard[j+1];
					iScoreboard[j+1] = s;
				}

			} else if ( iGameType == GMT_CTF ) {

				// CAPTURE THE FLAG
				if(cRemoteWorms[iScoreboard[j]].getKills() < cRemoteWorms[iScoreboard[j + 1]].getKills()) {

					// Swap the 2 scoreboard entries
					s = iScoreboard[j];
					iScoreboard[j] = iScoreboard[j+1];
					iScoreboard[j+1] = s;
				} else if(cRemoteWorms[iScoreboard[j]].getKills() == cRemoteWorms[iScoreboard[j + 1]].getKills()) {

					// Equal kills, so compare lives
					if(cRemoteWorms[iScoreboard[j]].getLives() < cRemoteWorms[iScoreboard[j + 1]].getLives()) {

						// Swap the 2 scoreboard entries
						s = iScoreboard[j];
						iScoreboard[j] = iScoreboard[j+1];
						iScoreboard[j+1] = s;
					}
				}
			} else {

				// DEATHMATCH or TEAM DEATHMATCH or VIP or TEAMCTF
				if(cRemoteWorms[iScoreboard[j]].getLives() < cRemoteWorms[iScoreboard[j + 1]].getLives()) {

					// Swap the 2 scoreboard entries
					s = iScoreboard[j];
					iScoreboard[j] = iScoreboard[j+1];
					iScoreboard[j+1] = s;
				} else if(cRemoteWorms[iScoreboard[j]].getLives() == cRemoteWorms[iScoreboard[j + 1]].getLives()) {

					// Equal lives, so compare kills
					if(cRemoteWorms[iScoreboard[j]].getKills() < cRemoteWorms[iScoreboard[j + 1]].getKills()) {

						// Swap the 2 scoreboard entries
						s = iScoreboard[j];
						iScoreboard[j] = iScoreboard[j+1];
						iScoreboard[j+1] = s;
					}
				}
			}
		}
	}

	bUpdateScore = true;
}


///////////////////
// Destroy a bonus (not explosively)
void CClient::DestroyBonus(int id, bool local, int wormid)
{
	cBonuses[id].setUsed(false);


	// Tell the server the bonus is gone if we grabbed it
	if(local) {
		// TODO: move this out here
		CBytestream bs;
		bs.writeByte(C2S_GRABBONUS);
		bs.writeByte(id);
		bs.writeByte(wormid);
		bs.writeByte(cRemoteWorms[wormid].getCurrentWeapon());

		cNetChan.AddReliablePacketToSend(bs);
	}
}


///////////////////
// Check the demolitions game
void CClient::CheckDemolitionsGame(void)
{
    // If the map has less then a 1/5th of the dirt it once had, the game is over
    // And the worm with the highest dirt count wins

    if( iGameType != GMT_DEMOLITION || tGameInfo.iGameType != GME_LOCAL )
        return;

    // Add up all the worm's dirt counts
    int nDirtCount = 0;
    int highest = -99999;
    int winner = -1;

    CWorm *w = cRemoteWorms;
    for( short i=0; i<MAX_WORMS; i++,w++ ) {
        if( !w->isUsed() )
            continue;

        nDirtCount += w->getDirtCount();

        // Get the highest dirt count
        if( w->getDirtCount() > highest ) {
            highest = w->getDirtCount();
            winner = i;
        }
    }

    if( nDirtCount > (float)cMap->GetDirtCount()*0.8f ) {

        // Make the server trigger a game over
        cServer->DemolitionsGameOver(winner);
    }
}


///////////////////
// Show a laser sight
void CClient::LaserSight(CWorm *w)
{
	wpnslot_t *Slot = w->getCurWeapon();

	// Safety
	if(!Slot->Weapon)
		return;

	// Only show the sight if the player is NOT reloading
	if(Slot->Reloading)
		return;


	// Get the direction angle
	float Angle = w->getAngle();
	if(w->getDirection() == DIR_LEFT)
		Angle=180-Angle;


	// Trace a line from the worm to length or until it hits something
	CVec pos = w->getPos();
	CVec dir;
	GetAngles((int)Angle,&dir,NULL);

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
				w->setTarget(true);
				break;
			}
		}

		if(stopbeam)
			break;

		pos = pos + dir*(float)divisions;
	}

	// Spawn a laser sight entity
	SpawnEntity(ENT_LASERSIGHT, i, w->getPos(), dir, 0, NULL);
}


///////////////////
// Process the shots
void CClient::ProcessShots(void)
{
	int num = cShootList.getNumShots();

	for(int i=0; i<num; i++) {
		shoot_t *sh = cShootList.getShot(i);

		if(sh)
			ProcessShot(sh);
	}
}


///////////////////
// Process a shot
void CClient::ProcessShot(shoot_t *shot)
{
	CWorm *w = &cRemoteWorms[shot->nWormID];

    // If this worm is dead, ignore the shot
    if(!w->getAlive()) {
    	printf("WARNING: dead worm was shooting\n");
    	return;
    }

	// Weird
	if (!cGameScript.GetWeapons()) {
		printf("WARNING: weapons not loaded while a client was shooting\n");
		return;
	}

	const weapon_t *wpn = cGameScript.GetWeapons() + shot->nWeapon;

	// Process beam weapons differently
	if(wpn->Type == WPN_BEAM) {
		ProcessShot_Beam(shot);
		return;
	}


	CVec dir;
	GetAngles(shot->nAngle,&dir,NULL);
	CVec pos = shot->cPos + dir*8;


	// Play the weapon's sound
	if(wpn->UseSound)
		StartSound(wpn->smpSample, w->getPos(), w->getLocal(), 100, cLocalWorms[0]);

	// Add the recoil
	CVec *vel = w->getVelocity();
	*vel -= dir*(float)wpn->Recoil;

	// Draw the muzzle flash
	w->setDrawMuzzle(true);

	// fServerTime is the time we calculated for the server,
	// shot->fTime was the fServerTime given by the other client when it produced the shot
	float fSpawnTime = tLX->fCurTime - (fServerTime - shot->fTime);
	CVec sprd;

	for(int i=0; i<wpn->ProjAmount; i++) {

		int rot = 0;

		// Spread
		float a = (float)shot->nAngle + GetFixedRandomNum(shot->nRandom)*(float)wpn->ProjSpread;

		if(a < 0)
			a+=360;
		if(a>360)
			a-=360;

		GetAngles((int)a,&sprd,NULL);

		// Calculate a random starting angle for the projectile rotation (if used)
		if(wpn->Projectile) {
			if(wpn->Projectile->Rotating) {

				// Prevent div by zero
				if(wpn->Projectile->RotIncrement == 0)
					wpn->Projectile->RotIncrement = 1;
				rot = GetRandomInt( 360 / wpn->Projectile->RotIncrement ) * wpn->Projectile->RotIncrement;
			}
		}

        shot->nRandom++;
		shot->nRandom %= 255;

		float speed = (float)wpn->ProjSpeed + (float)wpn->ProjSpeedVar * GetFixedRandomNum(shot->nRandom);

        shot->nRandom *= 5;
		shot->nRandom %= 255;

        CVec v = sprd*speed + shot->cWormVel;

		SpawnProjectile(pos, v, rot, w->getID(), wpn->Projectile, shot->nRandom, fSpawnTime);

		shot->nRandom++;
		shot->nRandom %= 255;
	}
}


///////////////////
// Process a shot - Beam
void CClient::ProcessShot_Beam(shoot_t *shot)
{
	CWorm *w = &cRemoteWorms[shot->nWormID];
	const weapon_t *wpn = cGameScript.GetWeapons() + shot->nWeapon;

	// Trace a line from the worm to length or until it hits something
	CVec dir;
	GetAngles(shot->nAngle,&dir,NULL);
	CVec pos = shot->cPos;

	int divisions = 1;			// How many pixels we go through each check (more = slower)

	if( wpn->Bm_Length < divisions)
		divisions = wpn->Bm_Length;

	// Make sure we have at least 1 division
	divisions = MAX(divisions,1);

	int stopbeam = false;
	CWorm *w2;
	short n;

	for(int i=0; i<wpn->Bm_Length; i+=divisions) {
		uchar px = cMap->GetPixelFlag( (int)pos.x, (int)pos.y );

		// Check bonus colision and destroy the bonus, if damage isn't -1
		if (wpn->Bm_Damage != -1)  {
			CBonus *b = cBonuses;
			for(int n=0;n<MAX_BONUSES;n++,b++) {
				if(!b->getUsed())
					continue;

				float bonussize = 3;
				if(fabs(pos.x - b->getPosition().x) < bonussize) {
					Explosion(pos,0,5,w->getID()); // Destroy the bonus by an explosion
					break;
				}
			}
		}

		if((px & (PX_DIRT|PX_ROCK))) {
			// No explosion if damage is -1
			if(wpn->Bm_Damage != -1) {
				//SpawnEntity(ENT_EXPLOSION, 5, pos, CVec(0,0), 0, NULL);
				int d = cMap->CarveHole(wpn->Bm_Damage, pos);
				w->incrementDirtCount(d);
			}

			// Stop the beam regardless of explosion being shown
			break;
		}

		// Check if it has hit any of the worms
		w2 = cRemoteWorms;
		for(n=0;n<MAX_WORMS;n++,w2++) {
			if(!w2->isUsed() || !w2->getAlive())
				continue;

			// Don't check against the creator
			if(w2->getID() == w->getID())
				continue;

			static const float wormsize = 5;
			if((pos - w2->getPos()).GetLength2() < wormsize*wormsize) {
				InjureWorm(w2, wpn->Bm_PlyDamage, w->getID());
				stopbeam = true;
				break;
			}
		}

		if(stopbeam)
			break;

		pos += dir*(float)divisions;
	}

	// Spawn a beam entity and don't draw 255,0,255 (pink) beams
	/*if(wpn->Bm_Colour[0] != 255 || wpn->Bm_Colour[1] != 0 || wpn->Bm_Colour[2] != 255) {
		Uint32 col = MakeColour(wpn->Bm_Colour[0], wpn->Bm_Colour[1], wpn->Bm_Colour[2]);
		SpawnEntity(ENT_BEAM, i, w->getPos(), dir, col, NULL);
	}*/

    CheckDemolitionsGame();
}




///////////////////
// Process any chatter
void CClient::processChatter(void)
{
	if (tGameInfo.iGameType == GME_LOCAL)
		return;

    keyboard_t *kb = GetKeyboard();

	// If we're currently typing a message, add any keys to it
	if(bChat_Typing) {

		fChat_BlinkTime += tLX->fDeltaTime;
		if (fChat_BlinkTime > 0.5f)  {
			bChat_CursorVisible = !bChat_CursorVisible;
			fChat_BlinkTime = 0;
		}
        
        // Go through the keyboard queue
        for(short i=0; i<kb->queueLength; i++) {
			
			if(kb->keyQueue[i].down && kb->keyQueue[i].sym == SDLK_ESCAPE) {
				// Stop typing
				bChat_Typing = false;
				sChat_Text = "";
				cLocalWorms[0]->clearInput();

				kb->keys[SDLK_ESCAPE] = false;
				kb->KeyDown[SDLK_ESCAPE] = false;
				kb->KeyUp[SDLK_ESCAPE] = false;
				
				break;
			}
			
			bChat_CursorVisible = true;
            processChatCharacter(kb->keyQueue[i]);
        }

        return;
    }


	// Check if we have hit the chat key and we're in a network game
	if( ( cChat_Input.wasDown() || cTeamChat_Input.wasDown() ) && tGameInfo.iGameType != GME_LOCAL) {

		// Initialize the chatter
		fChat_BlinkTime = 0;
		bChat_CursorVisible = true;
		bChat_Typing = true;
		iChat_Pos = 0;
		sChat_Text = "";
		iChat_Lastchar = 0;
		bChat_Holding = false;
		fChat_TimePushed = -9999;
		bTeamChat = false;
		if( cTeamChat_Input.wasDown() )
			bTeamChat = true;

		// Clear the input
		for (uint j=0;j<iNumWorms;j++)
			if (cLocalWorms[j]->getType() == PRF_HUMAN)
				cLocalWorms[j]->clearInput();

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
#ifdef WITH_MEDIAPLAYER
				cToggleMediaPlayer.isDown() || 
#endif
				(input.sym == SDLK_BACKQUOTE) ||
				cTakeScreenshot.isDown();

			if(controls) continue;

			for(ushort j=0; j < iNumWorms; j++)  {
				if (cLocalWorms[j]->getType() == PRF_HUMAN)  {
					// Can we type?
					if (!cLocalWorms[j]->CanType() && cLocalWorms[j]->isUsed())
						return;

					// Clear the input
					cLocalWorms[j]->clearInput();
				}
			}

			// Initialize the chatter
			fChat_BlinkTime = 0;
			bChat_CursorVisible = true;
			bChat_Typing = true;
			iChat_Pos = 0;
			sChat_Text = "";
			iChat_Lastchar = 0;
			bChat_Holding = false;
			fChat_TimePushed = -9999;
		}

        processChatCharacter(input);
    }
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
            if(tLX->fCurTime - fChat_TimePushed < 0.4f)
                return;
        }
    }

    if(!bChat_Holding) {
        bChat_Holding = true;
        fChat_TimePushed = tLX->fCurTime;
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
		cLocalWorms[0]->clearInput();
		
        // Send chat message to the server
		if(sChat_Text != "") {
			if( bTeamChat )	// No "/me" macro in teamchat - server won't recognize such command
				SendText("/teamchat \"" + sChat_Text + "\"", cLocalWorms[0]->getName());
			else
				SendText(sChat_Text, cLocalWorms[0]->getName());
		}
		sChat_Text = "";
        return;
    }

	// Paste
	if (input.ch == 22)  {
		size_t text_len = Utf8StringSize(sChat_Text);

		// Safety
		if (iChat_Pos > text_len)
			iChat_Pos = text_len;

		// Get the text
		std::string buf = copy_from_clipboard();

		// Paste
		Utf8Insert(sChat_Text, iChat_Pos, buf);
		iChat_Pos += Utf8StringSize(buf);
		return;
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
