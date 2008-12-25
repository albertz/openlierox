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

#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "DeprecatedGUI/CTextButton.h"
#include "Cursor.h"


namespace DeprecatedGUI {

int		CTextButton::MouseOver(mouse_t *tMouse)
{
	SetGameCursor(CURSOR_HAND);
	bMouseOver = true;
	return TXB_MOUSEOVER;
};

void	CTextButton::Draw(SDL_Surface * bmpDest) 
{
	CLabel::ChangeColour( bMouseOver ? iColGlow : iColNormal );
	bMouseOver = false;
	CLabel::Draw( bmpDest );
};

static bool CTextButton_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "textbutton", & CTextButton::WidgetCreator )
							( "text", SVT_STRING )
							( "color", SVT_COLOR )
							( "glowcolor", SVT_COLOR )
							( "click", SVT_STRING );

}; // namespace DeprecatedGUI
