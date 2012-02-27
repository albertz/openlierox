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


// Worm frame state
struct worm_state_t {
	worm_state_t() {
		bShoot = bCarve = bMove = bJump = false;
	}
	
	// Command byte
	bool	bShoot;
	bool	bCarve;
	bool	bMove;
	bool	bJump;
};



#endif  //  __FRAME_H__
