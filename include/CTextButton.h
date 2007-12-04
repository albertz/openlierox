/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// The CTextButton (clickable label) - the current buttons are using images only, 
// this is not very handy when writing skins.
// TODO: Add large font to OLX graphics and add option to use large font for labels and CTextButton.

#ifndef __CTEXTBUTTON_H__
#define __CTEXTBUTTON_H__

#include "CLabel.h"

enum {
	TXB_MOUSEUP=0,
	TXB_MOUSEOVER
};

class CTextButton : public CLabel {
public:

	CTextButton(const std::string& text, Uint32 colNormal, Uint32 colGlow):
		CLabel ( text, colNormal )
	{
		iColNormal = colNormal;
		iColGlow = colGlow;
		iGlowCoef = 0;
		iType = wid_Textbutton;
	}

private:
	// Attributes

	Uint32	iColNormal;
	Uint32	iColGlow;
	int		iGlowCoef;
	CGuiSkin::CallbackHandler cClick;

public:

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse);
	int		MouseUp(mouse_t *tMouse, int nDown)	{ return TXB_MOUSEUP; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return CLabel::MouseDown( tMouse, nDown ); }
	int		MouseWheelDown(mouse_t *tMouse)		{ return CLabel::MouseWheelDown( tMouse ); }
	int		MouseWheelUp(mouse_t *tMouse)		{ return CLabel::MouseWheelUp( tMouse ); }
	int		KeyDown(UnicodeChar c, int keysym)	{ return CLabel::KeyDown( c, keysym ); }
	int		KeyUp(UnicodeChar c, int keysym)	{ return CLabel::KeyUp( c, keysym ); }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return CLabel::SendMessage( iMsg, Param1, Param2 ); }
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return CLabel::SendMessage( iMsg, sStr, Param ); }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return CLabel::SendMessage( iMsg, sStr, Param ); }

	void	Draw(SDL_Surface *bmpDest);

	static CWidget * WidgetCreator( const std::vector< CGuiSkin::WidgetVar_t > & p )
	{
		CTextButton * w = new CTextButton( p[0].s, p[1].c, p[2].c );
		w->cClick.Init( p[3].s, w );
		return w;
	};
	
	void	ProcessGuiSkinEvent(int iEvent) 
	{
		if( iEvent == TXB_MOUSEUP )
			cClick.Call();
	};
};

static bool CTextButton_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "textbutton", & CTextButton::WidgetCreator )
							( "text", CGuiSkin::WVT_STRING )
							( "color", CGuiSkin::WVT_COLOR )
							( "glowcolor", CGuiSkin::WVT_COLOR )
							( "click", CGuiSkin::WVT_STRING );

#endif  //  __CTEXTBUTTON_H__
