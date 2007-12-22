/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Checkbox
// Created 30/7/02
// Jason Boettcher


#ifndef __CCHECKBOX_H__
#define __CCHECKBOX_H__


#include "InputEvents.h"


// Checkbox events
enum {
	CHK_NONE=-1,
	CHK_CHANGED
};

// Checkbox messages
enum {
	CKM_SETCHECK=0,
	CKM_GETCHECK
};


class CCheckbox : public CWidget {
public:
	// Constructor
	CCheckbox(int val) {
		iValue = val;
        bmpImage = NULL;
		iType = wid_Checkbox;
		bVar = NULL;
		iVar = NULL;
	}


private:
	// Attributes

	int			iValue;
	SDL_Surface	*bmpImage;
	bool		*bVar;
	int			*iVar;
	CGuiSkin::CallbackHandler cClick;

public:
	// Methods

	void	Create(void);
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return CHK_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ iValue = !iValue;		return CHK_CHANGED; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return CHK_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return CHK_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return CHK_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return CHK_NONE; }
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return CHK_NONE; }
	

	// Process a message sent
	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2) {

				switch(iMsg) {
					case CKM_SETCHECK:
						iValue = Param1;
						return 0;
					case CKM_GETCHECK:
						return iValue;
				}

				return 0;
			}
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }
		

	// Draw the title button
	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void);

	int		getValue(void)						{ return iValue; }

	static CWidget * WidgetCreator( const std::vector< CScriptableVars::ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
	{
		CCheckbox * w = new CCheckbox(0);
		w->bVar = CScriptableVars::GetVar( p[0].s, CScriptableVars::SVT_BOOL ).b;
		w->iVar = CScriptableVars::GetVar( p[0].s, CScriptableVars::SVT_INT ).i;
		if( w->bVar )
			w->iValue = *w->bVar;
		if( w->iVar )
			w->iValue = *w->iVar;
		w->cClick.Init( p[1].s, w );
		layout->Add( w, id, x, y, dx, dy );
		return w;
	};
	
	void	ProcessGuiSkinEvent(int iEvent) 
	{
		if( iEvent == CGuiSkin::SHOW_WIDGET )
		{
			if( bVar )
				iValue = *bVar;
			if( iVar )
				iValue = *iVar;
		};
		if( iEvent == CHK_CHANGED )
		{
			if( bVar )
				*bVar = ( iValue != 0 );
			if( iVar )
				*iVar = iValue;
			cClick.Call();
		};
	};
};

static bool CCheckBox_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "checkbox", & CCheckbox::WidgetCreator )
							( "var", CScriptableVars::SVT_STRING )
							( "click", CScriptableVars::SVT_STRING );

#endif  //  __CCHECKBOX_H__
