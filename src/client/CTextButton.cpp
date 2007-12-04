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

enum 
{
	CTextButton_GlowTime = 32	// In frames, 100 = 1 second, GCC will optimize "i / 32" into "i >> 5"
};

int		CTextButton::MouseOver(mouse_t *tMouse)
{
	SetGameCursor(CURSOR_HAND);
	iGlowCoef += 2;
	return TXB_MOUSEOVER;
};

void	CTextButton::Draw(SDL_Surface *bmpDest) 
{
	// Handy macros (optimized with integer-shifts by compiler)
	#define _GetR( C ) ( ( C & ALPHASURFACE_RMASK ) / ( ( ALPHASURFACE_RMASK + ( ALPHASURFACE_RMASK / 0xff ) ) / 0x100 ) )
	#define _GetG( C ) ( ( C & ALPHASURFACE_GMASK ) / ( ( ALPHASURFACE_GMASK + ( ALPHASURFACE_GMASK / 0xff ) ) / 0x100 ) )
	#define _GetB( C ) ( ( C & ALPHASURFACE_BMASK ) / ( ( ALPHASURFACE_BMASK + ( ALPHASURFACE_BMASK / 0xff ) ) / 0x100 ) )
	#define _MakeColor( R, G, B ) ( R * ( ( ALPHASURFACE_RMASK + 1 ) / 0x100 ) + \
									G * ( ( ALPHASURFACE_GMASK + 1 ) / 0x100 ) + \
									B * ( ( ALPHASURFACE_BMASK + 1 ) / 0x100 ) )
	#define	_MiddleVal( V1, V2, Coef ) ( V1 + ( V2 - V1 ) * Coef )
	#define _MiddleColor( C1, C2, Coef ) _MakeColor( \
							(unsigned) _MiddleVal( (int)_GetR( C1 ), (int)_GetR( C2 ), Coef ), \
							(unsigned) _MiddleVal( (int)_GetG( C1 ), (int)_GetG( C2 ), Coef ), \
							(unsigned) _MiddleVal( (int)_GetB( C1 ), (int)_GetB( C2 ), Coef ) )
	
	iGlowCoef --;
	if( iGlowCoef < 0 ) iGlowCoef = 0;
	if( iGlowCoef > CTextButton_GlowTime ) iGlowCoef = CTextButton_GlowTime;
	CLabel::ChangeColour( _MiddleColor( iColNormal, iColGlow, iGlowCoef / CTextButton_GlowTime ) );
	
	CLabel::Draw( bmpDest );
};
