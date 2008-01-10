/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Custom browser class
// Created 7/6/02
// Jason Boettcher


#include <stack>
#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CBrowser.h"

#define BORDER_SIZE 2
#define CONTROL_CHARACTER_BEGIN (char)4
#define CONTROL_CHARACTER_END (char)5
#define CONTROL_COLOR 'c'
#define CONTROL_UNDERLINE 'u'
#define CONTROL_BOLD 'b'



///////////////////
// The create event
void CBrowser::Create(void)
{
	tLines.clear();
	bUseScroll = false;
	iClientWidth = iWidth - 2*BORDER_SIZE;
	iClientHeight = iHeight - 2*BORDER_SIZE;

	// Setup the scrollbar
	cScrollbar.Create();

	cScrollbar.Setup(0, iX+iWidth-15, iY+BORDER_SIZE, 14, iHeight-BORDER_SIZE);
	cScrollbar.setMin(0);
	cScrollbar.setValue(0);
	cScrollbar.setItemsperbox(iHeight);
	cScrollbar.setMax(0);
}

///////////////////
// Load the cmht file
void CBrowser::Load(const std::string& url)
{
	// Send the HTTP request
	bFinished = false;
	tLines.clear();
	cHttp.RequestData(url);
}


///////////////////
// Process the HTTP downloading
void CBrowser::ProcessHTTP()
{
	if (bFinished)
		return;

	int res = cHttp.ProcessRequest();
	switch (res)  {
	case HTTP_PROC_PROCESSING:
		break;
	case HTTP_PROC_ERROR:
		tLines.push_back("An error occured while loading: " + cHttp.GetError().sErrorMsg);
		break;
	case HTTP_PROC_FINISHED:
		Parse();
		bFinished = true;
		break;
	}
}

///////////////////
// Parses the HTTP data
void CBrowser::Parse()
{
	// We take care only of 3 tags in HTML: font, br and hr, others are ignored
	const std::string& data = cHttp.GetData();
	std::string::const_iterator it = data.begin();
	std::string::const_iterator last = data.end();

	std::string cur_line;
	bool last_space = false;
	bool last_tag = false;

	while (it != last)  {
		if (*it == '<')  {
			ParseTag(it, last, cur_line);
			last_tag = true;
			continue;
		}

		// Check for blank characters (they're handled as spaces if they are not repeated)
		if (isspace((uchar)*it))  {
			if (!last_space && !((*it == '\r' || *it == '\n') && last_tag)) // Skip repeated
				cur_line += ' '; // force a space

			last_space = true;
			it++;
			continue;

		} else {
			last_space = false;
		}

		last_tag = false;

		// Add it to the line
		cur_line += *it;

		it++;
	}

	// Add the last line
	tLines.push_back(cur_line);

	// Setup the scrollbar
	cScrollbar.setItemsperbox(iClientHeight / tLX->cFont.GetHeight());
	cScrollbar.setMax(tLines.size());
	cScrollbar.setValue(0);

}

////////////////
// Parse a HTML tag
void CBrowser::ParseTag(std::string::const_iterator& it, std::string::const_iterator& last, std::string& cur_line)
{
	// The iterator points at the opening bracket of the tag
	it++; if (it == last) return;
	std::string tag_name;
	bool close_tag = false;

	// Read the tag name
	while (it != last)  {
		if (!isalnum((uchar)*it) || *it == '>')  {
			// Closing tag?
			if (*it == '/')  {
				it++;
				close_tag = true;
				continue;
			}

			// Tag name has been read
			stringlwr(tag_name);
			if (tag_name == "br")  {
				tLines.push_back(cur_line);
				cur_line = "";
			} else if (tag_name == "hr")  {
				tLines.push_back(cur_line);
				tLines.push_back("<line>");
				cur_line = "";
			} else if (tag_name == "u")  {
				if (close_tag)
					cur_line += CONTROL_CHARACTER_END;
				else
					cur_line += CONTROL_CHARACTER_BEGIN;
				cur_line += CONTROL_UNDERLINE;
			} else if (tag_name == "b")  {
				if (close_tag)
					cur_line += CONTROL_CHARACTER_END;
				else
					cur_line += CONTROL_CHARACTER_BEGIN;
				cur_line += CONTROL_BOLD;
			}

			break;
		}

		tag_name += *it;

		it++;
	}

	// Read the color parameter for font tag (if any)
	if (tag_name == "font") {
		if (close_tag)  {  // font tag got closed
			cur_line += CONTROL_CHARACTER_END;  // remove any color settings
			cur_line += CONTROL_COLOR;

		} else {  // Get the color parameter

			// Param name
			while (it != last)  {
				std::string param_name;
				while (it != last)  {
					// The '=' character divides param and value
					if (*it == '=')  {
						it++;
						break;
					}

					if (isalnum((uchar)*it))
						param_name += *it;

					it++;
				}

				// Look for 
				if (stringcasecmp(param_name, "color") == 0)
					break;
			}

			// Param value
			std::string param_value;
			while (it != last)  {
				if (*it == '\"')  {
					it++;
					continue;
				}

				// End?
				if (isspace((uchar)*it) || *it == '>')  {
					// Convert the color
					Uint32 color = StrToCol(param_value);
					Uint8 r, g, b;
					GetColour3(color, getMainPixelFormat(), &r, &g, &b);

					cur_line += CONTROL_CHARACTER_BEGIN;
					cur_line += CONTROL_COLOR;
					cur_line += (char)r;
					cur_line += (char)g;
					cur_line += (char)b;
					break;
				}

				param_value += *it;

				it++;
			}
		}
	}

	// Skip the tag
	while (it != last)  {
		if (*it == '>')  {
			it++;
			break;
		}
		it++;
	}
}

///////////////////
// Mouse down event
int CBrowser::MouseDown(mouse_t *tMouse, int nDown)
{
	if(bUseScroll && tMouse->X > iX+iWidth-20)
		return cScrollbar.MouseDown(tMouse, nDown);

	return BRW_NONE;
}


///////////////////
// Mouse over event
int CBrowser::MouseOver(mouse_t *tMouse)
{
	if(bUseScroll && tMouse->X > iX+iWidth-20)
		return cScrollbar.MouseOver(tMouse);

	return BRW_NONE;
}

//////////////////
// Mouse wheel down event
int CBrowser::MouseWheelDown(mouse_t *tMouse)
{
	if(bUseScroll)
		return cScrollbar.MouseWheelDown(tMouse);
	return BRW_NONE;
}

//////////////////
// Mouse wheel up event
int CBrowser::MouseWheelUp(mouse_t *tMouse)
{
	if(bUseScroll)
		return cScrollbar.MouseWheelUp(tMouse);
	return BRW_NONE;
}

//////////////////
// Mouse wheel down event
int CBrowser::KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	if (bUseScroll)  {
		switch (keysym)  {
		case SDLK_DOWN:
			return cScrollbar.MouseWheelDown(GetMouse());
		case SDLK_UP:
			return cScrollbar.MouseWheelUp(GetMouse());
		}
	}

	return BRW_NONE;
}


///////////////////
// Render the browser
void CBrowser::Draw(SDL_Surface *bmpDest)
{
	// 3D Box
	DrawRect(bmpDest, iX, iY, iX + iWidth, iY + iHeight, tLX->clBoxDark);
	DrawRect(bmpDest, iX + 1, iY + 1, iX + iWidth - 1, iY + iHeight - 1, tLX->clBoxLight);
	
	// Background (white)
	DrawRectFill(bmpDest, iX + BORDER_SIZE, iY + BORDER_SIZE, iX + iWidth, iY + iHeight, tLX->clWhite);

	// Render the content
	RenderContent(bmpDest);

	// Scrollbar
	if (bUseScroll)
		cScrollbar.Draw(bmpDest);
}

//////////////////
// Renders the textual content
void CBrowser::RenderContent(SDL_Surface *bmpDest)
{
	int curX = iX + BORDER_SIZE;
	int curY = iY + BORDER_SIZE - cScrollbar.getValue() * tLX->cFont.GetHeight();
	Uint32 curColor = tLX->clBlack;
	std::stack<Uint32> color_stack;
	int lines = 0;

	// Setup the clipping
	SDL_Rect clip = {iX + BORDER_SIZE, iY + BORDER_SIZE, iWidth - BORDER_SIZE, iHeight - BORDER_SIZE};
	SDL_SetClipRect(bmpDest, &clip);

	// Get the line from which we start drawing
	std::list<std::string>::iterator it = tLines.begin();

	// Text style & wrapping
	bool underline = false;
	bool bold = false;
	int expect_hard_breaks = 0;

	// Render the text
	for (int i = 0; it != tLines.end() && i <= cScrollbar.getItemsperbox(); ++i, ++it)  {
		curX = iX + 2;

		if (*it == "<line>")  {  // horizontal line
			DrawHLine(bmpDest, curX, curX + iClientWidth, curY + tLX->cFont.GetHeight()/2, tLX->clBlack);
			curY += tLX->cFont.GetHeight();
			lines++;
			continue;
		}

		std::string::iterator str_it = it->begin();

		// Check if the new line will need a hard break
		{
			std::string next_word = GetNextWord(str_it, *it);
			int width = tLX->cFont.GetWidth(next_word);
			if (width >= iClientWidth)
				expect_hard_breaks = width / iClientWidth;
		}

		while (str_it != it->end())  {

			// Handle control characters
			if (*str_it == CONTROL_CHARACTER_BEGIN)  {
				str_it++; if (str_it == it->end()) break; // Skip the control character
				switch (*str_it)  {
				case CONTROL_COLOR:  {
					str_it++; if (str_it == it->end()) break; // Skip the control character
					Uint8 r = (Uint8)*str_it; str_it++; if (str_it == it->end()) break;
					Uint8 g = (Uint8)*str_it; str_it++; if (str_it == it->end()) break;
					Uint8 b = (Uint8)*str_it; str_it++; if (str_it == it->end()) break;
					color_stack.push(curColor);  // save the current color
					curColor = MakeColour(r, g, b);  // new color
				} break;
				case CONTROL_UNDERLINE:
					str_it++; if (str_it == it->end()) break; // Skip the control character
					underline = true;
				break;
				case CONTROL_BOLD:
					str_it++; if (str_it == it->end()) break; // Skip the control character
					bold = true;
				break;
				}

				continue;

			// End of control section
			} else if (*str_it == CONTROL_CHARACTER_END) {
				str_it++; if (str_it == it->end()) break;

				switch (*str_it)  {
				case CONTROL_COLOR:
					// Restore the previous color
					if (!color_stack.empty())  {  // Safety...
						curColor = color_stack.top();
						color_stack.pop();
					}
				break;
				case CONTROL_UNDERLINE:
					underline = false;
				break;
				case CONTROL_BOLD:
					bold = false;
				break;
				}

				str_it++; if (str_it == it->end()) break; // Skip the control character
				continue;
			}

			// Get the character width
			int ch_width = tLX->cFont.GetCharacterWidth((UnicodeChar)*str_it) + tLX->cFont.GetSpacing();
			if (bold)
				ch_width++;

			// Check if it's an expected hard break time
			if (expect_hard_breaks > 0 && curX + ch_width >= iX + BORDER_SIZE + iClientWidth)  {
				curX = iX + BORDER_SIZE;
				curY += tLX->cFont.GetHeight();
				lines++;
				expect_hard_breaks--;
			}

			// Draw the character
			char buf[2]; buf[0] = *str_it; buf[1] = '\0';

			// Bold
			if (bold)  {
				tLX->cFont.Draw(bmpDest, curX, curY, curColor, std::string(buf));
				tLX->cFont.Draw(bmpDest, curX + 1, curY, curColor, std::string(buf));

			// Normal
			} else {
				tLX->cFont.Draw(bmpDest, curX, curY, curColor, std::string(buf));
			}

			// Underline
			if (underline)
				DrawHLine(bmpDest, curX, curX + ch_width, curY + tLX->cFont.GetHeight() - 1, curColor);
			
			curX += ch_width;

			// Space or newline, check for wrapping
			if (*str_it == ' ')  {
				// Get next word and check if it will fit in the window
				// If not, make a break
				std::string next_word = GetNextWord(str_it, *it);
				int word_w = tLX->cFont.GetWidth(next_word);
				if (curX + word_w >= iX + BORDER_SIZE + iClientWidth)  {
					curY += tLX->cFont.GetHeight();  // Break
					curX = iX + BORDER_SIZE;
					lines++;
					if (word_w >= iClientWidth)  // if the next word itself is bigger than the box, we should expect hard break(s)
						expect_hard_breaks = word_w / iClientWidth;
				}
			}

			str_it++;
		}

		curY += tLX->cFont.GetHeight();
		lines++;
	}

	// Setup the scrollbar if needed
	if (lines > cScrollbar.getItemsperbox())  {
		if (!bUseScroll)
			iClientWidth -= cScrollbar.getWidth();
		bUseScroll = true;
		cScrollbar.setMax(lines);
	} else {
		if (bUseScroll)
			iClientWidth += cScrollbar.getWidth();
		bUseScroll = false;
	}

	// Restore clipping
	SDL_SetClipRect(bmpDest, NULL);
}
