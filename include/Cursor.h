/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Cursor header file
// Created 18/6/07
// Dark Charlie

#ifndef __CURSOR_H_
#define __CURSOR_H_

#include <string>


// Cursor indexes
enum {
	CURSOR_NONE = -1,
	CURSOR_ARROW = 0,
	CURSOR_HAND,
	CURSOR_TEXT,
	CURSOR_RESIZE,
	CURSOR_COUNT
};

// Cursor types
enum {
	CUR_ARROW=0,
	CUR_SPLITTER,
	CUR_TEXT,
	CUR_AIM  // for future use, cursors like cross
};

extern int iMaxCursorWidth;
extern int iMaxCursorHeight;

// Cursor class
class CCursor  {
public:
	CCursor(const std::string& filename, int type);
	~CCursor();
private:
	SmartPointer<SDL_Surface> bmpCursor;
	CCursor			*cDown;
	CCursor			*cUp;
	int				iFrame;
	int				iFrameWidth;
	float			fAnimationSwapTime;
	bool			bAnimated;
	int				iNumFrames;
	int				iType;
public:
	void			Draw(SDL_Surface * dst);
	inline bool		IsAnimated()  { return bAnimated; }
	inline int		GetType()  { return iType; }
	inline void		SetType(int _t)  { iType = _t; }
	inline int		GetHeight()  { return bmpCursor.get() ? bmpCursor.get()->h : 0; }
	inline int		GetWidth()  { return bmpCursor.get() ? bmpCursor.get()->w : 0; }
};

// Game cursors handling

bool InitializeCursors();
void ShutdownCursors();
void SetGameCursor(int c);
void DrawCursor(SDL_Surface * dst);
int GetCursorHeight(int c);
int GetCursorWidth(int c);
inline int GetMaxCursorHeight()  { return iMaxCursorHeight; }
inline int GetMaxCursorWidth()  { return iMaxCursorWidth; }

#endif // __CURSOR_H_
