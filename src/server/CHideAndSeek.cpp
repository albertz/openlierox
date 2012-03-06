/*
 OpenLieroX
 
 hide and seek gamemode
 
 created 2009-02-09
 code under LGPL
 */

#include <iostream>
#include "CHideAndSeek.h"
#include "game/CWorm.h"
#include "Options.h"
#include "Consts.h"
#include "CServer.h"
#include "CClient.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "game/CMap.h"
#include "LieroX.h"
#include "CGameMode.h"
#include "Consts.h"
#include "olx-types.h"
#include "Cache.h"
#include "game/Game.h"
#include "ProfileSystem.h"

class CHideAndSeek : public CGameMode {
public:
	virtual float TimeLimit() { return fGameLength; }
	virtual void PrepareGame();
	virtual void PrepareWorm(CWorm* worm);
	virtual bool Spawn(CWorm* worm, CVec pos);
	virtual void Kill(CWorm* victim, CWorm* killer);
	virtual bool Shoot(CWorm* worm);
	virtual void Simulate();
	virtual bool CheckGameOver();
	virtual int  GeneralGameType();
	virtual int  GameTeams();
	virtual std::string TeamName(int t);
	virtual int  Winner();
	virtual bool NeedUpdate(CServerConnection* cl, CWorm* worm);
	virtual std::string Name() { return "Hide and Seek"; }
	virtual GameInfoGroup getGameInfoGroupInOptions() { return GIG_HideAndSeek; }
	
	// Show or hide a worm to/from the opposing team
	void Show(CWorm* worm, bool message = true);
	void Hide(CWorm* worm, bool message = true);
	// Returns true if worm1 can see worm2
	bool CanSee(CWorm* worm1, CWorm* worm2);
	// Generates an approximation of the time needed to finish the game
	void GenerateTimes();
		
protected:
	float fGameLength;           // The length of the game
	TimeDiff fWarmupTime[MAX_WORMS]; // The time for which worm should be invisible and untouchable
	TimeDiff fLastAlert[MAX_WORMS]; // The last time the worms were seen by other worms
	bool visible[MAX_WORMS];   // The visibility of the woms
	
	void makeVisible(CWorm* worm, bool vis);
};


std::string CHideAndSeek::TeamName(int t) {
	static const std::string teamname[2] = { "hiding", "seeking" };
	if(t >= 0 && t < 2) return teamname[t];
	return itoa(t);
}


void CHideAndSeek::makeVisible(CWorm* worm, bool vis) {
	visible[worm->getID()] = vis;
	
	for(int ii = 0; ii < MAX_CLIENTS; ii++) {
		CServerConnection* cl = &cServer->getClients()[ii];
		if(!cl->isConnected()) continue;
		
		// make worm (in)visible to all non-teammates
		for_each_iterator(CWorm*, w, game.worms()) {
			if(w->get()->getTeam() != worm->getTeam())
				cl->getNetEngine()->SendHideWorm(worm, w->get()->getID(), vis, false);
		}
	}	
}

void CHideAndSeek::PrepareGame()
{
	GenerateTimes();
	for(int i = 0; i < MAX_WORMS; i++) {
		fLastAlert[i] = 0;
		visible[i] = false;
	}
}

void CHideAndSeek::PrepareWorm(CWorm* worm)
{
	// in case this client connected later, update the visibility for all worms
	for_each_iterator(CWorm*, otherw, game.worms()) {
		if(!visible[otherw->get()->getID()] && otherw->get()->getTeam() != worm->getTeam())
			worm->getClient()->getNetEngine()->SendHideWorm(otherw->get(), worm->getID(), visible[otherw->get()->getID()], true);
	}
	
	std::string teamhint[2];
	if (networkTexts->sHiderMessage != "<none>")
		replace(networkTexts->sHiderMessage, "<time>", itoa((int)fGameLength), teamhint[0]);
	if (networkTexts->sSeekerMessage != "<none>")
		replace(networkTexts->sSeekerMessage, "<time>", itoa((int)fGameLength), teamhint[1]);

	// Gameplay hints
	if(worm->getType() != PRF_COMPUTER)
		worm->getClient()->getNetEngine()->SendText(teamhint[CLAMP(worm->getTeam(),0,1)], TXT_NORMAL);
}

bool CHideAndSeek::Spawn(CWorm* worm, CVec pos)
{
	pos = game.gameMap()->FindSpot();
	worm->Spawn(pos);
	visible[worm->getID()] = true; // So that Hide() doesn't ignore our request.
	Hide(worm, false);
	fWarmupTime[worm->getID()] = game.serverTime() + TimeDiff((float)gameSettings[FT_HS_HideTime]);

	// Note: Earlier, we spawned only for the same team. This was a hacky workaround to avoid some sparkles to show up.
	// We have solved this in a clean way now. Of course, we want spawning here. Where the worm is invisible,
	// a spawn also will be invisible.
	return true;
}

void CHideAndSeek::Kill(CWorm* victim, CWorm* killer)
{
	if(killer && killer->getTeam() == HIDEANDSEEK_SEEKER && killer != victim) {
		if (networkTexts->sCaughtMessage != "<none>")  {
			std::string msg;
			replace(networkTexts->sCaughtMessage, "<seeker>", killer->getName(), msg);
			replace(msg, "<hider>", victim->getName(), msg);
			cServer->SendGlobalText(msg, TXT_NORMAL);
		}
		killer->addKill();
	}
	victim->Kill(true);
	makeVisible(victim, false);
}

bool CHideAndSeek::Shoot(CWorm* worm)
{
	return false;
}

void CHideAndSeek::Simulate()
{
	TimeDiff GameTime = game.serverTime();
	// Game time up
	if(GameTime > fGameLength) {
		for_each_iterator(CWorm*, w, game.worms())
			if(w->get()->getLives() != WRM_OUT) {
				if(w->get()->getTeam() == HIDEANDSEEK_SEEKER)  {
					if (w->get()->getLives() == WRM_UNLIM)
						w->get()->setLives(0);  // Kill also worms with unlimited lives
					cServer->killWorm(w->get()->getID(), w->get()->getID(), w->get()->getLives() + 1);
				} else
					Show(w->get(), false); // People often want to see where the hiders were
			}
		return;
	}
	// Hiders have some time free from being caught and seen
	if(GameTime < (float)gameSettings[FT_HS_HideTime])
		return;
	// Check if any of the worms can see eachother
	for_each_iterator(CWorm*, w1, game.aliveWorms())
	{
		if( w1->get()->getLives() == WRM_OUT )
			continue;
		// Hide the worm if the alert time is up
		if(fLastAlert[w1->get()->getID()] + TimeDiff((float)gameSettings[FT_HS_AlertTime]) < GameTime)
			Hide(w1->get());
		for_each_iterator(CWorm*, w2, game.aliveWorms())
		{
			if( w2->get()->getLives() == WRM_OUT )
				continue;
			if( w1->get()->getTeam() == w2->get()->getTeam() )
				continue;
			if( fWarmupTime[w2->get()->getID()] > GameTime )
				continue;

			if(CanSee(w1->get(), w2->get()))
				Show(w2->get());
			// Catch the hiders if they are within 10 pixels
			if(w1->get()->getTeam() == HIDEANDSEEK_SEEKER && w2->get()->getTeam() == HIDEANDSEEK_HIDER)
				if((w1->get()->getPos() - w2->get()->getPos()).GetLength() < 10)
				{
					int type;
					float length;
					w1->get()->traceLine(w2->get()->getPos(), &length, &type, 1);
					if( type & PX_EMPTY )	// Do not touch through thin wall
						cServer->killWorm(w2->get()->getID(), w1->get()->getID(), 0);
				}
		}
	}
}

bool CHideAndSeek::CheckGameOver()
{
	// We ignore the standard GameOver-check, we just do this check.
	// In Simulate, we kill the seeker worms when the timelimit is hit.
	
	int worms[2] = { 0, 0 };
	int winners = -1;

	// TODO: move out here
	for_each_iterator(CWorm*, w, game.worms())
		if(w->get()->getLives() != WRM_OUT && w->get()->getTeam() < 2)
			worms[w->get()->getTeam()]++;
	if(worms[0] == 0)
		winners = HIDEANDSEEK_SEEKER;
	else if(worms[1] == 0)
		winners = HIDEANDSEEK_HIDER;
	if(winners != -1) {
		return true;
	}
	
	return false;
}

int CHideAndSeek::GeneralGameType()
{
	return GMT_TEAMS;
}

int CHideAndSeek::GameTeams()
{
	return 2;
}

int CHideAndSeek::Winner()
{
	// there is no single winner in hideandseek, only the team
	return -1;
}

bool CHideAndSeek::NeedUpdate(CServerConnection* cl, CWorm* worm)
{
	// Clients don't recieve dirt updates without getting a full update
	// No worms, but we don't want the client to see nothing
	if(game.wormsOfClient(cl)->size() == 0)
		return true;

	// Different teams, and invisible so no need I think
	if(game.wormsOfClient(cl)->get()->getTeam() != worm->getTeam() && !visible[worm->getID()] && !worm->tState.get().bCarve)
		return false;

	return true;
}

void CHideAndSeek::Show(CWorm* worm, bool message)
{
	fLastAlert[worm->getID()] = game.serverTime();
	if(visible[worm->getID()]) return;
	makeVisible(worm, true);

	if(worm->getType() != PRF_COMPUTER && worm->getTeam() == HIDEANDSEEK_HIDER && message)  {
		if (networkTexts->sHiderVisible != "<none>")
			worm->getClient()->getNetEngine()->SendText(networkTexts->sHiderVisible, TXT_NORMAL);
	}
	// Seekers will know they are seen with this here. This reduces the effectiveness of 'wall-vision'
	/*else {
		if (networkTexts->sSeekerVisible != "<none>")
			worm->getClient()->getNetEngine()->SendText(networkTexts->sSeekerVisible, TXT_NORMAL);
	}*/

	for_each_iterator(CWorm*, w, game.worms()) {
		if(w->get()->getTeam() == worm->getTeam()) continue;
		if(w->get()->getType() == PRF_COMPUTER) continue;
		if (networkTexts->sVisibleMessage != "<none>" && message)  {
			std::string msg;
			replace(networkTexts->sVisibleMessage, "<player>", worm->getName(), msg);
			if(w->get()->getClient() && w->get()->getClient()->getNetEngine())
				w->get()->getClient()->getNetEngine()->SendText(msg, TXT_NORMAL);
		}
	}
}

void CHideAndSeek::Hide(CWorm* worm, bool message)
{
	if(!visible[worm->getID()]) return;
	makeVisible(worm, false);

	// Removed message for seekers because it is confusing since they don't get the "you are visible" one
	if(worm->getType() != PRF_COMPUTER && networkTexts->sYouAreHidden != "<none>" && message && worm->getTeam() == HIDEANDSEEK_HIDER)
		worm->getClient()->getNetEngine()->SendText(networkTexts->sYouAreHidden, TXT_NORMAL);

	for_each_iterator(CWorm*, w, game.worms()) {
		if(w->get()->getTeam() == worm->getTeam()) continue;
		if(w->get()->getType() == PRF_COMPUTER) continue;
		if(networkTexts->sHiddenMessage != "<none>" && message) {
			std::string msg;
			replace(networkTexts->sHiddenMessage, "<player>", worm->getName(), msg);
			if(w->get()->getClient() && w->get()->getClient()->getNetEngine())
				w->get()->getClient()->getNetEngine()->SendText(msg, TXT_NORMAL);
		}
	}	
}

bool CHideAndSeek::CanSee(CWorm* worm1, CWorm* worm2)
{
	CVec dist;
	dist = worm1->getPos() - worm2->getPos();
	if(worm1->getTeam() == HIDEANDSEEK_SEEKER)
	{
		if( dist.GetLength() < (int)gameSettings[FT_HS_SeekerVisionRangeThroughWalls] )
			return true;
		if( dist.GetLength() < (int)gameSettings[FT_HS_SeekerVisionRange] )
		{
			int type;
			float length;
			worm1->traceLine(worm2->getPos(), &length, &type, 1);
			if( !( type & PX_EMPTY ) )
				return false;
				
			float angle = worm1->getAngle();
			if(worm1->getFaceDirectionSide() == DIR_LEFT)
				angle = 180.0f - angle;
				
			while( angle > 180.0f )	// Normalize it between 180 and -180
				angle -= 360.0f;
			
			float wormAngle = VectorAngle( worm2->getPos(), worm1->getPos() ) * 180.0f / (float)PI;
			
			float angleDiff = angle - wormAngle;
			
			while( angleDiff > 180.0f ) // Normalize it
				angleDiff -= 360.0f;
			while( angleDiff < -180.0f )
				angleDiff += 360.0f;
			
			return (int)fabs(angleDiff*2.0f) < (int)gameSettings[FT_HS_SeekerVisionAngle];
		}
		return false;
	}
	else 
	{
		if( dist.GetLength() < (int)gameSettings[FT_HS_HiderVisionRangeThroughWalls] )
			return true;
		if( dist.GetLength() < (int)gameSettings[FT_HS_HiderVisionRange] )
		{
			int type;
			float length;
			worm1->traceLine(worm2->getPos(), &length, &type, 1);
			if(type & PX_EMPTY)
				return true;
		}
		return false;
	}
}

void CHideAndSeek::GenerateTimes()
{
	if((float)gameSettings[FT_TimeLimit] > 0) {
		fGameLength = (float)gameSettings[FT_TimeLimit] * 60.0f;
		return;
	}

	// E.g. in writelobbyupdate by server, we don't have the map loaded yet.
	// Thus try a preloaded version.
	CMap* cMap = game.gameMap();
	if(!cMap) {
		warnings << "CHideAndSeek::GenerateTimes(): cannot get map" << endl;
		fGameLength = 160;
		return;
	}
	
	int volume = cMap->GetWidth() * cMap->GetHeight();
	enum { SMALL = 1, MEDIUM, LARGE, XLARGE } size;
	// Decide on the level size
	if(volume < 500 * 400)
		size = SMALL;
	else if(volume < 700 * 600)
		size = MEDIUM;
	else if(volume < 900 * 800)
		size = LARGE;
	else
		size = XLARGE;
	// Calculate the ratio of hiders to seekers
	float ratio = 1.0f;
	// TODO: move that out here (to GameServer)
	int worms[2] = { 0, 0 };
	for_each_iterator(CWorm*, w, game.worms())
		if(w->get()->getTeam() < 2)
			worms[w->get()->getTeam()]++;
	if(worms[0] != 0 && worms[1] != 0)
		ratio = (float)worms[0] / worms[1];

	// TODO: Is this actually any good? 
	fGameLength = (45 * size * ratio);
}

static CHideAndSeek gameMode;
CGameMode* gameMode_HideAndSeek = &gameMode;
