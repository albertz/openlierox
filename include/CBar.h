/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// CBar header file
// Created 22/6/07
// Dark Charlie

#ifndef __CBAR_H_
#define __CBAR_H_

// Bar directions
enum {
	BAR_LEFTTORIGHT,
	BAR_RIGHTTOLEFT,
	BAR_TOPTOBOTTOM,
	BAR_BOTTOMTOTOP
};

class CBar {
public:
	CBar(SDL_Surface *bmp, int x, int y, int label_x, int label_y, int dir);

private:
	// Variables
	int  X;
	int  Y;
	int  LabelX;
	int  LabelY;
	bool LabelVisible;
	int  Direction;
	int  Position;

	SDL_Surface *bmpBar;

	Uint32	bgColor;
	Uint32	foreColor;

public:
	// Methods
	void		Draw(SDL_Surface *dst);
	inline int	GetPosition()  { return Position; }
	inline void SetPosition(int _p) { Position = _p; }
	inline int	GetX() { return X; }
	inline void SetX(int _x) { X = _x; }
	inline int	GetY() { return Y; }
	inline void SetY(int _y) { Y = _y; }
	inline int	GetLabelX() { return LabelX; }
	inline void SetLabelX(int _x) { LabelX = _x; }
	inline int	GetLabelY() { return LabelY; }
	inline void SetLabelY(int _y) { LabelY = _y; }
	inline bool IsLabelVisible()  { return LabelVisible; }
	inline void SetLabelVisible(bool _v)  { LabelVisible = _v; }
	inline Uint32 GetBgColor()  { return bgColor; }
	inline void SetBgColor(Uint32 _cl)  { bgColor = _cl; }
	inline Uint32 GetForeColor()  { return foreColor; }
	inline void SetForeColor(Uint32 _cl)  { foreColor = _cl; }
	inline bool IsProperlyLoaded()  { return bmpBar != NULL; }
};

#endif // __CBAR_H_	