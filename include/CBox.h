// Open LieroX


// Image
// Created 29/10/06
// Dark Charlie


#ifndef __CFRAME_H__
#define __CFRAME_H__


// Event types
enum {
	FRM_NONE=-1,
};


class CFrame : public CWidget {
public:
	// Constructor
	CFrame(int round,int border,Uint32 lightcolour,Uint32 darkcolour) {
		iType = wid_Frame;
		iRound = round;
		iBorder = border;
		iLightColour = lightcolour;
		iDarkColour = darkcolour;
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
	int		MouseOver(mouse_t *tMouse)				{ return IMG_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return IMG_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return IMG_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)			{ return IMG_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)			{ return IMG_NONE; }
	int		KeyDown(int c)							{ return IMG_NONE; }
	int		KeyUp(int c)							{ return IMG_NONE; }

	int		SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return 0; }
    
	void	PreDraw(void);
	void	Draw(SDL_Surface *bmpDest);
};


#endif  //  __CIMAGE_H__


