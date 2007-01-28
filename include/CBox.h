// Open LieroX


// Image
// Created 29/10/06
// Dark Charlie


#ifndef __CBOX_H__
#define __CBOX_H__


// Event types
enum {
	BOX_NOEVENT=-2,
	BOX_NONE=-1
};


class CBox : public CWidget {
public:
	// Constructor
	CBox(int round,int border,Uint32 lightcolour,Uint32 darkcolour, Uint32 bgcolour) {
		iType = wid_Frame;
		iRound = round;
		iBorder = border;
		iLightColour = lightcolour;
		iDarkColour = darkcolour;
		iBgColour = bgcolour;
		bmpBuffer = NULL;
	}

private:
    // Attributes
	int		iRound;
	int		iBorder;
	Uint32	iLightColour;
	Uint32	iDarkColour;
	Uint32	iBgColour;

	SDL_Surface *bmpBuffer;


public:
    // Methods

    void			Create(void)		{ iType = wid_Frame; PreDraw(); }
	void			Destroy(void);

	void			setRound(int round)			{ iRound = round; }
	int				getRound(void)				{ return iRound; }
	void			setBorder(int border)		{ iBorder = border; }
	int				getBorder(void)				{ return iBorder; }
	void			setLightColour(Uint32 col)	{ iLightColour = col; }
	Uint32			getLightColour(void)		{ return iLightColour; }
	void			setDarkColour(Uint32 col)	{ iDarkColour = col;  }
	Uint32			getDarkColour(void)			{ return iDarkColour; }
	void			setBgColour(Uint32 col)		{ iBgColour = col; }
	int				getBgColour(void)			{ return iBgColour; }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)				{ return CheckEvent(); }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return CheckEvent(); }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return CheckEvent(); }
	int		MouseWheelDown(mouse_t *tMouse)			{ return CheckEvent(); }
	int		MouseWheelUp(mouse_t *tMouse)			{ return CheckEvent(); }
	int		KeyDown(int c)							{ return BOX_NONE; }
	int		KeyUp(int c)							{ return BOX_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return 0; }

	int		CheckEvent(void);

	void	PreDraw(void);
	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void);
};


#endif  //  __CBOX_H__


