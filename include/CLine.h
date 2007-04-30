// OpenLieroX

// Line
// Created 5/11/06
// Dark Charlie

// code under LGPL


#ifndef __CLINE_H__
#define __CLINE_H__


// Line events
enum {
	LIN_NONE=-1
};


class CLine : public CWidget {
public:
	// Constructor
	CLine(int x1,int y1,int x2,int y2,Uint32 col) {
		iX1 = x1;
		iY1 = y1;
		iX2 = x2;
		iY2 = y2;
		iColour = col;
	}


private:
	// Attributes
	int		iX1,iY1,iX2,iY2;
	Uint32	iColour;

public:
	// Methods

	void	Create(void) { }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return LIN_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return LIN_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return LIN_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return LIN_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return LIN_NONE; }
	int		KeyDown(int c)						{ return LIN_NONE; }
	int		KeyUp(int c)						{ return LIN_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ 
							return 0;
						}
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	ChangeColour(Uint32 col)			{ iColour = col; }

	// Draw the line
	void	Draw(SDL_Surface *bmpDest) {
				DrawLine(bmpDest,iX1,iY1,iX2,iY2,iColour); 
	}

	void	LoadStyle(void) {}

};



#endif  //  __CLINE_H__
