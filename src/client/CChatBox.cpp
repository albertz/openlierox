/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Chat Box class
// Created 26/8/03
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


///////////////////
// Clear the chatbox
void CChatBox::Clear(void)
{
	Lines.clear();
	WrappedLines.clear();
	nWidth = 500;
	iNewLine = 0;
}


///////////////////
// Add a line of text to the chat box
void CChatBox::AddText(const tString& txt, int colour, float time)
{
	if (txt == "")
		return;

	// Create a new line and copy the info
	line_t newline;

	newline.fTime = time;
	newline.iColour = colour;
	newline.strLine = txt;

	// Add to lines
	Lines.push_back(newline);

	// Add to wrapped lines
	AddWrapped(txt,colour,time);
}


////////////////////
// Adds the text to wrapped lines
void CChatBox::AddWrapped(const tString& txt, int colour, float time)
{
	//
	// Wrap
	//

	static tString buf;

	if (txt == "")
		return;

    // If this line is too long, break it up
	buf = txt;
	if((uint)tLX->cFont.GetWidth(txt) >= nWidth) {
		// TODO: this cannot work and has to be redone
		// 1. *it = '\0' don't cut tString! (it's not a C-string)
		// 2. what if buf.size()<2?
		// 3. don't ever use int as a replacement for size_t
		// (I would fix it myself, but no time atm, sorry ...)

		size_t i;
		for (i=buf.length()-2; (uint)tLX->cFont.GetWidth(buf) > nWidth && i >= 1; i--)
			buf.erase(i);

		size_t j = buf.length()-1;
		// Find the nearest space
		for (tString::iterator it2=buf.end()-2; it2!=buf.begin() && *it2 != ' '; it2--,j--) {}

		// Hard break
		if(j < 24)
			j = i;

		// Add the lines recursively
		// Note: if the second line is also too long, it will be wrapped, because of recursion
		AddWrapped(txt.substr(0,j),colour,time);  // Line 1
		AddWrapped(txt.substr(j),colour,time);  // Line 2

		return;
	}

	//
	//	Add the wrapped line
	//
	line_t newline;

	newline.fTime = time;
	newline.iColour = colour;
	newline.bNew = true;
	newline.strLine = txt;

	WrappedLines.push_back(newline);

	iNewLine = MAX(0,(int)MIN(iNewLine,(int)WrappedLines.size()-1));

	if (!WrappedLines[iNewLine].bNew)
		iNewLine = WrappedLines.size()-1;
}

///////////////////
// Set the chatbox width
void CChatBox::setWidth(int w)
{
	nWidth = w;
	WrappedLines.clear();

	// Recalculate the wrapped lines
	for (ct_lines_t::const_iterator i=Lines.begin();i!=Lines.end();i++)
		AddWrapped(i->strLine,i->iColour,i->fTime);
	iNewLine = WrappedLines.size();

}

///////////////////
// Get a line from chatbox
line_t *CChatBox::GetLine(int n)
{
	if (n >= 0 && n < (int)WrappedLines.size())  {
		line_t *ln = &WrappedLines[n];
		if (ln->bNew)  {
			ln->bNew = false;
			if (n == (int)iNewLine)  {
				iNewLine++;
				iNewLine = MAX(0,(int)MIN(iNewLine,(int)WrappedLines.size()-1));
			}
		}
		return ln;
	}
	return NULL;
}

/////////////////////
// Get a new line from the chatbox
line_t *CChatBox::GetNewLine(void)
{
	if (iNewLine >= 0 && iNewLine < WrappedLines.size())  {
		line_t *ln = &WrappedLines[iNewLine];
		if (ln->bNew)  {
			ln->bNew = false;
			iNewLine = MAX(0,(int)MIN(++iNewLine,(int)WrappedLines.size()-1));
			ln->bNew = false;
			return ln;
		}
	}
	return NULL;
}

