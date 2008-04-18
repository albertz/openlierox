/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Slider
// Created 30/6/02
// Jason Boettcher


#ifndef __CSLIDER_H__
#define __CSLIDER_H__

#include <string>
#include "InputEvents.h"



// Slider events
enum {
	SLD_NONE=-1,
	SLD_CHANGE=0
};


// Slider messages
enum {
	SLM_GETVALUE=0,
	SLM_SETVALUE
};


class CSlider : public CWidget {
public:
	// Constructor
	CSlider(int max, int min = 0) {
		Create();
		iMax = max;
		iMin = min;
		iType = wid_Slider;
		iVar = NULL;
	}


private:
	// Attributes

	int		iValue;
	int		iMax;
	int		iMin;
	int		*iVar;
	CGuiSkin::CallbackHandler cClick;

public:
	// Methods

	void	Create(void) { iValue=0; }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return SLD_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return SLD_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse)		{ return SLD_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return SLD_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return SLD_NONE; }
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return SLD_NONE; }

	void	Draw(const SmartPointer<SDL_Surface> & bmpDest);

	void	LoadStyle(void) {}

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	int		getValue(void)						{ return iValue; }
	void	setValue(int v)						{ iValue = v; }

	void	setMax(int _m)						{ iMax = _m; }
	void	setMin(int _m)						{ iMin = _m; }

	static CWidget * WidgetCreator( const std::vector< CScriptableVars::ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
	{
		CSlider * w = new CSlider( p[1].i, p[0].i );
		layout->Add( w, id, x, y, dx, dy );
		w->iVar = CScriptableVars::GetVar( p[2].s, CScriptableVars::SVT_INT ).i;
		if( w->iVar )
			w->setValue( *w->iVar );
		w->cClick.Init( p[3].s, w );
		return w;
	};
	
	void	ProcessGuiSkinEvent(int iEvent) 
	{
		if( iEvent == CGuiSkin::SHOW_WIDGET )
		{
			if( iVar )
				setValue( *iVar );
		};
		if( iEvent == SLD_CHANGE )
		{
			if( iVar )
				*iVar = iValue;
			cClick.Call();
		};
	};
};

#endif  //  __CSLIDER_H__
