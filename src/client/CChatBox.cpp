/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
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
    nWidth = 500;
    for(int n=0;n<MAX_CLINES;n++) {
        Lines[n].strLine[0] = '\0';
        Lines[n].iUsed = false;
        Lines[n].iColour = 0xffff;
        Lines[n].fTime = 0;
    }
}


///////////////////
// Add a line of text to the chat box
void CChatBox::AddText(char *txt, int colour, float time)
{
    int     n;
    int     l=-1;
    static char    buf[128];

	if (strlen(txt)<=0)
		return;

    // If this line is too long, break it up
	fix_strncpy(buf,txt);
	if(tLX->cFont.GetWidth(txt) > nWidth) {
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
		AddText(txt,colour,time);  // Line 1
		AddText(strcpy(buf,&txt[j+1]),colour,time);  // Line 2

		return;
	}


    // Move up the lines?
    for(n=0;n<MAX_CLINES;n++) {
        if(!Lines[n].iUsed) {
            l=n;
            break;
        }
    }
    
    if(l<0) {
        MoveUp();
        l=MAX_CLINES-1;
    }
    
    fix_strncpy(Lines[l].strLine,txt);
    Lines[l].iUsed = true;
    Lines[l].iColour = colour;
    Lines[l].fTime = time;
}



///////////////////
// Move up one line	
void CChatBox::MoveUp(void)
{
    for(short n=0;n<MAX_CLINES-1;n++) {
        fix_strncpy(Lines[n].strLine,Lines[n+1].strLine);
        Lines[n].iColour = Lines[n+1].iColour;
        Lines[n].fTime = Lines[n+1].fTime;
    }
}
