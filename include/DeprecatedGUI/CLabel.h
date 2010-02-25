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


#ifndef __CLABEL_H__DEPRECATED_GUI__
#define __CLABEL_H__DEPRECATED_GUI__



#include "InputEvents.h"
#include "StringUtils.h"
#include "Color.h"


namespace DeprecatedGUI {

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
	CLabel(const std::string& text, Color col, bool center = false) {
		sText = text;
		iColour = col;
		iType = wid_Label;
		bCenter = center;
		bVar = NULL;
		iVar = NULL;
		fVar = NULL;
		sVar = NULL;
	}


private:
	// Attributes

	std::string	sText;
	Color		iColour;
	bool		bCenter;

	bool		*bVar;
	int			*iVar;
	float		*fVar;
	std::string	*sVar;

public:
	// Methods

	void	Create() 
	{ 
		iWidth = tLX->cFont.GetWidth(sText); 
		iHeight = tLX->cFont.GetHeight(sText);
		if( bCenter ) 
			iX -= iWidth / 2;
	}
	void	Destroy() { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return LBL_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return LBL_NONE; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return LBL_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return LBL_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return LBL_NONE; }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return LBL_NONE; }
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)	{ return LBL_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return 0; }
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { 
			if (iMsg == LBS_SETTEXT) 
				{sText = sStr; iWidth = tLX->cFont.GetWidth(sText); iHeight = tLX->cFont.GetHeight(sText);} 
			return 0; 	}

	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	ChangeColour(Color col)			{ iColour = col; }
	void	setText(const std::string& sStr)	{sText = sStr; iWidth = tLX->cFont.GetWidth(sText); iHeight = tLX->cFont.GetHeight(sText);};
	std::string getText() const { return sText; }
	
	// Draw the label
	inline void	Draw(SDL_Surface * bmpDest) {
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
		if( bCenter )
		{
			int width = tLX->cFont.GetWidth(sText);
			iX += ( iWidth - width ) / 2;
			iWidth = width;
		}
		tLX->cFont.Draw(bmpDest, iX, iY, iColour,sText); 
	}

	void	LoadStyle() {}

	static CWidget * WidgetCreator( const std::vector< ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
	{
		CLabel * w = new CLabel( p[0].str.get(), p[1].col.get(), p[2].b );
		w->bVar = CScriptableVars::GetVarP<bool>( p[3].str.get() );
		w->iVar = CScriptableVars::GetVarP<int>( p[3].str.get() );
		w->fVar = CScriptableVars::GetVarP<float>( p[3].str.get() );
		w->sVar = CScriptableVars::GetVarP<std::string>( p[3].str.get() );
		layout->Add( w, id, x, y, dx, dy );
		return w;
	}
	
	void	ProcessGuiSkinEvent(int iEvent) {};
};

}; // namespace DeprecatedGUI

#endif  //  __CLABEL_H__DEPRECATED_GUI__
