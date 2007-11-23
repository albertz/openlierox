/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Label
// Created 30/6/02
// Jason Boettcher


#ifndef __CLABEL_H__
#define __CLABEL_H__


#include "InputEvents.h"
#include "StringUtils.h"


// Label events
enum {
	LBL_NONE=-1
};

// Label messages
enum {
	LBS_SETTEXT
};

class CLabel : public CWidget {
public:
	CLabel() {}

	// Constructor
	CLabel(const std::string& text, Uint32 col) {
		sText = text;
		iColour = col;
		iType = wid_Label;
		bVar = NULL;
		iVar = NULL;
		fVar = NULL;
		sVar = NULL;
	}


private:
	// Attributes

	std::string	sText;
	Uint32	iColour;

	bool		*bVar;
	int			*iVar;
	float		*fVar;
	std::string	*sVar;

public:
	// Methods

	void	Create(void) { iWidth = tLX->cFont.GetWidth(sText); iHeight = tLX->cFont.GetHeight(sText); }
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return LBL_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return LBL_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return LBL_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return LBL_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return LBL_NONE; }
	int		KeyDown(UnicodeChar c, int keysym)	{ return LBL_NONE; }
	int		KeyUp(UnicodeChar c, int keysym)	{ return LBL_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return 0; }
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { 
			if (iMsg == LBS_SETTEXT) 
				{sText = sStr; iWidth = tLX->cFont.GetWidth(sText); iHeight = tLX->cFont.GetHeight(sText);} 
			return 0; 	}

	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	ChangeColour(Uint32 col)			{ iColour = col; }

	// Draw the label
	inline void	Draw(SDL_Surface *bmpDest) {
		if (bRedrawMenu)
			redrawBuffer();
		if( bVar )
			if( *bVar )
				sText = "yes";
			else
				sText = "no";
		else if( iVar )
			sText = itoa( *iVar );
		else if( fVar )
			sText = ftoa( *fVar );
		else if( sVar )
			sText = *sVar;
		tLX->cFont.Draw(bmpDest, iX, iY, iColour,sText); 
	}

	void	LoadStyle(void) {}

	static CWidget * WidgetCreator( const std::vector< CGuiSkin::WidgetVar_t > & p )
	{
		CLabel * w = new CLabel( p[0].s, p[1].c );
		w->bVar = CGuiSkin::GetVar( p[2].s, CGuiSkin::SVT_BOOL ).b;
		w->iVar = CGuiSkin::GetVar( p[2].s, CGuiSkin::SVT_INT ).i;
		w->fVar = CGuiSkin::GetVar( p[2].s, CGuiSkin::SVT_FLOAT ).f;
		w->sVar = CGuiSkin::GetVar( p[2].s, CGuiSkin::SVT_STRING ).s;
		return w;
	};
	
	void	ProcessGuiSkinEvent(int iEvent) {};
};

static bool CLabel_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "label", & CLabel::WidgetCreator )
							( "text", CGuiSkin::WVT_STRING )
							( "color", CGuiSkin::WVT_COLOR )
							( "var", CGuiSkin::WVT_STRING );

#endif  //  __CLABEL_H__
