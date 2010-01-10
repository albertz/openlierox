/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Frame structure
// Created 22/7/02
// Jason Boettcher


#ifndef __FRAME_H__
#define __FRAME_H__

#include <SDL.h>


// Frame flags
#define		FRM_POSX		0x01
#define		FRM_POSY		0x02
#define		FRM_ANGLE		0x04
#define		FRM_COMMAND		0x08		// Wraps the four command vars

// Rope types
#define		ROP_NONE		0x00
#define		ROP_SHOOTING	0x01
#define		ROP_HOOKED		0x02
#define		ROP_FALLING		0x04
#define		ROP_PLYHOOKED	0x08

#define		NUM_FRAMES		16
#define		FRAME_MASK		(NUM_FRAMES-1)




// Worm frame state
struct worm_state_t {
	worm_state_t() {
		iFlags = iX = iY = iAngle = iHookType = iHookX = iHookY = 0;
		bShoot = bCarve = bMove = bJump = false;
		iFaceDirectionSide = iWeapon = 0;
	}
	
	int		iFlags;

	int		iX, iY;
	int		iAngle;

	int		iHookType;
	int		iHookX, iHookY;

	// Command byte
	bool	bShoot;
	bool	bCarve;
	int		iFaceDirectionSide;
	bool	bMove;
	bool	bJump;
	int		iWeapon;
};




struct frame_t {
	Uint32	iFrameNum;
	float	fTime;
	float	fPingTime;
	float	fSentTime;

	worm_state_t	tWormStates[8];
};




#endif  //  __FRAME_H__
