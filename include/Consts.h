/*
	OpenLieroX

	some constants

	created 2008-10-03
	code under LGPL
*/

#ifndef __OLX_CONSTS_H__
#define __OLX_CONSTS_H__


#define		MAX_CLIENTS		32
#define		MAX_PLAYERS		32
#define		MAX_CHATLINES	8


#define     NUM_VIEWPORTS   3
#define     GAMEOVER_WAIT   3


// Net status
enum {
	NET_DISCONNECTED=0,
	NET_CONNECTING,			// Server doesn't use this state, only client side
	NET_CONNECTED,			// Server usage: when client connected or playing, client usage: only when in server lobby
	NET_PLAYING,			// Server doesn't use this state, only client side
	NET_ZOMBIE				// Server side only state - server won't accept any client packets and will send disconnect packets to client
};



enum {	
	LX_PORT = 23400, 
	SPAWN_HOLESIZE = 4,
	MAX_WORMS = 32
};

static const float LX_ENDWAIT = 9.0f;

// Game modes
enum {
	GM_DEATHMATCH = 0,
	GM_TEAMDEATH,
	GM_TAG,
	GM_DEMOLITION
};

// Game mode types
enum {
	GMT_NORMAL,  // Worms appear as normal
	GMT_TEAMS,   // Worms appear in teams
	GMT_TIME,    // One worm appears special, worms have a time attribute
	GMT_DIRT     // There is a dirt counter
};

enum AFK_TYPE {
	AFK_BACK_ONLINE = 0,
	AFK_TYPING_CHAT,
	AFK_AWAY
};


#endif

