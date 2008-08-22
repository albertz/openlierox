/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// gui scrollbar class
// Created 30/6/02
// Jason Boettcher


#ifndef __CSCROLLBAR_H__DEPRECATED_GUI__
#define __CSCROLLBAR_H__DEPRECATED_GUI__


#include "InputEvents.h"
#include "DeprecatedGUI/CWidget.h"
#include "DeprecatedGUI/CGuiLayout.h"

namespace DeprecatedGUI {

// Scrollbar events
enum {
	SCR_NONE=-1,
	SCR_CHANGE=0
};

// Scrollbar messages
enum {
    SCM_GETVALUE,
    SCM_SETVALUE,
    SCM_SETITEMSPERBOX,
    SCM_SETMAX,
    SCM_SETMIN
};


class CScrollbar : public CWidget {
public:
	// Constructor
	CScrollbar() {
		iType = wid_Scrollbar;
		iMin = 0;
		iMax = 1;
		iValue = 0;
		iItemsperbox = 1;
		iVar = NULL;
	}


private:
	// Attributes
	int		iMin;
	int		iMax;
	int		iValue;

	int		iScrollPos;
	int		iItemsperbox;
	bool	bSliderGrabbed;
	int		iSliderGrabPos;

	bool	bTopButton;
	bool	bBotButton;

	int		nButtonsDown;
	float	fMouseNext[3];

	int		*iVar;
	CGuiSkin::CallbackHandler cClick;

public:
	// Methods

	void	Create(void);
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse);
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse);
	int		MouseWheelUp(mouse_t *tMouse);
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return SCR_NONE; }
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return SCR_NONE; }

	void	Draw(SDL_Surface * bmpDest);

	void	LoadStyle(void) {}

	void	UpdatePos(void);


	void	setMin(int _min)				{ iMin = _min; UpdatePos(); }
	void	setMax(int _max)				{ iMax = _max; UpdatePos(); }
	void	setValue(int _value)			{ iValue = _value; UpdatePos(); }

	void	setItemsperbox(int _i)			{ iItemsperbox = _i; }
    int     getItemsperbox(void)            { return iItemsperbox; }

	int		getValue(void)					{ return iValue; }
	int		getMax(void)					{ return iMax; }
	bool	getGrabbed(void)				{ return bSliderGrabbed; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2);
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	static CWidget * WidgetCreator( const std::vector< CScriptableVars::ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
	{
		CScrollbar * w = new CScrollbar();
		layout->Add( w, id, x, y, dx, dy );
		// Should be set after scrollbar is added to layout
		w->cClick.Init( p[4].s, w );
		w->setMin( p[0].i );
		w->setMax( p[1].i );
		w->setItemsperbox( p[2].i );
		w->iVar = CScriptableVars::GetVar( p[3].s, CScriptableVars::SVT_INT ).i;
		if( w->iVar )
			w->setValue( *w->iVar );
		return w;
	};
	
	void	ProcessGuiSkinEvent(int iEvent) 
	{
		if( iEvent == CGuiSkin::SHOW_WIDGET )
		{
			if( iVar )
				setValue( *iVar );
		};
		if( iEvent == SCR_CHANGE )
		{
			if( iVar )
				*iVar = iValue;
			cClick.Call();
		};
	};
};

}; // namespace DeprecatedGUI

#endif  //  __CSCROLLBAR_H__DEPRECATED_GUI__
