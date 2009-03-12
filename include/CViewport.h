/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Viewport class
// Created 21/1/02
// Jason Boettcher


#ifndef __CVIEWPORT_H__
#define	__CVIEWPORT_H__

#include <SDL.h>

#include "CInput.h"
#include "CVec.h"
#include "Options.h" // for control_t
#include "SmartPointer.h"


// Viewport types
// !!! Don't change the order !!!
enum {
    VW_FOLLOW = 0,
    VW_CYCLE,
    VW_FREELOOK,
    VW_ACTIONCAM
};


class CWorm;


class CViewport {
public:
	// Constructor
	CViewport() {

		bUsed = false;
        nID = 0;
		Left = Top = 0;
		Width = 320;
		Height = 280;
		VirtWidth = 640;
		VirtHeight = 480;
		WorldX = WorldY = 0;

		bShaking = false;
		iShakeAmount = 0;

        pcTargetWorm = NULL;
        nType = VW_FOLLOW;
        fTimer = AbsTime();
		bSmooth = false;		
	}

private:
	// Attributes

	bool	bUsed;
    int     nID;
	int		Left,Top;
	int		Width,Height;

	int		VirtWidth, VirtHeight;

	int  	WorldX, WorldY;

    CVec    curPos, tgtPos;

	AbsTime	fShakestart;
	bool	bShaking;
	int		iShakeAmount;

    int     nType;
    CWorm   *pcTargetWorm;

    AbsTime   fTimer;

	// HINT: we have to use pointers here as we have CViewport in a global variable
    SmartPointer<CInput>  cUp, cRight, cDown, cLeft;
	
	bool	bSmooth;
	CVec	cSmoothVel, cSmoothAccel;


public:
	// Methods

	void	Setup(int l, int t, int vw, int vh, int type);

	void	Process(CWorm *pcWormList, CViewport *pcViewList, int MWidth, int MHeight, int iGameMode);
	void	Clamp(int MWidth, int MHeight);
	void	ClampFiltered(int MWidth, int MHeight);

	bool	inView(CVec pos);	
	void	Shake(int amount);

    void    setupInputs(const controls_t& Inputs);

    void    reset(void);

    CWorm   *findTarget(CWorm *pcWormList, CViewport *pcViewList, bool bAlive);

	SDL_Rect getRect(void);
	
	void	setSmoothPosition( float X, float Y, TimeDiff DeltaTime );

	//
	// Variables
	//
	int		GetLeft(void)		{ return Left; }
	int		GetTop(void)		{ return Top; }
	int		GetWidth(void)		{ return Width; }
	int		GetHeight(void)		{ return Height; }
	int		GetWorldX(void)		{ return WorldX; }
	int		GetWorldY(void)		{ return WorldY; }
	int		GetVirtW(void)		{ return VirtWidth; }
	int		GetVirtH(void)		{ return VirtHeight; }

	void	SetLeft(int _l)		{ Left = _l; }
	void	SetTop(int _t)		{ Top = _t; }
	void	SetVirtWidth(int _w)	{ Width = _w/2; VirtWidth = _w; }
	void	SetVirtHeight(int _h)	{ Height = _h/2; VirtHeight = _h; }
	void	SetWorldX(int _x)	{ WorldX = _x; }
	void	SetWorldY(int _y)	{ WorldY = _y; }

	bool	getUsed(void)		{ return bUsed; }
	void	setUsed(bool _u)	{ bUsed = _u; }

    void    setTarget(CWorm *w) { pcTargetWorm = w; }
    CWorm   *getTarget(void)    { return pcTargetWorm; }

    int     getType(void)       { return nType; }
	void	setType(int _t)		{ nType = _t; }

    void    setID(int id)       { nID = id; }

    void    setSmooth(bool _b);

};



#endif  //  __CVIEWPORT_H__
