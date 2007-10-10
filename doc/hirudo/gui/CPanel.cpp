// CPanel class

#include "CPanel.h"

void CPanel::Draw(CSurface &dest)
{
	// Buffer not initialized...
	if (!tBuffer)
		return;

	// Repaint if needed
	if (bRepaintRequired)
		Repaint();

	dest.DrawImage(tBuffer, 0, 0);
}

void CPanel::Repaint()
{
	// Background
	if (bmpBackground)
		tBuffer.DrawImage(bmpBackground, 0, 0);

	// Border
	cBorder.Draw(tBuffer);

	// Subwidgets
	DrawChildren(tBuffer);
}

void CPanel::Setup(int x, int y, int w, int h)
{
	iX = x;
	iY = y;
	iWidth = w;
	iHeight = h;

	bRepaintRequired = true;
}