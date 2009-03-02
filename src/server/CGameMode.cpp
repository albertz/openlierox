/*
 *  CGameMode.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 02.03.09.
 *  code under LGPL
 *
 */

#include "CGameMode.h"
#include "CServer.h"
#include "Options.h"
#include "CWorm.h"
#include "Protocol.h"

bool CGameMode::CheckGameOver() {
	// In game?
	if (!cServer || cServer->getState() == SVS_LOBBY)
		return true;

	// Check if the timelimit has been reached
	if(tLXOptions->tGameInfo.fTimeLimit > 0) {
		if (cServer->getServerTime() > tLXOptions->tGameInfo.fTimeLimit*60.0) {
			if(networkTexts->sTimeLimit != "<none>")
				cServer->SendGlobalText(networkTexts->sTimeLimit, TXT_NORMAL);
			notes << "time limit (" << (tLXOptions->tGameInfo.fTimeLimit*60.0) << ") reached with current time " << cServer->getServerTime();
			notes << " -> game over" << endl;
			return true;
		}
	}
	
	if(!tLXOptions->tGameInfo.features[FT_AllowEmptyGames] || tLX->iGameType == GME_LOCAL) {
		// TODO: move that to GameServer
		int worms = 0;
		for(int i = 0; i < MAX_WORMS; i++)
			if(cWorms[i].isUsed()) {
				if (cWorms[i].getLives() != WRM_OUT)
					worms++;
			}

		if(worms <= (tLX->iGameType == GME_LOCAL ? 0 : 1)) {
			notes << "only " << worms << " players left in game -> game over" << endl;
			return true;
		}
	}
	
	return false;
}

int CGameMode::Winner() {
	return HighestScoredWorm();
}

int CGameMode::HighestScoredWorm() {
	int wormid = -1;
	for(int i = 0; i < MAX_WORMS; i++)
		if(cWorms[i].isUsed()) {
			if(wormid < 0) wormid = i;
			else
				wormid = CompareWormsScore(&cWorms[i], &cWorms[wormid]) > 0 ? i : wormid; // Take the worm with the best score
		}
	
	return wormid;
}

int CGameMode::CompareWormsScore(CWorm* w1, CWorm* w2) {
	// Lives first
	if(tLXOptions->tGameInfo.iLives >= 0) {
		if (w1->getLives() > w2->getLives()) return 1;
		if (w1->getLives() < w2->getLives()) return -1;		
	}
	
	// Kills
	if (w1->getKills() > w2->getKills()) return 1;
	if (w1->getKills() < w2->getKills()) return -1;
	
	// Damage
	return w1->getDamage() - w2->getDamage();
}

