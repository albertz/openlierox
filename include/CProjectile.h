/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Projectile class
// Created 26/6/02
// Jason Boettcher


#ifndef __CPROJECTILE_H__
#define __CPROJECTILE_H__


#define		MAX_PROJECTILES	3000

#define		COL_TOP		    0x01
#define		COL_RIGHT	    0x02
#define		COL_BOTTOM	    0x04
#define		COL_LEFT	    0x08


// Projectile collisions
#define     PJC_NONE        0x00
#define     PJC_TERRAIN     0x01
#define     PJC_WORM        0x02
#define     PJC_EXPLODE     0x04
#define     PJC_TOUCH       0x08



class CProjectile {
public:
	// Constructor
	CProjectile() {
        iID = 0;
		fSpeed = 0;
		iUsed = false;
		fLife = 0;
		tProjInfo = NULL;
		fLastTrailProj = 0;
		iSpawnPrjTrl = false;
		iColour = 0;
		iRandom = 0;
		iRemote = false;
        nExplode = false;
        nTouched = false;
	}


private:
	// Attributes

    int         iID;
	int			iUsed;
	int			iType;
	float		fLife;
	float		fExtra;
	int			iOwner;
	float		fSpeed;
	Uint32		iColour;

	// Projectile trail
	float		fLastTrailProj;
	int			iSpawnPrjTrl;
    float       fTimeVarRandom;

	proj_t		*tProjInfo;

	CVec		vOldPos;
	CVec		vPosition;
	CVec		vVelocity;
	float		fRotation;
	int			CollisionSide;

	// Network
	int			iRandom;
	int			iRemote;
	float		fRemoteFrameTime;

	// Animation
	int			iFrameDelta;
	float		fFrame;
    int         iFrameX;

	// Beam info
	CVec		vStart, vEnd;
	int			iWidth;

    // Queued events
    int         nExplode;
    float       fExplodeTime;
    int         nTouched;

	// Debug info
	int			firstbounce;



public:
	// Methods
	

	void	Spawn(proj_t *_proj, CVec _pos, CVec _vel, int _rot, int _owner, int _random, int _remote, float _remoteframe);	
	int		Simulate(float dt, CMap *map, CWorm *worms, int *wormid);
	int		Collision(uchar pf);

    void	Draw(SDL_Surface *bmpDest, CViewport *view);
    void	DrawShadow(SDL_Surface *bmpDest, CViewport *view, CMap *map);

	int		CheckWormCollision(CWorm *worms);
	int		ProjWormColl(CVec pos, CWorm *worms);
	int		CheckCollision(float dt, CMap *map, CVec pos, CVec vel);

	void	Bounce(float fCoeff);

	int		isUsed(void)			{ return iUsed; }
	void	setUsed(int u)			{ iUsed = u; }

	float	getLife(void)			{ return fLife; }

	int		getSpawnPrjTrl(void)	{ return iSpawnPrjTrl; }
	void	setSpawnPrjTrl(int p)	{ iSpawnPrjTrl = p; }

	CVec	GetPosition(void)		{ return vPosition; }
	CVec	GetVelocity(void)		{ return vVelocity; }
	proj_t	*GetProjInfo(void)		{ return tProjInfo; }
	int		GetOwner(void)			{ return iOwner; }

    float   getTimeVarRandom(void)  { return fTimeVarRandom; }

	float	getRandomFloat(void);
	int		getRandomIndex(void)	{ return iRandom; }

    void    setID(int id)           { iID = id; }
    int     getID(void)             { return iID; }

    void    setExplode(float t, int _e)     { fExplodeTime = t; nExplode = _e; }
    void    setTouched(int _t)      { nTouched = _t; }


};


#endif  //  __CPROJECTILE_H__