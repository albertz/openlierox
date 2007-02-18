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

		iUsed = false;
        nID = 0;
		Left = Top = 0;
		Width = 320;
		Height = 280;
		VirtWidth = 640;
		VirtHeight = 480;
		WorldX = WorldY = 0;

		iShaking = false;
		iShakeAmount = 0;

        pcTargetWorm = NULL;
        nType = VW_FOLLOW;
        fTimer = -1;
	}

private:
	// Attributes

	int		iUsed;
    int     nID;
	int		Left,Top;
	int		Width,Height;

	int		VirtWidth, VirtHeight;

	int  	WorldX, WorldY;

    CVec    curPos, tgtPos;

	float	fShakestart;
	int		iShaking;
	int		iShakeAmount;

    int     nType;
    CWorm   *pcTargetWorm;

    float   fTimer;

    CInput  cUp, cRight, cDown, cLeft;




public:
	// Methods

	void	Setup(int l, int t, int vw, int vh, int type);

	void	Process(CWorm *pcWormList, CViewport *pcViewList, int MWidth, int MHeight, int nGameType);
	void	Clamp(int MWidth, int MHeight);
	void	ClampFiltered(int MWidth, int MHeight);

	int		inView(CVec pos);	
	void	Shake(int amount);

	// TODO: this better
    void    setupInputs(std::string Inputs[32]);

    void    reset(void);

    CWorm   *findTarget(CWorm *pcWormList, CViewport *pcViewList, bool bAlive);

	SDL_Rect getRect(void);


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

	void	SetLeft(int _l)		{ Left=_l; }
	void	SetTop(int _t)		{ Top=_t; }
	void	SetWorldX(int _x)	{ WorldX = _x; }
	void	SetWorldY(int _y)	{ WorldY = _y; }

	int		getUsed(void)		{ return iUsed; }
	void	setUsed(int _u)		{ iUsed = _u; }

    void    setTarget(CWorm *w) { pcTargetWorm = w; }
    CWorm   *getTarget(void)    { return pcTargetWorm; }

    int     getType(void)       { return nType; }

    void    setID(int id)       { nID = id; }
	


};



#endif  //  __CVIEWPORT_H__
