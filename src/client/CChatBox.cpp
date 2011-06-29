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

// TODO: why is everything here so complicated? make it simpler! use one single list and not 3!


#include "LieroX.h"

#include "StringUtils.h"
#include "CChatBox.h"

#define MAX_LINES 200

///////////////////
// Clear the chatbox
void CChatBox::Clear()
{
	Lines.clear();
	NewLines.clear();
}


///////////////////
// Add a line of text to the chat box
void CChatBox::AddText(const std::string& txt, Color colour, TXT_TYPE TextType, const AbsTime& time)
{
	if (txt.empty())
		return;

	// Create a new line and copy the info
	line_t newline;

	newline.fTime = time;
	newline.iColour = colour;
	newline.iTextType = TextType;
	newline.strLine = txt;
	newline.iID = Lines.size();

	// Add to lines
	Lines.push_back(newline);

	while(Lines.size() > MAX_LINES)
		Lines.pop_front(); //deletes the top lines of the chat box

	// Add to new lines
	NewLines.push_back(newline);
}

/////////////////////
// Get a new line from the chatbox
// WARNING: not threadsafe
bool CChatBox::GetNewLine(line_t& res)
{
	if (NewLines.empty())
		return false;

	res = *NewLines.begin();
	NewLines.erase(NewLines.begin());

	return true;
}

////////////////////
// Convert the index to iterator
lines_iterator CChatBox::At(int i)  {
	// Checks
	if (i <= 0)
		return Lines.begin();

	if (i >= (int)Lines.size())
		return Lines.end();

	// Go to the right iterator
	lines_iterator it = Lines.begin();
	while (i)  {
		it++;
		i--;
	}

	return it;
}
