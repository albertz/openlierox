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
void CChatBox::AddText(const std::string& txt, int colour, float time)
{
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
void CChatBox::AddWrapped(const std::string& txt, int colour, float time)
{
	//
	// Wrap
	//

	static std::string buf;

	if (txt == "")
		return;

    // If this line is too long, break it up
	buf = txt;
	if((uint)tLX->cFont.GetWidth(txt) >= nWidth) {
		int i; // We need it to be defined after FOR ends
		for (i=buf.length()-2; (uint)tLX->cFont.GetWidth(buf) > nWidth && i >= 0; i--)
            buf.erase(i);

		int j;
		// Find the nearest space
		for (j=buf.length()-1; j>=0 && buf[j] != ' '; j--)
			continue;

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

	iNewLine = MAX(0,(int)MIN(iNewLine,(int)WrappedLines.size()));

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
	iNewLine = WrappedLines.size()-1;

}

///////////////////
// Get a line from chatbox
line_t *CChatBox::GetLine(int n)
{
	if (n >= 0 && n < (int)WrappedLines.size())  {
		WrappedLines[n].bNew = false;
		iNewLine++;
		iNewLine = MAX(0,(int)MIN(iNewLine,(int)WrappedLines.size()));
		return &WrappedLines[n];
	}
	return NULL;
}

/////////////////////
// Get a new line from the chatbox
line_t *CChatBox::GetNewLine(void)
{
	static unsigned int tmp = 0;
	if (iNewLine >= 0 && iNewLine < WrappedLines.size())  {
		if (WrappedLines[iNewLine].bNew)  {
			tmp = iNewLine;
			WrappedLines[iNewLine++].bNew = false;
			iNewLine = MAX(0,(int)MIN(iNewLine,(int)WrappedLines.size()));
			WrappedLines[tmp].bNew = false;
			return &WrappedLines[tmp];
		}
	}
	return NULL;
}

