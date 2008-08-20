/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Protocol
// Created 1/7/02
// Jason Boettcher


#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__


#define		PROTOCOL_VERSION	8



// Text type
#define		TXT_CHAT			0
#define		TXT_NORMAL			1
#define		TXT_NOTICE			2
#define		TXT_IMPORTANT		3
#define		TXT_NETWORK			4
#define		TXT_PRIVATE			5
#define		TXT_TEAMPM			6



// Client->Server
#define		C2S_TXTCOMMAND		0
#define		C2S_IMREADY			1
#define		C2S_CARVE			2
#define		C2S_UPDATE			3
#define		C2S_DEATH			4
#define		C2S_CHATTEXT		5
#define		C2S_UPDATELOBBY		6
#define		C2S_DISCONNECT		7
#define		C2S_GRABBONUS		8
#define     C2S_SENDFILE        9 // Beta4+ only, enabled only in Beta6+


// Server->Client
#define		S2C_PREPAREGAME		0
#define		S2C_STARTGAME		1
#define		S2C_SPAWNWORM		2
#define		S2C_WORMINFO		3
#define		S2C_TEXT			4
#define		S2C_SCOREUPDATE		5
#define		S2C_GAMEOVER		6
#define		S2C_SPAWNBONUS		7
#define		S2C_TAGUPDATE		8
#define		S2C_CLREADY			9
#define		S2C_UPDATELOBBY		10
#define		S2C_CLLEFT			11
#define		S2C_UPDATEWORMS		12
#define		S2C_UPDATELOBBYGAME	13
#define		S2C_WORMDOWN		14
#define		S2C_LEAVING			15
#define		S2C_SINGLESHOOT		16
#define		S2C_MULTISHOOT		17
#define		S2C_UPDATESTATS		18
#define		S2C_GOTOLOBBY		19
#define		S2C_DESTROYBONUS	20
#define     S2C_DROPPED         21
#define     S2C_SENDFILE        22 // Beta4+ only, enabled only in Beta6+


#endif  //  __PROTOCOL_H__
