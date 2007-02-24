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


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Create the text box
void CTextbox::Create(void)
{
	iCurpos = 0;
	iLastCurpos = 0;
	iLength = 0;
	sText = "";
	iMax = MAX_TEXTLENGTH;
	iHolding = false;
	iHoldingMouse = false;
	fTimeHolding = 0;
	fTimePushed = -9999;
	fLastRepeat = -9999;
	iLastchar = -1;
}


///////////////////
// Draw the text box
void CTextbox::Draw(SDL_Surface *bmpDest)
{
	std::string buf = "";
	std::string text = "";

	text = sText;

    Menu_redrawBufferRect(iX,iY, iWidth,iHeight);
	Menu_DrawBoxInset(bmpDest, iX, iY, iX+iWidth, iY+iHeight);

	int i;
	if(iFlags & TXF_PASSWORD) {

		// Draw astericks for password
		for(std::string::iterator t=text.begin();t!=text.end();t++)
			*t='*';
	}

	int cursorpos = iCurpos;

	// User can scroll to left
	cursorpos -= iScrollPos;
	if (cursorpos < 0)  {
		cursorpos = 5;  // Number of characters, that will be displayed on left if we scroll left and cursor is on the most left
		iScrollPos -= 5;
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
	text.erase(0,iScrollPos);

	// The scrollpos can be 0 and the text still overlapps
	// User can move in the editbox using keys/mouse
	i=iLength-1;
	text = strip(text,iWidth-5);

	// Determine the cursor position in pixels
	int x = 0;
	if(cursorpos)  {
		buf = text.substr(0,(unsigned int)cursorpos);
	}

	x = tLX->cFont.GetWidth(buf);

	// Draw selection
	if (iSelLength)  {
		buf = ""; // Delete the buffer
		int x2 = 0;  // Position of the non-cursor side of the selection

		// The cursor is on the right side of the selection
		if (iSelLength < 0)  {
			size_t length = -iSelLength;
			if (length > text.length())
				length = cursorpos;
			buf = text.substr(cursorpos-length,(unsigned int)length);

			// Update the SelStart
			iSelStart = iCurpos+iSelLength;
		}
		// The cursor is on the left side of the selection
		else  {
			size_t length = iSelLength;
			if (length > text.length())
				length = text.length();//cursorpos;
			buf = text.substr(cursorpos,(unsigned int)length);
			// Update the SelStart
			iSelStart = iCurpos;
		}

		// Update the selected text
		sSelectedText = sText.substr(iSelStart,(unsigned int)abs(iSelLength));

		// Cursor on the left side of the selection
		if (iSelLength > 0)  {
			x2 = iX+x+3+tLX->cFont.GetWidth(buf);  // Count the position
			if(x2 > iX+iWidth-3)
				x2 = iX+iWidth-3;
			DrawRectFill(bmpDest,iX+x+4,iY+3,x2,iY+iHeight-3,MakeColour(0,100,150));
		}
		// Cursor on the right side of the selection
		else  {
			x2 = iX+x+3-tLX->cFont.GetWidth(buf);  // Count the position
			if (x2 < iX+3)
				x2 = iX+3;
			DrawRectFill(bmpDest,x2,iY+3,iX+x+3,iY+iHeight-3,MakeColour(0,100,150));
		}
	}

	// Draw text
	tLX->cFont.Draw(bmpDest, iX+3, iY+3, tLX->clTextBox,  text);

	// Draw cursor only when focused
	if(iFocused) {

		if(buf == "" && iScrollPos)
			iScrollPos--;

		if ((GetMilliSeconds()-fBlinkTime) > 0.5)  {
			iDrawCursor = !iDrawCursor;
			fBlinkTime = GetMilliSeconds();
		}

		if (iDrawCursor)  {
			DrawVLine(bmpDest, iY+3, iY+iHeight-3, iX+x+3,MakeColour(50,150,200));
		}
	}
}


///////////////////
// Keydown event
int CTextbox::KeyDown(int c)
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
	if((char) c == '\b') {
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
		if(iCurpos <= iLength)  {
			if (bShift)  {
				if (iCurpos != iLength)
					iSelLength--;
			}
			else
				iSelLength = 0;

			if (iCurpos != iLength)
				iCurpos++;
		}

		// Prevents weird behavior when we're at the end of text
		if(iScrollPos)  {
			if(tLX->cFont.GetWidth(sText.substr(iScrollPos)) < (iWidth-4))
				return TXT_NONE;
		}

		// Set the scroll position
		if (iCurpos)
			if(tLX->cFont.GetWidth(sText.substr(0,iCurpos)) > (iWidth-7))
				iScrollPos++;

		return TXT_NONE;
	}

	// Home
	if(c == SDLK_HOME) {
		// If the shift key is down, select the text
		if (bShift)
			if (iSelLength > 0)
				iSelLength += iCurpos;
			else
				iSelLength = iCurpos;
		else
			iSelLength = 0;

		// Safety
		if (iSelLength > iLength)
			iSelLength = iLength;

		iCurpos=0;
		iScrollPos=0;
		return TXT_NONE;
	}

	// End
	if(c == SDLK_END) {
		if (bShift)
			iSelLength = -(iLength-iCurpos);
		else
			iSelLength = 0;

		iCurpos = iLength;

		if (tLX->cFont.GetWidth(sText) > iWidth-3)  {
			iScrollPos = 0;
			static std::string buf;
			for (std::string::iterator it=sText.begin(); it != sText.end(); it++)  {
				iScrollPos++;
				if (tLX->cFont.GetWidth(it) < iWidth)
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
	if((char) c == '\r') {
		return TXT_ENTER;
	}

    // Ctrl-v (paste)
    if((char) c == 22 ) {
        PasteText();
        return TXT_CHANGE;
    }

    // Ctrl-c (copy)
    if((char) c == 3 ) {
        CopyText();
        return TXT_NONE;
    }

    // Ctrl-x (cut)
    if((char) c == 24 ) {
        CopyText();
		Delete();
        return TXT_CHANGE;
    }

	if((char)c > 31 && (char)c <127)
		// Insert character
		Insert((char) c);

	return TXT_CHANGE;
}


///////////////////
// Keyup event
int CTextbox::KeyUp(int c)
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

	// Scroll the textbox using mouse
	fScrollTime += tLX->fDeltaTime;

	// Save the current position
	iLastCurpos = iCurpos;

	bool scrolled = false;

	if(fScrollTime > 0.05f)  {
		// Scroll left
		if(iScrollPos && tMouse->X-iX <= -5)  {
			if(iScrollPos > 0)
				iScrollPos--;
				iCurpos--;
				scrolled = true;
		}

		// Scroll right
		if(tLX->cFont.GetWidth(sText.substr(iScrollPos)) > iWidth-5 && tMouse->X > iX+iWidth)  {
			if (iCurpos <= iLength)  {
				iScrollPos++;
				iCurpos++;
				scrolled = true;
			}
		}

		fScrollTime = 0;
	}

	// If we click somewhere behind the text, but in the text box, set the cursor to the end
	if(deltaX > tLX->cFont.GetWidth(sText.substr(iScrollPos)) && InBox(tMouse->X,tMouse->Y))  {
		iLastCurpos = iCurpos = iLength;
	}
	else  {
		// Set the new cursor pos
		if (deltaX < (tLX->cFont.GetWidth(sText.substr(0,1))/2))  // Area before the first character
			iCurpos = iScrollPos;
		else  {
			int curWidth = 0;
			int nextWidth = 0;
			for(int i=iScrollPos; i<iLength-1; i++)  {
				curWidth = tLX->cFont.GetWidth(sText.substr(iScrollPos,i));
				nextWidth = tLX->cFont.GetWidth(sText.substr(iScrollPos,i+1));
				if ((curWidth + (nextWidth-curWidth)/3) >= deltaX)  {
					iCurpos = i;
					break;
				}  // if
			} // for
		}  // else 2
	}  // else 1

	fTimeHolding += tLX->fDeltaTime;

	// Holding the mouse
	if (fTimeHolding > 0.25)
		iHoldingMouse = true;

	// Safety (must be *after* mouse scrolling)
	if (iCurpos > iLength)  {
		iCurpos = iLength;
	}

	if(iHoldingMouse)  {
		if ((abs(tMouse->X-iLastMouseX) && (iCurpos-iLastCurpos)) || (scrolled))  {
			//MessageBeep(0);
			if (scrolled)
				scrolled = true;
			iSelLength += -(iCurpos-iLastCurpos);
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

	// We can scroll using mouse
	//if (iHolding)  {


	//}

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

//	memmove(sText+iCurpos-1,sText+iCurpos,iLength-iCurpos+1);
	sText = sText.substr(0,iCurpos-1)+sText.substr(iCurpos);

	iCurpos--;
	iLength--;
	if (iScrollPos)
		iScrollPos--;
}


///////////////////
// Delete
void CTextbox::Delete(void)
{
	// Delete selection
	if(iSelLength)  {
		sText.erase(iSelStart,abs(iSelLength));
		iCurpos = iSelStart;
		iSelLength = 0;
		iLength = sText.length();
		if (iScrollPos > iCurpos)
			iScrollPos = iCurpos;
		return;
	}


	if(iCurpos >= iLength)
		return;

	//memmove(sText+iCurpos,sText+iCurpos+1,iLength-iCurpos+1);
	sText.erase(iCurpos,1);

	iLength--;
}


///////////////////
// Insert a character
void CTextbox::Insert(char c)
{
	// Delete any selection
	if (iSelLength)
		Delete();

	// Check for the max
	if(iLength >= iMax-2)
		return;

	if(c==0)
		return;

	// Safety
	if(iCurpos > iLength)
		iCurpos = iLength;

	//memmove(sText+iCurpos+1,sText+iCurpos,iLength-iCurpos+1);
	char buf[2];
	buf[0]=c;
	buf[1]=0;
	sText.insert(iCurpos++,buf);
	iLength++;

	//sText[iCurpos++] = c;
	//sText[++iLength] = '\0';

    // If the text size is greater than the textbox size, scroll the text
	if (iCurpos == iLength)
		while(tLX->cFont.GetWidth(&sText[iScrollPos]) > iWidth-5) {
			iScrollPos++;
		}
}

/////////////////////
// Replace the current text with the buf
void CTextbox::setText(const std::string& buf)
{
	sText = "";

	// Copy the text and ignore unknown characters
	for (std::string::const_iterator it=buf.begin(); it != buf.end(); it++)
		if(*it > 31 && *it <127)
			sText += *it;

	iCurpos=iLength=sText.length();
	iScrollPos=0;
	iSelStart=iCurpos;
	iSelLength=0;
	sSelectedText = "";
	iLastCurpos = iCurpos;
	fScrollTime = 0;
}

///////////////////
// This widget is send a message
DWORD CTextbox::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{

	switch(iMsg) {

		// Get the text length
		case TXM_GETTEXTLENGTH:
			return iLength;
			break;

		// Set some flags
		case TXM_SETFLAGS:
			iFlags = Param1;
			break;

		// Set the max length
		case TXM_SETMAX:
			iMax = Param1;
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
	for(std::string::const_iterator i = text.begin(); i != text.end(); i++)
		Insert( *i );
}

///////////////////
// Copy selection to the clipboard
void CTextbox::CopyText(void)
{
	if (!iSelLength)
		return;
	SetClipboardText(sSelectedText);
}
