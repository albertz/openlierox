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
void CChatBox::Clear(void)
{
	Lines.clear();
	NewLines.clear();
}


///////////////////
// Add a line of text to the chat box
void CChatBox::AddText(const std::string& txt, int colour, TXT_TYPE TextType, const Time& time)
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
		Lines.pop_front();

	// Add to new lines
	NewLines.push_back(newline);
}

/////////////////////
// Get a new line from the chatbox
// WARNING: not threadsafe
line_t *CChatBox::GetNewLine(void)
{
	if (NewLines.empty())
		return NULL;

	// TODO: change it!!!
	static line_t result; // We're giving a pointer to it, must always exist
	result = *NewLines.begin();
	NewLines.erase(NewLines.begin());

	return &result;
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
