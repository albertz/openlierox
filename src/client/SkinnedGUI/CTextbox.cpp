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

#include "Clipboard.h"
#include "LieroX.h"

#include "AuxLib.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "FindFile.h"
#include "CFont.h"
#include "SkinnedGUI/CTextbox.h"
#include "Cursor.h"
#include "XMLutils.h"


namespace SkinnedGUI {

///////////////////
// Create the text box
CTextbox::CTextbox(COMMON_PARAMS) : CWidget(name, parent)
{
	iType = wid_Textbox;
	bPassword.set(false, DEFAULT_PRIORITY);
	bNoUnicode.set(false,DEFAULT_PRIORITY);
	bDrawCursor = true;
	iScrollPos = 0;
	iSelLength = 0;
	iSelStart = 0;
	sSelectedText = "";
	sText = "";
	iMax = (size_t) -1; // highest possible size_t value
	bHolding = false;
	iCurpos = 0;
	iLastCurpos = 0;
	fTimePushed = AbsTime();
	fLastRepeat = AbsTime();
	fLastClick = AbsTime();
	iLastchar = 0;
	iLastKeysym = 0;
	fScrollTime = 0;  // We can scroll
	clSelection.set(Color(0, 0, 255), DEFAULT_PRIORITY);
	clCaret.set(Color(127, 127, 255), DEFAULT_PRIORITY);
	cBackground.setColor(Color(255, 255, 255), DEFAULT_PRIORITY); // White background as default
	tTimer = NULL;
	CLEAR_EVENT(OnChange);
	CLEAR_EVENT(OnEnterPress);
	
	tTimer = new Timer;
	tTimer->interval = 500;
	tTimer->once = false;
	tTimer->onTimer.handler() = getEventHandler(this, &CTextbox::OnTimerEvent);
	tTimer->userData = (void *)this;
	tTimer->start();
}

/////////////////
// Destroy the textbox
CTextbox::~CTextbox()
{
	delete tTimer;
}

//////////////////
// Handles an event coming from the timer
void CTextbox::OnTimerEvent(Timer::EventData ev)
{
	bDrawCursor = !bDrawCursor;
	Repaint();
}


////////////////////
// Apply a selector to the textbox
void CTextbox::ApplySelector(const CSSParser::Selector &sel, const std::string &prefix)
{
	CWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cBackground.ApplySelector(sel, prefix);
	cFont.ApplySelector(sel, prefix);
	cText.ApplySelector(sel, prefix);

	FOR_EACH_ATTRIBUTE(sel, it)  {
		if (it->getName() == prefix + "selection-color" || it->getName() == prefix + "selection-colour")  {
			clSelection.set(it->getFirstValue().getColor(clSelection), it->getPriority());
		} else if (it->getName() == prefix + "caret-color" || it->getName() == prefix + "caret-colour")  {
			clCaret.set(it->getFirstValue().getColor(clCaret), it->getPriority());
		} else if (it->getName() == prefix + "password")  {
			bPassword.set(it->getFirstValue().getBool(bPassword), it->getPriority());
		} else if (it->getName() == prefix + "no-unicode")  {
			bNoUnicode.set(it->getFirstValue().getBool(bNoUnicode), it->getPriority());
		}
	}

	UpdateSize();

}

///////////////////
// Apply a tag to the textbox
void CTextbox::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	sText = xmlGetString(node, "value", sText);  // Either as the value parameter
	sText = xmlNodeText(node, sText);  // or in the tag body

	if (xmlPropExists(node, "password"))
		bPassword.set(xmlGetBool(node, "password", bPassword), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "unicode"))
		bNoUnicode.set(!xmlGetBool(node, "unicode", !bNoUnicode), TAG_ATTR_PRIORITY);

	iMax = MAX((size_t)1, (size_t)xmlGetInt(node, "maxlen", -1));

	if (xmlPropExists(node, "selcolor"))
		clSelection.set(xmlGetColour(node, "selcolor", clSelection), TAG_ATTR_PRIORITY);
	if (xmlPropExists(node, "caretcolor"))
		clCaret.set(xmlGetColour(node, "caretcolor", clCaret), TAG_ATTR_PRIORITY);
}

///////////////
// Set the textbox size automatically
void CTextbox::UpdateSize()
{
	Resize(getX(), getY(), ((getWidth() == 0) ? 100 : getWidth()), getHeight() == 0 ? GetTextHeight(cFont, sText) : getHeight());
}

///////////////////
// Draw the text box
void CTextbox::DoRepaint()
{
	CHECK_BUFFER;

	CWidget::DoRepaint();

	std::string text = sText;

	cBackground.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());

	if(bPassword) {

		// Draw astericks for password
		text = "";
		size_t len = Utf8StringSize(sText);
		for(size_t i = 0; i < len; i++)
			text += '*';
	}

	int cursorpos = (int)iCurpos;

	// User can scroll to left
	cursorpos -= (int)iScrollPos;
	if (cursorpos < 0)  {
		cursorpos = 5;  // Number of characters, that will be displayed on left if we scroll left and cursor is on the most left
		iScrollPos -= 5;
		if ((int)iScrollPos < 0)
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
		int x1=0,x2=0;  // Positions of the selection

		if (iSelLength > 0)  
			iSelStart = iCurpos;
		else
			iSelStart = iCurpos + iSelLength;


		if (iSelStart <= iScrollPos) {
			x1 = cBorder.getLeftW();
			x2 = MIN(
				x1 + GetTextWidth(cFont,
					Utf8SubStr(text, 0, abs(iSelLength) - iScrollPos + iSelStart)),
				getWidth() - cBorder.getRightW());
		} else {
			x1 = cBorder.getLeftW() + GetTextWidth(cFont,
				Utf8SubStr(text, 0, iSelStart - iScrollPos));
			x2 = MIN(
				x1 + GetTextWidth(cFont,
					Utf8SubStr(text, iSelStart - iScrollPos, abs(iSelLength))),
				getWidth() - cBorder.getRightW());
		}

		// Update the selected text
		sSelectedText = Utf8SubStr(sText, iSelStart, abs(iSelLength));

		if (x1 < x2)
			DrawRectFill(
				bmpBuffer.get(),
				x1, cBorder.getTopW(), x2, getHeight() - cBorder.getBottomW(),
				clSelection);
	}

	// Draw text
	SDL_Rect r = { cBorder.getLeftW(), cBorder.getTopW(), getWidth() - cBorder.getLeftW() - cBorder.getRightW(),
		getHeight() - cBorder.getTopW() - cBorder.getBottomW()};
	cText.tFontRect = &r;
	DrawGameText(bmpBuffer.get(), text, cFont, cText);

	// Draw cursor (only when focused)
	if(bFocused) {

		if (bDrawCursor)  {
			// Determine the cursor position in pixels
			int x = GetTextWidth(cFont, Utf8SubStr(text, 0, (size_t)cursorpos)) + cBorder.getLeftW();

			DrawVLine(
				bmpBuffer.get(),
				cBorder.getTopW(), getHeight() - cBorder.getBottomW(), MIN(x, getWidth() - cBorder.getRightW()),
				clCaret);
		}
	}

	// Border
	cBorder.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());
}

////////////////////
// Create event
int CTextbox::DoCreate()
{
	UpdateSize();
	CWidget::DoCreate();
	tTimer->start();

	return WID_PROCESSED;
}

///////////////////
// Keydown event
int CTextbox::DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	Repaint();

	bDrawCursor = true;

	// Handle holding keys
	if(bHolding) {
		if(iLastchar != c || iLastKeysym != keysym)
			bHolding = false;
		else {
			if(tLX->currentTime - fTimePushed < 0.25f)  {
				//CWidget::DoKeyUp(c, keysym, modstate);
				return WID_PROCESSED;
			}

			if (tLX->currentTime - fLastRepeat < 0.03f)  {
				//CWidget::DoKeyUp(c, keysym, modstate);
				return WID_PROCESSED;
			}

			fLastRepeat = tLX->currentTime;
		}
	}

	if(!bHolding) {
		bHolding = true;
		fTimePushed = tLX->currentTime;
	}

	iLastchar = c;
	iLastKeysym = keysym;


	// Backspace
	if(keysym == SDLK_BACKSPACE) {
		Backspace();
		
		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
	}


	// Left arrow
	if (keysym == SDLK_LEFT) {
		if (modstate.bShift)  {
			if (iCurpos)
				iSelLength++;
		}
		else
			iSelLength = 0;

		if(iCurpos)
			iCurpos--;

		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
	}

	// Right arrow
	if(keysym == SDLK_RIGHT) {
		size_t len = Utf8StringSize(sText);
		if(iCurpos <= len)  {
			if (modstate.bShift)  {
				if (iCurpos != len)
					iSelLength--;
			}
			else
				iSelLength = 0;

			if (iCurpos != len)
				iCurpos++;
		}

		// Prevents weird behavior when we're at the end of text
		if(iScrollPos)  {
			if(GetTextWidth(cFont, Utf8SubStr(sText, iScrollPos, (size_t)-1)) < 
				(getWidth() - cBorder.getLeftW() - cBorder.getRightW()))  {
				CWidget::DoKeyUp(c, keysym, modstate);
				return WID_PROCESSED;
			}
		}

		// Set the scroll position
		if (iCurpos)
			if(GetTextWidth(cFont, Utf8SubStr(sText, 0, iCurpos)) > 
				(getWidth() - cBorder.getLeftW() - cBorder.getRightW() - 4)) // 4 - leave some space
				iScrollPos++;

		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
	}

	// Home
	if(keysym == SDLK_HOME) {
		// If the shift key is down, select the text
		if(modstate.bShift)
			if(iSelLength > 0)
				iSelLength += (int)iCurpos;
			else
				iSelLength = (int)iCurpos;
		else
			iSelLength = 0;

		// Safety
		if((size_t)iSelLength > Utf8StringSize(sText))
			iSelLength = (int)Utf8StringSize(sText);

		iCurpos = 0;
		iScrollPos = 0;
		
		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
	}

	// End
	if(keysym == SDLK_END) {
		if (modstate.bShift)
			iSelLength = -(int)Utf8StringSize(sText) + (int)iCurpos + iSelLength;
		else
			iSelLength = 0;

		iCurpos = Utf8StringSize(sText);

		if (GetTextWidth(cFont, sText) > getWidth() - cBorder.getLeftW() - cBorder.getRightW())  {
			iScrollPos = 0;
			for(std::string::const_iterator it = sText.begin(); it != sText.end(); IncUtf8StringIterator(it, sText.end())) {
				iScrollPos++;
				if(GetTextWidth(cFont, std::string(it, (std::string::const_iterator)sText.end())) < getWidth())
					break;
			}
		}
		else
			iScrollPos = 0;

		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
	}

	// Select all
	if ((modstate.bCtrl || modstate.bGui) && keysym == SDLK_a) {
		iCurpos = Utf8StringSize(sText);
		iSelStart = 0;
		iSelLength = -((int)Utf8StringSize(sText));

		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
	}

	// Delete
	if(keysym == SDLK_DELETE) {
		Delete();
		
		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
	}

	// Enter
	if(keysym == SDLK_RETURN || keysym == SDLK_KP_ENTER) {
		CALL_EVENT(OnEnterPress, (this));
		
		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
	}

    // Ctrl-v or Super-v (paste)
    if((modstate.bCtrl || modstate.bGui) && keysym == SDLK_v) {
        PasteText();
        
		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
    }

    // Ctrl-c or Super-c (copy)
    if((modstate.bCtrl || modstate.bGui) && keysym == SDLK_c) {
        CopyText();

		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
    }

    // Ctrl-x or Super-x (cut)
    if((modstate.bCtrl || modstate.bGui) && keysym == SDLK_x) {
        CopyText();
		Delete();

		CWidget::DoKeyUp(c, keysym, modstate);
		return WID_PROCESSED;
    }

	// Alt + numbers
	if(modstate.bAlt)  {
		if (c >= 0x30 && c <= 0x39)
			sAltKey += (char)c;
	} else {  // Alt not pressed
		sAltKey = "";
	}


	// Insert character
	if (c != 0)
		Insert(c);

	CWidget::DoKeyUp(c, keysym, modstate);
	return WID_PROCESSED;
}


///////////////////
// Keyup event
int CTextbox::DoKeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	bHolding = false;

	// Check if alt has been released and insert any character code that has been typed
	if (keysym == SDLK_LALT || keysym == SDLK_RALT)
		if (sAltKey.size() != 0)  {  // If there's some code saved, insert it

			// Convert the code
			bool fail = false;
#if _MSC_VER <= 1200
			UnicodeChar in = (UnicodeChar)from_string<int>(sAltKey, fail);  // MSVC cannot convert string to UnicodeChar
#else
			UnicodeChar in = from_string<UnicodeChar>(sAltKey, fail);
#endif
			// Insert
			if (!fail)
				Insert(in);

			// Clear
			sAltKey = "";
		}

	Repaint();

	CWidget::DoKeyUp(c, keysym, modstate);
	return WID_PROCESSED;
}

///////////////////
// Mouse move event
int	CTextbox::DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate)
{
	SetGameCursor(CURSOR_TEXT);

	if (sText.size() == 0 || !down)  {
		CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
		return WID_PROCESSED;
	}

	bDrawCursor = true;

	// Scroll the textbox using mouse
	fScrollTime += tLX->fDeltaTime;

	// Save the current position
	iLastCurpos = iCurpos;

	bool scrolled = false;

	if(fScrollTime > 0.05f && down)  {
		// Scroll left
		if(iScrollPos && x <= -5)  {
			if(iScrollPos > 0)  {
				iScrollPos--;
				iCurpos--;
				scrolled = true;
			}
		}

		// Scroll right
		else if(GetTextWidth(cFont, Utf8SubStr(sText, iScrollPos)) > getWidth() - cBorder.getLeftW() - cBorder.getRightW() && x > getWidth())  {
			if (iCurpos <= Utf8StringSize(sText))  {
				iScrollPos++;
				iCurpos++;
				scrolled = true;
			}
		}

		fScrollTime = 0;
	}

	// Safety (must be *after* mouse scrolling)
	if (iCurpos > Utf8StringSize(sText))  {
		iCurpos = Utf8StringSize(sText);
	}

	// Set the cursor position
	if (!scrolled && down)
		SetCursorPosByCoord(x, y);

	if ((dx && (iCurpos - iLastCurpos)) || scrolled)  {
		if (down)
			iSelLength += -(int)(iCurpos - iLastCurpos);
	}

	Repaint();

	CWidget::DoMouseMove(x, y, dx, dy, down, button, modstate);
	return WID_PROCESSED;
}

/////////////////////
// Sets a cursor position according to the coordinate
void CTextbox::SetCursorPosByCoord(int x, int y)
{
	// If we click somewhere behind the text, but in the text box, set the cursor to the end
	if(x > GetTextWidth(cFont, Utf8SubStr(sText, iScrollPos)) + cBorder.getLeftW() && RelInBox(x, y))  {
		iCurpos = Utf8StringSize(sText);
	}  else  {
		// Click on the border, ignore
		if (x <= cBorder.getLeftW() || x >= getWidth() - cBorder.getRightW())  {
			return;
		}

		// Set the new cursor pos
		if (x < (GetTextWidth(cFont, Utf8SubStr(sText, 0, 1)) / 2))  // Area before the first character
			iCurpos = iScrollPos;
		else  {
			std::string buf = Utf8SubStr(sText, iScrollPos);
			size_t pos = Utf8StringSize(sText);
			int w, prev_w;
			w = prev_w = 0;
			while ((w = GetTextWidth(cFont, buf)) > x && pos)  {
				pos--;
				Utf8Erase(buf, pos);
				prev_w = w;
			}

			if ((prev_w - w) / 3 < (x - w))
				pos++;

			iCurpos = pos + iScrollPos;

		}  // else 2
	}  // else 1
}

///////////////////
// Mouse down event
int	CTextbox::DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	// Set the cursor position
	SetCursorPosByCoord(x, y);

	// Remove any previous selection
	iSelLength = 0;
	iSelStart = 0;
	sSelectedText = "";
	iLastCurpos = iCurpos;

	// Set up the variables for selection
	iLastCurpos = iCurpos;

	Repaint();

	CWidget::DoMouseDown(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}

///////////////////
// Mouse up event
int	CTextbox::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	SetGameCursor(CURSOR_TEXT);

	fScrollTime = 0;  // We can scroll the textbox using mouse

	// Doubleclick
	if (tLX->currentTime - fLastClick <= 0.25)  {
		SelectWord();
		Repaint();
	}

	fLastClick = tLX->currentTime;

	Repaint();

	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}

////////////
// Select a word
void CTextbox::SelectWord()
{
	// Empty text
	if (sText.size() == 0)
		return;

	std::string::iterator curpos = Utf8PositionToIterator(sText, iCurpos);
	std::string::iterator left = curpos;
	std::string::iterator right = curpos;

	iSelStart = iCurpos;

	// Go from cursor to left, until a beginning of a word or beginning of the text
	do  {
		DecUtf8StringIterator(left, sText.begin());
		iSelStart--;
	} while (left != sText.begin() && (isalnum((uchar)*left) || (uchar)*left >= 128));  // >= 128 - UTF8 hack, if you have a better idea how to do it, please fix it

	if (!isalnum((uchar)*left) && (uchar)*left < 128)  {
		iSelStart++;
	}

	// Go from cursor to right, until an end of a word or end of the text
	do  {
		IncUtf8StringIterator(right, sText.end());
		iCurpos++;
	} while (right != sText.end() && (isalnum((uchar)*right) || (uchar)*right >= 128));

	// Set the selection
	iSelLength = (int)iSelStart - (int)iCurpos;
}


///////////////////
// Backspace
void CTextbox::Backspace()
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
void CTextbox::Delete()
{
	// Delete selection
	if(iSelLength)  {
		std::string newtext = sText;
		Utf8Erase(newtext, iSelStart, abs(iSelLength));

		bool cancel = false;
		CALL_EVENT(OnChange, (this, newtext, cancel));
		if (cancel)	return;

		sText = newtext;
		iCurpos = iSelStart;
		iSelLength = 0;
		if (iScrollPos > iCurpos)
			iScrollPos = iCurpos;
		if (GetTextWidth(cFont, sText) <= getWidth() - cBorder.getLeftW() - cBorder.getRightW())
			iScrollPos = 0;
		return;
	}


	if(iCurpos >= Utf8StringSize(sText))
		return;

	// No selection, erase the next character
	std::string newtext = sText;
	Utf8Erase(newtext, iCurpos, 1);

	bool cancel = false;
	CALL_EVENT(OnChange, (this, newtext, cancel));
	if (!cancel)
		sText = newtext;
}


///////////////////
// Insert a character
void CTextbox::Insert(UnicodeChar c)
{
	// Delete any selection
	if (iSelLength)
		Delete();

	// If no unicode, try to convert to ascii
	if (bNoUnicode)  {
		c = UnicodeCharToAsciiChar(c);
		if (c == 0xFF)  // Cannot convert
			return;
	}

	// Check for the max
	if(Utf8StringSize(sText) >= iMax)
		return;

	// Check that the current font can display this character, if not, quit
	if (!tLX->cFont.CanDisplayCharacter(c))
		return;

	// Safety
	if(iCurpos > Utf8StringSize(sText))
		iCurpos = Utf8StringSize(sText);

	// Insert the character (if we can)
	std::string newtext = sText;
	bool cancel = false;
	Utf8Insert(newtext, iCurpos++, GetUtf8FromUnicode(c));
	CALL_EVENT(OnChange, (this, newtext, cancel));
	if (cancel) return;
	sText = newtext;

    // If the text size is greater than the textbox size, scroll the text
	if (iCurpos == Utf8StringSize(sText))
		while(GetTextWidth(cFont, Utf8SubStr(sText, iScrollPos)) > getWidth() - cBorder.getLeftW() - cBorder.getRightW()) {
			iScrollPos++;
		}
}

/////////////////////
// Replace the current text with the buf
void CTextbox::setText(const std::string& buf) {
	bool cancel = false;
	CALL_EVENT(OnChange, (this, buf, cancel));
	if (cancel) return;

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
// Paste some text from the clipboard
void CTextbox::PasteText()
{
	std::string text;

	if(iSelLength)
		Delete();

    text = copy_from_clipboard();

	// Fire the event
	bool cancel = false;
	CALL_EVENT(OnChange, (this, text, cancel));
	if (cancel) return;

	// Insert the text
	for(std::string::const_iterator i = text.begin(); i != text.end(); )
		Insert( GetNextUnicodeFromUtf8(i, text.end()) );
}

///////////////////
// Copy selection to the clipboard
void CTextbox::CopyText()
{
	if (!iSelLength)
		return;
	copy_to_clipboard(sSelectedText);
}

}; // namespace SkinnedGUI
