/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////



#ifndef __CWORMBOT_H__
#define __CWORMBOT_H__

#include "CWorm.h"
#include "game/WormInputHandler.h"

class searchpath_base;

class CWormBotInputHandler : public CWormInputHandler {
public:
	CWormBotInputHandler(CWorm* w);
	virtual ~CWormBotInputHandler();
	
	virtual std::string name() { return "Bot input handler"; }
	
	virtual void initWeaponSelection();
	virtual void doWeaponSelectionFrame(SDL_Surface * bmpDest, CViewport *v);
	
	// simulation
	virtual void startGame();
	virtual void getInput();
    virtual void clearInput() {}

	virtual void onRespawn();

	virtual void quit();
	
protected:
	
    /*
	 Artificial Intelligence
	 */
    int         nAIState;
    int         nAITargetType;
    CWorm       *psAITarget;
    CBonus      *psBonusTarget;
    CVec        cPosTarget;
    int         nPathStart[2];
    int         nPathEnd[2];
    CVec        cStuckPos;
    TimeDiff       fStuckTime;
    bool        bStuck;
    AbsTime       fStuckPause;
    AbsTime       fLastThink;
    CVec        cNinjaShotPos;
	AbsTime		fLastFace;
	TimeDiff		fBadAimTime;
    int			iAiGameType; // AI game type, we have predefined behaviour for mostly played settings
	int			iAiDiffLevel;
	int			iRandomSpread;
	CVec		vLastShootTargetPos;
	
	AbsTime		fLastShoot;
	AbsTime		fLastJump;
	AbsTime		fLastWeaponChange;
	AbsTime		fLastCreated;
	AbsTime		fLastCompleting;
	AbsTime		fLastRandomChange;
	AbsTime		fLastGoBack;
	
	float		fCanShootTime;
	
	TimeDiff		fRopeAttachedTime;
	TimeDiff		fRopeHookFallingTime;
	
    // Path Finding
	AbsTime       fLastPathUpdate;
	bool		bPathFinished;
	AbsTime		fSearchStartTime;
	
	searchpath_base*	pathSearcher;
	
	NEW_ai_node_t	*NEW_psPath;
	NEW_ai_node_t	*NEW_psCurrentNode;
	NEW_ai_node_t	*NEW_psLastNode;
	
	
public:
	
	
    //
    // AI
    //
    bool        AI_Initialize();
    void        AI_Shutdown();
	
	void		AI_Respawn();
    void        AI_Think();
	bool		AI_FindHealth();
    bool        AI_FindBonus(int bonustype);
    bool        AI_SetAim(CVec cPos);
    CVec        AI_GetTargetPos();
	
    void        AI_InitMoveToTarget();
    void        AI_SimpleMove(bool bHaveTarget=true);
	//    void        AI_PreciseMove();
	
    void		AI_splitUpNodes(NEW_ai_node_t* start, NEW_ai_node_t* end);
    void		AI_storeNodes(NEW_ai_node_t* start, NEW_ai_node_t* end);
	
    int         AI_FindClearingWeapon();
    bool        AI_Shoot();
    int         AI_GetBestWeapon(int iGameMode, float fDistance, bool bDirect, float fTraceDist);
    void        AI_ReloadWeapons();
    int         cycleWeapons();
	void		AI_SetGameType(int type)  { iAiGameType = type; }
	int			AI_GetGameType()  { return iAiGameType; }
	
	void		setAiDiff(int aiDiff);
	
	bool		findNewTarget(); // uses findTarget() or searches for a position; true iff we have target or we can stay
	bool		findRandomSpot(bool highSpot = false);
    CWorm       *findTarget(); // for games where we have to kill other people
	CWorm*		nearestEnemyWorm();
	bool		weaponCanHit(int gravity,float speed, CVec cTrgPos);
	int			traceWeaponLine(CVec target, float *fDist, int *nType);
	
	
	CVec		AI_GetBestRopeSpot(CVec trg);
	CVec		AI_FindClosestFreeCell(CVec vPoint);
	bool		AI_CheckFreeCells(int Num);
	bool		AI_IsInAir(CVec pos, int area_a=3);
	CVec		AI_FindClosestFreeSpotDir(CVec vPoint, CVec vDirection, int Direction);
	int			AI_CreatePath(bool force_break = false);
	void		AI_MoveToTarget();
	void		AI_Carve();
	bool		AI_Jump();
	CVec		AI_FindShootingSpot();
	int			AI_GetRockBetween(CVec pos,CVec trg);
#ifdef _AI_DEBUG
	void		AI_DrawPath();
#endif
	
	
	// ----------- Gusanos ---------------

	virtual void subThink();

};

#endif  //  __CWORMBOT_H__
