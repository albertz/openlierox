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
void CChatBox::AddText(char *txt, int colour, float time)
{
	// Create a new line and copy the info
	line_t newline;

	newline.fTime = time;
	newline.iColour = colour;
	fix_strncpy(newline.strLine,txt);

	// Add to lines
	Lines.push_back(newline);

	// Add to wrapped lines
	AddWrapped(txt,colour,time);
}


////////////////////
// Adds the text to wrapped lines
void CChatBox::AddWrapped(char *txt, int colour, float time)
{
	//
	// Wrap
	//

    int     l=-1;
    static char    buf[128];

	if (strlen(txt)<=0)
		return;

    // If this line is too long, break it up
	fix_strncpy(buf,txt);
	if(tLX->cFont.GetWidth(txt) >= nWidth) {
		int i; // We need it to be defined after FOR ends
		for (i=fix_strnlen(buf)-2; tLX->cFont.GetWidth(buf) > nWidth && i >= 0; i--)
			buf[i] = '\0';

		int j;
		// Find the nearest space
		for (j=fix_strnlen(buf)-1; j>=0 && buf[j] != ' '; j--)
			continue;

		// Hard break
		if(j < 24)
			j = i;

		txt[j] = '\0';

		// Add the lines recursively
		// Note: if the second line is also too long, it will be wrapped, because of recursion
		AddWrapped(txt,colour,time);  // Line 1
		AddWrapped(strcpy(buf,&txt[j+1]),colour,time);  // Line 2

		return;
	}

	//
	//	Add the wrapped line
	//
	line_t newline;

	newline.fTime = time;
	newline.iColour = colour;
	newline.bNew = true;
	fix_strncpy(newline.strLine,txt);

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
	int i;

	// Recalculate the wrapped lines
	for (i=0;i<Lines.size();i++)  
		AddWrapped(Lines[i].strLine,Lines[i].iColour,Lines[i].fTime);
	iNewLine = WrappedLines.size()-1;

}

///////////////////
// Get a line from chatbox
line_t *CChatBox::GetLine(int n)
{
	if (n >= 0 && n < WrappedLines.size())  {
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
	static int tmp = 0;
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

