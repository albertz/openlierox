/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
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
	sText[0] = 0;
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
	static char buf[MAX_TEXTLENGTH] = "";
	static char text[MAX_TEXTLENGTH] = "";

	fix_strncpy(text, sText);

    Menu_redrawBufferRect(iX,iY, iWidth,iHeight);
	Menu_DrawBoxInset(bmpDest, iX, iY, iX+iWidth, iY+iHeight);

	int i, len;
	if(iFlags & TXF_PASSWORD) {

		// Draw astericks for password
		len = (int)strlen(sText);
		for(i=0;i<len;i++)
			text[i]='*';
		text[i+1] = '\0';
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
		sSelectedText[0] = '\0';
		iCurpos = 0;
		iLastCurpos = 0;
		fTimeHolding = 0;
		iHoldingMouse = false;
		iLastMouseX = 0;
	}*/
	
	// Shift the text, if it overlapps
	static char tmp[MAX_TEXTLENGTH] = "";
	fix_strncpy(tmp, &text[iScrollPos]);
	fix_strncpy(text, tmp);
	
	// The scrollpos can be 0 and the text still overlapps
	// User can move in the editbox using keys/mouse
	i=iLength-1;
	strip(text,iWidth-5);

	// Determine the cursor position in pixels
	int x = 0;
	if(cursorpos)  {
		strncpy(buf,text,MIN(sizeof(buf)-1,(unsigned int)cursorpos));
	}

	buf[MIN(sizeof(buf)-1,(unsigned int)cursorpos)] = '\0';
	x = tLX->cFont.GetWidth(buf);
	
	// Draw selection
	if (iSelLength)  {
		buf[0] = '\0'; // Delete the buffer
		int x2 = 0;  // Position of the non-cursor side of the selection

		// The cursor is on the right side of the selection
		if (iSelLength < 0)  { 
			int length = -iSelLength;
			if (length > (int)fix_strnlen(text))
				length = cursorpos;
			strncpy(buf,&text[cursorpos-length],MIN(sizeof(buf)-1,(unsigned int)length));
			buf[MIN(sizeof(buf)-1,(unsigned int)length)] = '\0';
			
			// Update the SelStart
			iSelStart = iCurpos+iSelLength;
		}
		// The cursor is on the left side of the selection
		else  {
			int length = iSelLength;
			if (length > (int)fix_strnlen(text))
				length = fix_strnlen(text);
			strncpy(buf,&text[cursorpos],MIN(sizeof(buf)-1,(unsigned int)length));
			buf[MIN(sizeof(buf)-1,(unsigned int)length)] = '\0';
			// Update the SelStart
			iSelStart = iCurpos;
		}

		// Update the selected text
		strncpy(sSelectedText,&sText[iSelStart],MIN(sizeof(sSelectedText),(unsigned int)abs(iSelLength)));
		sSelectedText[MIN(sizeof(sSelectedText)-1,(unsigned int)abs(iSelLength))] = '\0';
		
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
	tLX->cFont.Draw(bmpDest, iX+3, iY+3, tLX->clTextBox, "%s", text);

	// Draw cursor only when focused
	if(iFocused) {

		if(strlen(buf) == 0 && iScrollPos)
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
		static char buf[MAX_TEXTLENGTH];
		if(iScrollPos)  {
			fix_strncpy(buf,&sText[iScrollPos]);
			if(tLX->cFont.GetWidth(buf) < (iWidth-4))
				return TXT_NONE;
		}

		// Set the cursor position
		if(iCurpos)  {
			strncpy(buf,sText,iCurpos);
			buf[iCurpos] = '\0';
		}

		// Set the scroll position
		if(tLX->cFont.GetWidth(buf) > (iWidth-7))
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
			static char buf[MAX_TEXTLENGTH];
			for (int i=0; i<iLength; i++)  {
				fix_strncpy(buf,&sText[i]);
				iScrollPos++;
				if (tLX->cFont.GetWidth(buf) < iWidth)
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

	static char buf[MAX_TEXTLENGTH];

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
		if(tLX->cFont.GetWidth(&sText[iScrollPos]) > iWidth-5 && tMouse->X > iX+iWidth)  {
			if (iCurpos <= iLength)  {
				iScrollPos++;
				iCurpos++;
				scrolled = true;
			}
		}

		fScrollTime = 0;
	}

	// If we click somewhere behind the text, but in the text box, set the cursor to the end
	if(deltaX > tLX->cFont.GetWidth(&sText[iScrollPos]) && InBox(tMouse->X,tMouse->Y))
		iCurpos = iLength;
	else  {
		fix_strncpy(buf,sText);
		buf[1] = '\0';
		// Set the new cursor pos
		if (deltaX < (tLX->cFont.GetWidth(buf)/2))  // Area before the first character
			iCurpos = iScrollPos;
		else  {
			buf[0] = '\0';  // Delete the buffer
			int curWidth = 0;
			int nextWidth = 0;
			int j=0;
			for(int i=iScrollPos; i<iLength-1; i++,j++)  {
				buf[j] = sText[i];
				buf[j+1] = '\0';
				curWidth = tLX->cFont.GetWidth(buf);
				buf[j+1] = sText[i+1];
				buf[j+2] = '\0';
				nextWidth = tLX->cFont.GetWidth(buf);
				if ((curWidth + (nextWidth-curWidth)/3) >= deltaX)  {
					iCurpos = i+1;
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
		sSelectedText[0] = '\0';
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

	memmove(sText+iCurpos-1,sText+iCurpos,iLength-iCurpos+1);

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
		memmove(sText+iSelStart,sText+iSelStart+abs(iSelLength),iLength-(iSelStart+abs(iSelLength))+1);
		iCurpos = iSelStart;
		iSelLength = 0;
		iLength = strlen(sText);
		if (iScrollPos > iCurpos)
			iScrollPos = iCurpos;
		return;
	}


	if(iCurpos >= iLength)
		return;

	memmove(sText+iCurpos,sText+iCurpos+1,iLength-iCurpos+1);
	
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

	memmove(sText+iCurpos+1,sText+iCurpos,iLength-iCurpos+1);
	
	sText[iCurpos++] = c;
	sText[++iLength] = '\0';

    // If the text size is greater than the textbox size, scroll the text
	if (iCurpos == iLength)
		while(tLX->cFont.GetWidth(&sText[iScrollPos]) > iWidth-5) {
			iScrollPos++;
		}
}

/////////////////////
// Replace the current text with the buf
void CTextbox::setText(char *buf)
{
	// Copy the text and ignore unknown characters
	int j=0;
	for (int i=0; i<(int)strlen(buf); i++)
		if(buf[i] > 31 && buf[i] <127)
			sText[j++] = buf[i];

	sText[j] = '\0';
	
	iCurpos=iLength=strlen(buf); 
	iScrollPos=0; 
	iSelStart=iCurpos; 
	iSelLength=0;
	sSelectedText[0] = '\0';
	iLastCurpos = iCurpos;
	fScrollTime = 0;
}

///////////////////
// This widget is send a message
int CTextbox::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{
	char *p;

	switch(iMsg) {

		// Get the text
		case TXM_GETTEXT:
			strncpy((char *)Param1, sText, Param2);
			p = (char *)Param1;
			p[Param2-1] = '\0';
			break;

		// Get the text length
		case TXM_GETTEXTLENGTH:
			return iLength;
			break;

		// Set the text
		case TXM_SETTEXT:
			setText( (char *)Param1 );
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


///////////////////
// Paste some text from the clipboard
void CTextbox::PasteText(void)
{
    static char text[MAX_TEXTLENGTH];

	if(iSelLength)
		Delete();

    int length = GetClipboardText(text, MAX_TEXTLENGTH-1);

    if(length > 0 ) {
        // Insert the text
        for( int i=0; i<length; i++)
            Insert( text[i] );
    }
}

///////////////////
// Copy selection to the clipboard
void CTextbox::CopyText(void)
{
	if (!iSelLength)
		return;
	SetClipboardText(sSelectedText);
}
