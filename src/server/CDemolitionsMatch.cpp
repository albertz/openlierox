/*
 *  CDemolitionsMatch.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 02.03.09.
 *  code under LGPL
 *
 */

#include "CGameMode.h"
#include "CServer.h"
#include "CWorm.h"
#include "game/Game.h"

class CDemolitionsMatch : public CGameMode {

public:
	
	virtual std::string Name() {
		return "Demolitions";
	}
	
	virtual void Carve(CWorm*, int) {
		cServer->RecheckGame();
	}
	
	virtual bool CheckGameOver() {
		if(CGameMode::CheckGameOver()) return true;
		
		// If the map has less then a 1/5th of the dirt it once had, the game is over
		// And the worm with the highest dirt count wins
		
		// Add up all the worm's dirt counts
		int nDirtCount = 0;
		
		CWorm *w = cServer->getWorms();
		for( short i=0; i<MAX_WORMS; i++,w++ ) {
			if( !w->isUsed() )
				continue;
			
			nDirtCount += w->getDirtCount();
		}
		
		if( nDirtCount > (float)game.gameMap()->GetDirtCount()*0.8f ) {
			// TODO: make configureable
			cServer->SendGlobalText("Demolition limit hit", TXT_NORMAL);
			// Make the server trigger a game over
			return true;
		}
		
		return false;
	}
	
	virtual int CompareWormsScore(CWorm* w1, CWorm* w2) {
		int d = w1->getDirtCount() - w2->getDirtCount();
		if(d != 0) return d;
		return CGameMode::CompareWormsScore(w1, w2);
	}

};

static CDemolitionsMatch gameMode;
CGameMode* gameMode_Demolitions = &gameMode;
