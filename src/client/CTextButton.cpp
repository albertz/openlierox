/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "CTextButton.h"
#include "Cursor.h"


int		CTextButton::MouseOver(mouse_t *tMouse)
{
	SetGameCursor(CURSOR_HAND);
	bMouseOver = true;
	return TXB_MOUSEOVER;
};

void	CTextButton::Draw(const SmartPointer<SDL_Surface> & bmpDest) 
{
	CLabel::ChangeColour( bMouseOver ? iColGlow : iColNormal );
	bMouseOver = false;
	CLabel::Draw( bmpDest );
};

static bool CTextButton_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "textbutton", & CTextButton::WidgetCreator )
							( "text", CScriptableVars::SVT_STRING )
							( "color", CScriptableVars::SVT_COLOR )
							( "glowcolor", CScriptableVars::SVT_COLOR )
							( "click", CScriptableVars::SVT_STRING );
