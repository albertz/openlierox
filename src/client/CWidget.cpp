/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Widget class
// Created 5/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Setup the widget
void CWidget::Setup(int id, int x, int y, int w, int h)
{
	iID = id;
	iX = x;
	iY = y;
	iWidth = w;
	iHeight = h;
	iEnabled = true;	
}


///////////////////
// Returns true if a point is inside this widget
int CWidget::InBox(int x, int y)
{
	if(x > iX && x < iX+iWidth)
		if(y > iY && y < iY+iHeight)
			return true;

	return false;
}


///////////////////
// Redraw the buffer on the widget
void CWidget::redrawBuffer(void)
{
    Menu_redrawBufferRect(iX, iY, iWidth, iHeight);
}
