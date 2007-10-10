// Dialog class implementation
#include "CDialog.h"

////////////////
// Draw me
void CDialog::Draw(CSurface &dest)
{
	// Weird, buffer not initialized...
	if (!tBuffer)
		return;

	// Check if we need a repaint
	if (bRepaintRequired || ChildrenNeedRepaint())
		Repaint();

	// Draw
	dest.DrawImage(tBuffer, iX, iY);
}

//////////////
// Repaint the dialog
void CDialog::Repaint()
{
	// Background
	if (bmpBackground)
		tBuffer.DrawImage(bmpBackground, 0, 0);
	else
		tBuffer.Fill(iBgColor);

	// Title
	if (bTitlebarVisible && iTitlebarHeight > 0)  {
		if (!bmpTitleLeft || !bmpTitleMain || !bmpTitleRight)
			tBuffer.FillRect(0, 0, iWidth, iTitlebarHeight);
		else  {
			// Left part
			if (bmpTitleLeft) 
				tBuffer.DrawImage(bmpTitleLeft, 0, 0);

			// Main bit
			if (bmpTitleMain)
				tBuffer.DrawImageTiled(bmpTitleMain,
										bmpTitleLeft.getWidth(),
										0,
										iWidth - bmpTitleLeft.getWidth() - bmpTitleRight.getWidth(),
										bmpTitleMain.getHeight());

			// Right part
			if (bmpTitleRight)
				tBuffer.DrawImage(bmpTitleRight, iWidth - bmpTitleRight.getWidth(), 0);
		}
	}

	// Border
	cBorder.Draw(tBuffer);

	// Close button
	cCloseButton.Draw(tBuffer);

	// Subcontrols
	DrawChildren(tBuffer);
}

/////////////////
// Setup the dialog
void CDialog::Setup(int x, int y, int w, int h)	
{
	iX = x;
	iY = y;
	iWidth = w;
	iHeight = h;

	tBuffer = CSurface(w, h);
	cBorder = CBorder(0, 0, w, h);

	bRepaintRequired = true;
}

/////////////////
// Mouse down event
void CDialog::DoMouseDown(int x, int y, MouseButtons buttons, KeyState keys)
{
	// Grab the dialog if the mouse has been clicked in the titlebar
	if (buttons.Left)
		bGrabbed =  x >= iX && x <= iX + iWidth && y >= iY && y <= iY + iTitlebarHeight;

	// Process the children
	if (!ProcessMouseDown(x, y, buttons, keys))
		if (OnClick)
			OnClick(this, x, y, buttons, keys);
}

/////////////////
// Mouse click
void CDialog::DoMouseClick(int x, int y, MouseButtons buttons, KeyState keys)
{
	bGrabbed = false; // Mouse released

	// Process the children
	if (!ProcessMouseClick(x, y, buttons, keys))
		if (OnClick)
			OnClick(this, x, y, buttons, keys);
}

////////////////
// Moving the mouse
void CDialog::DoMouseMove(int x, int y, MouseButtons buttons, KeyState keys)
{
	// User is moving the dialog
	if (bGrabbed && buttons.Left)  {
		iX += System->Mouse->DeltaX;
		iY += System->Mouse->DeltaY;

		// Make sure the dialog is in the screen
		iX = CLAMP(iX, 0, System->Screen->Width - Width);
		iY = CLAMP(iY, 0, System->Screen->Height - Height);
	}

	// Process the children
	if (!ProcessMouseMove(x, y, buttons, keys))
		if (OnMouseMove)
			OnMouseMove(this, x, y, buttons, keys);
}
