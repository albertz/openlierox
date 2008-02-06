// OpenLieroX


// Image
// Created 29/10/06
// Dark Charlie

// code under LGPL


#ifndef __CANIMATION_H__
#define __CANIMATION_H__

#include "InputEvents.h"
#include "GfxPrimitives.h"

// Events
enum {
	ANI_NONE = -1
};

class CAnimation : public CWidget {
public:
	CAnimation () {}

	// Constructor
	CAnimation(const std::string& Path, float frametime) {
		iType = wid_Animation;
		sPath = Path;
		tAnimation = NULL;
		if (Path != "")  {
			tAnimation = LoadImage(Path, true);

			if (tAnimation)  {
				iWidth = tAnimation->w;
				iHeight = tAnimation->h;
			} else {
				iWidth = iHeight = 0;
			}
		}
		fLastFrameChange = -9999;
		iCurFrame = 0;
		fFrameTime = frametime;

		Parse();
	}

private:
    // Attributes
	SDL_Surface	*tAnimation;
	std::string	sPath;
	std::vector<int> tFrameOffsets;
	std::vector<int> tFrameWidths;
	int			iNumFrames;
	int			iCurFrame;
	float		fLastFrameChange;
	float		fFrameTime;


public:
    // Methods

    void			Create(void);
	void			Destroy(void)		{  }
	void			Parse();

	std::string		getPath(void)		{ return sPath; }
	SDL_Surface		*getSurface(void)	{ return tAnimation; }
	float			getFrameTime()		{ return fFrameTime; }
	void			setFrameTime(float t) { fFrameTime = t; }
	void			Change(const std::string& Path, float frametime);

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)				{ return ANI_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return ANI_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return ANI_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)			{ return ANI_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)			{ return ANI_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)		{ return ANI_NONE; }
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)		{ return ANI_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2) { return 0; }
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}

	static CWidget * WidgetCreator( const std::vector< CScriptableVars::ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
	{
		CWidget * w = new CAnimation( p[0].s, p[1].f );
		layout->Add( w, id, x, y, dx, dy );
		return w;
	};
	
	void	ProcessGuiSkinEvent(int iEvent) {};

};

#endif  //  __CANIMATION_H__


