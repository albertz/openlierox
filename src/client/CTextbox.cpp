/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Text box
// Created 30/6/02
// Jason Boettcher

/*
	HINT: all string-positions used in this code are interpreted as the position if it were an Utf8 encoded string (and of course also the Draw-functions handle it like this)
*/

#include <string>

#include "AuxLib.h"
#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"


///////////////////
// Create the text box
void CTextbox::Create(void)
{
	iCurpos = 0;
	iLastCurpos = 0;
	sText = "";
	iMax = -1; // highest possible size_t value
	iHolding = false;
	iHoldingMouse = false;
	fTimeHolding = 0;
	fTimePushed = -9999;
	fLastRepeat = -9999;
	iLastchar = 0;
}


///////////////////
// Draw the text box
void CTextbox::Draw(SDL_Surface *bmpDest)
{
	std::string buf = "";
	std::string text = sText;

    Menu_redrawBufferRect(iX,iY, iWidth,iHeight);
	Menu_DrawBoxInset(bmpDest, iX, iY-2, iX+iWidth, iY+iHeight+2);

	if(iFlags & TXF_PASSWORD) {

		// Draw astericks for password
		text = "";
		size_t len = Utf8StringSize(sText);
		for(size_t i = 0; i < len; i++)
			text += '*';
	}

	int cursorpos = iCurpos;

	// User can scroll to left
	cursorpos -= iScrollPos;
	if (cursorpos < 0)  {
		cursorpos = 5;  // Number of characters, that will be displayed on left if we scroll left and cursor is on the most left
		iScrollPos -= 5;// TODO utf string
		if (iScrollPos < 0)
			iScrollPos = 0;
	}

	// Destroy the selection when we lose focus
	/*if(!iFocused)  {
		iSelLength = 0;
		iSelStart = 0;
		sSelectedText = "";
		iCurpos = 0;
		iLastCurpos = 0;
		fTimeHolding = 0;
		iHoldingMouse = false;
		iLastMouseX = 0;
	}*/

	// Shift the text, if it overlapps
	Utf8Erase(text,0,iScrollPos);

	// Draw selection
	if (iSelLength)  {
		buf = ""; // Delete the buffer
		int x1,x2;  // Positions of the selection
		x1=x2=0;

		if (iSelLength > 0)  
			iSelStart = iCurpos;
		else
			iSelStart = iCurpos + iSelLength;


		if (iSelStart <= iScrollPos) {
			x1 = 3;
			x2 = MIN(
				x1 + tLX->cFont.GetWidth(
					Utf8SubStr(text, 0, abs(iSelLength) - iScrollPos + iSelStart)),
				iWidth - 3);
		} else {
			x1 = 3 + tLX->cFont.GetWidth(
				Utf8SubStr(text, 0, iSelStart - iScrollPos));
			x2 = MIN(
				x1 + tLX->cFont.GetWidth(
					Utf8SubStr(text, iSelStart - iScrollPos, abs(iSelLength))),
				iWidth - 3);
		}

		// Update the selected text
		sSelectedText = Utf8SubStr(sText, iSelStart, abs(iSelLength));

		if (x1 < x2)
			DrawRectFill(
				bmpDest,
				iX + x1, iY + 2, iX + x2, iY + iHeight,
				tLX->clSelection);
	}

	// Draw text
	tLX->cFont.DrawAdv(bmpDest, iX + 3, iY, iWidth-4, tLX->clTextBox, text);

	// Draw cursor (only when focused)
	if(iFocused) {

		if ((GetMilliSeconds()-fBlinkTime) > 0.5)  {
			iDrawCursor = !iDrawCursor;
			fBlinkTime = GetMilliSeconds();
		}

		if (iDrawCursor)  {
			// Determine the cursor position in pixels
			int x = tLX->cFont.GetWidth(Utf8SubStr(text, 0, (unsigned int)cursorpos));

			DrawVLine(
				bmpDest,
				iY + 3, iY + iHeight - 3, MIN(iX + x + 3, iX + iWidth),
				tLX->clTextboxCursor);
		}
	}
}


///////////////////
// Keydown event
int CTextbox::KeyDown(UnicodeChar c)
{
	keyboard_t *kb = GetKeyboard();

    if(c == 0)  {
        return -1;
	}


	bool bShift = kb->KeyDown[SDLK_RSHIFT] || kb->KeyDown[SDLK_LSHIFT];

	iDrawCursor = true;

	// Handle holding keys
	if(iHolding) {
		if(iLastchar != c)
			iHolding = false;
		else {
			if(tLX->fCurTime - fTimePushed < 0.25f)
				return TXT_NONE;
			if (tLX->fCurTime - fLastRepeat < 0.03f)
				return TXT_NONE;
			fLastRepeat = tLX->fCurTime;
		}
	}

	if(!iHolding) {
		iHolding = true;
		fTimePushed = tLX->fCurTime;
	}

	iLastchar = c;


	// Backspace
	if(c == '\b') {
		Backspace();
		return TXT_CHANGE;
	}


	// Left arrow
	if (c == SDLK_LEFT) {
		if(iCurpos >= 0) {
			if (bShift)  {
				if (iCurpos)
					iSelLength++;
			}
			else
				iSelLength = 0;

			if(iCurpos)
				iCurpos--;
		}
		return TXT_NONE;
	}

	// Right arrow
	if(c == SDLK_RIGHT) {
		if(iCurpos < Utf8StringSize(sText))  {
			if (bShift)  {
				if (iCurpos != Utf8StringSize(sText))
					iSelLength--;
			}
			else
				iSelLength = 0;

			if (iCurpos != Utf8StringSize(sText))
				iCurpos++;
		}

		// Prevents weird behavior when we're at the end of text
		if(iScrollPos)  {
			if(tLX->cFont.GetWidth(Utf8SubStr(sText, iScrollPos, (size_t)-1)) < (iWidth - 4))
				return TXT_NONE;
		}

		// Set the scroll position
		if (iCurpos)
			if(tLX->cFont.GetWidth(Utf8SubStr(sText, 0, iCurpos)) > (iWidth - 7))
				iScrollPos++;

		return TXT_NONE;
	}

	// Home
	if(c == SDLK_HOME) {
		// If the shift key is down, select the text
		if(bShift)
			if(iSelLength > 0)
				iSelLength += iCurpos;
			else
				iSelLength = iCurpos;
		else
			iSelLength = 0;

		// Safety
		if((size_t)iSelLength > Utf8StringSize(sText))
			iSelLength = Utf8StringSize(sText);

		iCurpos = 0;
		iScrollPos = 0;
		return TXT_NONE;
	}

	// End
	if(c == SDLK_END) {
		if (bShift)
			iSelLength = -(int)Utf8StringSize(sText) + iCurpos;
		else
			iSelLength = 0;

		iCurpos = Utf8StringSize(sText);

		if (tLX->cFont.GetWidth(sText) > iWidth - 3)  {
			iScrollPos = 0;
			for(std::string::const_iterator it = sText.begin(); it != sText.end(); IncUtf8StringIterator(it, sText.end())) {
				iScrollPos++;
				if(tLX->cFont.GetWidth(std::string(it, (std::string::const_iterator)sText.end())) < iWidth)
					break;
			}
		}
		else
			iScrollPos = 0;

		return TXT_NONE;
	}

	// Delete
	if(c == SDLK_DELETE) {
		Delete();
		return TXT_CHANGE;
	}

	// Enter
	if(c == '\r') {
		return TXT_ENTER;
	}

    // Ctrl-v (paste)
    if(c == 22 ) {
        PasteText();
        return TXT_CHANGE;
    }

    // Ctrl-c (copy)
    if(c == 3 ) {
        CopyText();
        return TXT_NONE;
    }

    // Ctrl-x (cut)
    if(c == 24 ) {
        CopyText();
		Delete();
        return TXT_CHANGE;
    }

	// Insert character
	Insert(c);

	return TXT_CHANGE;
}


///////////////////
// Keyup event
int CTextbox::KeyUp(UnicodeChar c)
{
	iHolding = false;

	return TXT_NONE;
}

///////////////////
// Mouse down event
int	CTextbox::MouseDown(mouse_t *tMouse, int nDown)
{
	int deltaX = tMouse->X - iX;
	iDrawCursor = true;

	//static std::string buf;

	if (sText == "")
		return TXT_MOUSEOVER;

	// Scroll the textbox using mouse
	fScrollTime += tLX->fDeltaTime;

	// Save the current position
	iLastCurpos = iCurpos;

	bool scrolled = false;

	if(fScrollTime > 0.05f)  {
		// Scroll left
		if(iScrollPos && tMouse->X-iX <= -5)  {
			if(iScrollPos > 0)  {
				iScrollPos--;
				iCurpos--;
				scrolled = true;
			}
		}

		// Scroll right
		else if(tLX->cFont.GetWidth(Utf8SubStr(sText, iScrollPos)) > iWidth-5 && tMouse->X > iX+iWidth)  {
			if (iCurpos <= Utf8StringSize(sText))  {
				iScrollPos++;
				iCurpos++;
				scrolled = true;
			}
		}

		fScrollTime = 0;
	}

	// If we click somewhere behind the text, but in the text box, set the cursor to the end
	if(deltaX > tLX->cFont.GetWidth(Utf8SubStr(sText, iScrollPos)) && InBox(tMouse->X,tMouse->Y))  {
		iCurpos = Utf8StringSize(sText);
	}
	else  {
		// Set the new cursor pos
		if (deltaX < (tLX->cFont.GetWidth(Utf8SubStr(sText, 0, 1)) / 2))  // Area before the first character
			iCurpos = iScrollPos;
		else  {
			std::string buf = Utf8SubStr(sText, iScrollPos);
			size_t pos = Utf8StringSize(sText);
			int w, prev_w;
			w = prev_w = 0;
			while ((w = tLX->cFont.GetWidth(buf)) > deltaX && pos)  {
				pos--;
				Utf8Erase(buf, pos);
				prev_w = w;
			}

			if ((prev_w - w) / 3 < (deltaX - w))
				pos++;

			iCurpos = pos;

		}  // else 2
	}  // else 1

	fTimeHolding += tLX->fDeltaTime;

	// Holding the mouse
	if (fTimeHolding > 0.25)
		iHoldingMouse = true;

	// Safety (must be *after* mouse scrolling)
	if (iCurpos > Utf8StringSize(sText))  {
		iCurpos = Utf8StringSize(sText);
	}

	if(iHoldingMouse)  {
		if ((abs(tMouse->X - iLastMouseX) && (iCurpos - iLastCurpos)) || (scrolled))  {
			if (scrolled)
				scrolled = true;
			iSelLength += -(int)(iCurpos - iLastCurpos);
			// We can't lose the focus
			iCanLoseFocus = false;
		}
	}
	else  {
		// Remove any previous selection
		iSelLength = 0;
		iSelStart = 0;
		sSelectedText = "";
		iLastCurpos = iCurpos;
		// We can lose focus now
		iCanLoseFocus = true;
	}

	// Set up the variables for selection
	//iLastCurpos = iCurpos;
	iLastMouseX = tMouse->X;
	if(!iHoldingMouse)
		iHoldingMouse = true;

	return TXT_MOUSEOVER;

}

///////////////////
// Mouse up event
int	CTextbox::MouseUp(mouse_t *tMouse, int nDown)
{
	// Remove any previous selection

	fTimeHolding = 0;

	iHoldingMouse = false;

	iLastMouseX = 0;

	fScrollTime = 0;  // We can scroll the textbox using mouse

	// We can lose focus now
	iCanLoseFocus = true;

	return TXT_NONE;
}


///////////////////
// Backspace
void CTextbox::Backspace(void)
{
	// Delete any selection
	if(iSelLength)  {
		Delete();
		return;
	}

	if(iCurpos<=0)
		return;

	Utf8Erase(sText, --iCurpos, 1);
	
	if (iScrollPos)
		iScrollPos--;
}


///////////////////
// Delete
void CTextbox::Delete(void)
{
	// Delete selection
	if(iSelLength)  {
		Utf8Erase(sText, iSelStart, abs(iSelLength));
		iCurpos = iSelStart;
		iSelLength = 0;
		if (iScrollPos > iCurpos)
			iScrollPos = iCurpos;
		if (tLX->cFont.GetWidth(sText) <= iWidth-5)
			iScrollPos = 0;
		return;
	}


	if(iCurpos >= Utf8StringSize(sText))
		return;

	Utf8Erase(sText, iCurpos, 1);
}


///////////////////
// Insert a character
void CTextbox::Insert(UnicodeChar c)
{
	// Delete any selection
	if (iSelLength)
		Delete();

	// If no unicode, try to convert to ascii
	if (iFlags & TXF_NOUNICODE)  {
		c = UnicodeCharToAsciiChar(c);
		if (c == 0xFF)  // Cannot convert
			return;
	}

	// Check for the max
	if(Utf8StringSize(sText) >= iMax - 2)
		return;

	// Check that the current font can display this character, if not, quit
	if (!tLX->cFont.CanDisplayCharacter(c))
		return;

	// Safety
	if(iCurpos > Utf8StringSize(sText))
		iCurpos = Utf8StringSize(sText);

	Utf8Insert(sText, iCurpos++, GetUtf8FromUnicode(c));

    // If the text size is greater than the textbox size, scroll the text
	if (iCurpos == Utf8StringSize(sText))
		while(tLX->cFont.GetWidth(Utf8SubStr(sText, iScrollPos)) > iWidth-5) {
			iScrollPos++;
		}
}

/////////////////////
// Replace the current text with the buf
void CTextbox::setText(const std::string& buf) {
	sText = buf;

	iCurpos = Utf8StringSize(sText);
	iScrollPos = 0;
	iSelStart = iCurpos;
	iSelLength = 0;
	sSelectedText = "";
	iLastCurpos = iCurpos;
	fScrollTime = 0;
}

///////////////////
// This widget is send a message
DWORD CTextbox::SendMessage(int iMsg, DWORD Param1, DWORD Param2) {

	switch(iMsg) {

		// Get the text length
		case TXM_GETTEXTLENGTH:
			return Utf8StringSize(sText);
			break;

		// Set some flags
		case TXM_SETFLAGS:
			iFlags = Param1;
			break;

		// Set the max length
		case TXM_SETMAX:
			iMax = Param1;
			// TODO: cut to big string here?
			break;

	}

	return 0;
}

DWORD CTextbox::SendMessage(int iMsg, const std::string& sStr, DWORD Param)
{
	switch (iMsg)  {

	// Get the text
	case TXS_SETTEXT:
		setText(sStr);
		break;
	}

	return 0;
}


DWORD CTextbox::SendMessage(int iMsg, std::string *sStr, DWORD Param)
{
	switch (iMsg)  {

	// Get the text
	case TXS_GETTEXT:
		*sStr = sText;
		return sText != "";
		break;
	}

	return 0;
}


///////////////////
// Paste some text from the clipboard
void CTextbox::PasteText(void)
{
	std::string text;

	if(iSelLength)
		Delete();

    text = GetClipboardText();

	// Insert the text
	for(std::string::const_iterator i = text.begin(); i != text.end(); )
		Insert( GetNextUnicodeFromUtf8(i, text.end()) );
}

///////////////////
// Copy selection to the clipboard
void CTextbox::CopyText(void)
{
	if (!iSelLength)
		return;
	SetClipboardText(sSelectedText);
}
