/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Worm class - AI
// Created 13/3/03
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"



/*
===============================

    Artificial Intelligence

===============================
*/


///////////////////
// Initialize the AI
bool CWorm::AI_Initialize(CMap *pcMap)
{
    assert(pcMap);

    // Because this can be called multiple times, shutdown any previous allocated data
    AI_Shutdown();

    // Allocate the Open/Close grid
    nGridCols = pcMap->getGridCols();
    nGridRows = pcMap->getGridRows();

    pnOpenCloseGrid = new int[nGridCols*nGridRows];
    if(!pnOpenCloseGrid)
        return false;

    psPath = NULL;
    psCurrentNode = NULL;
    fLastCarve = -9999;
    cStuckPos = CVec(-999,-999);
    fStuckTime = -9999;
    fLastPathUpdate = -9999;
	fLastJump = -9999;
	fLastCarve = -9999;
    bStuck = false;
	bPathFinished = true;
	//iAiGameType = GAM_OTHER;

    return true;
}


///////////////////
// Shutdown the AI stuff
void CWorm::AI_Shutdown(void)
{
    AI_CleanupPath(psPath);
	NEW_AI_CleanupPath();
	NEW_psPath = NULL;
	NEW_psCurrentNode = NULL;
	NEW_psLastNode = NULL;
    psPath = NULL;
    psCurrentNode = NULL;
    if(pnOpenCloseGrid)
        delete[] pnOpenCloseGrid;
}




/*
  Algorithm:
  ----------

  1) Find nearest worm and set it as a target
  2) If we are within a good distance, aim and shoot the target
  3) If we are too far, try and get closer by digging and ninja rope

*/


///////////////////
// Simulate the AI
void CWorm::AI_GetInput(int gametype, int teamgame, int taggame, CMap *pcMap)
{
	// Behave like humans and don't play immediatelly after spawn
	if ((tLX->fCurTime-fSpawnTime) < 0.4)
		return;

	worm_state_t *ws = &tState;
	gs_worm_t *wd = cGameScript->getWorm();

	// Init the ws
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;

	iAiGame = gametype;
	iAiTeams = teamgame;
	iAiTag = taggame;

    strcpy(tLX->debug_string, "");

	iAiDiffLevel = tProfile->nDifficulty;
	float dt = tLX->fDeltaTime;

    // Every 3 seconds we run the think function
    if(tLX->fCurTime - fLastThink > 3 && nAIState != AI_THINK)
        nAIState = AI_THINK;


    // If we have a good shooting 'solution', shoot
    if(AI_CanShoot(pcMap, gametype)) {

        // Shoot
        AI_Shoot(pcMap);
        return;
    }

	// Reload weapons when we can't shoot
	AI_ReloadWeapons();

    // Process depending on our current state
    switch(nAIState) {

        // Think; We spawn in this state
        case AI_THINK:
            AI_Think(gametype, teamgame, taggame, pcMap);
            break;

        // Moving towards a target
        case AI_MOVINGTOTARGET:
            NEW_AI_MoveToTarget(pcMap);
            break;
    }


    //
    // Find a target worm
    //
    /*CWorm *trg = findTarget(gametype, teamgame, taggame, pcMap);

	// If we have no target, do something
    if(!trg)
        // TODO Something
        return;


    //
    // Shoot the target
    //

    // If we can shoot the target, shoot it
    if(AI_CanShoot(trg, pcMap)) {

        // Shoot the target

    } else {

        // If we cannot shoot the target, walk towards the target
        AI_MoveToTarget(trg, pcMap);
    }


	//
	// Shoot the target
	//


	CVec	tgPos = trg->getPos();
	CVec	tgDir = tgPos - vPos;    

	int		iAngleToBig = false;

	float   fDistance = NormalizeVector(&tgDir);
	
	// Make me face the target
	if(tgPos.GetX() > vPos.GetX())
		iDirection = DIR_RIGHT;
	else
		iDirection = DIR_LEFT;


	// Aim at the target
	float ang = (float)atan2(tgDir.GetX(), tgDir.GetY());
	ang = RAD2DEG(ang);

	if(iDirection == DIR_LEFT)
		ang+=90;
	else
		ang = -ang + 90;

	// Clamp the angle
	ang = MAX(-90, ang);

	if(ang > 60) {
		ang = 60;
		iAngleToBig = true;
	}


	// AI Difficulty level effects accuracy
	float Levels[] = {12, 6, 3, 0};

	// If we are going to shoot, we need to lower the accuracy a little bit
	ang += GetRandomNum() * Levels[DiffLevel];


	// Move the angle at the same speed humans are allowed to move the angle
	if(ang > fAngle)
		fAngle += wd->AngleSpeed * dt;
	else if(ang < fAngle)
		fAngle -= wd->AngleSpeed * dt;

	// If the angle is within +/- 3 degrees, just snap it
	if( fabs(fAngle - ang) < 3)
		fAngle = ang;

	// Clamp the angle
	fAngle = MIN(60,fAngle);
	fAngle = MAX(-90,fAngle);


	// ???: If the angle is to big, (ie, the target is below us and we can't aim directly at him) don't shoot


	// If we are close enough, shoot the target
	if(fDistance < 200) {

        // Find the best weapon for the job
        int wpn = getBestWeapon( gametype, fDistance, tgPos, pcMap );
        if( wpn >= 0 ) {
            iCurrentWeapon = wpn;
            ws->iShoot = true;
        }
    } else {

        // If we're outside shooting range, cycle through the weapons so they can be reloaded
        iCurrentWeapon = cycleWeapons();
    }


	// If we are real close, jump to get a better angle
	// Only do this if we are better then easy level
	if(fDistance < 10 && DiffLevel > AI_EASY)
		ws->iJump = true;



	// We need to move closer to the target

	// We carve every few milliseconds so we don't go too fast
	if(tLX->fCurTime - fLastCarve > 0.35f) {
		fLastCarve = tLX->fCurTime;
		ws->iCarve = true;
	}

	ws->iMove = true;



	// If the target is above us, we point up & use the ninja rope
	if(fAngle < -60 && (vPos - tgPos).GetY() > 50) {
		
		fAngle = -90;
        CVec dir;
        dir.SetX( (float)cos(fAngle * (PI/180)) );
	    dir.SetY( (float)sin(fAngle * (PI/180)) );
	    if(iDirection==DIR_LEFT)
		    dir.SetX(-dir.GetX());

        if( !cNinjaRope.isReleased() )
            cNinjaRope.Shoot(vPos,dir);
	}


    // If the hook of the ninja rope is below us, release it
    if( cNinjaRope.isReleased() && cNinjaRope.isAttached() && cNinjaRope.getHookPos().GetY() > vPos.GetY() ) {
        cNinjaRope.Release();
    }

    // If the target is not too far above us, we should release the ninja rope
    if( (vPos - tgPos).GetY() < 30 && cNinjaRope.isAttached() && cNinjaRope.isReleased() ) {
        cNinjaRope.Release();
    }*/
}


///////////////////
// Find a target worm
CWorm *CWorm::findTarget(int gametype, int teamgame, int taggame, CMap *pcMap)
{
	CWorm	*w = cClient->getRemoteWorms();
	CWorm	*trg = NULL;
	CWorm	*nonsight_trg = NULL;
	float	fDistance = 99999;
	float	fSightDistance = 99999;

	int NumTeams = 0;
	int i;
	for (i=0; i<4; i++)
		if (cClient->getTeamScore(i) > -1)
			NumTeams++;


    //
	// Just find the closest worm
	//

	for(i=0; i<MAX_WORMS; i++, w++) {

		// Don't bother about unused or dead worms
		if(!w->isUsed() || !w->getAlive())
			continue;

		// Make sure i don't target myself
		if(w->getID() == iID)
			continue;

		// If this is a team game, don't target my team mates
		// BUT, if there is only one team, play it like deathmatch
		if(teamgame && w->getTeam() == iTeam && NumTeams > 1)
			continue;

		// If this is a game of tag, only target the worm it (unless it's me)
		if(taggame && !w->getTagIT() && !iTagIT)
			continue;

		// Calculate distance between us two
		float l = CalculateDistance(w->getPos(), vPos);

		// Prefer targets we have free line of sight to
		float length;
		int type;
		traceLine(w->getPos(),pcMap,&length,&type,1);
		if (type != PX_ROCK)  {
			// Line of sight not blocked
			if (l < fSightDistance)  {
				trg = w;
				fSightDistance = l;
				if (l < fDistance)  {
					nonsight_trg = w;
					fDistance = l;
				}
			}
		}
		else
			// Line of sight blocked
			if(l < fDistance) {
				nonsight_trg = w;
				fDistance = l;
			}
	}

	// If the target we have line of sight to is too far, switch back to the closest target
	if ((fSightDistance-fDistance > 50.0f) || trg == NULL)  {
		if (nonsight_trg)
			trg = nonsight_trg;
	}

    return trg;
}


///////////////////
// Think State
void CWorm::AI_Think(int gametype, int teamgame, int taggame, CMap *pcMap)
{
    /*
      We start of in an think state. When we're here we decide what we should do.
      If there is an unfriendly worm, or a game target we deal with it.
      In the event that we have no unfriendly worms or game targets we should remain in the idle state.
      While idling we can walk around to appear non-static, reload weapons or grab a bonus.
    */

    // Clear the state
    psAITarget = NULL;
    psBonusTarget = NULL;
    nAITargetType = AIT_NONE;
    fLastThink = tLX->fCurTime;


    // Reload our weapons in idle mode
    AI_ReloadWeapons();


    // If our health is less than 15% (critical), our main priority is health
    if(iHealth < 15)
        if(AI_FindHealth(pcMap))
            return;

    // Search for an unfriendly worm
    psAITarget = findTarget(gametype, teamgame, taggame, pcMap);

    
    // Any unfriendlies?
    if(psAITarget) {
        // We have an unfriendly target, so change to a 'move-to-target' state
        nAITargetType = AIT_WORM;
        nAIState = AI_MOVINGTOTARGET;
        //AI_InitMoveToTarget(pcMap);
		NEW_AI_CreatePath(pcMap);
        return;
    }

	if(!psAITarget)
		fLastShoot = 0;


    // If we're down on health (less than 80%) we should look for a health bonus
    if(iHealth < 80) {
        if(AI_FindHealth(pcMap))
            return;
    }


    /*
      If we get here that means we have nothing to do
    */


    //
    // Typically, high ground is safer. So if we don't have anything to do, lets up up
    //
    
    // Our target already on high ground?
    if(cPosTarget.GetY() < pcMap->getGridHeight()*5 && nAIState == AI_MOVINGTOTARGET)  {

		// Nothing todo, so go find some health if we even slightly need it
		if(iHealth < 100) {
			if(AI_FindHealth(pcMap))
				return;
		}
        return;
	}

    // Find a random spot to go to high in the level
    int x, y;
    for(y=1; y<5 && y<pcMap->getGridRows()-1; y++) {
        for(x=1; x<pcMap->getGridCols()-1; x++) {
            uchar pf = *(pcMap->getGridFlags() + y*pcMap->getGridCols() + x);

            if(pf & PX_ROCK)
                continue;

            // Set the target
            cPosTarget = CVec((float)(x*pcMap->getGridWidth()+(pcMap->getGridWidth()/2)), (float)(y*pcMap->getGridHeight()+(pcMap->getGridHeight()/2)));
            nAITargetType = AIT_POSITION;
            nAIState = AI_MOVINGTOTARGET;
            //AI_InitMoveToTarget(pcMap);
			NEW_AI_CreatePath(pcMap);
            break;
        }
    }

    /*int     x, y;
    int     px, py;
    bool    first = true;
    int     cols = pcMap->getGridCols()-1;       // Note: -1 because the grid is slightly larger than the
    int     rows = pcMap->getGridRows()-1;       // level size
    int     gw = pcMap->getGridWidth();
    int     gh = pcMap->getGridHeight();
	CVec	resultPos;

    
    // Find a random cell to start in
    px = (int)(fabs(GetRandomNum()) * (float)cols);
	py = (int)(fabs(GetRandomNum()) * (float)rows);

    x = px; y = py;
	bool breakme = false;

    // Start from the cell and go through until we get to an empty cell
    while(1) {
        while(1) {
            // If we're on the original starting cell, and it's not the first move we have checked all cells
            // and should leave
            if(!first) {
                if(px == x && py == y) {
                    resultPos = CVec((float)x*gw+gw/2, (float)y*gh+gh/2);
					breakme = true;
					break;
                }
            }
            first = false;

            uchar pf = *(pcMap->getGridFlags() + y*pcMap->getGridCols() + x);
            if(pf != PX_ROCK)  {
                resultPos = CVec((float)x*gw+gw/2, (float)y*gh+gh/2);
				breakme = true;
				break;
			}
            
            if(++x >= cols) {
                x=0;
                break;
            }
        }

		if (breakme)
			break;

        if(++y >= rows) {
            y=0;
            x=0;
        }
    }


   //cPosTarget = resultPos;
   nAITargetType = AIT_POSITION;
   nAIState = AI_MOVINGTOTARGET;
   AI_InitMoveToTarget(pcMap);*/


}


///////////////////
// Find a health pack
// Returns true if we found one
bool CWorm::AI_FindHealth(CMap *pcMap)
{
	if (!tGameInfo.iBonusesOn)
		return false;

    CBonus  *pcBonusList = cClient->getBonusList();
    int     i;
    bool    bFound = false;
    CBonus  *pcBonus = NULL;
    float   dist = 99999;

    // Find the closest health bonus
    for(i=0; i<MAX_BONUSES; i++) {
        if(pcBonusList[i].getUsed() && pcBonusList[i].getType() == BNS_HEALTH) {
            
            float d = CalculateDistance(pcBonusList[i].getPosition(), vPos);

            if(d < dist) {
                pcBonus = &pcBonusList[i];
                dist = d;
                bFound = true;
            }
        }
    }


    // TODO: Verify that the target is not in some small cavern that is hard to get to

    
    // If we have found a bonus, setup the state to move towards it
    if(bFound) {
        psBonusTarget = pcBonus;
        nAITargetType = AIT_BONUS;
        nAIState = AI_MOVINGTOTARGET;
        //AI_InitMoveToTarget(pcMap);
		NEW_AI_CreatePath(pcMap);
    }

    return bFound;
}


///////////////////
// Reloads the weapons
void CWorm::AI_ReloadWeapons(void)
{
    int     i;

    // Go through reloading the weapons
    for(i=0; i<5; i++) {
        if(tWeapons[i].Reloading) {
            iCurrentWeapon = i;
            break;
        }
    }
}


///////////////////
// Initialize a move-to-target
void CWorm::AI_InitMoveToTarget(CMap *pcMap)
{
    memset(pnOpenCloseGrid, 0, nGridCols*nGridRows*sizeof(int));

    // Cleanup the old path
    AI_CleanupPath(psPath);

    cPosTarget = AI_GetTargetPos();

    // Put the target into a cell position
    int tarX = (int) cPosTarget.GetX() / pcMap->getGridWidth();
    int tarY = (int) cPosTarget.GetY() / pcMap->getGridHeight();

    // Current position cell
    int curX = (int) vPos.GetX() / pcMap->getGridWidth();
    int curY = (int) vPos.GetY() / pcMap->getGridHeight();

    nPathStart[0] = curX;
    nPathStart[1] = curY;
    nPathEnd[0] = tarX;
    nPathEnd[1] = tarY;
   
    // Go down the path
    psPath = AI_ProcessNode(pcMap, NULL, curX,curY, tarX, tarY);
    psCurrentNode = psPath;

    fLastPathUpdate = tLX->fCurTime;
    
    // Draw the path
    if(!psPath)
        return;

#ifdef _AI_DEBUG
    pcMap->DEBUG_DrawPixelFlags();
    AI_DEBUG_DrawPath(pcMap,psPath);
#endif // _AI_DEBUG

}


///////////////////
// Path finding 
ai_node_t *CWorm::AI_ProcessNode(CMap *pcMap, ai_node_t *psParent, int curX, int curY, int tarX, int tarY)
{
    int     i;
    bool    bFound = false;

    // Bounds checking
    if(curX < 0 || curY < 0 || tarX < 0 || tarY < 0)
        return NULL;
    if(curX >= nGridCols || curY >= nGridRows || tarX >= nGridCols || tarY >= nGridRows)
        return NULL;

    // Are we on target?
    if(curX == tarX && curY == tarY)
        bFound = true;

    // Is this node impassable (rock)?
    uchar pf = *(pcMap->getGridFlags() + (curY*nGridCols) + curX);
    if(pf == PX_ROCK && !bFound) {
        if(psParent != NULL)  // Parent can be on rock
            return NULL;
    }

    int movecost = 1;
    // Dirt is a higher cost
    if(pf == PX_DIRT)
        movecost = 2;

    // Is this node closed?    
    if(pnOpenCloseGrid[curY*nGridCols+curX] == 1)
        return NULL;
    
    // Close it
    pnOpenCloseGrid[curY*nGridCols+curX] = 1;

    // Create a node
    ai_node_t *node = new ai_node_t;
    if(!node)
        return NULL;

    // Fill in the node details
    node->psParent = psParent;
    node->nX = curX;
    node->nY = curY;
    node->nCost = 0;
    node->nCount = 0;
    node->nFound = false;
    node->psPath = NULL;
    for(i=0; i<8; i++)
        node->psChildren[i] = NULL;
    if(psParent) {
        node->nCost = psParent->nCost+movecost;
        node->nCount = psParent->nCount+1;
    }

    // Are we on target?
    if(bFound) {
		tState.iJump = true;
        node->nFound = true;
        // Traverse back up the path
        return node;
    }

    /*DrawRectFill(pcMap->GetImage(), curX*pcMap->getGridWidth(), curY*pcMap->getGridHeight(), 
                                    curX*pcMap->getGridWidth()+pcMap->getGridWidth(),
                                    curY*pcMap->getGridHeight()+pcMap->getGridHeight(), MakeColour(0,0,255));*/


    //
    // TEST
    //

    int closest = -1;
    int cost = 99999;
    /*int norm[] = {-1,-1, 0,-1, 1,-1, 
                  -1,0,  1,0,
                  -1,1,  0,1, 1,1};
    for(i=0; i<8; i++) {
        node->psChildren[i] = AI_ProcessNode(pcMap, node, curX+norm[i*2], curY+norm[i*2+1], tarX, tarY);
        if(node->psChildren[i]) {
            if(node->psChildren[i]->nFound) {
                if(node->psChildren[i]->nCost < cost) {
                    cost = node->psChildren[i]->nCost;
                    closest = i;
                }
            }
        }
    }
    
    // Kill the children
    for(i=0; i<8; i++) {
        if(i != closest) {
            delete node->psChildren[i];
            node->psChildren[i] = NULL;
        }
    }
    
    // Found any path?
    if(closest == -1) {
        delete node;
        return NULL;
    }
    
    // Set the closest path
    node->psPath = node->psChildren[closest];
    node->nFound = true;
    return node;*/



    // Here we go down the children that are closest to the target
    int vertDir = 0;    // Middle
    int horDir = 0;     // Middle

    if(tarY < curY)
        vertDir = -1;   // Go upwards
    else if(tarY > curY)
        vertDir = 1;    // Go downwards
    if(tarX < curX)
        horDir = -1;    // Go left
    else if(tarX > curX)
        horDir = 1;     // Go right


    int diagCase[] = {horDir,vertDir,  horDir,0,  0,vertDir,  -horDir,vertDir,
                      horDir,-vertDir, -horDir,0, 0,-vertDir, -horDir,-vertDir};

    //
    // Diagonal target case
    //
    closest = -1;
    cost = 99999;
    if(horDir != 0 && vertDir != 0) {
        for(i=0; i<8; i++) {
            node->psChildren[i] = AI_ProcessNode(pcMap, node, curX+diagCase[i*2], curY+diagCase[i*2+1], tarX, tarY);
            if(node->psChildren[i]) {
                if(node->psChildren[i]->nFound) {
                    if(node->psChildren[i]->nCost < cost) {
                        cost = node->psChildren[i]->nCost;
                        closest = i;
                    }
                }
            }
        }

        // Kill the children
        for(i=0; i<8; i++) {
            if(i != closest) {
				if(node->psChildren[i]) {
					delete node->psChildren[i];
					node->psChildren[i] = NULL;
				}
            }
        }

        // Found any path?
        if(closest == -1) {
			if(node)
				delete node;
            return NULL;
        }

        // Set the closest path
        node->psPath = node->psChildren[closest];
        node->nFound = true;
        return node;
    }


    //
    // Straight target case
    //
    int upCase[] = {0,-1,  1,-1,  -1,-1,  1,0,  -1,0,  1,1,  -1,1,  0,1};
    int rtCase[] = {1,0,   1,1,   1,-1,   0,1,  0,-1,  -1,1, -1,-1, -1,0};
    
    closest = -1;
    cost = 99999;
    for(i=0; i<8; i++) {
        int h,v;
        // Up/Down
        if(horDir == 0) {
            h = upCase[i*2];
            v = upCase[i*2+1];
            if(vertDir == 1) {
                h = -h;
                v = -v;
            }
        }

        // Left/Right
        if(vertDir == 0) {
            h = rtCase[i*2];
            v = rtCase[i*2+1];
            if(horDir == -1) {
                h = -h;
                v = -v;
            }
        }

        node->psChildren[i] = AI_ProcessNode(pcMap, node, curX+h, curY+v, tarX, tarY);
        if(node->psChildren[i]) {
            if(node->psChildren[i]->nFound) {
                if(node->psChildren[i]->nCost < cost) {
                    cost = node->psChildren[i]->nCost;
                    closest = i;
                }
            }
        }
        
        // Kill the children
        for(i=0; i<8; i++) {
            if(i != closest) {
				if(node->psChildren[i]) {
					delete node->psChildren[i];
					node->psChildren[i] = NULL;
				}
            }
        }
        
        // Found any path?
        if(closest == -1) {
			if(node)
				delete node;
            return NULL;
        }
        
        // Set the closest path
        node->psPath = node->psChildren[closest];
        node->nFound = true;
        return node;
    }
    delete node;
    return NULL;
}


///////////////////
// Cleanup a path
void CWorm::AI_CleanupPath(ai_node_t *node)
{
    if(node) {

        AI_CleanupPath(node->psPath);
        delete node;
    }
}


///////////////////
// Move towards a target
void CWorm::AI_MoveToTarget(CMap *pcMap)
{
    int     nCurrentCell[2];
    int     nTargetCell[2];
    int     nFinalTarget[2];
    worm_state_t *ws = &tState;

    int     hgw = pcMap->getGridWidth()/2;
    int     hgh = pcMap->getGridHeight()/2;

	// Better target?
	CWorm *newtrg = findTarget(iAiGame, iAiTeams, iAiTag, pcMap);
	if (psAITarget && newtrg)
		if (newtrg->getID() != psAITarget->getID())
			nAIState = AI_THINK;

    // Clear the state
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;

    cPosTarget = AI_GetTargetPos();

    // If we're really close to the target, perform a more precise type of movement
    if(fabs(vPos.GetX() - cPosTarget.GetX()) < 20 && fabs(vPos.GetY() - cPosTarget.GetY()) < 20) {
        AI_PreciseMove(pcMap);
        return;
    }

    // If we're stuck, just get out of wherever we are
    if(bStuck) {
        ws->iMove = true;
        ws->iJump = true;
        cNinjaRope.Release();
        if(tLX->fCurTime - fStuckPause > 2)
            bStuck = false;
        return;
    }



    /*
      First, if the target or ourself has deviated from the path target & start by a large enough amount:
      Or, enough time has passed since the last path update:
      recalculate the path
    */
    int     i;
    int     nDeviation = 2;     // Maximum deviation allowance
    bool    recalculate = false;

    
    // Cell positions
    nTargetCell[0] = (int)cPosTarget.GetX() / pcMap->getGridWidth();
    nTargetCell[1] = (int)cPosTarget.GetY() / pcMap->getGridHeight();
    nCurrentCell[0] = (int)vPos.GetX() / pcMap->getGridWidth();
    nCurrentCell[1] = (int)vPos.GetY() / pcMap->getGridHeight();
    nFinalTarget[0] = (int)cPosTarget.GetX() / pcMap->getGridWidth();
    nFinalTarget[1] = (int)cPosTarget.GetY() / pcMap->getGridHeight();

    for(i=0 ;i<2; i++) {
        if(abs(nPathStart[i] - nCurrentCell[i]) > nDeviation ||
            abs(nPathEnd[i] - nTargetCell[i]) > nDeviation) {
            recalculate = true;
        }
    }

    if(tLX->fCurTime - fLastPathUpdate > 3)
        recalculate = true;

    // Re-calculate the path?
    if(recalculate)
        AI_InitMoveToTarget(pcMap);

    
    // We carve every few milliseconds so we don't go too fast
	if(tLX->fCurTime - fLastCarve > 0.35f) {
		fLastCarve = tLX->fCurTime;
		ws->iCarve = true;
	}


    /*
      Move through the path.
      We have a current node that we must get to. If we go onto the node, we go to the next node, and so on.
      Because the path finding isn't perfect we can sometimes skip a whole group of nodes.
      If we are close to other nodes that are _ahead_ in the list, we can skip straight to that node.
    */

    int     nNodeProx = 3;      // Maximum proximity to a node to skip to

    if(psCurrentNode == NULL || psPath == NULL) {
        #ifdef _AI_DEBUG
          pcMap->DEBUG_DrawPixelFlags();
          AI_DEBUG_DrawPath(pcMap, psPath);
		#endif // _AI_DEBUG

        // If we don't have a path, resort to simpler AI methods
        AI_SimpleMove(pcMap,psAITarget != NULL);
        return;
    }

    float xd = (float)(nCurrentCell[0] - psCurrentNode->nX);
    float yd = (float)(nCurrentCell[1] - psCurrentNode->nY);
    float fCurDist = fastSQRT(xd*xd + yd*yd);
    
    // Go through the path
    ai_node_t *node = psPath;    
    for(; node; node=node->psPath) {

        // Don't go back down the list
        if(node->nCount <= psCurrentNode->nCount)
            continue;

         // If the node is blocked, don't skip ahead
        float tdist;
        int type;
        traceLine(CVec((float)(node->nX*pcMap->getGridWidth()+hgw), (float) (node->nY*pcMap->getGridHeight()+hgh)), pcMap, &tdist, &type,1);
        if(tdist < 0.75f && type == PX_ROCK)
            continue;

        // If we are near the node skip ahead to it
        if(abs(node->nX - nCurrentCell[0]) < nNodeProx &&
           abs(node->nY - nCurrentCell[1]) < nNodeProx) {

            psCurrentNode = node;

			#ifdef _AI_DEBUG
				pcMap->DEBUG_DrawPixelFlags();
				AI_DEBUG_DrawPath(pcMap, node);
			#endif  // _AI_DEBUG
            continue;
        }

        // If we're closer to this node than to our current node, skip to this node
        xd = (float)(nCurrentCell[0] - node->nX);
        yd = (float)(nCurrentCell[1] - node->nY);
        float dist = fastSQRT(xd*xd + yd*yd);

        if(dist < fCurDist) {
            psCurrentNode = node;
			#ifdef _AI_DEBUG
				pcMap->DEBUG_DrawPixelFlags();
				AI_DEBUG_DrawPath(pcMap, node);
			#endif  // _AI_DEBUG
        }
    }

    if(psCurrentNode == NULL || psPath == NULL) {
         // If we don't have a path, resort to simpler AI methods
        AI_SimpleMove(pcMap);
        return;
    }

    nTargetCell[0] = psCurrentNode->nX;
    nTargetCell[1] = psCurrentNode->nY;


    /*
      We get the 5 next nodes in the list and generate an average position to get to
    */
    int avgCell[2] = {0,0};
    node = psCurrentNode;    
    for(i=0; node && i<5; node=node->psPath, i++) {
        avgCell[0] += node->nX;
        avgCell[1] += node->nY;
    }
    if(i != 0) {
    	avgCell[0] /= i;
    	avgCell[1] /= i;
    }
    //nTargetCell[0] = avgCell[0];
    //nTargetCell[1] = avgCell[1];
    
    tLX->debug_pos = CVec((float)(nTargetCell[0]*pcMap->getGridWidth()+hgw), (float)(nTargetCell[1]*pcMap->getGridHeight()+hgh));



    /*
      Now that we've done all the boring stuff, our single job here is to reach the node.
      We have walking, jumping, move-through-air, and a ninja rope to help us.
    */
    int     nRopeHeight = 3;                // Minimum distance to release rope    


    // Aim at the node
    bool aim = AI_SetAim(CVec((float)(nTargetCell[0]*pcMap->getGridWidth()+hgw), (float)(nTargetCell[1]*pcMap->getGridHeight()+hgh)));

    // If we are stuck in the same position for a while, take measures to get out of being stuck
    if(fabs(cStuckPos.GetX() - vPos.GetX()) < 25 && fabs(cStuckPos.GetY() - vPos.GetY()) < 25) {
        fStuckTime += tLX->fDeltaTime;

        // Have we been stuck for a few seconds?
        if(fStuckTime > 4/* && !ws->iJump && !ws->iMove*/) {
            // Jump, move, switch directions and release the ninja rope
            ws->iJump = true;
            ws->iMove = true;
            bStuck = true;
            fStuckPause = tLX->fCurTime;
            cNinjaRope.Release();

			iDirection = !iDirection;
            
            fAngle -= 45;
            // Clamp the angle
	        fAngle = MIN((float)60,fAngle);
	        fAngle = MAX((float)-90,fAngle);

            // Recalculate the path
            AI_InitMoveToTarget(pcMap);
            fStuckTime = 0;
        }
    }
    else {
        bStuck = false;
        fStuckTime = 0;
        cStuckPos = vPos;
    }


    // If the ninja rope hook is falling, release it & walk
    if(!cNinjaRope.isAttached() && !cNinjaRope.isShooting()) {
        cNinjaRope.Release();
        ws->iMove = true;
    }

    // Walk only if the target is a good distance on either side
    if(abs(nCurrentCell[0] - nTargetCell[0]) > 3)
        ws->iMove = true;

    
    // If the node is above us by a lot, we should use the ninja rope
    if(nTargetCell[1] < nCurrentCell[1]) {
        
        // If we're aimed at the point, just leave and it'll happen soon
        if(!aim)
            return;

        bool fireNinja = true;

        CVec dir;
        dir.SetX( (float)cos(fAngle * (PI/180)) );
	    dir.SetY( (float)sin(fAngle * (PI/180)) );
	    if(iDirection == DIR_LEFT)
		    dir.SetX(-dir.GetX());
       
        /*
          Got aim, so shoot a ninja rope
          We shoot a ninja rope if it isn't shot
          Or if it is, we make sure it has pulled us up and that it is attached
        */
        if(fireNinja) {
            if(!cNinjaRope.isReleased())
                cNinjaRope.Shoot(vPos,dir);
            else {
                float length = CalculateDistance(vPos, cNinjaRope.getHookPos());
                if(cNinjaRope.isAttached()) {
                    if(length < cNinjaRope.getRestLength() && vVelocity.GetY()<-10)
                        cNinjaRope.Shoot(vPos,dir);
                }
            }
        }
    }    


    /*
      If there is dirt between us and the next node, don't shoot a ninja rope
      Instead, carve or use some clearing weapon
    */
    float traceDist = -1;
    int type = 0;
	if (!psCurrentNode)
		return;
    CVec v = CVec((float)(psCurrentNode->nX*pcMap->getGridWidth()+hgw), (float)(psCurrentNode->nY*pcMap->getGridHeight()+hgh));
    int length = traceLine(v, pcMap, &traceDist, &type);
    float dist = CalculateDistance(v, vPos);
    if(length < dist && type == PX_DIRT) {
        ws->iJump = true;
        ws->iMove = true;
		ws->iCarve = true; // Carve
        
        // Shoot a path
        /*int wpn;
        if((wpn = AI_FindClearingWeapon()) != -1) {
            iCurrentWeapon = wpn;
            ws->iShoot = true;
            cNinjaRope.Release();
        }*/
    }



    // If we're above the node, let go of the rope and move towards to node
    if(nTargetCell[1] >= nCurrentCell[1]+nRopeHeight) {
        // Let go of any rope
        cNinjaRope.Release();

        // Walk in the direction of the node
        ws->iMove = true;
    }

    if(nTargetCell[1] >= nCurrentCell[1]) {

		if(!IsEmpty(CELL_DOWN,pcMap))  {
			if(IsEmpty(CELL_LEFT,pcMap))
				iDirection = DIR_LEFT;
			else if (IsEmpty(CELL_RIGHT,pcMap))
				iDirection = DIR_RIGHT;
		}

        // Walk in the direction of the node
        ws->iMove = true;
    }


    // If we're near the height of the final target, release the rope
    if(abs(nFinalTarget[0] - nCurrentCell[0]) < 2 && abs(nFinalTarget[1] - nCurrentCell[1]) < 2) {
        // Let go of any rope
        cNinjaRope.Release();

        // Walk in the direction of the target
        ws->iMove = true;

        // If we're under the final target, jump
        if(nFinalTarget[1] < nCurrentCell[1])
            ws->iJump = true;
    }
    





    






    

  



}


///////////////////
// DEBUG: Draw the path
void CWorm::AI_DEBUG_DrawPath(CMap *pcMap, ai_node_t *node)
{
	return;

    if(!node)
        return;

    int x = node->nX * pcMap->getGridWidth();
    int y = node->nY * pcMap->getGridHeight();
    
    //DrawRectFill(pcMap->GetImage(), x,y, x+pcMap->getGridWidth(), y+pcMap->getGridHeight(), 0);
    
    if(node->psPath) {
        int cx = node->psPath->nX * pcMap->getGridWidth();
        int cy = node->psPath->nY * pcMap->getGridHeight();
        DrawLine(pcMap->GetDrawImage(), (x+pcMap->getGridWidth()/2)*2, (y+pcMap->getGridHeight()/2)*2, 
                                    (cx+pcMap->getGridWidth()/2)*2,(cy+pcMap->getGridHeight()/2)*2,
                                    0xffff);
    }
    else {
        // Final target
        DrawRectFill(pcMap->GetDrawImage(), x*2-5,y*2-5,x*2+5,y*2+5, MakeColour(0,255,0));

    }

    AI_DEBUG_DrawPath(pcMap, node->psPath);
}


///////////////////
// Get the target's position
// Also checks the target and resets to a think state if needed
CVec CWorm::AI_GetTargetPos(void)
{
    // Put the target into a position
    switch(nAITargetType) {

        // Bonus target
        case AIT_BONUS:
            if(psBonusTarget) {
                if(!psBonusTarget->getUsed())
                    nAIState = AI_THINK;
                return psBonusTarget->getPosition();
            }
            break;

        // Worm target
        case AIT_WORM:
            if(psAITarget) {
                if(!psAITarget->getAlive() || !psAITarget->isUsed())
                    nAIState = AI_THINK;
                return psAITarget->getPos();
            }
            break;

        // Position target
        case AIT_POSITION:
            return cPosTarget;
    }

    // No target
    nAIState = AI_THINK;
    return CVec(0,0);
}


///////////////////
// Aim at a spot
// Returns true if we're aiming at it
bool CWorm::AI_SetAim(CVec cPos)
{
    float   dt = tLX->fDeltaTime;
    CVec	tgPos = cPos;
	CVec	tgDir = tgPos - vPos;
    bool    goodAim = false;
    gs_worm_t *wd = cGameScript->getWorm();

	float   fDistance = NormalizeVector(&tgDir);

	// We can't aim target straight below us
	if(tgPos.GetX()-10 < vPos.GetX() && tgPos.GetX()+10 > vPos.GetX())
		return false;
	
	if (tLX->fCurTime - fLastFace > 0.1)  {  // prevent turning
	// Make me face the target
		if(tgPos.GetX() > vPos.GetX())
			iDirection = DIR_RIGHT;
		else
			iDirection = DIR_LEFT;

		fLastFace = tLX->fCurTime;
	}

	// Aim at the target
	float ang = (float)atan2(tgDir.GetX(), tgDir.GetY());
	ang = RAD2DEG(ang);

	if(iDirection == DIR_LEFT)
		ang+=90;
	else
		ang = -ang + 90;

	// Clamp the angle
	ang = MAX((float)-90, ang);

	// Move the angle at the same speed humans are allowed to move the angle
	if(ang > fAngle)
		fAngle += wd->AngleSpeed * dt;
	else if(ang < fAngle)
		fAngle -= wd->AngleSpeed * dt;

	// If the angle is within +/- 3 degrees, just snap it
    if( fabs(fAngle - ang) < 3) {
		fAngle = ang;
        goodAim = true;
    }

	// Clamp the angle
	fAngle = MIN((float)60,fAngle);
	fAngle = MAX((float)-90,fAngle);

    return goodAim;
}


///////////////////
// A simpler method to get to a target
// Used if we have no path
float fLastTurn = 0;  // Time when we last tried to change the direction
float fLastJump = 0;  // Time when we last tried to jump
void CWorm::AI_SimpleMove(CMap *pcMap, bool bHaveTarget)
{
    worm_state_t *ws = &tState;

    // Simple    
	ws->iMove = true;
	ws->iShoot = false;
	ws->iJump = false;

    //strcpy(tLX->debug_string, "AI_SimpleMove invoked");

    cPosTarget = AI_GetTargetPos();
  
    // We carve every few milliseconds so we don't go too fast
	if(tLX->fCurTime - fLastCarve > 0.35f) {
		fLastCarve = tLX->fCurTime;
		ws->iCarve = true;
	}
    
    // Aim at the node
    bool aim = AI_SetAim(cPosTarget);

    // If our line is blocked, try some evasive measures
    float fDist = 0;
    int type = 0;
    int nLength = traceLine(cPosTarget, pcMap, &fDist, &type, 1);
    if(fDist < 0.75f || cPosTarget.GetY() < vPos.GetY()) {

        // Change direction
		if (bHaveTarget && (tLX->fCurTime-fLastTurn) > 1.0)  {
			iDirection = !iDirection;
			fLastTurn = tLX->fCurTime;
		}

        // Look up for a ninja throw
        aim = AI_SetAim(vPos+CVec(GetRandomNum()*10,GetRandomNum()*10+10));
        if(aim) {
            CVec dir;
            dir.SetX( (float)cos(fAngle * (PI/180)) );
	        dir.SetY( (float)sin(fAngle * (PI/180)) );
	        if(iDirection==DIR_LEFT)
		        dir.SetX(-dir.GetX());

            cNinjaRope.Shoot(vPos,dir);
        }

		// Jump and move
		else  {
			if (tLX->fCurTime-fLastJump > 3.0)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			}
			ws->iMove = true;
			cNinjaRope.Release();
		}

        return;
    }

    // Release the ninja rope
    cNinjaRope.Release();
}

float fLastDirChange = 99999;
///////////////////
// Perform a precise movement
void CWorm::AI_PreciseMove(CMap *pcMap)
{
    worm_state_t *ws = &tState;

    //strcpy(tLX->debug_string, "AI_PreciseMove invoked");

    ws->iJump = false;
    ws->iMove = false;
    ws->iCarve = false;
    
    // If we're insanely close, just stop
    if(fabs(vPos.GetX() - cPosTarget.GetX()) < 10 && fabs(vPos.GetY() - cPosTarget.GetY()) < 10)
        return;


    // Aim at the target
    //bool aim = AI_SetAim(cPosTarget);
	bool aim = AI_CanShoot(pcMap,iAiGame);
    if(aim) {
        // Walk towards the target
        ws->iMove = true;

        // If the target is above us, jump
        if(fabs(vPos.GetX() - cPosTarget.GetX()) < 10 && vPos.GetY() - cPosTarget.GetY() > 5)
            ws->iJump = true;
    } else  {
		ws->iJump = true;
		// Randomly change direction
		if (tLX->fCurTime - fLastDirChange > 2.0f)  {
			if (GetRandomNum() < 0)
				iDirection = DIR_LEFT;
			else
				iDirection = DIR_RIGHT;
			fLastDirChange = tLX->fCurTime;
		}
		ws->iMove = true;
	}
}


///////////////////
// Finds a suitable 'clearing' weapon
// A weapon used for making a path
// Returns -1 on failure
int CWorm::AI_FindClearingWeapon(void)
{
	if(iAiGameType == GAM_MORTARS)
		return -1;
    //if(!strcmp(tWeapons[0].Name,"Rifle"))
        for (int i=0; i<5; i++)
			if(!tWeapons[i].Reloading)
				return i;

    // No suitable weapons
    return -1;
}



///////////////////
// Can we shoot the target
bool CWorm::AI_CanShoot(CMap *pcMap, int nGameType)
{

    // Make sure the target is a worm
    if(nAITargetType != AIT_WORM)
        return false;
    
    // Make sure the worm is good
    if(!psAITarget)
        return false;
    if(!psAITarget->getAlive() || !psAITarget->isUsed())
        return false;
    

    CVec    cTrgPos = psAITarget->getPos();
    bool    bDirect = true;


    /*
      Here we check if we have a line of sight with the target.
      If we do, and our 'direct' firing weapons are loaded, we shoot.

      If the line of sight is blocked, we use 'indirect' shooting methods.
      We have to be careful with indirect, because if we're in a confined space, or the target is above us
      we shouldn't shoot at the target

      The aim of the game is killing worms, so we put a higher priority on shooting rather than
      thinking tactically to prevent injury to self

	  If we play a mortar game, make sure there's enough free space to shoot - avoid suicides
    */


    // If the target is too far away we can't shoot at all (but not when a rifle game)
    float d = CalculateDistance(cTrgPos, vPos);
    if(d > 300.0f && iAiGameType != GAM_RIFLES)
        return false;

	// If we're on the target, simply shoot
	if(d < 10.0f)
		return true;

    float fDist;
    int nType = -1;
    int length = 0;

	length = traceWeaponLine(cTrgPos, pcMap, &fDist, &nType);

    // If target is blocked by rock we can't use direct firing
    if(nType == PX_ROCK)  {
		/*if(iAiGameType == GAM_RIFLES || iAiGameType == GAM_MORTARS)
			return false;
		if (((float)length/d) < 0.7f)
			return false;
        bDirect = false;*/
		return false;
	}

	// Don't shoot teammates
	if(tGameInfo.iGameType == GMT_TEAMDEATH && nType == PX_WORM)
		return false;

	// If target is blocked by large amount of dirt, we can't shoot it with rifle
	if (nType == PX_DIRT)  {
		if(d-fDist > 40.0f)
			return false;
	}

	// In mortar game there must be enough of free cells around us
	if (iAiGameType == GAM_MORTARS)  {
		if (!NEW_AI_CheckFreeCells(1,pcMap))  {
			return false;
		}
		if (!traceWormLine(cTrgPos,vPos,pcMap))
			return false;
	}

	// If our velocity is big and we shoot in the direction of the flight, we can suicide
	// We will avoid this here
	/*if (vVelocity.GetY() > 30 && fAngle >= 50)
		return false;
	if (vVelocity.GetY() < -30 && fAngle <= 10)
		return false;
	if (vVelocity.GetX() < -30 && iDirection == DIR_LEFT && fAngle > 20)
		return false;
	if (vVelocity.GetX() > 30 && iDirection == DIR_RIGHT && fAngle > 20)
		return false;*/

    // Set the best weapon for the situation
    // If there is no good weapon, we can't shoot
    tLX->debug_float = d;
    int wpn = AI_GetBestWeapon(nGameType, d, bDirect, pcMap, fDist);
    if(wpn == -1) {
        //strcpy(tLX->debug_string, "No good weapon");
        return false;
    }     

    iCurrentWeapon = wpn;

    // Shoot
    return true;
}

////////////////////
// Returns true, if the weapon can hit the target
bool CWorm::weaponCanHit(float alpha,int gravity,float speed,CMap *pcMap)
{
	// Get the target position
	if(!psAITarget)
		return false;
    CVec cTrgPos = psAITarget->getPos();

	// Check
	if (fabs(alpha) == 1 || speed == 0)
		return false;

	// Convert the alpha to radians
	alpha = DEG2RAD(alpha);
	// Get the maximal X
	int max_x = (int)(cTrgPos.GetX()-vPos.GetX());
	// If we're in the X coordinate of the target, we can shoot (else we wouldn't be called)
	if (max_x == 0)
		return true;
	// Get the maximal Y
	int max_y = (int)(vPos.GetY()-cTrgPos.GetY());

	// Check the pixels in the projectile trajectory
	int x,y;

#ifdef _AI_DEBUG
	DrawRectFill(pcMap->GetDebugImage(),0,0,pcMap->GetDebugImage()->w,pcMap->GetDebugImage()->h,MakeColour(255,0,255));
#endif

	// Target on left
	int tmp;
	if (max_x < 0)  {
		for (x=0;x>max_x;x--)  {
			tmp = (int)(2*speed*speed*cos(alpha)*cos(alpha));
			if(tmp != 0) // please do such checks
				y = -x*(int)tan(alpha)+(gravity*x*x)/tmp;
			else
				y = 0;
			// If we have reached the target, the trajectory is free
			if (max_y < 0)  {
				if (y < max_y)
					return true;
			} else  {
				if (y > max_y)
					return true;
			}
#ifdef _AI_DEBUG
			PutPixel(pcMap->GetDebugImage(),x*2+(int)vPos.GetX()*2,y*2+(int)vPos.GetY()*2,0xffff);
#endif
			// Rock, trajectory not free
			if (pcMap->GetPixelFlag(x+(int)vPos.GetX(),y+(int)vPos.GetY()) == PX_ROCK)  {
				return false;
			}
		}
	}
	// Target on right
	else  {
		for (x=0;x<max_x;x++)  {
			tmp = (int)(2*speed*speed*cos(alpha)*cos(alpha));
			if(tmp != 0)
				y = -x*(int)tan(alpha)+(gravity*x*x)/tmp;
			else
				y = 0;
			// If we have reached the target, the trajectory is free
			if (max_y < 0)  {
				if (y < max_y)
					return true;
			} else  {
				if (y > max_y)
					return true;
			}
#ifdef _AI_DEBUG
			PutPixel(pcMap->GetDebugImage(),x*2+(int)vPos.GetX()*2,y*2+(int)vPos.GetY()*2,0xffff);
#endif
			// Rock, trajectory not free
			if (pcMap->GetPixelFlag(x+(int)vPos.GetX(),y+(int)vPos.GetY()) == PX_ROCK)  {
				return false;
			}
		}
	}

	return true;
}


///////////////////
// Shoot!
void CWorm::AI_Shoot(CMap *pcMap)
{
	if(!psAITarget)
		return;
    CVec    cTrgPos = psAITarget->getPos();

    //
    // Aim at the target
    //
    bool    bAim = true;//AI_SetAim(cTrgPos);

    // TODO: Aim in the right direction to account of weapon speed, gravity and worm velocity
	weapon_t *weap = getCurWeapon()->Weapon;
	switch (weap->Type)  {
	case WPN_BEAM:
		// Direct aim
		bAim = AI_SetAim(cTrgPos);
	break;
	case WPN_PROJECTILE:  {
		switch (weap->Projectile->Type)  {
		case PJ_NOTHING:
		case PJ_CARVE:
		case PJ_DIRT:
			return;
		default:
			// Random
			int	  Random = GetRandomInt(255);
			// Worm speed
			float MySpeed = VectorLength(vVelocity);
			// Enemy speed
			float EnemySpeed = VectorLength(*psAITarget->getVelocity());
			// Projectile speed
			float v = (float)weap->ProjSpeed+(float)weap->ProjSpeedVar*GetFixedRandomNum(Random)*weap->Projectile->Dampening;
			// Projectile spread
			float Spread = (float)GetFixedRandomNum(Random)*(float)weap->ProjSpread;
			// Gravity
			int	  g = 100;
			if (weap->Projectile->UseCustomGravity)
				g = weap->Projectile->Gravity;
			// Distance
			float x = (cTrgPos.GetX()-vPos.GetX());
			float y = (vPos.GetY()-cTrgPos.GetY());

			// Get the alpha
			float tmp =
				(x*sqrt(fabs(
						sqrt(fabs(
							-pow(g,2)*pow(x,2)
							-2*g*pow(v,2)*y
							+pow(v,4)))
						-g*y
						+pow(v,2))));
			float alpha = PI/2; // the value, if tmp == 0
			if(tmp != 0)
				alpha = 
					(float)atan(
						sqrt(fabs(
							- pow(x,2) * sqrt(fabs(
								-pow(g,2)*pow(x,2)
								- 2*g*pow(v,2)*y
								+ pow(v,4)))
							+ pow(x,2) * (g*y+pow(v,2))
							+ 2 * pow(v,2) * pow(y,2)))
						/ tmp);
				
			if (y > 0)
				alpha = -alpha;
			if (x < 0)
				alpha = -alpha;

			// Convert to degrees
			alpha = RAD2DEG(alpha);

			// We can't aim it
			if (alpha > 60 || alpha < -90)  {
				bAim = false;
				break;
			}
			// Aim
			else
				fAngle = alpha;

			// Face the target
			if (x < 0)
				iDirection = DIR_LEFT;
			else
				iDirection = DIR_RIGHT;

			// Can we hit the target?
			bAim = weaponCanHit(alpha,g,v,pcMap);

			/*strcpy(tLX->debug_string,weap->Name);
			if (tLX->fCurTime-flast > 1.0f)  {
				tLX->debug_float = alpha;
				flast = tLX->fCurTime;
			}*/
		break;
		}
	}
	break;
	}

    if(!bAim)  {

		// In mortars we can hit the target below us
		if (iAiGameType == GAM_MORTARS)  {
			if (cTrgPos.GetY() > (vPos.GetY()-20.0f))
				tState.iShoot = true;
			return;
		}

		tState.iMove = true;
		fBadAimTime += tLX->fDeltaTime;
		if((fBadAimTime) > 4) {
			if(IsEmpty(CELL_UP,pcMap))
				tState.iJump = true;
			fBadAimTime = 0;
		}
        return;
	}

	fBadAimTime = 0;

    // Shoot
	tState.iShoot = true;
	fLastShoot = tLX->fCurTime;
}


///////////////////
// AI: Get the best weapon for the situation
// Returns weapon id or -1 if no weapon is suitable for the situation
int CWorm::AI_GetBestWeapon(int nGameType, float fDistance, bool bDirect, CMap *pcMap, float fTraceDist)
{
    // Note: This assumes that the weapons are the same as when we started the game
    // We could have picked up bonuses, but we will assume we havn't
    // (which will lead to interesting game scenarios if we have)


    // We need to wait a certain time before we change weapon
    if( tLX->fCurTime - fLastWeaponChange > 0.15f )
        fLastWeaponChange = tLX->fCurTime;
    else
        return iCurrentWeapon;

	// For rifles and mortars just get the first unreloaded weapon
	if (iAiGameType == GAM_RIFLES || iAiGameType == GAM_MORTARS)  {
		for (int i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				return i;
		return 0;
	}

    CVec    cTrgPos = AI_GetTargetPos();


	if (iAiGameType == GAM_100LT)  {
		// We're above the worm

		// If we are close enough, shoot the napalm
		if (CalculateDistance(vPos,cTrgPos) < 40 && vPos.GetY() <= cTrgPos.GetY())  {
			// If we are not above enough, jump
			if (fabs(vPos.GetY() - cTrgPos.GetY()) < 20)  {
				tState.iJump = true;
				return -1;
			}
			
			// Check that there is enough of free cells in the direction of our shooting
			if (NEW_AI_CheckFreeCells(2,pcMap) && !tWeapons[1].Reloading)  {
				return 1;
			}
		}


		float d = CalculateDistance(vPos,cTrgPos);
		// We're close to the target
		if (d < 50.0f)  {
			// We see the target
			if(bDirect)  {
				// Super shotgun
				if (!tWeapons[0].Reloading)
					return 0;

				// Chaingun
				if (!tWeapons[4].Reloading)  
					return 4;
				

				// Doomsday
				if (!tWeapons[3].Reloading)
					return 3;

				// Let's try cannon
				if (!tWeapons[2].Reloading)
				// Don't use cannon when we're on ninja rope, we will avoid suicides
					if (!cNinjaRope.isAttached())  {
						tState.iMove = false;  // Don't move, avoid suicides
						return 2;
					}
			}
			// We don't see the target
			else  {
				tState.iJump = true; // Jump, we might get better position
				return -1;
			}
		}

		// Not close, not far
		if (d > 50.0f && d<=300.0f)  {
			if (bDirect)  {

				// Chaingun is the best weapon for this situation
				if (!tWeapons[4].Reloading)  {
					return 4;
				}

				// Let's try cannon
				if (!tWeapons[2].Reloading)
					// Don't use cannon when we're on ninja rope, we will avoid suicides
					if (!cNinjaRope.isReleased())  {
						// Aim a bit up
						AI_SetAim(CVec(cTrgPos.GetX(),cTrgPos.GetY()+5.0f));
						tState.iMove = false;  // Don't move, avoid suicides
						return 2;
					}

				// Super Shotgun makes it sure
				if (!tWeapons[0].Reloading)
					return 0;

				// As for almost last, try doomsday
				if (!tWeapons[3].Reloading)
					// Don't use doomsday when we're on ninja rope, we will avoid suicides
					if (!cNinjaRope.isShooting())  {
						tState.iMove = false;  // Don't move, avoid suicides
						return 3;
					}
			} // End of direct shooting weaps

			return -1;
		}

		// Quite far
		if (d > 300.0f && bDirect)  {

			// First try doomsday
			if (!tWeapons[3].Reloading)  {
				// Don't use doomsday when we're on ninja rope, we will avoid suicides
				if (!cNinjaRope.isAttached())  {
					tState.iMove = false;  // Don't move, avoid suicides
					return 3;
				}
			}

			// Super Shotgun
			if (!tWeapons[0].Reloading)
				return 0;

			// Chaingun
			if (!tWeapons[4].Reloading)
				return 4;

			// Cannon, the worst possible for this
			if (!tWeapons[2].Reloading)
				// Don't use cannon when we're on ninja rope, we will avoid suicides
				if (!cNinjaRope.isReleased())  {
					// Aim a bit up
					AI_SetAim(CVec(cTrgPos.GetX(),cTrgPos.GetY()+5.0f));
					tState.iMove = false;  // Don't move, avoid suicides
					return 2;
				}
		}
					
		return -1;

	}

    /*
    0: "minigun"
	1: "shotgun"
	2: "chiquita bomb"
	3: "blaster"
	4: "big nuke"
    */


    /*
    
      Special firing cases
    
    */

    //
    // Case 1: The target is on the bottom of the level, a perfect spot to lob an indirect weapon
    //
    if(cTrgPos.GetY() > pcMap->GetHeight()-50 && fDistance < 200) {
		for (int i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_PROJECTILE)
					if (tWeapons[i].Weapon->Projectile->Type == PJ_EXPLODE)
						return i;
    }



    /*
    
      Direct firing weapons
    
    */

    //
    // If we're close, use some beam or projectile weapon
    //
    if(fDistance < 100 && bDirect) {
        // First try beam
		int i;
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_BEAM)
					return i;

		// If beam not available, try projectile
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_PROJECTILE)
					if (tWeapons[i].Weapon->Type == PRJ_PIXEL)
						return i;
    }


    //
    // If we're at a medium distance, use any weapon, but prefer the exact ones
    //
    if(fDistance < 150 && bDirect) {

		// First try beam
		int i;
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_BEAM)
					return i;

		// If beam not available, try projectile
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_PROJECTILE)
					if (tWeapons[i].Weapon->Projectile->Type == PRJ_PIXEL || tWeapons[i].Weapon->Projectile->Type == PJ_BOUNCE)
						return i;

		// If everything fails, try any weapon
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				return i;
    }


    //
    // Any greater distance for direct firing uses a projectile weapon first
    //
    if(bDirect) {
		// First try projectile
		int i;
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_PROJECTILE)
					return i;

		// If projectile not available, try beam
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				if (tWeapons[i].Weapon->Type == WPN_BEAM)
					return i;

		// If everything fails, try any weapon
		for (i=0; i<5; i++)
			if (!tWeapons[i].Reloading)  
				return i;
    }


    //
    // Indirect firing weapons
    //


    // If we're above the target, try any special weapon, for Liero mod try napalm
    // BUT only if our health is looking good
    // AND if there is no rock/dirt nearby
    if(fDistance > 190 && iHealth > 25 && fTraceDist > 0.5f && !bDirect && (cTrgPos.GetY()-20) > vPos.GetY()) {
        if (!NEW_AI_CheckFreeCells(5,pcMap))
			return -1;

		for (int i=0; i<5; i++)
			if (!tWeapons[i].Reloading && tWeapons[i].Weapon->Type == WPN_PROJECTILE)
				if (tWeapons[i].Weapon->Projectile->Type == PJ_EXPLODE || tWeapons[i].Weapon->Projectile->Type == PJ_BOUNCE)
					return i;

    }


    //
    // Last resort
    //

    // Shoot a beam (we cant suicide with that)
	int i;
	for (i=0; i<5; i++)
		if (!tWeapons[i].Reloading && tWeapons[i].Weapon->Type == WPN_BEAM)  
			return i;
    
    
    
    // No suitable weapon found
	/*for (i=0;i<5;i++)
		if (!tWeapons[i].Reloading)
		{	
			return i;
		}*/

    return -1;
}


///////////////////
// Return any unloaded weapon in the list
/*int CWorm::cycleWeapons(void)
{
    // Don't do this in easy mode
    if( tProfile->nDifficulty == 0 )
        return iCurrentWeapon;

    // Find the first reloading weapon
    for( int i=0; i<5; i++) {
        if( tWeapons[i].Reloading )
            return i;
    }

    // Default case (all weapons loaded)
    return 0;
}*/


///////////////////
// Trace a line from this worm to the target
// Returns the distance the trace went
int CWorm::traceLine(CVec target, CMap *pcMap, float *fDist, int *nType, int divs)
{
    assert( pcMap );

    // Trace a line from the worm to length or until it hits something
	CVec    pos = vPos;
	CVec    dir = target-pos;
    int     nTotalLength = (int)NormalizeVector(&dir);

	int divisions = divs;			// How many pixels we go through each check (more = slower)

	if( nTotalLength < divisions)
		divisions = nTotalLength;

    *nType = PX_EMPTY;

	// Make sure we have at least 1 division
	divisions = MAX(divisions,1);

	int i;
	for(i=0; i<nTotalLength; i+=divisions) {
		uchar px = pcMap->GetPixelFlag( (int)pos.GetX(), (int)pos.GetY() );
		//pcMap->PutImagePixel((int)pos.GetX(), (int)pos.GetY(), MakeColour(255,0,0));

        if(px & PX_DIRT || px & PX_ROCK) {
        	if(nTotalLength != 0)
            	*fDist = (float)i / (float)nTotalLength;
            else
            	*fDist = 0;
            *nType = px;
            return i;
        }

        pos = pos + dir * (float)divisions;
    }


    // Full length    
	if(nTotalLength != 0)
		*fDist = (float)i / (float)nTotalLength;
	else
		*fDist = 0;
    return nTotalLength;
}

///////////////////
// Returns true, if the cell is empty
// Cell can be: CELL_CURRENT, CELL_LEFT,CELL_DOWN,CELL_RIGHT,CELL_UP,CELL_LEFTDOWN,CELL_RIGHTDOWN,CELL_LEFTUP,CELL_RIGHTUP
bool CWorm::IsEmpty(int Cell, CMap *pcMap)
{
  bool bEmpty = false;
  int cx = (int)(vPos.GetX() / pcMap->getGridWidth());
  int cy = (int)(vPos.GetY() / pcMap->getGridHeight());

  switch (Cell)  {
  case CELL_LEFT:
	  cx--;
	  break;
  case CELL_DOWN:
	  cy++;
	  break;
  case CELL_RIGHT:
	  cx++;
	  break;
  case CELL_UP:
	  cy--;
	  break;
  case CELL_LEFTDOWN:
	  cx--;
	  cy++;
	  break;
  case CELL_RIGHTDOWN:
	  cx++;
	  cy++;
	  break;
  case CELL_LEFTUP:
	  cx--;
	  cy--;
	  break;
  case CELL_RIGHTUP:
	  cx++;
	  cy--;
	  break;
  }

  if (cx < 0 || cx > pcMap->getGridCols())
	  return false;

  if (cy < 0 || cy > pcMap->getGridRows())
	  return false;

  /*int dx = cx*pcMap->getGridCols()*2;
  int dy = pcMap->getGridRows()*2*cy;

  DrawRect(pcMap->GetDrawImage(),dx,dy,dx+(pcMap->GetWidth()/pcMap->getGridCols())*2,dy+(pcMap->GetHeight()/pcMap->getGridRows())*2,0xFFFF);

  dx /= 2;
  dy /= 2;
  DrawRect(pcMap->GetImage(),dx,dy,dx+pcMap->GetWidth()/pcMap->getGridCols(),dy+pcMap->GetHeight()/pcMap->getGridRows(),0xFFFF);*/

  uchar   *f = pcMap->getGridFlags() + cy*pcMap->getGridWidth()+cx;
  bEmpty = *f == PX_EMPTY;

  return bEmpty;
}

/////////////////////////////
// TEST TEST TEST
//////////////////
// Finds the nearest free cell in the map and returns coordinates of its midpoint
CVec CWorm::NEW_AI_FindClosestFreeCell(CVec vPoint, CMap *pcMap)
{
	// NOTE: highly unoptimized, looks many times to the same cells

	// Get the cell
	int cellX = (int) fabs((vPoint.GetX())/pcMap->getGridWidth());
	int cellY = (int) fabs((vPoint.GetY())/pcMap->getGridHeight());	

	int cellsSearched = 1;
	const int numCells = pcMap->getGridCols() * pcMap->getGridRows();
	int i=1;
	int x,y;
	uchar tmp_pf = PX_ROCK;
	while (cellsSearched < numCells) {
		for (y=cellY-i;y<=cellY+i;y++)  {

			// Clipping
			if (y > pcMap->getGridRows())
				break;
			if (y < 0)
				continue;

			for (x=cellX-i;x<=cellX+i;x++)  {
				// Don't check the entry cell
				if (x == cellX && y == cellY)
					continue;

				// Clipping
				if (x > pcMap->getGridCols())
					break;
				if (x < 0)
					continue;

				tmp_pf = *(pcMap->getGridFlags() + y*pcMap->getGridCols() +x);
				if (tmp_pf != PX_ROCK)
					return CVec((float)x*pcMap->getGridWidth()+pcMap->getGridWidth()/2, (float)y*pcMap->getGridHeight()+pcMap->getGridHeight()/2);
			}
		}
		i++;
		cellsSearched++;
	}

	// Can't get here
	return CVec(0,0);
}

/////////////////////
// Trace the line for the current weapon
int CWorm::traceWeaponLine(CVec target, CMap *pcMap, float *fDist, int *nType)
{
   assert( pcMap );

    // Trace a line from the worm to length or until it hits something
	CVec    pos = vPos;
	CVec    dir = target-pos;
    int     nTotalLength = (int)NormalizeVector(&dir);

	int first_division = 7;	// How many pixels we go through first check (we can shoot through walls)		
	int divisions = 5;		// How many pixels we go through each check (more = slower)
	
	//
	// Predefined divisions
	//

	// Beam
	if (tWeapons[iCurrentWeapon].Weapon->Type == WPN_BEAM)  {
		first_division = 1;
		divisions = 1;
	}

	// Rifles
	else if (iAiGameType == GAM_RIFLES)  {
		first_division = 10;
		divisions = 6;
	}

	// Mortars
	else if (iAiGameType == GAM_MORTARS)  {
		first_division = 7;
		divisions = 2;
	}

	// 100lt
	else if (iAiGameType == GAM_100LT)  {
		first_division = 6;
		divisions = 3;
	}

	// Add the worm thickness
	first_division += 5;

	// Check
	if( nTotalLength < divisions)
		divisions = nTotalLength;

    *nType = PX_EMPTY;

	// Make sure we have at least 1 division
	divisions = MAX(divisions,1);
	first_division = MAX(first_division,1);

	// Check that we don't hit any teammate
	CVec WormsPos[MAX_WORMS];
	int	WormCount = 0;
	int i;
	if (cClient && tGameInfo.iGameType == GMT_TEAMDEATH)  {
		CWorm *w = cClient->getRemoteWorms();
		for (i=0;i<MAX_WORMS;i++,w++)  {
			if (w)
				if (w->isUsed() && w->getAlive() && w->getTeam() == iTeam)
					WormsPos[WormCount++] = w->getPos();
		}
	}

			

	// Trace the line
	int divs = first_division;
	int j;
	for(i=0; i<nTotalLength; i+=divs) {
		uchar px = pcMap->GetPixelFlag( (int)pos.GetX(), (int)pos.GetY() );

		if (i>first_division)  // we aren't close to a wall, so we can shoot through only thin wall
			divs = divisions;

		// Dirt or rock
		if(px & PX_DIRT || px & PX_ROCK) {
        	if(nTotalLength != 0)
            	*fDist = (float)i / (float)nTotalLength;
            else
            	*fDist = 0;
			*nType = px;
			return i;
		}

		// Friendly worm
		for (j=0;j<WormCount;j++) {
			if (CalculateDistance(pos,WormsPos[j]) < 15.0f)  {
				if(nTotalLength != 0)
					*fDist = (float)i / (float)nTotalLength;
				else
					*fDist = 0;
				*nType = PX_WORM;
				return i;
			}
		}

		pos = pos + dir * (float)divs;
	}

	// Full length
	if(nTotalLength != 0)
		*fDist = (float)i / (float)nTotalLength;
	else
		*fDist = 0;
	return nTotalLength;
}

////////////////////
// Trace the line with worm width
int CWorm::traceWormLine(CVec target, CVec start, CMap *pcMap)
{

/*#ifdef _AI_DEBUG
	SDL_Surface *bmpDest = pcMap->GetDebugImage();
	if (!bmpDest)
		return true;

	DrawRectFill(bmpDest,0,0,bmpDest->w,bmpDest->h,MakeColour(255,0,255));
#endif*/

	// If the positions are really close, just return true
	/*if (CalculateDistance(target,start) < 3)
		return true;*/

	const int worm_size = 10;

    // Trace lines from worm to the target
	/*float fi = VectorAngle(CVec(target.GetX()-start.GetX(),target.GetY()-start.GetY()),CVec(1,0));
	float debug_fi = RAD2DEG(fi);
	float x_ratio = (float)sin(fi);
	float y_ratio = (float)cos(fi);*/

	int j;
	int num_good = worm_size;
	for (j=0;j<worm_size;j++)  {
		// Get the positions
		CVec    pos = start+CVec((float)(j-worm_size/2),(float)(j-worm_size/2));
		CVec    dir = target-start;
		int     nTotalLength = (int)NormalizeVector(&dir);

		// Trace the line
		int i;
		uchar px;
		for(i=0; i<nTotalLength; i++) {
			px = pcMap->GetPixelFlag( (int)pos.GetX(), (int)pos.GetY() );

			if(px & PX_ROCK) {
				// If we almost reached the target, just take it as if we reached it really
				if (nTotalLength-i > 8)
					num_good--;
				if (num_good < 8)
					return false;
				break;
			}

			pos = pos + dir;

/*#ifdef _AI_DEBUG
			int _x = 2*(int)pos.GetX();
			int _y = 2*(int)pos.GetY();
			if (_x < 0 || _x > bmpDest->w)
				continue;
			if (_y < 0 || _y > bmpDest->h)
				continue;
			PutPixel(bmpDest,_x, _y, MakeColour(255,j*15,0));
			PutPixel(bmpDest,_x+1,_y, MakeColour(255,j*15,0));
			PutPixel(bmpDest,_x,_y+1, MakeColour(255,j*15,0));
			PutPixel(bmpDest,_x+1, _y+1, MakeColour(255,j*15,0));
#endif*/
		}
	}

	return num_good >= 8;

}

////////////////////////
// Checks if there is enough free cells around us to shoot
bool CWorm::NEW_AI_CheckFreeCells(int Num,CMap *pcMap)
{
	// Get the cell
	int cellX = (int) fabs((vPos.GetX())/pcMap->getGridWidth());
	int cellY = (int) fabs((vPos.GetY())/pcMap->getGridHeight());

	// First of all, check our current cell
	if (*(pcMap->getGridFlags() + cellY*pcMap->getGridCols() +cellX) == PX_ROCK)
		return false;
	
/*#ifdef _AI_DEBUG
	SDL_Surface *bmpDest = pcMap->GetDebugImage();
	if (!bmpDest)
		return false;
	DrawRectFill(bmpDest,0,0,bmpDest->w,bmpDest->h,MakeColour(255,0,255));
#endif*/

	// Direction to left
	if (iDirection == DIR_LEFT)  {
		int dir = 0;
		if (fAngle > 210)
			dir = 1;
		if (fAngle < 210 && fAngle > 160)
			dir = -Num/2;
		if (fAngle < 160)
			dir = -1*Num;

/*#ifdef _AI_DEBUG
		int dX = (cellX-Num-1)*pcMap->getGridWidth()+pcMap->getGridWidth()/2;
		int dY = (cellY+dir)*pcMap->getGridHeight()+pcMap->getGridHeight()/2;
		int dX2 = (cellX-1)*pcMap->getGridWidth()+pcMap->getGridWidth()/2;
		int dY2 = (cellY+dir+Num)*pcMap->getGridHeight()+pcMap->getGridHeight()/2;
		DrawRect(bmpDest,dX*2,dY*2,dX2*2,dY2*2,MakeColour(255,0,0));
#endif*/

		// Check the Num*Num square
		int x,y;
		for (x=cellX-Num-1;x<cellX;x++)  {
			// Clipping means rock
			if (x < 0 || x > pcMap->getGridCols())
				return false;
			for (y=cellY+dir;y<cellY+dir+Num;y++)  {
				// Clipping means rock
				if (y < 0 || y > pcMap->getGridRows())
					return false;

				// Clipping means rock
				if (*(pcMap->getGridFlags() + y*pcMap->getGridCols() +x) == PX_ROCK)
					return false;
			}
		}

		return true;
	// Direction to right
	}  else  {
		int dir = 0;
		if (fAngle > 20)
			dir = 1;
		else if (fAngle < 20 && fAngle > -20)
			dir = -Num/2;
		else if (fAngle < -20)
			dir = -1*Num;

/*#ifdef _AI_DEBUG
		int dX = (cellX)*pcMap->getGridWidth()+pcMap->getGridWidth()/2;
		int dY = (cellY+dir)*pcMap->getGridHeight()+pcMap->getGridHeight()/2;
		int dX2 = (cellX+Num)*pcMap->getGridWidth()+pcMap->getGridWidth()/2;
		int dY2 = (cellY+dir+Num)*pcMap->getGridHeight()+pcMap->getGridHeight()/2;
		DrawRect(bmpDest,dX*2,dY*2,dX2*2,dY2*2,MakeColour(255,0,0));
#endif*/

		// Check the square Num*Num
		int x,y;
		for (x=cellX;x<=cellX+Num;x++)  {
			// Clipping means rock
			if (x< 0 || x > pcMap->getGridCols())
				return false;
			for (y=cellY+dir;y<cellY+dir+Num;y++)  {
				// Clipping means rock
				if (y < 0 || y > pcMap->getGridRows())
					return false;

				// Rock cell
				if (*(pcMap->getGridFlags() + y*pcMap->getGridCols() +x) == PX_ROCK)
					return false;
			}
		}

		return true;
	}

	// Weird, shouldn't happen
	return false;
}

///////////////////
// Cleanup the path
void CWorm::NEW_AI_CleanupPath(void)
{
	if (!NEW_psPath)
		return;

	// Delete the nodes
	NEW_ai_node_t *node = NEW_psPath;
	NEW_ai_node_t *next = NULL;
	for (;node;node=next)  {
		next = node->psNext;
		delete node;
		node = NULL;
	}

	NEW_psPath = NULL;
}

////////////////////
// Creates the path
float fLastCreated = -9999;
int CWorm::NEW_AI_CreatePath(CMap *pcMap)
{
	// Don't create the path so often!
	if (tLX->fCurTime - fLastCreated <= 2.0f)  {
		fLastCreated = tLX->fCurTime;
		return false;
	}

	CVec trg = AI_GetTargetPos();
	CVec pos = vPos;

	iProcessedNodes = 0;
	bPathFinished = true;

/*#ifdef _AI_DEBUG
	SDL_Surface *bmpDest = pcMap->GetDebugImage();
	if (bmpDest)
		DrawRectFill(bmpDest,0,0,bmpDest->w,bmpDest->h,MakeColour(255,0,255));
#endif*/


	NEW_AI_CleanupPath();
	//NEW_AI_ProcessPathNonRec(trg,pos,pcMap);
	NEW_AI_ProcessPath(trg,pos,pcMap);
	NEW_AI_SimplifyPath(pcMap);

#ifdef _AI_DEBUG
	NEW_AI_DrawPath(pcMap);
#endif

	// Set the current node to the beginning of the path
	NEW_psCurrentNode = NEW_psPath;

	return NEW_psPath != NULL;
}

//////////////////////
// Adds a new node
NEW_ai_node_t *CWorm::NEW_AI_AddNode(CVec Pos,NEW_ai_node_t *psPrev,NEW_ai_node_t *psNext)
{
	// Create the node
	NEW_ai_node_t *temp = new NEW_ai_node_t;
	if (!temp)
		return NULL;

	// Fill in the details
	temp->fX = Pos.GetX();
	temp->fY = Pos.GetY();
	temp->psPrev = psPrev;
	temp->psNext = psNext;

	if (psPrev)
		psPrev->psNext = temp;
	if (psNext)
		psNext->psPrev = temp;

	return temp;
}

int GetRockBetween(CVec pos,CVec trg, CMap *pcMap)
{
    assert( pcMap );

	int result = 0;

    // Trace a line from the worm to length or until it hits something
	CVec    dir = trg-pos;
    int     nTotalLength = (int)NormalizeVector(&dir);

	const int divisions = 4;			// How many pixels we go through each check (less = slower)

	int i;
	for(i=0; i<nTotalLength; i+=divisions) {
		uchar px = pcMap->GetPixelFlag( (int)pos.GetX(), (int)pos.GetY() );
		//pcMap->PutImagePixel((int)pos.GetX(), (int)pos.GetY(), MakeColour(255,0,0));

        if (px & PX_ROCK)
			result++;

        pos = pos + dir * (float)divisions;
    }

	return result;
}

CVec NEW_AI_FindBestSpot(CVec trg, CVec pos, CMap *pcMap)
{
	// Get the midpoint
	CVec middle = CVec((pos.GetX()+trg.GetX())/2,(pos.GetY()+trg.GetY())/2);
	
	float a = CalculateDistance(middle,pos);
	float b = a;
	float step = DEG2RAD(20);
	int min = 99999999;
	CVec result = CVec(0,0);

	// We make a circle and check from the points the way with least rock
	float i;
	float x,y;
	for (;b > 10.0f; b-=b/2)
		for (i=0;i<2*PI; i+=step)  {
			x = a*(float)sin(2*PI*i)+middle.GetX();
			y = b*(float)cos(2*PI*i)+middle.GetY();
			CVec point = CVec(x,y);
			if (pcMap->GetPixelFlag( (int)pos.GetX(), (int)pos.GetY() ) == PX_ROCK)
				continue;

			int rock_pixels = GetRockBetween(point,pos,pcMap)+GetRockBetween(point,trg,pcMap);
			if (rock_pixels < min)  {
				min = rock_pixels;
				result.SetX(point.GetX());
				result.SetY(point.GetY());

				if (!min)
					return result;
			}
		}

	return result;
}

///////////////////////
// Creates the path
void CWorm::NEW_AI_ProcessPathNonRec(CVec trg, CVec pos, CMap *pcMap)
{
	// How many times we've processed the path
	int Cycles = 0;

#ifdef _AI_DEBUG
	SDL_Surface *bmpDest = pcMap->GetDebugImage();
	if (!bmpDest)
		return;
	DrawRectFill(bmpDest,0,0,bmpDest->w,bmpDest->h,MakeColour(255,0,255));
#endif

	// Add the start node to the path
	NEW_psPath = NEW_AI_AddNode(pos,NULL,NULL);
	if (!NEW_psPath)
		return;

	// Add the target node to the path
	// The target node will be the last node
	NEW_psLastNode = NEW_AI_AddNode(trg,NEW_psPath,NULL); 
	if (!NEW_psLastNode)
		return;

	// Points to the first node in the path that need to be processed
	NEW_ai_node_t *ptr = NEW_psPath;
	
	while (Cycles < 25) {
		// Check
		if (!ptr)
			break;
		if (!ptr->psNext)
			break;

		if (traceWormLine(CVec(ptr->psNext->fX,ptr->psNext->fY),CVec(ptr->fX,ptr->fY),pcMap))  {
			ptr = ptr->psNext;
			// Path found
			if (!ptr)
				break;
			if (!ptr->psNext)
				break;

			// The two nodes are visible from each other so we won't add a new node
			continue;
		}

		// Get the position of first two nodes in the temporary path
		CVec pos = CVec(ptr->fX,ptr->fY);
		CVec trg = CVec(ptr->psNext->fX,ptr->psNext->fY);

		// Get the midpoint
		//CVec middle = CVec((pos.GetX()+trg.GetX())/2,(pos.GetY()+trg.GetY())/2);
		//CVec dir = CVec(trg.GetY()-pos.GetY(),pos.GetX()-trg.GetX());

		// Get nearest free spot to the midpoint and create a new node there
		//CVec cNewNodePos1 = NEW_AI_FindClosestFreeSpotDir(middle,dir,pcMap,DIR_LEFT);
		//CVec cNewNodePos2 = NEW_AI_FindClosestFreeSpotDir(middle,dir,pcMap,DIR_RIGHT);
		//CVec cNewNodePos = NEW_AI_FindClosestFreeCell(middle,pcMap);
		CVec cNewNodePos = NEW_AI_FindBestSpot(pos,trg,pcMap);

	
		//
		// Now decide, which one of the two spots is better
		//

		/*CVec *cNewNodePos;

		// We can see the Node1 from the current end of the path, so it will be PROBABLY better
		if (traceWormLine(cNewNodePos1,CVec(ptr->fX,ptr->fY),pcMap))  {
			// If we can see also the target node, this is definitelly better spot
			if (traceWormLine(CVec(ptr->psNext->fX,ptr->psNext->fY),cNewNodePos1,pcMap))
				cNewNodePos = &cNewNodePos1;
			// If the second node can be also seen from the current end of the path, choose the one closer to the final target
			else if (traceWormLine(cNewNodePos2,CVec(ptr->fX,ptr->fY),pcMap))  {
				// Y distance only has bigger priority
				if (ptr->fY - ptr->psNext->fY > 20)  {
					if (cNewNodePos1.GetY() < cNewNodePos2.GetY())
						cNewNodePos = &cNewNodePos1;
					else
						cNewNodePos = &cNewNodePos2;
				}
				else  {
					if (CalculateDistance(cNewNodePos1,CVec(ptr->psNext->fX,ptr->psNext->fY)) < CalculateDistance(cNewNodePos2,CVec(ptr->psNext->fX,ptr->psNext->fY)))
						cNewNodePos = &cNewNodePos1;
					else
						cNewNodePos = &cNewNodePos2;
				}
			}
			// The Node1 is better
			else {
				cNewNodePos = &cNewNodePos1;
			}
		}
		// Node2 can be seen from the current end of the path and Node1 not, so Node2 is better
		else if (traceWormLine(cNewNodePos2,CVec(ptr->fX,ptr->fY),pcMap))  {
			cNewNodePos = &cNewNodePos2;
		}
		// None of the two nodes can be seen from the current end of the path, choose the one closer to final target
		else  {
			// Y distance only has bigger priority
			if (ptr->fY - ptr->psNext->fY > 20)  {
				if (cNewNodePos1.GetY() < cNewNodePos2.GetY())
					cNewNodePos = &cNewNodePos1;
				else
					cNewNodePos = &cNewNodePos2;
			}
			else  {
				if (CalculateDistance(cNewNodePos1,CVec(ptr->psNext->fX,ptr->psNext->fY)) < CalculateDistance(cNewNodePos2,CVec(ptr->psNext->fX,ptr->psNext->fY)))
					cNewNodePos = &cNewNodePos1;
				else
					cNewNodePos = &cNewNodePos2;
			}
		}*/




#ifdef _AI_DEBUG
		DrawRectFill(bmpDest,(int)cNewNodePos.GetX()*2-8,(int)cNewNodePos.GetY()*2-8,(int)cNewNodePos.GetX()*2+8,(int)cNewNodePos.GetY()*2+8,MakeColour(0,0,255));
		tLX->cFont.DrawCentre(bmpDest,(int)cNewNodePos.GetX()*2,(int)cNewNodePos.GetY()*2-8,0xffff,"%i",Cycles);
#endif

		// Add the node to the path
		if (!NEW_AI_AddNode(cNewNodePos,ptr,ptr->psNext))
			break;

		Cycles++;
	}

}

//////////////////
// Finds the closest free spot, looking only in one direction
CVec CWorm::NEW_AI_FindClosestFreeSpotDir(CVec vPoint, CVec vDirection, CMap *pcMap, int Direction = -1)
{
#ifdef _AI_DEBUG
//	SDL_Surface *bmpDest = pcMap->GetDebugImage();
#endif

	NormalizeVector(&vDirection);
	//CVec vDev = CVec(vDirection.GetX()*5,vDirection.GetY()*5);
	CVec vDev = CVec(0,0);

	int i;
	int emptyPixels = 0;
	CVec pos = vPoint+vDev;
	int firstClosest = 9999;
	int secondClosest = 9999;
	CVec rememberPos1 = CVec(0,0);
	CVec rememberPos2 = CVec(0,0);

	for(i=0; 1; i++) {
		uchar px = pcMap->GetPixelFlag( (int)pos.GetX(), (int)pos.GetY() );

		// Empty pixel? Add it to the count
		if(!(px & PX_ROCK)) {
#ifdef _AI_DEBUG
			//PutPixel(bmpDest,pos.GetX()*2,pos.GetY()*2,MakeColour(255,255,0));
#endif
			emptyPixels++;
		}
		// Rock pixel? This spot isn't good
		else {
#ifdef _AI_DEBUG
			//PutPixel(bmpDest,pos.GetX()*2,pos.GetY()*2,MakeColour(255,0,0));
#endif
			if (emptyPixels >= 10)
				break;
			emptyPixels = 0;
			rememberPos1.SetX(0);
			rememberPos1.SetY(0);
		}

		// Good spot
		if (emptyPixels >= 10)  {
			firstClosest = i-emptyPixels;
			if (emptyPixels >= 30)
				break;
			rememberPos1.SetX(rememberPos1.GetX()+vDirection.GetX());
			rememberPos1.SetY(rememberPos1.GetY()+vDirection.GetY());
		}

		if (emptyPixels == 5)  {
			rememberPos1.SetX(pos.GetX());
			rememberPos1.SetY(pos.GetY());
		}

		pos = pos + vDirection;
		// Clipping
		if (pos.GetX() > pcMap->GetWidth() || pos.GetX() < 0)
			break;
		if (pos.GetY() > pcMap->GetHeight() || pos.GetY() < 0)
			break;
	}

	if (firstClosest != 9999 && Direction == DIR_LEFT)
		return rememberPos1;

	emptyPixels = 0;
	vDirection.SetY(-vDirection.GetY());
	vDirection.SetX(-vDirection.GetX());
	vDev.SetX(-vDev.GetX());
	vDev.SetY(-vDev.GetY());
	pos = vPoint+vDev;

	for(i=0; 1; i++) {
		uchar px = pcMap->GetPixelFlag( (int)pos.GetX(), (int)pos.GetY() );

		// Empty pixel? Add it to the count
		if(!(px & PX_ROCK)) {
#ifdef _AI_DEBUG
			//PutPixel(bmpDest,pos.GetX()*2,pos.GetY()*2,MakeColour(255,255,0));
#endif
			emptyPixels++;
		}
		// Rock pixel? This spot isn't good
		else {
#ifdef _AI_DEBUG
			//PutPixel(bmpDest,pos.GetX()*2,pos.GetY()*2,MakeColour(255,0,0));
#endif
			if (emptyPixels >= 10)  
				break;

			rememberPos2.SetX(0);
			rememberPos2.SetY(0);
			emptyPixels = 0;
		}

		// Good spot
		if (emptyPixels > 10)  {
			secondClosest = i-emptyPixels;
			if (emptyPixels >= 30)
				break;
			rememberPos2.SetX(rememberPos2.GetX()+vDirection.GetX());
			rememberPos2.SetY(rememberPos2.GetY()+vDirection.GetY());
		}

		// Remember this special position (in the middle of possible free spot)
		if (emptyPixels == 5)  {
			rememberPos2.SetX(pos.GetX());
			rememberPos2.SetY(pos.GetY());
		}

		pos = pos + vDirection;
		// Clipping
		if (pos.GetX() > pcMap->GetWidth() || pos.GetX() < 0)
			break;
		if (pos.GetY() > pcMap->GetHeight() || pos.GetY() < 0)
			break;
	}

	if (secondClosest != 9999 && Direction == DIR_RIGHT)
		return rememberPos2;

	// In what direction was the closest spot?
	if (firstClosest < secondClosest)
		return rememberPos1;
	else
		return rememberPos2;
}

///////////////////
// Process the path
void CWorm::NEW_AI_ProcessPath(CVec trg, CVec pos, CMap *pcMap)
{
	bPathFinished = true;
	// Too many recursions? End
	iProcessedNodes++;
	if (iProcessedNodes > 20)  {
		bPathFinished = false;
		return;
	}

	// Trivial task, end the recursion
	if(traceWormLine(trg,pos,pcMap))  {

		// Build the path from Start to Target and add the path to the global path
		NEW_ai_node_t *target = new NEW_ai_node_t;
		if (!target)
			return;
		NEW_ai_node_t *start = new NEW_ai_node_t;
		if (!start)
			return;

		start->fX = pos.GetX();
		start->fY = pos.GetY();
		start->psNext = target;
		start->psPrev = NULL;

		target->fX = trg.GetX();
		target->fY = trg.GetY();
		target->psNext = NEW_psPath;
		target->psPrev = start;

		if (NEW_psPath)
			NEW_psPath->psPrev = target;
		else
			NEW_psLastNode = target;

		NEW_psPath = start;

		return;
	}

	
	// The two nodes are not visible from each other

	// Get the midpoint
	CVec middle = CVec((pos.GetX()+trg.GetX())/2,(pos.GetY()+trg.GetY())/2);
	CVec dir = CVec(trg.GetY()-pos.GetY(),pos.GetX()-trg.GetX());

	// Get nearest free spot to the midpoint and create a new node there
	CVec cNewNodePos1 = NEW_AI_FindClosestFreeSpotDir(middle,dir,pcMap,DIR_LEFT);
	CVec cNewNodePos2 = NEW_AI_FindClosestFreeSpotDir(middle,dir,pcMap,DIR_RIGHT);
	CVec *cNewNodePos;

	if (GetRockBetween(pos,cNewNodePos1,pcMap)+GetRockBetween(trg,cNewNodePos1,pcMap) > GetRockBetween(pos,cNewNodePos2,pcMap)+GetRockBetween(trg,cNewNodePos2,pcMap))
		cNewNodePos = &cNewNodePos2;
	else
		cNewNodePos = &cNewNodePos1;

#ifdef _AI_DEBUG
	/*SDL_Surface *bmpDest = pcMap->GetDebugImage();
	if (bmpDest)  {
		if (x >= 0 && x*2 <= bmpDest->w)
			if (y >= 0 && y*2 <= bmpDest->h)  {
				DrawRectFill(bmpDest,x*2-4,y*2-4,x*2+4,y*2+4,MakeColour(0,255,0));
				if (pos.GetX()*2 < bmpDest->w && pos.GetY()*2 < bmpDest->h)
					DrawLine(bmpDest,pos.GetX()*2,pos.GetY()*2,x*2,y*2,0xffff);
				if (trg.GetX()*2 < bmpDest->w && trg.GetY()*2 < bmpDest->h)
					DrawLine(bmpDest,trg.GetX()*2,trg.GetY()*2,x*2,y*2,0xffff);
			}
		//DrawRectFill(bmpDest,(int)cNewNodePos.GetX()*2-4,(int)cNewNodePos.GetY()*2-4,(int)cNewNodePos.GetX()*2+4,(int)cNewNodePos.GetY()*2+4,MakeColour(0,255,0));
		//return;
	}*/

#endif

	// Must be in this order
	NEW_AI_ProcessPath(trg,*cNewNodePos,pcMap);  // From new node to the target
	NEW_AI_ProcessPath(*cNewNodePos,pos,pcMap);  // From the start to new node
}

////////////////
// Simplifies the path found by CreatePath
void CWorm::NEW_AI_SimplifyPath(CMap *pcMap)
{
	// No path
	if (!NEW_psPath)
		return;

	// Go through the path
	NEW_ai_node_t *node = NEW_psPath;
	for(;node;node=node->psNext)  {
		NEW_ai_node_t *closest_node = node->psNext;
		// Short path
		if (!closest_node)
			return;
		closest_node = closest_node->psNext;
		// Short path
		if (!closest_node)
			return;
		// While we see the two nodes, delete all nodes between them and skip to next node
		while (traceWormLine(CVec(closest_node->fX,closest_node->fY),CVec(node->fX,node->fY),pcMap))  {
			node->psNext = closest_node;
			delete closest_node->psPrev;
			closest_node->psPrev = node;
			closest_node=closest_node->psNext;
			if (!closest_node)
				break;
		}

		// test
		/*for (;closest_node;closest_node=closest_node->psNext)  {
			if (traceWormLine(CVec(closest_node->fX,closest_node->fY),CVec(node->fX,node->fY),pcMap))  {
				NEW_ai_node_t *next = NULL;
				NEW_ai_node_t *delete_node = node->psNext;
				for (;delete_node && delete_node != closest_node;delete_node = next)  {
					next = delete_node->psNext;
					delete delete_node;
				}
			}
			closest_node->psPrev = node;
			node->psNext = closest_node;

		}*/


	}

}

#ifdef _AI_DEBUG
///////////////////
// Draw the AI path
void CWorm::NEW_AI_DrawPath(CMap *pcMap)
{
	//return;
	if (!NEW_psPath)
		return;

	SDL_Surface *bmpDest = pcMap->GetDebugImage();
	if (!bmpDest)
		return;

	const int NodeColour = MakeColour(255,0,0);
	const int LineColour = 0xffff;
	const int transparent = MakeColour(255,0,255);

	DrawRectFill(bmpDest,0,0,bmpDest->w,bmpDest->h,transparent);

	// Go down the path
	NEW_ai_node_t *node = NEW_psPath;
	int node_x = 0;
	int node_y = 0;
	for (;node;node=node->psNext)  {

		// Get the node position
		node_x = Round(node->fX*2);
		node_y = Round(node->fY*2);

		// Clipping
		if (node_x-4 < 0 || node_x+4 > bmpDest->w)
			continue;
		if (node_y-4 < 0 || node_y+4 > bmpDest->h)
			continue;

		// Draw the node
		DrawRectFill(bmpDest,node_x-4,node_y-4,node_x+4,node_y+4,NodeColour);

		// Draw the line
		if (node->psPrev)
			DrawLine(bmpDest,MIN(Round(node->psPrev->fX*2),bmpDest->w),MIN(Round(node->psPrev->fY*2),bmpDest->h),node_x,node_y,LineColour);
	}
	
}
#endif

////////////////////
// Finds the nearest spot to the target, where the rope can be hooked
CVec CWorm::NEW_AI_GetNearestRopeSpot(CVec trg, CMap *pcMap)
{
	CVec dir = trg-vPos;
	NormalizeVector(&dir);
	dir = dir*10;
	while (CalculateDistance(vPos,trg) >= cNinjaRope.getRestLength()) 
		trg = trg-dir;

	//
	// Find the nearest cell with rock or dirt
	//

	// Get the current cell
	uchar tmp_pf = PX_ROCK;
	int cellX = (int) (trg.GetX())/pcMap->getGridWidth();
	int cellY = (int) (trg.GetY())/pcMap->getGridHeight();
	
	// Clipping means rock
	if (cellX > pcMap->getGridCols() || cellX < 0)
		return trg;
	if (cellY > pcMap->getGridRows() || cellY < 0)
		return trg;

	// Check the current cell first
	tmp_pf = *(pcMap->getGridFlags() + cellY*pcMap->getGridCols() +cellX);
	if (tmp_pf == PX_ROCK || tmp_pf == PX_DIRT)
		return trg;

	// Note: unoptimized

	const int numCells = pcMap->getGridCols() * pcMap->getGridRows();
	int i=1;
	int x,y;
	bool bFound = false;
	while (!bFound) {
		for (y=cellY-i;y<=cellY+i;y++)  {

			// Clipping means rock
			if (y > pcMap->getGridRows())  {
				bFound = true;
				break;
			}
			if (y < 0)  {
				bFound = true;
				break;
			}


			for (x=cellX-i;x<=cellX+i;x++)  {
				// Don't check the entry cell
				if (x == cellX && y == cellY)
					continue;

				// Clipping means rock
				if (x > pcMap->getGridCols())  {
					bFound = true;
					break;
				}
				if (x < 0)  {
					bFound = true;
					break;
				}

				// Get the pixel flag of the cell
				tmp_pf = *(pcMap->getGridFlags() + y*pcMap->getGridCols() +x);
				if (tmp_pf == PX_ROCK || tmp_pf == PX_DIRT)  {
					bFound = true;
					break;
				}	
			}
		}
		i++;
	}

	return CVec((float)x*pcMap->getGridWidth()+pcMap->getGridWidth()/2, (float)y*pcMap->getGridHeight()+pcMap->getGridHeight()/2);

}

/////////////////////
// Move to the target
float fLastCompleting = -9999;
void CWorm::NEW_AI_MoveToTarget(CMap *pcMap)
{
//	printf("Moving to target");

    worm_state_t *ws = &tState;

	// Better target?
	CWorm *newtrg = findTarget(iAiGame, iAiTeams, iAiTag, pcMap);
	if (psAITarget && newtrg)
		if (newtrg->getID() != psAITarget->getID())
			nAIState = AI_THINK;

	if (!psAITarget && newtrg)  {
		nAIState = AI_THINK;
	}

	// No target?
	if (nAITargetType == AIT_NONE || (nAITargetType == AIT_WORM && !psAITarget))  {
		nAIState = AI_THINK;
		return;
	}

    // Clear the state
	ws->iCarve = false;
	ws->iMove = false;
	ws->iShoot = false;
	ws->iJump = false;

    cPosTarget = AI_GetTargetPos();

    // If we're really close to the target, perform a more precise type of movement
    if(fabs(vPos.GetX() - cPosTarget.GetX()) < 20 && fabs(vPos.GetY() - cPosTarget.GetY()) < 20) {
        AI_PreciseMove(pcMap);
        return;
    }

    // If we're stuck, just get out of wherever we are
    if(bStuck) {
//		printf("Stucked");

        ws->iMove = true;
		if (tLX->fCurTime-fLastJump > 1.0f)  {
			ws->iJump = true;
			fLastJump = tLX->fCurTime;
		}
		// Try to unstuck
		if (cClient)
			cClient->FindNearestSpot(this);
        if(tLX->fCurTime - fStuckPause > 2)
            bStuck = false;
        return;
    }



    /*
      First, if the target or ourself has deviated from the path target & start by a large enough amount:
      Or, enough time has passed since the last path update:
      recalculate the path
    */
    int     nDeviation = 40;     // Maximum deviation allowance
    bool    recalculate = false;

	// If our or target's velocity is high, don't check so often
	if (psAITarget)  {
		CVec *vel = psAITarget->getVelocity();
		if (vel) {
			float len = VectorLength(*vel);
			if (len > 30)
				nDeviation = (int)len*5;
		}

		vel = getVelocity();
		if (vel) {
			float len = VectorLength(*vel);
			if (len > 30)
				nDeviation = (int)len*5;
		}
	}
    
	// Check
	if (!NEW_psPath || !NEW_psLastNode)  {
		printf("Pathfinding problem 1; ");
		return;
	}

	// Deviated?
	if(fabs(NEW_psPath->fX-vPos.GetX()) > nDeviation || fabs(NEW_psPath->fY-vPos.GetY()) > nDeviation)
		recalculate = true;

	if(fabs(NEW_psLastNode->fX-cPosTarget.GetX()) > nDeviation || fabs(NEW_psLastNode->fY-cPosTarget.GetY()) > nDeviation)
		recalculate = true;

    // Re-calculate the path?
    if(recalculate)
        NEW_AI_CreatePath(pcMap);

	// If the CreatePath hasn't created whole path, we'll try to finish it
/*	if (!bPathFinished && !recalculate && (tLX->fCurTime-fLastCompleting <= 0.2f))  {
		fLastCompleting = tLX->fCurTime;
		NEW_AI_ProcessPath(CVec(NEW_psPath->fX,NEW_psPath->fY),vPos,pcMap);
		if (!NEW_psPath || !NEW_psLastNode)
			return;
#ifdef _AI_DEBUG
		NEW_AI_DrawPath(pcMap);
#endif
	}*/

    /*
      Move through the path.
      We have a current node that we must get to. If we go onto the node, we go to the next node, and so on.
      Because the path finding isn't perfect we can sometimes skip a whole group of nodes.
      If we are close to other nodes that are _ahead_ in the list, we can skip straight to that node.
    */

	//return;

    if(NEW_psPath == NULL) {
        // If we don't have a path, resort to simpler AI methods
        AI_SimpleMove(pcMap,psAITarget != NULL);
		printf("Pathfinding problem 2; ");
        return;
    }

//	printf("We should move now...");

	// Get the target node position
    CVec nodePos = CVec(NEW_psCurrentNode->fX,NEW_psCurrentNode->fY);


    /*
      Now that we've done all the boring stuff, our single job here is to reach the node.
      We have walking, jumping, move-through-air, and a ninja rope to help us.
    */   

    // Aim at the node
    bool aim = AI_SetAim(nodePos);

    // If we are stuck in the same position for a while, take measures to get out of being stuck
    if(fabs(cStuckPos.GetX() - vPos.GetX()) < 25 && fabs(cStuckPos.GetY() - vPos.GetY()) < 25) {
        fStuckTime += tLX->fDeltaTime;

        // Have we been stuck for a few seconds?
        if(fStuckTime > 3) {
            // Jump, move, switch directions and release the ninja rope
			if (tLX->fCurTime-fLastJump > 1.0f)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			}
            ws->iMove = true;
            bStuck = true;
            fStuckPause = tLX->fCurTime;

			// Try to use unstuck command
			if (cClient)
				cClient->FindNearestSpot(this);

			iDirection = !iDirection;
            
            fAngle -= 45;
            // Clamp the angle
	        fAngle = MIN((float)60,fAngle);
	        fAngle = MAX((float)-90,fAngle);

            // Recalculate the path
            AI_InitMoveToTarget(pcMap);
            fStuckTime = 0;
        }
    }
    else {
        bStuck = false;
        fStuckTime = 0;
        cStuckPos = vPos;
    }


    // If the ninja rope hook is falling, release it & walk
    if(!cNinjaRope.isAttached() && !cNinjaRope.isShooting()) {
        cNinjaRope.Release();
        ws->iMove = true;
    }

	// If the rope is hooked wrong, release it
	/*if (cNinjaRope.isAttached())  {
		if (cNinjaRope.getHookPos().GetX()+20 < vPos.GetX() && cNinjaRope.getHookPos().GetX()+20 < NEW_psCurrentNode->fX)
			cNinjaRope.Release();
		else if (cNinjaRope.getHookPos().GetX()-20 > vPos.GetX() && cNinjaRope.getHookPos().GetX()-20 > NEW_psCurrentNode->fX)
			cNinjaRope.Release();
	}*/

    // Walk only if the target is a good distance on either side
    if(fabs(vPos.GetX() - NEW_psCurrentNode->fX) > 30)
        ws->iMove = true;

	// If the node is above us by a little, jump
	if ((vPos.GetY()-NEW_psCurrentNode->fY) <= 20 && (vPos.GetY()-NEW_psCurrentNode->fY) > 0)
		// Don't jump so often
		if (tLX->fCurTime - fLastJump > 1.0f)  {
			ws->iJump = true;
			fLastJump = tLX->fCurTime;
		}

	// If the next node is above us by a little, jump too
	NEW_ai_node_t *nextNode = NEW_psCurrentNode->psNext;
	if (nextNode)  {
		if ((vPos.GetY()-nextNode->fY) <= 30 && (vPos.GetY()-nextNode->fY) > 0)
			// Don't jump so often
			if (tLX->fCurTime - fLastJump > 1.0f)  {
				ws->iJump = true;
				fLastJump = tLX->fCurTime;
			}
	}
    
    // If the node is above us by a lot, we should use the ninja rope
	// If the node is far, use the rope, too
    if(NEW_psCurrentNode->fY+20 < vPos.GetY()/* || (fabs(NEW_psCurrentNode->fX-vPos.GetX()) >= 50)*/) {
        
        bool fireNinja = true;

		CVec cAimPos = CVec(NEW_psCurrentNode->fX,NEW_psCurrentNode->fY);

		// If the path is going up, get an average position of the two nodes
		if (vPos.GetY() > NEW_psCurrentNode->fY) 
			if (NEW_psCurrentNode->psNext) {
				if (NEW_psCurrentNode->fY-20 > NEW_psCurrentNode->psNext->fY)  {
					cAimPos.SetX((NEW_psCurrentNode->fX+NEW_psCurrentNode->psNext->fX)/2);
					cAimPos.SetY((NEW_psCurrentNode->fY+NEW_psCurrentNode->psNext->fY)/2);
				}
			}

		cAimPos = NEW_AI_GetNearestRopeSpot(cAimPos,pcMap);


		// Aim
		AI_SetAim(cAimPos);


        CVec dir;
        dir.SetX( (float)cos(fAngle * (PI/180)) );
	    dir.SetY( (float)sin(fAngle * (PI/180)) );
	    if(iDirection == DIR_LEFT)
		    dir.SetX(-dir.GetX());
       
        /*
          Got aim, so shoot a ninja rope
          We shoot a ninja rope if it isn't shot
          Or if it is, we make sure it has pulled us up and that it is attached
        */
        if(fireNinja) {
            if(!cNinjaRope.isReleased())
                cNinjaRope.Shoot(vPos,dir);
            else {
                float length = CalculateDistance(vPos, cNinjaRope.getHookPos());
                if(cNinjaRope.isAttached()) {
                    if(length < cNinjaRope.getRestLength() && vVelocity.GetY()<-10)
                        cNinjaRope.Shoot(vPos,dir);
                }
            }
        }
    }    

    /*
      If there is dirt between us and the next node, don't shoot a ninja rope
      Instead, carve
    */
    float traceDist = -1;
    int type = 0;
	if (!NEW_psCurrentNode)
		return;
    CVec v = CVec(NEW_psCurrentNode->fX, NEW_psCurrentNode->fY);
    int length = traceLine(v, pcMap, &traceDist, &type);
    float dist = CalculateDistance(v, vPos);
    if(length < dist && type == PX_DIRT) {
		cNinjaRope.Release();
        ws->iJump = true;
        ws->iMove = true;
		// Don't carve so fast!
		if (tLX->fCurTime-fLastCarve > 0.2f)  {
			fLastCarve = tLX->fCurTime;
			ws->iCarve = true; // Carve
			if (NEW_psCurrentNode->fY < vPos.GetY())
				ws->iJump = true;
		}
		else  {
			ws->iCarve = false;
			ws->iJump = false;
		}

		// If the node is right above us, use a carving weapon
		if (v.GetX()-20 <= vPos.GetX() && v.GetX()+20 >= vPos.GetX()) 
			if (v.GetY() < vPos.GetY())  {
				int wpn;
				if((wpn = AI_FindClearingWeapon()) != -1) {
					iCurrentWeapon = wpn;
					ws->iShoot = true;
				}
			}
    }



    // If we're above the node, let go of the rope and move towards to node
    if(NEW_psCurrentNode->fY >= vPos.GetY()) {
        // Let go of any rope
        cNinjaRope.Release();

        // Walk in the direction of the node
        ws->iMove = true;
    }

   
	// Move to next node
	if(CalculateDistance(vPos,CVec(NEW_psCurrentNode->fX,NEW_psCurrentNode->fY)) < 10)
		if (NEW_psCurrentNode->psNext)
			NEW_psCurrentNode = NEW_psCurrentNode->psNext;
	
}

