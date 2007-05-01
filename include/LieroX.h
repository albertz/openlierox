/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Game header file
// Created 28/6/02
// Jason Boettcher


#ifndef __LIEROX_H__
#define __LIEROX_H__

#ifdef _MSC_VER
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG
#endif // _MSC_VER

#if DEBUG == 1
#define		_AI_DEBUG
#endif

#define		LX_PORT			23400
#define		SPAWN_HOLESIZE	4
#ifndef		LX_VERSION
#	define		LX_VERSION		"0.57_beta3"
#endif
#define		LX_ENDWAIT		9.0f


// Game types
enum {
	GMT_DEATHMATCH,
	GMT_TEAMDEATH,
	GMT_TAG,
    GMT_DEMOLITION
};


const float	D2R(1.745329e-2f); // degrees to radians
const float	R2D(5.729578e+1f); // radians to degrees

#define DEG2RAD(a)  (a * D2R)
#define RAD2DEG(a)  (a * R2D)


// Game includes
#include "ProfileSystem.h"
#include "Networking.h"
#include "CChatBox.h"
#include "Frame.h"
#include "CViewport.h"
#include "CSimulation.h"
#include "Command.h"
#include "CWorm.h"
#include "CProjectile.h"
#include "CShootList.h"
#include "Entity.h"
#include "CWeather.h"
#include "Protocol.h"
#include "Options.h"


// LieroX structure
class lierox_t { public:
	float	fCurTime;
	float	fDeltaTime;
	CFont	cFont;
	CFont	cOutlineFont;
	CFont	cOutlineFontGrey;

	int		iQuitGame;
	int		iQuitEngine;

	int		debug_int;
	float	debug_float;
	CVec	debug_pos;

	// Default Colours
	Uint32			clNormalLabel;
	Uint32			clHeading;
	Uint32			clSubHeading;
	Uint32			clChatText;
	Uint32			clNetworkText;
	Uint32			clNormalText;
	Uint32			clNotice;
	Uint32			clDropDownText;
	Uint32			clDisabled;
	Uint32			clListView;
	Uint32			clTextBox;
	Uint32			clMouseOver;
	Uint32			clError;
	Uint32			clCredits1;
	Uint32			clCredits2;
	Uint32			clPopupMenu;
	Uint32			clWaiting;
	Uint32			clReady;
	Uint32			clPlayerName;
	Uint32			clBoxLight;
	Uint32			clBoxDark;
	Uint32			clWinBtnBody;
	Uint32			clWinBtnLight;
	Uint32			clWinBtnDark;
	Uint32			clMPlayerTime;
	Uint32			clMPlayerSong;

	Uint32			clPink;
	Uint32			clWhite;


	std::string	debug_string;
};


// Game types
enum {
	GME_LOCAL=0,
	GME_HOST,
	GME_JOIN
};



// Game structure
class game_t { public:
	int			iGameType;		// Local, remote, etc
	int			iGameMode;		// DM, team DM, etc
	std::string		sModName;
	std::string		sMapname;
    std::string        sPassword;
	std::string		sModDir;
    maprandom_t sMapRandom;
	int			iLoadingTimes;
	std::string		sServername;
	std::string		sWelcomeMessage;
	bool		bRegServer;
	bool		bTournament;

	int			iLives;
	int			iKillLimit;
	int			iTimeLimit;
	int			iTagLimit;
	int			iBonusesOn;
	int			iShowBonusName;
	
	int			iNumPlayers;
	profile_t	*cPlayers[MAX_WORMS];
};


// TODO: move this somewhere else
// Game lobby structure
class game_lobby_t { public:
	int		nSet;
	int		nGameMode;
	int		nLives;
	int		nMaxWorms;
	int		nMaxKills;
	int		nLoadingTime;
	int		nBonuses;
	std::string	szMapName;
	std::string	szDecodedMapName;
	std::string	szModName;
	std::string	szModDir;
	bool	bHaveMap;
	bool	bHaveMod;
	bool	bTournament;
};



extern	lierox_t		*tLX;
extern	game_t			tGameInfo;
extern	CVec			vGravity;
extern  CInput			cTakeScreenshot;
extern  CInput			cSwitchMode;
extern	CInput			cToggleMediaPlayer;
extern  int				nDisableSound;
extern	bool			bActivated;

extern	std::string		binary_dir;


// Main Routines
void    ParseArguments(int argc, char *argv[]);
int		InitializeLieroX(void);
void	StartGame(void);
void	ShutdownLieroX(void);
void	GameLoop(void);
void	QuittoMenu(void);
void	GotoLocalGameMenu(void);



// Miscellanous routines
float	GetFixedRandomNum(uchar index);
int		CheckCollision(CVec trg, CVec pos, uchar checkflags, CMap *map);
void	ConvertTime(float time, int *hours, int *minutes, int *seconds);
int 	CarveHole(CMap *cMap, CVec pos);
void	StripQuotes(char *dest, char *src);
void	StripQuotes(std::string& str);
void    lx_strncpy(char *dest, char *src, int count);
bool    MouseInRect(int x, int y, int w, int h);
char    *StripLine(char *szLine);
char    *TrimSpaces(char *szLine);
void TrimSpaces(std::string& szLine);
bool	replace(char *text, const char *what, const char *with, char *result);
bool	replace(const std::string& text, const std::string& what, const std::string& with, std::string& result);
bool	replace(std::string& text, const std::string& what, const std::string& with);
std::string replacemax(const std::string& text, const std::string& what, const std::string& with, std::string& result, int max);
std::string replacemax(const std::string& text, const std::string& what, const std::string& with, int max);
char	*strip(char *buf, int width);
std::string strip(const std::string& text, int width);
bool	stripdot(char *buf, int width);
bool stripdot(std::string& text, int width);
char	*ucfirst(char *text);
void	ucfirst(std::string& text);
void	stringtolower(std::string& text);
std::string	ReadUntil(const std::string& text, char until_character = '\n');
std::string	ReadUntil(FILE* fp, char until_character = '\n');
Uint32	StrToCol(const std::string& str);
const char* sex(short wraplen = 0);
const std::vector<std::string>& explode(const std::string& str, const std::string& delim);
std::string freadstr(FILE *fp, size_t maxlen);
inline std::string freadfixedcstr(FILE *fp, size_t maxlen) { return ReadUntil(freadstr(fp, maxlen), '\0'); }
size_t fwrite(const std::string& txt, size_t len, FILE* fp);
size_t findLastPathSep(const std::string& path);
void stringlwr(std::string& txt);
bool strincludes(const std::string& str, const std::string& what);
short stringcasecmp(const std::string& s1, const std::string& s2);
std::string GetFileExtension(const std::string& filename);
void printf(const std::string& txt);


typedef Uint32 UnicodeChar;
// the iterator shows at the next character after this operation
UnicodeChar GetNextUnicodeFromUtf8(std::string::const_iterator& it, const std::string::const_iterator& last);
std::string GetUtf8FromUnicode(UnicodeChar ch);



template<typename _RandomAccessType, typename _ValueType, typename _PosType = size_t>
class iterator {
protected:
	_RandomAccessType& base;
	_PosType pos;
public:
	iterator(_RandomAccessType& b, _PosType p) : base(b), pos(p) {}
	static iterator end(_RandomAccessType& b) {
		return iterator<_RandomAccessType, _ValueType>(b, -1); }

	template<typename _ValueType2, typename _PosType2>
	void operator==(const iterator<_RandomAccessType, _ValueType2, _PosType2>& it) {
		return base == it->base && (pos == it->pos || MIN(pos,it->pos) >= base.size()); }
	
	void operator++() { pos++; }
	void operator--() { pos--; }
	_ValueType operator*() { return base[pos]; }
};

template<typename _RandomAccessType, typename _ValueType, typename _PosType = size_t>
class reverse_iterator : public iterator<_RandomAccessType, _ValueType, _PosType> {
public:
	reverse_iterator(_RandomAccessType& b, _PosType p) : iterator<_RandomAccessType, _ValueType, _PosType>(b, p) {}
	void operator++() { this->pos--; }
	void operator--() { this->pos++; }
};

template<typename _Iterator>
void IncUtf8StringIterator(_Iterator& it, const _Iterator& last) {
	uchar c;
	for(it++; it != last; it++) {
		c = *it;
		if(!(c&0x80) || (c&0xC0)) break;
	}
}




template<typename T>
T from_string(const std::string& s, std::ios_base& (*f)(std::ios_base&), bool& failed) {
	std::istringstream iss(s); T t;
	failed = (iss >> f >> t).fail();
	return t;
}

template<typename T>
T from_string(const std::string& s, std::ios_base& (*f)(std::ios_base&)) {
	std::istringstream iss(s); T t;
	iss >> f >> t;
	return t;
}

template<typename T>
T from_string(const std::string& s, bool& failed) {
	std::istringstream iss(s); T t;
	failed = (iss >> t).fail();
	return t;
}

template<typename T>
T from_string(const std::string& s) {
	std::istringstream iss(s); T t;
	iss >> t;
	return t;
}



// std::string itoa
inline std::string itoa(int num,int base=10)  {
	// TODO: better!! (use ostringstream)
	static char buf[64];
	static std::string ret;
	ret = itoa(num,buf,base);
	fix_markend(buf);
	return ret;
}

inline int atoi(const std::string& str)  { return from_string<int>(str);  }
inline float atof(const std::string& str) { return from_string<float>(str);  }



class simple_reversestring_hasher { public:
	inline size_t operator() (const std::string& str) const {
		std::string::const_reverse_iterator pos = str.rbegin();
		unsigned short nibble = 0;
		size_t result = 0;
		for(; pos != str.rend() && nibble < sizeof(size_t)*2; pos++, nibble++)
			result += ((size_t)*pos % 16) << nibble*4;
		return result;
	}
};


// Useful XML functions
int		xmlGetInt(xmlNodePtr Node, const std::string& Name);
float	xmlGetFloat(xmlNodePtr Node, const std::string& Name);
Uint32	xmlGetColour(xmlNodePtr Node, const std::string& Name);
void	xmlEntities(std::string& text);

// Thread functions
#ifdef WIN32
void	nameThread(const DWORD threadId, const char *name);
#endif


#endif  //  __LIEROX_H__
