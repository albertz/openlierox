/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Server class - Game routines
// Created 11/7/02
// Jason Boettcher

#include <iostream>

#include "LieroX.h"
#include "CServer.h"

#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "StringUtils.h"
#include "Protocol.h"
#include "CWorm.h"
#include "MathLib.h"
#include "DedicatedControl.h"
#include "Physics.h"
#include "DeprecatedGUI/Menu.h"


using namespace std;

///////////////////
// Spawn a worm
void GameServer::SpawnWorm(CWorm *Worm, CVec * _pos, CServerConnection * client)
{
	if (bGameOver || Worm->isSpectating())
		return;

	CVec pos;

	if( _pos )
		pos = *_pos;
	else
	{
		pos = FindSpot();
		
		// Spawn worm closer to it's own team and away from other teams
		if( tLXOptions->tGameInfo.bRespawnGroupTeams &&
			( tLXOptions->tGameInfo.iGameMode == GMT_TEAMDEATH || tLXOptions->tGameInfo.iGameMode == GMT_TEAMCTF || tLXOptions->tGameInfo.iGameMode == GMT_VIP ) )
		{
			float team_dist = 0;
			CVec pos1;
			
			for( int k=0; k<100; k++ )
			{
				float team_dist1 = 0;
				pos1 = FindSpot();
				CWorm * w = cWorms;
				for(int i = 0; i < MAX_WORMS; i++, w++)
				{
					if( !w->isUsed() || w->getLives() == WRM_OUT || !w->getWeaponsReady() || 
						Worm->getID() == w->getID() || !w->getAlive() )
						continue;
					// sqrt will make sure there's no large dist between team1 and 2 and short dist between 2 and 3
					// The sum will get considerably smaller if any two teams are on short dist
					if( w->getTeam() == Worm->getTeam() )
						team_dist1 -= ( pos1 - w->getPos() ).GetLength() / 10.0f;
					else
						team_dist1 += sqrt( ( pos1 - w->getPos() ).GetLength() );
				};
				if( team_dist1 > team_dist )
				{
					team_dist = team_dist1;
					pos = pos1;
				};
			};
		};
	};

	// Spawn the worm in the flag position if possible
	if(Worm->getFlag() && tLXOptions->tGameInfo.iGameMode == GMT_CTF)
		pos = cMap->getFlagSpawn();

	if(pos.x == -1 && pos.y == -1)
		pos = FindSpot();

	Worm->Spawn(pos);

	if( client )	// Spawn all playing worms only for new client for connect-during-game
		client->getNetEngine()->SendSpawnWorm(Worm, pos);
	else
	{
		if( DedicatedControl::Get() )
			DedicatedControl::Get()->WormSpawned_Signal(Worm);
		for( int i = 0; i < MAX_CLIENTS; i++ )
			cClients[i].getNetEngine()->SendSpawnWorm(Worm, pos);
	}
}

///////////////////
// Find a spot with no rock
CVec GameServer::FindSpot(void)
{
    int     x, y;
    int     px, py;
    bool    first = true;
    int     cols = cMap->getGridCols() - 1;       // Note: -1 because the grid is slightly larger than the
    int     rows = cMap->getGridRows() - 1;       // level size
    int     gw = cMap->getGridWidth();
    int     gh = cMap->getGridHeight();

	uchar pf, pf1, pf2, pf3, pf4;
	cMap->lockFlags();
	
	// Find a random cell to start in - retry if failed
	for( int tries = 0; tries < 40; tries++ ) {
	    px = (int)(fabs(GetRandomNum()) * (float)cols);
		py = (int)(fabs(GetRandomNum()) * (float)rows);

	    x = px; y = py;

		pf = *(cMap->getAbsoluteGridFlags() + y * cMap->getGridCols() + x);
		pf1 = (x>0) ? *(cMap->getAbsoluteGridFlags() + y * cMap->getGridCols() + (x-1)) : PX_ROCK;
		pf2 = (x<cols-1) ? *(cMap->getAbsoluteGridFlags() + y * cMap->getGridCols() + (x+1)) : PX_ROCK;
		pf3 = (y>0) ? *(cMap->getAbsoluteGridFlags() + (y-1) * cMap->getGridCols() + x) : PX_ROCK;
		pf4 = (y<rows-1) ? *(cMap->getAbsoluteGridFlags() + (y+1) * cMap->getGridCols() + x) : PX_ROCK;
		if( !(pf & PX_ROCK) && !(pf1 & PX_ROCK) && !(pf2 & PX_ROCK) && !(pf3 & PX_ROCK) && !(pf4 & PX_ROCK) ) {
			cMap->unlockFlags();
            return CVec((float)x * gw + gw / 2, (float)y * gh + gh / 2);
		}
	}

    // Start from the cell and go through until we get to an empty cell
    while(true) {
        while(true) {
            // If we're on the original starting cell, and it's not the first move we have checked all cells
            // and should leave
            if(!first) {
                if(px == x && py == y) {
					cMap->unlockFlags();
                    return CVec((float)x * gw + gw / 2, (float)y * gh + gh / 2);
                }
            }
            first = false;

			pf = *(cMap->getAbsoluteGridFlags() + y * cMap->getGridCols() + x);
            if(!(pf & PX_ROCK))  {
				cMap->unlockFlags();
                return CVec((float)x * gw + gw / 2, (float)y * gh + gh / 2);
			}

            if(++y >= rows) {
                y = 0;
                break;
            }
        }

        if(++x >= cols) {
            x = 0;
            y = 0;
            break;
        }
    }
	cMap->unlockFlags();

    // Can't get here
    return CVec((float)x, (float)y);
}

void GameServer::killWorm( int victim, int killer, int suicidesCount )
{
	// Team names
	static const std::string TeamNames[] = {"blue", "red", "green", "yellow"};
	int TeamCount[4];

	// If the game is already over, ignore this
	if (bGameOver)  {
		printf("GameServer::killWorm: Game is over, ignoring.\n");
		return;
	}
	// Safety check
	if (victim < 0 || victim >= MAX_WORMS)  {
		printf("GameServer::killWorm: victim ID out of bounds.\n");
		return;
	}
	if (killer < 0 || killer >= MAX_WORMS)  {
		printf("GameServer::killWorm: killer ID out of bounds.\n");
		return;
	}

	if(victim != killer )
		suicidesCount = 0;
	if(victim == killer && suicidesCount == 0 )
		suicidesCount = 1;

	CWorm *vict = &cWorms[victim];
	CWorm *kill = &cWorms[killer];

	// Cheat prevention, game behaves weird if this happens
	if (vict->getLives() < 0 && tLXOptions->tGameInfo.iLives >= 0)  {
		vict->setLives(WRM_OUT);  // Safety
		printf("GameServer::ParseDeathPacket: victim is already out of the game.\n");
		return;
	}

	// Adjust the score if there were multiple suicides
	if (suicidesCount > 1)  {
		if (tLXOptions->tGameInfo.iLives != WRM_UNLIM) // Substracting from infinite makes no sense
			vict->setLives(MAX(WRM_OUT, vict->getLives() - suicidesCount + 1)); // HINT: +1 because one life is substracted in vict->Kill()
	}

	std::string buf;

	// Kill
	if( vict->getAFK() && killer != victim )
	{
		if (networkTexts->sKilledAFK != "<none>") // Take care of the <none> tag
		{
			replacemax(networkTexts->sKilledAFK, "<killer>", kill->getName(), buf, 1);
			replacemax(buf, "<victim>", vict->getName(), buf, 1);
			SendGlobalText(buf, TXT_NORMAL);
		}
	}
	else
	{
		if (networkTexts->sKilled != "<none>")  { // Take care of the <none> tag
			if (killer != victim)  {
				replacemax(networkTexts->sKilled, "<killer>", kill->getName(), buf, 1);
				replacemax(buf, "<victim>", vict->getName(), buf, 1);
			} else
				replacemax(networkTexts->sCommitedSuicide + (suicidesCount > 1 ? " (" + itoa(suicidesCount) + "x)" : ""),
				"<player>", vict->getName(), buf, 1);
	
			SendGlobalText(buf, TXT_NORMAL);
		}
	};

	// First blood
	if (bFirstBlood && killer != victim && networkTexts->sFirstBlood != "<none>")  {
		replacemax(networkTexts->sFirstBlood, "<player>", kill->getName(), buf, 1);
		bFirstBlood = false;
		SendGlobalText(buf, TXT_NORMAL);
	}

	// Teamkill
	if ((tLXOptions->tGameInfo.iGameMode == GMT_TEAMDEATH || tLXOptions->tGameInfo.iGameMode == GMT_VIP) && vict->getTeam() == kill->getTeam() && killer != victim)  {
		//Take care of the <none> tag
		if (networkTexts->sTeamkill != "<none>")  {
			replacemax(networkTexts->sTeamkill, "<player>", kill->getName(), buf, 1);
			SendGlobalText(buf, TXT_NORMAL);
		}
	}

	vict->addTotalDeaths();
	if (suicidesCount >= 1)
		vict->addTotalSuicides(); // Do not count multiple suicides from /suicide command

	// Update victim statistics
	vict->setKillsInRow(0);
	if (suicidesCount <= 1)  // HINT: don't add death in row for multi-suicides because the dying spree messages are not adequate for console suicides
		vict->addDeathInRow();

	// Kills don't count in capture the flag
	if (killer != victim && (tLXOptions->tGameInfo.iGameMode != GMT_CTF && tLXOptions->tGameInfo.iGameMode != GMT_TEAMCTF))  {
		// Don't add a kill for teamkilling (if enabled in options)
		// Client's score and scoreboard is controlled by the server which makes this backward compatible
		if((vict->getTeam() != kill->getTeam() && killer != victim) || 
			tLXOptions->tGameInfo.iGameMode != GMT_TEAMDEATH || tLXOptions->tGameInfo.bCountTeamkills ) {
			kill->addKillInRow();
			kill->AddKill();
			kill->setDeathsInRow(0);
			kill->addTotalKills();
		}
	}

	// Suicided or killed team member - decrease score if selected in options
	if( tLXOptions->tGameInfo.bSuicideDecreasesScore && ( killer == victim ||
		( (tLXOptions->tGameInfo.iGameMode == GMT_TEAMDEATH || tLXOptions->tGameInfo.iGameMode == GMT_VIP) && vict->getTeam() == kill->getTeam() )))
			if( kill->getKills() > 0 )
			{
				kill->setKills( kill->getKills() - 1 );
				kill->addTotalKills(-1);
			}

	// If the flag was attached to the dead worm then release the flag
	for(int j=0;j<MAX_WORMS;j++)
		if(getFlagHolder(j) == victim && (tLXOptions->tGameInfo.iGameMode == GMT_CTF || tLXOptions->tGameInfo.iGameMode == GMT_TEAMCTF))
			setFlagHolder(-1, j);

	// Killing spree message
	switch (kill->getKillsInRow())  {
	case 3:
		if (networkTexts->sSpree1 != "<none>")  {
			replacemax(networkTexts->sSpree1, "<player>", kill->getName(), buf, 1);
			SendGlobalText(buf, TXT_NORMAL);
		}
		break;
	case 5:
		if (networkTexts->sSpree2 != "<none>")  {
			replacemax(networkTexts->sSpree2, "<player>", kill->getName(), buf, 1);
			SendGlobalText(buf, TXT_NORMAL);
		}
		break;
	case 7:
		if (networkTexts->sSpree3 != "<none>")  {
			replacemax(networkTexts->sSpree3, "<player>", kill->getName(), buf, 1);
			SendGlobalText((buf), TXT_NORMAL);
		}
		break;
	case 9:
		if (networkTexts->sSpree4 != "<none>")  {
			replacemax(networkTexts->sSpree4, "<player>", kill->getName(), buf, 1);
			SendGlobalText((buf), TXT_NORMAL);
		}
		break;
	case 10:
		if (networkTexts->sSpree5 != "<none>")  {
			replacemax(networkTexts->sSpree5, "<player>", kill->getName(), buf, 1);
			SendGlobalText((buf), TXT_NORMAL);
		}
		break;
	}

	// Dying spree message
	switch (vict->getDeathsInRow()) {
	case 3:
		if (networkTexts->sDSpree1 != "<none>")  {
			replacemax(networkTexts->sDSpree1, "<player>", vict->getName(), buf, 1);
			SendGlobalText((buf), TXT_NORMAL);
		}
		break;
	case 5:
		if (networkTexts->sDSpree2 != "<none>")  {
			replacemax(networkTexts->sDSpree2, "<player>", vict->getName(), buf, 1);
			SendGlobalText((buf), TXT_NORMAL);
		}
		break;
	case 7:
		if (networkTexts->sDSpree3 != "<none>")  {
			replacemax(networkTexts->sDSpree3, "<player>", vict->getName(), buf, 1);
			SendGlobalText((buf), TXT_NORMAL);
		}
		break;
	case 9:
		if (networkTexts->sDSpree4 != "<none>")  {
			replacemax(networkTexts->sDSpree4, "<player>", vict->getName(), buf, 1);
			SendGlobalText((buf), TXT_NORMAL);
		}
		break;
	case 10:
		if (networkTexts->sDSpree5 != "<none>")  {
			replacemax(networkTexts->sDSpree5, "<player>", vict->getName(), buf, 1);
			SendGlobalText((buf), TXT_NORMAL);
		}
		break;
	}


	if (vict->Kill()) {

		// This worm is out of the game
		if (networkTexts->sPlayerOut != "<none>") {
			replacemax(networkTexts->sPlayerOut, "<player>", vict->getName(), buf, 1);
			SendGlobalText((buf), TXT_NORMAL);
		}

		// Check if only one person is left
		int wormsleft = 0;
		int wormid = 0;
		CWorm *w = cWorms;
		int i;
		for (i = 0;i < MAX_WORMS;i++, w++) {
			if (w->isUsed() && w->getLives() != WRM_OUT) {
				wormsleft++;
				wormid = i;
			}
		}

		if (wormsleft <= 1) { // There can be also 0 players left (you play alone and suicide)
			// Declare the winner
			switch (tLXOptions->tGameInfo.iGameMode)  {
			case GMT_DEATHMATCH:
				if (networkTexts->sPlayerHasWon != "<none>")  {
					CWorm *winner = cWorms + wormid;
					replacemax(networkTexts->sPlayerHasWon, "<player>", winner->getName(), buf, 1);
					SendGlobalText((buf), TXT_NORMAL);
				}
				break;  // DEATHMATCH
			case GMT_TAG:
				// Get the worm with greatest tag time
				float fMaxTagTime = 0;
				wormid = 0;
				CWorm *w = cWorms;
				for (int i = 0;i < MAX_WORMS;i++)
					if (w->isUsed())
						if (w->getTagTime() > fMaxTagTime)  {
							fMaxTagTime = w->getTagTime();
							wormid = i;
						}

				// Worm with greatest tag time
				w = cWorms + wormid;

				// Send the text
				if (networkTexts->sPlayerHasWon != "<none>")  {
					replacemax(networkTexts->sPlayerHasWon, "<player>", w->getName(), buf, 1);
					SendGlobalText((buf), TXT_NORMAL);
				}
				break;  // TAG

				// TEAM DEATHMATCH is handled below
			}

			cout << "only one player left" << endl;
			GameOver(wormid);
		}



		// If the game is still going and this is a teamgame, check if the team this worm was in still
		// exists
		if (!bGameOver && tLXOptions->tGameInfo.iGameMode == GMT_TEAMDEATH) {
			int team = vict->getTeam();
			int teamcount = 0;

			for (i = 0;i < 4;i++)
				TeamCount[i] = 0;

			// Check if anyone else is left on the team
			w = cWorms;
			for (i = 0;i < MAX_WORMS;i++, w++) {
				if (w->isUsed()) {
					if (w->getLives() != WRM_OUT && w->getTeam() == team)
						teamcount++;

					if (w->getLives() != WRM_OUT)
						TeamCount[w->getTeam()]++;
				}
			}

			// No-one left in the team
			if (teamcount == 0) {
				if (networkTexts->sTeamOut != "<none>")  {
					replacemax(networkTexts->sTeamOut, "<team>", TeamNames[team], buf, 1);
					SendGlobalText((buf), TXT_NORMAL);
				}
			}

			// If there is only 1 team left, declare the last team the winner
			int teamsleft = 0;
			team = 0;
			for (i = 0;i < 4;i++) {
				if (TeamCount[i]) {
					teamsleft++;
					team = i;
				}
			}

			if (teamsleft <= 1) { // There can be also 0 teams left (you play TDM alone and suicide)
				if (networkTexts->sTeamHasWon != "<none>")  {
					replacemax(networkTexts->sTeamHasWon, "<team>", TeamNames[team], buf, 1);
					SendGlobalText((buf), TXT_NORMAL);
				}

				cout << "no other team left" << endl;
				GameOver(team);
			}
		}
		if (!bGameOver && (tLXOptions->tGameInfo.iGameMode == GMT_VIP || tLXOptions->tGameInfo.iGameMode == GMT_CTF || tLXOptions->tGameInfo.iGameMode == GMT_TEAMCTF))
			RecheckGame();
	}


	// Check if the max kills has been reached
	if (tLXOptions->tGameInfo.iKillLimit != -1 && killer != victim && kill->getKills() == tLXOptions->tGameInfo.iKillLimit) {
		cout << "max kills reached" << endl;

		// Game over (max kills reached)
		GameOver(kill->getID());
	}


	// If the worm killed is IT, then make the killer now IT
	if (tLXOptions->tGameInfo.iGameMode == GMT_TAG && !bGameOver)  {
		if (killer != victim) {
			if (vict->getTagIT()) {
				vict->setTagIT(false);

				// If the killer is dead, tag a worm randomly
				if (!kill->getAlive() || kill->getLives() == WRM_OUT)
					TagRandomWorm();
				else
					TagWorm(kill->getID());
			}
		} else {
			// If it's a suicide and the worm was it, tag a random person
			if (vict->getTagIT()) {
				vict->setTagIT(false);
				TagRandomWorm();
			}
		}
	}

	CBytestream byte;
	// Update everyone on the victims & killers score
	vict->writeScore(&byte);
	if (killer != victim)
		kill->writeScore(&byte);
	if( tLXOptions->tGameInfo.bGroupTeamScore && (tLXOptions->tGameInfo.iGameMode == GMT_TEAMDEATH || tLXOptions->tGameInfo.iGameMode == GMT_VIP) )
	{	// All worms in the same team will have same grouped kill count
		CWorm *w = cWorms;
		for( int f = 0; f < MAX_WORMS; f++, w++ )
			if( w->isUsed() && f != killer )
				if ( w->getLives() != WRM_OUT && w->getTeam() == kill->getTeam() )
				{
					w->setKills( kill->getKills() );
					w->writeScore(&byte);
				}
	}

	// Let everyone know that the worm is now dead
	byte.writeByte(S2C_WORMDOWN);
	byte.writeByte(victim);

	SendGlobalPacket(&byte);

	if( DedicatedControl::Get() )
		DedicatedControl::Get()->WormDied_Signal(vict,kill);

};


///////////////////
// Simulate the game stuff
void GameServer::SimulateGame(void)
{
	if(iState != SVS_PLAYING)
		return;

	// If this is a remote game, and game over,
	// and we've seen the scoreboard for a certain amount of time, go back to the lobby
	if(bGameOver
	&& (tLX->fCurTime - fGameOverTime > LX_ENDWAIT || (bDedicated && iNumPlayers <= 1)) // dedicated server should go to lobby immediatly if alone
	&& iState != SVS_LOBBY
	&& tLX->iGameType == GME_HOST) {
		gotoLobby();
		return;
	}

	// Don't process if the game is paused (local play only)
	if (cClient->getGamePaused())
		return;

	// Process worms
	CWorm *w = cWorms;
	short i ;

	for(i=0;i<MAX_WORMS;i++,w++) {
		if(!w->isUsed())
			continue;

		if(!w->getAlive() && w->getLives() != WRM_OUT && w->getWeaponsReady()) {
			// Check to see if they have been dead for longer then 2.5 seconds
			if(tLX->fCurTime - w->getTimeofDeath() > tLXOptions->tGameInfo.fRespawnTime )
			{
				SpawnWorm(w);
				if( tLXOptions->tGameInfo.bEmptyWeaponsOnRespawn )
					SendEmptyWeaponsOnRespawn(w);
			}
		}

		// Add their time in a game of tag
		if(tLXOptions->tGameInfo.iGameMode == GMT_TAG && w->getTagIT() && w->getAlive())  {
			w->incrementTagTime(tLX->fRealDeltaTime);
		}

		// Simulate the worm's weapons
		// TODO: why are we doing this? we are not simulating the worm but why the weapon?
		// please try to remove this here and then remove also the dt parameter in PhysicsEngine
		if( w->getAlive() )
			PhysicsEngine::Get()->simulateWormWeapon(tLX->fRealDeltaTime * tLXOptions->tGameInfo.fGameSpeed, w);
/*
		// If the flag has been held for 5 seconds and the map doesn't have a base give the worm a point
		if(tLX->fCurTime - fLastFlagPoint > 5 && w->getID() == getFlag(0) && iGameType == GMT_CTF && cMap->getBaseStart().x == -1) {
			w->AddKill();
			CBytestream bs;
			bs.Clear();
			w->writeScore(&bs);
			SendGlobalPacket(&bs);
			fLastFlagPoint = tLX->fCurTime;
		}

		// If a worm possesses both flags then respawn them and give the worm a point
		if(w->getID() == getFlag() == getFlag(1) && !w->getFlag() && iGameType == GMT_TEAMCTF) {
			w->AddKill();
			CBytestream bs;
			bs.Clear();
			w->writeScore(&bs);
			SendGlobalPacket(&bs);
			SpawnWorm(f[0]);
			SpawnWorm(f[1]);
			setFlag(-1,0);
			setFlag(-1,1);
		}

		// HINT: Grab the simulated position of the flag
		if(w->getFlag()) {
			CWorm *rw = cClient->getRemoteWorms() + w->getID();
			w->setPos(rw->getPos());
		}

		// If the worm holds the flag and is within the base respawn the Flag and give the worm a point
		for(j=0;j<flags;j++) {
			if(w->getID() == getFlag(j) && iGameType == GMT_CTF) {
				if(w->getPos().x > cMap->getBaseStart().x && w->getPos().y > cMap->getBaseStart().y &&
						w->getPos().x < cMap->getBaseEnd().x && w->getPos().y < cMap->getBaseEnd().y) {
					w->AddKill();
					CBytestream bs;
					bs.Clear();
					w->writeScore(&bs);
					SendGlobalPacket(&bs);
					SpawnWorm(f[j]);
					// HINT: If the client worm isn't repositioned here then a worm will repeatedly score until they win
						CWorm *rw = cClient->getRemoteWorms() + f[j]->getID();
						rw->setPos(f[j]->getPos());
					setFlag(-1, j);
					SendGlobalText((replacemax(networkTexts->sHasScored,"<player>",w->getName(),1)),
						TXT_NORMAL);
					if(w->getKills()==iMaxKills)
						RecheckGame();
				}
			}
		}
*/	}



	// Check if any bonuses have been in for too long and need to be destroyed
	if (tLXOptions->tGameInfo.bBonusesOn)  {
		for(i=0; i<MAX_BONUSES; i++) {
			if(!cBonuses[i].getUsed())
				continue;

			// If it's been here too long, destroy it
			if( tLX->fCurTime - cBonuses[i].getSpawnTime() > tLXOptions->tGameInfo.fBonusLife ) {
				CBytestream bs;
				bs.Clear();
				cBonuses[i].setUsed(false);

				// TODO: move this out here
				bs.writeByte(S2C_DESTROYBONUS);
				bs.writeByte((byte)i);
				SendGlobalPacket(&bs);
			}
		}
	}



	// Check if we need to spawn a bonus
	if(tLX->fCurTime - fLastBonusTime > tLXOptions->tGameInfo.fBonusFreq && tLXOptions->tGameInfo.bBonusesOn && !bGameOver) {

		SpawnBonus();

		fLastBonusTime = tLX->fCurTime;
	}

	// Simulate the 'special' gametype stuff
	if(tLXOptions->tGameInfo.iGameMode == GMT_CTF)
		SimulateGameSpecial();

	if( tLXOptions->tGameInfo.fTimeLimit > 0 && fServertime > tLXOptions->tGameInfo.fTimeLimit*60.0 )
		RecheckGame();
}


///////////////////
// Spawn a bonus
void GameServer::SpawnBonus(void)
{
	// Find an empty spot for the bonus
	CVec pos = FindSpot();

	// Carve a hole for it
	//cMap->CarveHole(SPAWN_HOLESIZE,pos);

	// NOTE: Increase to 2 when we want to use the fullcharge bonus
	int type = (GetRandomInt(999) >= tLXOptions->tGameInfo.fBonusHealthToWeaponChance * 1000.0f) ? BNS_HEALTH : BNS_WEAPON;
	int wpn = GetRandomInt(cGameScript.get()->GetNumWeapons()-1);


	// Find a free bonus spot
	CBonus *b = cBonuses;
	int spot = -1;
	for(short i=0;i<MAX_BONUSES;i++,b++) {
		if(b->getUsed())
			continue;

		spot = i;
		break;
	}

	// No spots
	if(spot == -1)
		return;


    // Check if the weapon chosen is enabled or a 'bonus' weapon in the restrictions
    int orig = wpn;
    while(1) {
        int state = cWeaponRestrictions.getWeaponState( (cGameScript.get()->GetWeapons()+wpn)->Name );

        if( state != wpr_banned )
            break;

        wpn++;
        if( wpn >= cGameScript.get()->GetNumWeapons())
            wpn=0;

        // No good weapons? Just leave with original choice
        if( wpn == orig )
            break;
    }


	b->Spawn(pos, type, wpn, cGameScript.get());


	// Send the spawn the everyone
	CBytestream bs;

	bs.writeByte(S2C_SPAWNBONUS);
	bs.writeInt(type,1);
	if(type == BNS_WEAPON)
		bs.writeInt(wpn,1);
	bs.writeInt(spot,1);

	bs.writeInt((int)pos.x, 2);
	bs.writeInt((int)pos.y, 2);

	SendGlobalPacket(&bs);
}


///////////////////
// Tag a worm
void GameServer::TagWorm(int id)
{
	// Safety check
	if(id < 0 || id >= MAX_WORMS || getState() != SVS_PLAYING)
		return;

	CWorm *w = &cWorms[id];

	short i;

	// Go through all the worms, setting their tag to false
	for(i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed())  {
			cWorms[i].setTagIT(false);
		}
	}


	w->setTagIT(true);

	// Let everyone know this guy is tagged
	CBytestream bs;
	bs.writeByte(S2C_TAGUPDATE);
	bs.writeInt(id,1);
	bs.writeFloat(w->getTagTime());

	SendGlobalPacket(&bs);

	//Take care of the <none> tag
	if (networkTexts->sWormIsIt != "<none>")  {
		SendGlobalText((replacemax(networkTexts->sWormIsIt,"<player>",w->getName(),1)),
						TXT_NORMAL);
	}
}


///////////////////
// Tag a random worm
void GameServer::TagRandomWorm(void)
{
	float time = 99999;
	std::vector<int> all_lowest;


	// Go through finding the worm with the lowest tag time
	// A bit more fairer then random picking
	unsigned short i;
	for(i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT) {
			if(cWorms[i].getTagTime() < time) {
				time = cWorms[i].getTagTime();
			}
		}
	}

	// Find all the worms that have the lowest time
	for (i=0;i<MAX_WORMS;i++)  {
		if (cWorms[i].isUsed() && cWorms[i].getLives() != WRM_OUT)  {
			if (cWorms[i].getTagTime() == time)  {
				all_lowest.push_back(i);
			}
		}
	}

	// Choose a random worm from all those having the lowest time
	int random_lowest = GetRandomInt((int)all_lowest.size()-1);


	// Tag the lowest tagged worm
	TagWorm(all_lowest[random_lowest]);



	// Go through the worms finding the tagged worm
	/*int i,n=0;
	for(i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed()) {
			if(n==tag)
				tag=i;
			n++;
		}
	}*/

	// Tag the player
	//TagWorm(tag);
}


///////////////////
// Worm is shooting
void GameServer::WormShoot(CWorm *w, GameServer* gameserver)
{
	// Don't shoot when the game is over
	if (cClient->isGameOver())
		return;

	// If the worm is a VIP and the gametype is VIP don't shoot
	if(w->getVIP() && tLXOptions->tGameInfo.iGameMode == GMT_VIP)
		return;

	// If the worm is a Flag and the gametype is CTF don't shoot
	if(w->getFlag() && (tLXOptions->tGameInfo.iGameMode == GMT_CTF || tLXOptions->tGameInfo.iGameMode == GMT_TEAMCTF))
		return;

	wpnslot_t *Slot = w->getCurWeapon();

	if(Slot->Reloading)
		return;

	if(Slot->LastFire>0)
		return;

	// Don't shoot with banned weapons
	if (!Slot->Enabled)
		return;

	if(!Slot->Weapon) {
		printf("WARNING: trying to shoot with an unitialized weapon!\n");
		return;
	}

	// TODO: what is the effect of this?
	Slot->LastFire = Slot->Weapon->ROF;


	// Must be a projectile
	if(Slot->Weapon->Type != WPN_PROJECTILE && Slot->Weapon->Type != WPN_BEAM)
		return;

	// Get the direction angle
	float Angle = w->getAngle();
	if(w->getDirection() == DIR_LEFT)
		Angle=180-Angle;

	if(Angle < 0)
		Angle+=360;
	if(Angle > 360)
		Angle-=360;
	if(Angle == 360)
		Angle=0;

	float speed = 0.0f;

	// only projectile wpns have speed; Beam weapons have no speed
	if(Slot->Weapon->Type == WPN_PROJECTILE) {
		// Add the shot to the shooting list
		CVec vel = *w->getVelocity();
		speed = NormalizeVector( &vel );
	}
	
	if(gameserver) {
		float time = gameserver->getServerTime();
		if( w->hasOwnServerTime() )
			time = w->serverTime();
		
		// Add the shot to ALL the connected clients shootlist
		CServerConnection *cl = gameserver->getClients();
		for(short i=0; i<MAX_CLIENTS; i++,cl++) {
			if(cl->getStatus() == NET_DISCONNECTED)
				continue;

			cl->getShootList()->addShoot( time, speed, (int)Angle, w);
		}
	}
	

	//
	// Note: Drain does NOT have to use a delta time, because shoot timing is controlled by the ROF
	// (ROF = Rate of Fire)
	//

	// Drain the Weapon charge
	Slot->Charge -= Slot->Weapon->Drain / 100;
	if(Slot->Charge <= 0) {
		Slot->Charge = 0;
		Slot->Reloading = true;
	}
}




///////////////////
// Go back to the lobby
void GameServer::gotoLobby(void)
{
	printf("gotoLobby\n");

	// in lobby we need the events again
	AddSocketToNotifierGroup( tSocket );
	for( int f=0; f<MAX_CLIENTS; f++ )
		if(IsSocketStateValid(tNatTraverseSockets[f]))
			AddSocketToNotifierGroup(tNatTraverseSockets[f]);

	short i;

	// Tell all the clients
	CBytestream bs;
	bs.writeByte(S2C_GOTOLOBBY);
	SendGlobalPacket(&bs);

	// Clear the info
	iState = SVS_LOBBY;
	bool bUpdateWorms = false;
	for(i=0;i<MAX_WORMS;i++) {
		if(cWorms[i].isUsed()) {
			cWorms[i].getLobby()->bReady = false;
			cWorms[i].setGameReady(false);
			cWorms[i].setTagIT(false);
			cWorms[i].setTagTime(0);
			if( cWorms[i].getAFK() == AFK_TYPING_CHAT )
			{
				cWorms[i].setAFK(AFK_BACK_ONLINE, "");
				CBytestream bs;
				bs.writeByte( S2C_AFK );
				bs.writeByte( (uchar)i );
				bs.writeByte( AFK_BACK_ONLINE );
				bs.writeString( "" );
	
				CServerConnection *cl;
				int i;
				for( i=0, cl=cClients; i < MAX_CLIENTS; i++, cl++ )
					if( cl->getStatus() == NET_CONNECTED && cl->getClientVersion() >= OLXBetaVersion(7) )
						cl->getNetEngine()->SendPacket(&bs);
			}
		}
		if(cWorms[i].getFlag()) {
			cWorms[i].setUsed(false);
			cWorms[i].setFlag(false);
			CBytestream bs;
			bs.writeByte(S2C_WORMSOUT);
			bs.writeByte(1);
			bs.writeByte((uchar)i);
			SendGlobalPacket(&bs);
			iNumPlayers--;
		}
	}
	UpdateGameLobby();

	for(i=0; i<MAX_CLIENTS; i++) {
		cClients[i].setGameReady(false);
	}

	fLastUpdateSent = -9999;
	fGameOverTime = -9999;

	SendWormLobbyUpdate();
	
	if(bUpdateWorms)
		UpdateWorms();

	if( DedicatedControl::Get() )
		DedicatedControl::Get()->BackToLobby_Signal();

	// Goto the host lobby
	DeprecatedGUI::Menu_Net_GotoHostLobby();

	for( i=0; i<MAX_CLIENTS; i++ )
		cClients[i].getUdpFileDownloader()->allowFileRequest(true);

	// Re-register the server to reflect the state change
	RegisterServerUdp();

	// HINT: the gamescript is shut down by the cache
}


///////////////////
// Send out a game over for demolitions mode
// Called by client (for local games only!)
void GameServer::DemolitionsGameOver(int winner)
{
	GameOver(winner);
}


///////////////////
// Recheck the game status
// Called when a player has left the game (for various reasons)
void GameServer::RecheckGame(void)
{
    short i;

    //
    // If this is a tag game, make sure a person is tagged
    // If not, chances are the tagged person left
    //
    if( tLXOptions->tGameInfo.iGameMode == GMT_TAG && getState() == SVS_PLAYING ) {
        int tag = -1;

        CWorm *w = cWorms;
        for(i=0; i<MAX_WORMS; i++, w++) {
            if( !w->isUsed() )
                continue;

            if( w->getTagIT() )
                tag = i;
        }

        // Anyone tagged?
        // If not, tag a new person
        if( tag == -1 )
            TagRandomWorm();
    }

	// Check how many worms are alive
	if (getState() == SVS_PLAYING && !bGameOver)  {
		CWorm *w = cWorms;
		short wormcount = 0;
		short wormid = 0;
		for(i=0; i<MAX_WORMS; i++, w++) {
			if (w->isUsed() && w->getLives() != WRM_OUT)  {
				cout << "worm " << i << " has " << w->getLives() << " lives" << endl;
				wormcount++;
				wormid = i; // Save the worm id
			}
		}

		// TODO: there is a lot of double code in the following part; can't this be cleaned up?

		if (!bGameOver)  {

			bool EndGame = false;

			//
			// TEAM DEATHMATCH: Declare the team as winner
			//
			switch (tLXOptions->tGameInfo.iGameMode)  {
			case  GMT_TEAMDEATH:  {
				const std::string TeamNames[] = {"blue", "red", "green", "yellow"};
				int TeamCount[4];

				w = cWorms;

				for(i=0;i<4;i++)
					TeamCount[i]=0;

				// Check if anyone else is left on the team
				for(i=0;i<MAX_WORMS;i++,w++) {
					if(w->isUsed() && w->getLives() != WRM_OUT)
							TeamCount[w->getTeam()]++;
				}

				short teamsleft = 0;
				short team = 0;

				// Get the number of teams left
				for(i=0;i<4;i++)
					if(TeamCount[i])  {
						teamsleft++;
						team = i;
					}

				// Send the text
				if (teamsleft <= 1)  {
					if (networkTexts->sTeamHasWon != "<none>")  {
						SendGlobalText((replacemax(networkTexts->sTeamHasWon,"<team>",TeamNames[team],1)),
										TXT_NORMAL);
					}
					cout << "recheck: too less teams" << endl;
					EndGame = true;
				}
			}
			break; // TEAMDEATH

			//
			// DEATHMATCH: Declare the player who is alive as winner
			//
			case GMT_DEATHMATCH:  {
				// Only one worm left, end the game
				// though, in a local game it's sometimes nice to check also a map alone
				if (tLX->iGameType != GME_LOCAL && wormcount < 2)  {
					// Get the worm that is still alive and declare him as winner
					w = cWorms + wormid;

					// Send the text
					if (networkTexts->sPlayerHasWon != "<none>")  {
						SendGlobalText((replacemax(networkTexts->sPlayerHasWon,"<player>",w->getName(),1)),
										TXT_NORMAL);
					}
					cout << "recheck: too less worms" << endl;
					EndGame = true;
				}
			}
			break; // DEATHMATCH

			//
			// TAG: Declare the worm with greatest tag time as winner
			//
			case GMT_TAG:  {
				// Only one worm left, end the game
				if (wormcount < 2)  {
					// Get the worm with greatest tag time
					float fMaxTagTime = 0;
					int worm = 0;
					w = cWorms;
					for (int i=0;i<MAX_WORMS;i++)
						if (w->isUsed())
							if (w->getTagTime() > fMaxTagTime)  {
								fMaxTagTime = w->getTagTime();
								worm = i;
							}

					// Worm with greatest tag time
					w = cWorms+worm;

					// Send the text
					if (networkTexts->sPlayerHasWon!="<none>")  {
						SendGlobalText((replacemax(networkTexts->sPlayerHasWon,"<player>",w->getName(),1)),
										TXT_NORMAL);
					}

					cout << "recheck: too less worms" << endl;
					EndGame = true;
				}
			}

			break;  // TAG

			//
			// VIP: Declare the wining team to be the one that killed the VIP or the VIP depending on who is alive
			case GMT_VIP:  {
				const std::string TeamNames[] = {"VIP Defenders", "VIP Attackers", "VIP", "Neutral"};
				int TeamCount[4];

				w = cWorms;

				for(i=0;i<4;i++)
					TeamCount[i]=0;

				short VIPs = 0;

				// Check if anyone else is left on the team
				for(i=0;i<MAX_WORMS;i++,w++) {
					if(w->isUsed() && w->getLives() != WRM_OUT) {
							TeamCount[w->getTeam()]++;
							VIPs += w->getVIP();
					}
				}

				short teamsleft = 0;
				short team = 0;

				// Get the number of teams left
				for(i=0;i<2;i++)
					if(TeamCount[i])  {
						teamsleft++;
						team = i;
					}

				// Send the text
				if (teamsleft <= 1)  {
					if (networkTexts->sTeamHasWon != "<none>")  {
						SendGlobalText((replacemax(networkTexts->sTeamHasWon,"<team>",TeamNames[team],1)),
										TXT_NORMAL);
					}
					cout << "recheck: not enough teams anymore" << endl;
					EndGame = true;
				}

				if(VIPs == 0) { // All the VIPs are out of the game
					team = 1; // VIP Attackers win
					if (networkTexts->sTeamHasWon != "<none>")  {
						SendGlobalText((replacemax(networkTexts->sTeamHasWon,"<team>",TeamNames[team],1)),
										TXT_NORMAL);
					}
					cout << "recheck: all the VIPs are out of the game" << endl;
					EndGame = true;
				}
			}
			break; // VIP

			//
			// Capture the Flag: Declare the player with the most points the winner
			//
			case GMT_CTF:  {
				// Only two worms left (flag and another), end the game
				if (wormcount < 3)  {
					// Get the worm that is still alive and set his lives to out
					w = cWorms + wormid;
					w->setLives(WRM_OUT);
					// Find the worm with the highest kills
					int kills = 0;
					w = cWorms;
					CWorm *winner = cWorms;
					for(i = 0; i < MAX_WORMS; i++, w++) {
						if(!w->isUsed())
							continue;
						if(w->getKills()>kills) {
							kills = w->getKills();
							winner = w;
							wormid = i;
						}
					}
					for(i = 0, w = cWorms; i < MAX_WORMS; i++, w++) {
						if(!w->isUsed())
							continue;
						if(w->getKills() >= kills) {
							// Send the text
							if (networkTexts->sPlayerHasWon != "<none>")  {
								SendGlobalText((replacemax(networkTexts->sPlayerHasWon,"<player>",w->getName(),1)),
												TXT_NORMAL);
							}
						}
					}
					cout << "recheck: wormcount < 3" << endl;
					EndGame = true;
				}
				// If the max points has been reached
				else {
					w = cWorms;
					for(i = 0, w = cWorms; i < MAX_WORMS; i++, w++) {
						if(!w->isUsed())
							continue;
						if(w->getKills() == tLXOptions->tGameInfo.iKillLimit) {
							// Send the text
							if (networkTexts->sPlayerHasWon != "<none>")  {
								SendGlobalText((replacemax(networkTexts->sPlayerHasWon,"<player>",w->getName(),1)),
												TXT_NORMAL);
							}
							cout << "recheck: max kills for worm " << i << endl;
							EndGame = true;
						}
					}
				}
			}
			break; // CTF

			//
			// TEAM CTF: Declare the team with the most points as winner
			//
			case  GMT_TEAMCTF:  {
				const std::string TeamNames[] = {"blue", "red"};
				int TeamCount[2];
				int TeamScore[2];

				w = cWorms;

				for(i=0;i<2;i++) {
					TeamCount[i]=0;
					TeamScore[i]=0;
				}

				// Check if anyone else is left on the team
				for(i=0;i<MAX_WORMS;i++,w++) {
					if(w->isUsed() && w->getLives() != WRM_OUT) {
							TeamCount[w->getTeam()]++;
							TeamScore[w->getTeam()]+=w->getKills();
					}
				}

				short teamsleft = 0;
				short team = 0;

				// Get the number of teams left
				for(i=0;i<2;i++)
					if(TeamCount[i])
						teamsleft++;
				team = TeamScore[1]>TeamScore[0] ? 1 : 0;

				// Send the text
				if (teamsleft <= 1)  {
					if (networkTexts->sTeamHasWon != "<none>")  {
						SendGlobalText((replacemax(networkTexts->sTeamHasWon,"<team>",TeamNames[team],1)),
										TXT_NORMAL);
					}
					cout << "recheck: no more teams left" << endl;
					EndGame = true;
				}
			}
			break; // TEAMDEATH

			}

			if( tLXOptions->tGameInfo.fTimeLimit > 0 && fServertime > tLXOptions->tGameInfo.fTimeLimit*60.0 ) {
				if (networkTexts->sTimeLimit != "<none>")
					SendGlobalText( networkTexts->sTimeLimit, TXT_NORMAL );
				cout << "recheck: time limit reached" << endl;
				EndGame = true;
			}
			// End the game
			if (EndGame)  {
				GameOver(wormid);
			}
		}
	}


}


///////////////////
// Checks if all the clients are ready to play
void GameServer::CheckReadyClient(void)
{
    short c;

    if(iState != SVS_GAME)
        return;

	bool allready = true;
	CServerConnection *client = cClients;
	for(c=0; c<MAX_CLIENTS; c++, client++) {
		if(client->getStatus() == NET_DISCONNECTED || client->getStatus() == NET_ZOMBIE || client->getNumWorms() == 0)
			continue;

		if(!client->getGameReady())
			allready = false;
	}

	// All ready to go?
	if(allready)
		BeginMatch();
}


///////////////////
// Apply the stuff required for gametypes like CTF
void GameServer::SimulateGameSpecial()
{
	switch (tLXOptions->tGameInfo.iGameMode) {
		// Capture the flag
		case GMT_CTF:
		{
			CWorm *w = cWorms;
			std::list<CWorm*> flagworms;
			std::list<CWorm*>::iterator flagworm;

			// Get the flags
			int i = 0;
			for(; i < MAX_WORMS; i++, w++) {
				if(!w->isUsed())
					continue;
				if(!w->getFlag())
					continue;
				flagworms.push_back(w);
			}

			// Simulate the flags
			// HINT: The flag is simulated with a high delta time but it doesn't really matter
			for(flagworm = flagworms.begin(); flagworm != flagworms.end(); flagworm++) {
				// TODO: please, do not use simulateWorm for this; use a simulateFlag and don't use CWorm at all!!
				PhysicsEngine::Get()->simulateWorm(
					cClient->getRemoteWorms() + (*flagworm)->getID(),
					cClient->getRemoteWorms(), false);
				(*flagworm)->setPos((cClient->getRemoteWorms() + (*flagworm)->getID())->getPos());
			}

			// Attach the flag if a worm is within 10 pixels of it
			for(flagworm = flagworms.begin(), i = 0; flagworm != flagworms.end(); flagworm++, i++)
				for(i = 0, w = cWorms; i < MAX_WORMS; i++, w++) {
					if(!w->isUsed())
						continue;
					if(CalculateDistance(w->getPos(),(*flagworm)->getPos()) < 10 && cServer->getFlagHolder(i) == -1)
						cServer->setFlagHolder(w->getID(),i);
				}

			// Move the flag to where its holder is
			for(flagworm = flagworms.begin(), i = 0; flagworm != flagworms.end(); flagworm++, i++)
				if(getFlagHolder(i) != -1)
					(*flagworm)->setPos((cClient->getRemoteWorms() + getFlagHolder(i))->getPos());

			// If the flag has been held for 5 seconds and the map doesn't have a base give the worm a point
			if(tLX->fCurTime - fLastCTFScore > 5 && cMap->getBaseStart().x == -1) {
				cWorms[getFlagHolder(0)].AddKill();
				CBytestream bs;
				bs.Clear();
				cWorms[getFlagHolder(0)].writeScore(&bs);
				SendGlobalPacket(&bs);
				fLastCTFScore = tLX->fCurTime;
			}

			// Check if anyone has scored and respawn the flag they scored with if they have
			for(flagworm = flagworms.begin(), i = 0; flagworm != flagworms.end(); flagworm++, i++) {
				CWorm *w = &cWorms[getFlagHolder(i)];
				if(w->getPos().x > cMap->getBaseStart().x && w->getPos().y > cMap->getBaseStart().y &&
					w->getPos().x < cMap->getBaseEnd().x && w->getPos().y < cMap->getBaseEnd().y) {
						w->AddKill();
						CBytestream bs;
						bs.Clear();
						w->writeScore(&bs);
						SendGlobalPacket(&bs);
						SpawnWorm(*flagworm);
						(cClient->getRemoteWorms()+(*flagworm)->getID())->setPos((*flagworm)->getPos());
						setFlagHolder(-1, i);
						SendGlobalText((replacemax(networkTexts->sHasScored,"<player>",w->getName(),1)),
							TXT_NORMAL);
						if(w->getKills()==tLXOptions->tGameInfo.iKillLimit)
							RecheckGame();
				}
			}
			break;
		}
	}
	return;
}
