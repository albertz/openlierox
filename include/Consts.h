/*
	OpenLieroX

	some constants

	created 2008-10-03
	code under LGPL
*/

#ifndef __OLX_CONSTS_H__
#define __OLX_CONSTS_H__


// Net status
enum ClientNetState {
	NET_DISCONNECTED=0,
	NET_CONNECTING,			// Server doesn't use this state, only client side
	NET_CONNECTED,			// Server usage: when client connected or playing, client usage: only when in server lobby
	NET_PLAYING,			// Server doesn't use this state, only client side
	NET_ZOMBIE				// Server side only state - server won't accept any client packets and will send disconnect packets to client
};

static const char* NetStateString(ClientNetState state) {
	switch (state) {
		case NET_DISCONNECTED: return "NET_DISCONNECTED";
		case NET_CONNECTING: return "NET_CONNECTING";
		case NET_CONNECTED: return "NET_CONNECTED";
		case NET_PLAYING: return "NET_PLAYING";
		case NET_ZOMBIE: return "NET_ZOMBIE";
	}
	return "INVALID NETSTATE";
}

// Misc constants
enum {	
	LX_PORT = 23400, 
	SPAWN_HOLESIZE = 4,
	MAX_WORMS = 32,
	MAX_CLIENTS = 32,
	MAX_PLAYERS	= 32,
	MAX_CHATLINES = 8,
	NUM_VIEWPORTS = 3,
	GAMEOVER_WAIT = 3
};

static const float LX_ENDWAIT = 9.0f;

// Game modes
enum GameModeIndex {
	GM_DEATHMATCH = 0,
	GM_TEAMDEATH,
	GM_TAG,
	GM_DEMOLITIONS,
	GM_HIDEANDSEEK,
	GM_CTF,
	GM_RACE,
	GM_TEAMRACE,
};

// Game mode types
// HINT: do not change the order, this is used over the network
enum {
	GMT_NORMAL = 0,  // Worms appear as normal
	GMT_TEAMS,   // Worms appear in teams
	GMT_TIME,    // One worm appears special, worms have a time attribute
	GMT_DIRT,     // There is a dirt counter
	GMT_MAX = GMT_DIRT
};

enum AFK_TYPE {
	AFK_BACK_ONLINE = 0,
	AFK_TYPING_CHAT,
	AFK_AWAY,
	AFK_SELECTING_WPNS,
	AFK_CONSOLE,
	AFK_MENU,
};


#endif

