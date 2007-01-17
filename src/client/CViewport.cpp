/////////////////////////////////////////
//
//                  LieroX
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Viewport class
// Created 22/1/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


///////////////////
// Setup the viewport
void CViewport::Setup(int l, int t, int vw, int vh, int type)
{
	Left = l;
	Top = t;
	VirtWidth = vw;
	VirtHeight = vh;

	Width = vw/2;
	Height = vh/2;

    pcTargetWorm = NULL;
    nType = type;
}


///////////////////
// Setup the keyboard inputs for freelook
void CViewport::setupInputs(char Inputs[32][8])
{
    cUp.Setup(		Inputs[SIN_UP] );
	cDown.Setup(	Inputs[SIN_DOWN] );
	cLeft.Setup(	Inputs[SIN_LEFT] );
	cRight.Setup(	Inputs[SIN_RIGHT] );
}


///////////////////
// Process a viewport
void CViewport::Process(CWorm *pcWormList, CViewport *pcViewList, int MWidth, int MHeight, int nGameType)
{
    float hx = (float) Width/2;
	float hy = (float) Height/2;

    // Follow a player
    if( nType == VW_FOLLOW ) {
	    // The player will ideally be in the centre of the viewport
	    // The viewport will then rest against the edges if that cannot happen

        // If we have no target at all, find one
        if( !pcTargetWorm ) {
            // Try and find a living worm first
            CWorm *t = findTarget(pcWormList, pcViewList, true);

            // If no living worms, try a dead worm (but still in the game)
            if(!t)
                t = findTarget(pcWormList, pcViewList, false);

            if(t) {
                pcTargetWorm = t;
                fTimer = -1;
            } else {
                // If we didn't find a new worm, go into freelook mode                    
                pcTargetWorm = NULL;
                nType = VW_FREELOOK;
                fTimer = -1;
                return;
            }
        }


        // Check if the target is out of the game
        if( pcTargetWorm ) {
            if( pcTargetWorm->getLives() == WRM_OUT ) {
                // Setup the timer to wait 2.5 seconds before changing targets
                if( fTimer == -1 )
                    fTimer = tLX->fCurTime + 2.5f;

                // Time up? Change targets
                if( tLX->fCurTime > fTimer ) {

                    // Try and find a living worm first
                    CWorm *t = findTarget(pcWormList, pcViewList, true);

                    // If no living worms, try a dead worm (but still in the game)
                    if(!t)
                        t = findTarget(pcWormList, pcViewList, false);

                    if(t) {
                        pcTargetWorm = t;
                        fTimer = -1;
                    } else {

                        // If we didn't find a new worm, go into freelook mode                    
                        pcTargetWorm = NULL;
                        nType = VW_FREELOOK;
                        fTimer = -1;
                        return;
                    }
                }
            }
        }
	    
        // Follow the worm
		// TODO: fix for left viewport in splitscreen (the worm shakes there)
        if( pcTargetWorm ) {
            if( pcTargetWorm->getAlive() ) {
	            WorldX = (int)floor(pcTargetWorm->getPos().x-hx);
	            WorldY = Round(pcTargetWorm->getPos().y-hy);
										

                // Clear the timer
                fTimer = -1;
            }
        }
    }


    // Cycle
    if( nType == VW_CYCLE ) {
        // Cycles through players. If a player dies (but not necessarily out), move onto another living player

        // Check if the target is out of the game or has died
        if( pcTargetWorm ) {
            if( pcTargetWorm->getLives() == WRM_OUT || !pcTargetWorm->getAlive() ) {
                // Setup the timer to wait 0.5 seconds before changing targets
                if( fTimer == -1 )
                    fTimer = tLX->fCurTime + 0.5f;

                // Time up? Change targets
                if( tLX->fCurTime > fTimer ) {

                    // Try and find a living worm first
                    CWorm *t = findTarget(pcWormList, pcViewList, true);

                    // If no living worms, try a dead worm (but still in the game)
                    if(!t)
                        t = findTarget(pcWormList, pcViewList, false);

                    if(t) {
                        pcTargetWorm = t;
                        fTimer = -1;
                    } else {

                        // If we didn't find a new worm, go into freelook mode
                        pcTargetWorm = NULL;
                        nType = VW_FREELOOK;
                        fTimer = -1;
                        return;
                    }
                }
            }
        }

        // Follow the worm
        if( pcTargetWorm ) {
            if( pcTargetWorm->getAlive() ) {
	            WorldX = (int)floor(pcTargetWorm->getPos().x-hx);
	            WorldY = Round(pcTargetWorm->getPos().y-hy);

                // Clear the timer
                fTimer = -1;
            }
        }
    }


    // Action
    if( nType == VW_ACTIONCAM ) {
        int i,j;
        // Finds a group of worms and smoothly moves around to show the whole group of players in a fight

        pcTargetWorm = NULL;
        
        // Generate a score for each worm depending on closeness of other worms
        // The worm with the lowest score is used as a focus point        
        float fScores[MAX_WORMS];
        for(i=0; i<MAX_WORMS; i++)
            fScores[i] = -1;
        
        // Go through all permutations of the worms
        for(i=0; i<MAX_WORMS; i++) {
            if( !pcWormList[i].isUsed() || !pcWormList[i].getAlive() || pcWormList[i].getLives() == WRM_OUT )
                continue;
            
            // Set a zero score because we are at least alive and well
            fScores[i] = 0;
            
            for(j=0; j<MAX_WORMS; j++) {
                if( !pcWormList[j].isUsed() || !pcWormList[j].getAlive() || pcWormList[j].getLives() == WRM_OUT )
                    continue;
                
                fScores[i] += CalculateDistance(pcWormList[i].getPos(), pcWormList[j].getPos());
            }
        }
        
        CWorm *pcFocus = NULL;
        float lowest = 99999;
        
        for(i=0; i<MAX_WORMS; i++) {
            if( !pcWormList[i].isUsed() || !pcWormList[i].getAlive() || pcWormList[i].getLives() == WRM_OUT )
                continue;
            
            if( fScores[i] < lowest ) {
                pcFocus = &pcWormList[i];
                lowest = fScores[i];
            }
        }
        
        // No focus point? Leave
        if( !pcFocus )
            return;
        
        // The focus worm is our target
        tgtPos = pcFocus->getPos();
        
        CVec dir = tgtPos - curPos;
        float l = NormalizeVector(&dir);
        
        float speed = l*2.5f;
        speed = MIN((float)300,speed);
        
        if( l > 2 )
            curPos += dir*speed * tLX->fDeltaTime;
        
        WorldX = (int)floor(curPos.x-hx);
        WorldY = Round(curPos.y-hy);
    }


    // Free look
    if( nType == VW_FREELOOK ) {
        float scrollSpeed = 300.0f*tLX->fDeltaTime;

        // Uses the players keys to scroll around
        if( cUp.isDown() )
            curPos -= CVec(0,scrollSpeed);
        if( cDown.isDown() )
            curPos += CVec(0,scrollSpeed);
        if( cLeft.isDown() )
            curPos -= CVec(scrollSpeed,0);
        if( cRight.isDown() )
            curPos += CVec(scrollSpeed,0);

        // Clamp our movement
        curPos.x=( MAX((float)0,curPos.x) );
        curPos.y=( MAX((float)0,curPos.y) );
        curPos.x=( MIN((float)MWidth-Width,curPos.x) );
        curPos.y=( MIN((float)MHeight-Height,curPos.y) );
        
        WorldX = (int)curPos.x;
        WorldY = (int)curPos.y;
    }


	// Shake the viewport a bit
	if(iShaking) {
		if(tLX->fCurTime - fShakestart > 0.2f) {
			iShaking = false;
			iShakeAmount = 0;
		}
		else {

            // Don't shake the action/freelook cam
            if( nType != VW_ACTIONCAM && nType != VW_FREELOOK ) {

                // Clamp it to the edges, then shake. So we can still see shaking near edges
                Clamp(MWidth, MHeight);

			    // Shake
			    WorldX += (int)(GetRandomNum() * (float)iShakeAmount);
			    WorldY += (int)(GetRandomNum() * (float)iShakeAmount);
            }
		}
	}

	// Clamp it
	Clamp(MWidth, MHeight);
}


///////////////////
// Find a target worm
CWorm *CViewport::findTarget(CWorm *pcWormList, CViewport *pcViewList, bool bAlive)
{
    // Find a worm that isn't already a target by another viewport
    for( int w=0; w<MAX_WORMS; w++ ) {
        if( !pcWormList[w].isUsed() )
            continue;
        if( pcWormList[w].getLives() == WRM_OUT )
            continue;

        // If the worm isn't alive, and we want a living worm, skip the worm
        if( !pcWormList[w].getAlive() && bAlive )
            continue;

        int viewcount = 0;
        for( int v=0; v<NUM_VIEWPORTS; v++ ) {

            // Make sure this isn't our viewport
            if( pcViewList[v].nID == nID )
                continue;

            CWorm *t = pcViewList[v].getTarget();
            if(t) {
                if( pcWormList[w].getID() == t->getID() )
                    viewcount++;
            }
        }

        // If this worm was in none of the viewports, use the worm
        if( viewcount == 0 )
            return &pcWormList[w];
    }

    // No good target
    return NULL;
}


///////////////////
// Resets the viewport simulations (timer, movement, etc)
void CViewport::reset(void)
{
    fTimer = -1;
}


///////////////////
// Clamp the viewport if it exceeds any boundaries
void CViewport::Clamp(int MWidth, int MHeight)
{
	WorldX = MAX(0,WorldX);
	WorldY = MAX(0,WorldY);

	WorldX = MIN(WorldX,MWidth-Width);
	WorldY = MIN(WorldY,MHeight-Height);
}


///////////////////
// Clamp the viewport if it exceeds any boundaries (for filtered drawing)
void CViewport::ClampFiltered(int MWidth, int MHeight)
{
	// Note: Filtered drawing of the map blurs between pixels so we need to clamp the max size by 2
	WorldX = MAX(0,WorldX);
	WorldY = MAX(0,WorldY);

	WorldX = MIN(WorldX,MWidth-Width-2);
	WorldY = MIN(WorldY,MHeight-Height-2);
}


///////////////////
// Get the rectangle of the viewport
SDL_Rect CViewport::getRect(void)
{
	SDL_Rect r;

	r.x = Left;
	r.y = Top;
	r.w = VirtWidth;
	r.h = VirtHeight;

	return r;
}


///////////////////
// Shake the viewport
void CViewport::Shake(int amount)
{
	fShakestart = tLX->fCurTime;
	iShaking = true;
	if(amount > iShakeAmount)
		iShakeAmount = amount;
}


///////////////////
// Check if a point is inside this viewport
int CViewport::inView(CVec pos)
{
	int x = (int)pos.x;
	int y = (int)pos.y;

	if(x >= WorldX &&
	   y >= WorldY &&
	   x <= WorldX+Width &&
	   y <= WorldY+Height)
	   return true;

	return false;
}
