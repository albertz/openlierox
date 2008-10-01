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
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "DeprecatedGUI/CBrowser.h"
#include "XMLutils.h"
#include "Clipboard.h"
#include "Cursor.h"
#include "Timer.h"
#include "AuxLib.h"


namespace DeprecatedGUI {

#define BORDER_SIZE 2
#define LIST_SPACING 10

///////////////////
// The create event
void CBrowser::Create(void)
{
	tHtmlDocument = NULL;
	tRootNode = NULL;

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

void CBrowser::Destroy()
{
	if (tHtmlDocument)
		xmlFreeDoc(tHtmlDocument);
	tHtmlDocument = NULL;
	tRootNode = NULL;
}

///////////////////
// Load the HTML file
void CBrowser::LoadFromHTTP(const std::string& url)
{
	// Send the HTTP request
	bFinished = false;
	tData.clear();
	cHttp.RequestData(url, tLXOptions->sHttpProxy);
	sHostName = cHttp.GetHostName();
	sURL = cHttp.GetUrl();
}

/////////////////////////
// Load the contents from a file
void CBrowser::LoadFromFile(const std::string& file)
{
	tData.clear();
	bFinished = false;
	std::ifstream *fp = OpenGameFileR(file);
	if (!fp)
		return;

	sURL = file;
	sHostName = "";
	while (!fp->eof())  {
		std::string tmp;
		std::getline(*fp, tmp);
		tData += tmp;
	}

	fp->close();
	delete fp;

	bFinished = true;
	Parse();
}

////////////////////////
// Load the contents from a data string
void CBrowser::LoadFromString(const std::string& data)
{
	sHostName = "";
	sURL = "";
	tData = data;
	bFinished = true;
	Parse();
}

/////////////////////////
// Append data to the browser
void CBrowser::AppendData(const std::string& data)
{
	tData += data;
	Parse();
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
		tData = "An error occured while loading: " + cHttp.GetError().sErrorMsg;
		bFinished = true;
		break;
	case HTTP_PROC_FINISHED:
		tData = cHttp.GetData();
		Parse();
		bFinished = true;
		break;
	}
}

///////////////////
// Parses the HTTP data
void CBrowser::Parse()
{
	if (tHtmlDocument)
		xmlFreeDoc(tHtmlDocument);

	tHtmlDocument = NULL;
	tRootNode = NULL;

	// Get the context
	htmlParserCtxtPtr context = htmlCreateMemoryParserCtxt(tData.data(), tData.size());
	if (!context)
		return;

	// Parse the file
	htmlDocPtr document = htmlCtxtReadDoc(context, (xmlChar *)tData.data(), NULL, NULL, 0);
	if (!document)  {
		htmlFreeParserCtxt(context);
		return;
	}

	// Get the root element
	htmlNodePtr node = xmlDocGetRootElement(document);
	if (!node || !node->children)  {
		xmlFreeDoc(document);
		htmlFreeParserCtxt(context);
		return;
	}

	// Get the <body> element, to easily get "bgcolor" attr from it
	node = node->children;
	while (node)  {
		if( !xmlStrcasecmp(node->name, (xmlChar *)"body") )
			break;
		node = node->next;
	}

	if (!node)  {
		xmlFreeDoc(document);
		htmlFreeParserCtxt(context);
		return;
	}

	// Success
	tHtmlDocument = document;
	tRootNode = node;
}

///////////////////
// Mouse down event
int CBrowser::MouseDown(mouse_t *tMouse, int nDown)
{
	if(bUseScroll && tMouse->X > iX+iWidth-20)
		return cScrollbar.MouseDown(tMouse, nDown);

	MousePosToCursorPos(tMouse->X, tMouse->Y, iCursorColumn, iCursorLine);

	// Copy to clipboard with 2-nd or 3-rd mouse button
	std::string sel = GetSelectedText();
	if( sel.size() != 0 && 
		(tMouse->FirstDown & SDL_BUTTON(2) || tMouse->FirstDown & SDL_BUTTON(3)) )
	{
        copy_to_clipboard(sel);
		bSelectionGrabbed = false;
		return BRW_NONE;
	}

	// Clear the selection when clicked
	if (sel.size() != 0 && tMouse->FirstDown & SDL_BUTTON(1))  {
		ClearSelection();
		bSelectionGrabbed = false;
	}

	if (bSelectionGrabbed)  {
		iSelectionStartColumn = iSelectionGrabColumn;
		iSelectionStartLine = iSelectionGrabLine;
		iSelectionEndLine = iCursorLine;
		iSelectionEndColumn = iCursorColumn;

		// Swap the ends if necessary
		if (iSelectionStartLine > iSelectionEndLine)  {
			size_t tmp = iSelectionStartLine;
			iSelectionStartLine = iSelectionEndLine;
			iSelectionEndLine = tmp;

			tmp = iSelectionStartColumn;
			iSelectionStartColumn = iSelectionEndColumn;
			iSelectionEndColumn = tmp;
		} else if (iSelectionStartLine == iSelectionEndLine && iSelectionStartColumn > iSelectionEndColumn)  {
			size_t tmp = iSelectionStartColumn;
			iSelectionStartColumn = iSelectionEndColumn;
			iSelectionEndColumn = tmp;
		}

		bCanLoseFocus = false;
	} else {
		iSelectionGrabColumn = iCursorColumn;
		iSelectionGrabLine = iCursorLine;

		// Destroy any previous selection
		ClearSelection();
	}

	// Scroll if necessary
	AdjustScrollbar(true);

	bSelectionGrabbed = true;

	return BRW_NONE;
}


///////////////////
// Mouse over event
int CBrowser::MouseOver(mouse_t *tMouse)
{
	if(bUseScroll && tMouse->X > iX+iWidth-20)
		return cScrollbar.MouseOver(tMouse);

	// Check if any of the active areas has been clicked
	if (!tMouse->Down)  {
		for (std::list<CActiveArea>::iterator it = tActiveAreas.begin(); it != tActiveAreas.end(); it++) {
			if (it->InBox(tMouse->X, tMouse->Y))  {
				it->DoMouseMove(tMouse->X, tMouse->Y);
				break;
			}
		}
	}

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
// Mouse up event
int CBrowser::MouseUp(mouse_t *tMouse, int nDown)
{
	std::string sel = GetSelectedText();

	// Copy to clipboard with 2-nd or 3-rd mouse button
	if( sel.size() != 0 && bSelectionGrabbed && 
		(tMouse->Up & SDL_BUTTON(2) || tMouse->Up & SDL_BUTTON(3)) )
        copy_to_clipboard(sel);

	// Check if any of the active areas has been clicked
	if (sel.size() == 0)  {
		for (std::list<CActiveArea>::iterator it = tActiveAreas.begin(); it != tActiveAreas.end(); it++) {
			if (it->InBox(tMouse->X, tMouse->Y))  {
				it->DoClick(tMouse->X, tMouse->Y);
				break;
			}
		}
	}

	bSelectionGrabbed = false;
	bCanLoseFocus = true;

	return BRW_NONE;
}

//////////////////
// Mouse wheel down event
int CBrowser::KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
	size_t oldline = iCursorLine;
	size_t oldcol = iCursorColumn;

	switch (keysym)  {
	case SDLK_DOWN:
		if (iCursorLine < tPureText.size() - 1)  {
			++iCursorLine;
			iCursorColumn = MIN(tPureText[iCursorLine].size() - 1, iCursorColumn);
			AdjustScrollbar();

			if (modstate.bShift)  {
				// TODO: selection
			} else {
				ClearSelection();
			}
		}
	return BRW_NONE;
	case SDLK_UP:
		if (iCursorLine > 0)  {
			--iCursorLine;
			iCursorColumn = MIN(tPureText[iCursorLine].size() - 1, iCursorColumn);
			AdjustScrollbar();

			if (modstate.bShift)  {
				// TODO: selection
			} else {
				ClearSelection();
			}
		}
	return BRW_NONE;

	case SDLK_RIGHT:
		if (iCursorLine < tPureText.size())  {
			if (iCursorColumn >= tPureText[iCursorLine].size())  {
				if  (iCursorLine < tPureText.size() - 1)  {
					iCursorColumn = 0;
					++iCursorLine;
					AdjustScrollbar();
				}
			} else {
				++iCursorColumn;
			}

			if (modstate.bShift)  {
				// There is a selection and the cursor is at the beginning of it
				if (iSelectionStartLine == oldline && iSelectionStartColumn == oldcol && !IsSelectionEmpty())  {
					iSelectionStartLine = iCursorLine;
					iSelectionStartColumn = iCursorColumn;

				// No selection or cursor at the end of the selection
				} else {
					iSelectionEndLine = iCursorLine;
					iSelectionEndColumn = iCursorColumn;
				}
			} else {
				ClearSelection();
			}
		}
	return BRW_NONE;

	case SDLK_LEFT:
		if (iCursorLine >= 0)  {
			if (iCursorColumn == 0)  {
				if  (iCursorLine > 0)  {
					--iCursorLine;
					iCursorColumn = MAX(0, (int)tPureText[iCursorLine].size() - 1);
					AdjustScrollbar();
				}
			} else {
				--iCursorColumn;
			}

			if (modstate.bShift)  {
				// There is a selection and the cursor is at the end of it
				if (iSelectionEndLine == oldline && iSelectionEndColumn == oldcol && !IsSelectionEmpty())  {
					iSelectionEndLine = iCursorLine;
					iSelectionEndColumn = iCursorColumn;

				// No selection or cursor at the end of the selection
				} else {
					iSelectionStartLine = iCursorLine;
					iSelectionStartColumn = iCursorColumn;
				}
			} else {
				ClearSelection();
			}
		}
	return BRW_NONE;
	}

    // Ctrl-c or Super-c or Ctrl-Insert (copy)
    if ((modstate.bCtrl || modstate.bSuper) && keysym == SDLK_c ||
		((modstate.bCtrl || modstate.bSuper) && keysym == SDLK_INSERT )) {
		if (!IsSelectionEmpty())
			copy_to_clipboard(GetSelectedText());
        return BRW_NONE;
    }

	return BRW_NONE;
}


///////////////////
// Render the browser
void CBrowser::Draw(SDL_Surface * bmpDest)
{
	// 3D Box
	DrawRect(bmpDest, iX, iY, iX + iWidth, iY + iHeight, tLX->clBoxDark);
	DrawRect(bmpDest, iX + 1, iY + 1, iX + iWidth - 1, iY + iHeight - 1, tLX->clBoxLight);
	
	// Render the content
	RenderContent(bmpDest);

	// Scrollbar
	if (bUseScroll)
		cScrollbar.Draw(bmpDest);
}

/////////////////////
// Converts mouse position to cursor position
void CBrowser::MousePosToCursorPos(int ms_x, int ms_y, size_t& cur_x, size_t& cur_y)
{
	cur_x = cur_y = 0;

	size_t scroll_val = (bUseScroll ? cScrollbar.getValue() : 0);

	if (tPureText.empty())
		return;

	int x = ms_x - iX - BORDER_SIZE;
	int y = ms_y - iY - BORDER_SIZE;

	// Get the line
	int line = y / tLX->cFont.GetHeight() + scroll_val;
	if (line >= (int)tPureText.size())
		line = tPureText.size() - 1;
	if (line < 0)
		line = 0;

	// Get the column
	size_t column = 0;
	if (x > 0)  {
		int width = 0;
		int next_width = 0;

		std::string::const_iterator it = tPureText[line].begin();
		while (it != tPureText[line].end())  {

			UnicodeChar c = GetNextUnicodeFromUtf8(it, tPureText[line].end());
			width = next_width;
			next_width += tLX->cFont.GetCharacterWidth(c) + tLX->cFont.GetSpacing();

			if (width <= x && x <= next_width)
				break;

			++column;
		}

		//column = MAX((size_t)0, MIN(column, tPureText[line].size() - 1));
	}

	cur_x = column;
	cur_y = line;
}


////////////////////////
// Converts cursor position to a mouse (screen) position
void CBrowser::CursorPosToMousePos(size_t cur_x, size_t cur_y, int& ms_x, int& ms_y)
{
	ms_x = iX + BORDER_SIZE;
	ms_y = iY + BORDER_SIZE;

	int y = (int)cur_y * tLX->cFont.GetHeight();

	// Get the Y coordinate
	if (cur_y >= tPureText.size())
		return;

	// Get the X coordinate
	std::string::iterator end = tPureText[cur_y].begin();
	SafeAdvance(end, cur_x, tPureText[cur_y].end());
	std::string tmp(tPureText[cur_y].begin(), end);
	int x = tLX->cFont.GetWidth(tmp);

	ms_x = iX + BORDER_SIZE + x;
	ms_y = iY + BORDER_SIZE + y;
}

//////////////////////
// Returns width of the given text
int CBrowser::TextW(const std::string& text, FontFormat& fmt)
{
	if (fmt.bold)
		return tLX->cFont.GetWidth(text) + 1;
	else
		return tLX->cFont.GetWidth(text);
}

////////////////////
// Creates a new line and pushes the current one to the pure text stack
void CBrowser::EndLine()
{
	// Trim trailing space if present
	if (sCurLine.size())
		if (*sCurLine.rbegin() == ' ')  {
			sCurLine.resize(sCurLine.size() - 1);
			curX -= tLX->cFont.GetCharacterWidth(' ') + 1;
		}

	size_t cur_line_size = Utf8StringSize(sCurLine);

	// Check if the cursor is at the end of the line
	if (tPureText.size() == iCursorLine && iCursorColumn >= cur_line_size)
		DrawVLine(tDestSurface, curY, curY + tLX->cFont.GetHeight(), curX, tLX->clTextboxCursor);

	// Multiline link? Split it into multiple links
	if (bInLink)  {
		std::string url = cCurrentLink.getStringData();
		EndLink();
		StartLink(url);
	}

	// Add the line to the pure text and start a new line
	tPureText.push_back(sCurLine);
	sCurLine.clear();
	curY += tLX->cFont.GetHeight();
	curX = iX + BORDER_SIZE;
}

////////////////////
// Converts the given URL to a standard format (accepts both absolute and relative URLs)
std::string CBrowser::GetFullURL(const std::string& url)
{
	// Absolute URL
	if (url.size() > 8)  {
		if (stringcaseequal(url.substr(0, 7), "http://") ||
			stringcaseequal(url.substr(0, 8), "https://") || stringcaseequal(url.substr(0, 6), "ftp://") ||
			stringcaseequal(url.substr(0, 7), "mailto:"))
			return url;

		if (stringcaseequal(url.substr(0, 4), "www."))
			return "http://" + url;
	}

	// Relative URL

	// Remove any slashes at the beginning of the given URL
	std::string link_url = url;
	if (link_url.size())
		if (*link_url.begin() == '/')
			link_url.erase(0);

	// Find the last slash of the base URL
	size_t slashpos = sURL.rfind('/');
	if (slashpos == std::string::npos)
		link_url = sHostName + "/" + sURL + "/" + url; // No slash in the base URL, just append the given URL
	else
		link_url = sHostName + "/" + sURL.substr(0, slashpos) + url; // Remove the filename from the base URL and append the given URL instead

	// Prepend HTTP if not present
	if (link_url.size() > 7)
		if (!stringcaseequal(link_url.substr(0, 7), "http://"))
			link_url = "http://" + link_url;

	return link_url;
}

//////////////////////////
// Starts a link area
void CBrowser::StartLink(const std::string& url)
{
	// Already in a link, ignore
	if (bInLink)
		return;

	bInLink = true;

	cCurrentLink.Clear();


	// Setup the information
	cCurrentLink.tRect.x = curX;
	cCurrentLink.tRect.y = curY;
	cCurrentLink.sData = GetFullURL(url);
	cCurrentLink.tParent = this;
	cCurrentLink.tClickFunc = &CBrowser::LinkClickHandler;
	cCurrentLink.tMouseMoveFunc = &CBrowser::LinkMouseMoveHandler;
}

//////////////////////////
// Ends the link area
void CBrowser::EndLink()
{
	bInLink = false;

	// Finish the rect
	cCurrentLink.tRect.w = curX - cCurrentLink.tRect.x;
	cCurrentLink.tRect.h = curY - cCurrentLink.tRect.y + tLX->cFont.GetHeight();

	// Add it to the list
	tActiveAreas.push_back(cCurrentLink);

	// Cleanup
	cCurrentLink.Clear();
}

///////////////////////////
// Renders a chunk of text with the given style
void CBrowser::RenderText(SDL_Surface *bmpDest, FontFormat& fmt, int& curX, int& curY, int maxX, const std::string& text)
{
	if (!text.size())
		return;

	// Text
	bool was_space = sCurLine.size() > 0 ? (*sCurLine.rbegin() == ' ') : false;
	size_t current_column = Utf8StringSize(sCurLine);
	for (std::string::const_iterator it = text.begin(); it != text.end();)  {
		// Handle spaces & newlines
		if (*it == '\r')  {
			it++;
			continue;
		}

		std::string word;

		// Handle multiple blank characters
		if ((*it == '\n' || *it == ' ' || *it == '\t') && sCurLine.size() != 0)  {
			if (!was_space)  {
				word = " ";
			} else {
				it++;
				was_space = true;
				continue;
			}

		// Normal word
		} else {
			was_space = false;
			
			// Ignore empty words
			word = GetNextWord(it, text);
			if (word.size() == 0)  {
				it++;
				continue;
			}
		}

		// Word wrap
		int width = TextW(word, fmt);
		if (curX + width >= maxX)  {
			EndLine();
			current_column = 0;
		}
		sCurLine += word;
		it += word.size();


		// Don't draw the line if it's not visible
		if (bUseScroll && ((int)tPureText.size() < cScrollbar.getValue() ||
			(int)tPureText.size() > cScrollbar.getValue() + cScrollbar.getItemsperbox()))  {
			current_column += word.size();
			continue;
		}

		// Render the word
		std::string::const_iterator word_it = word.begin();
		while (word_it != word.end())  {

			// Get the character
			UnicodeChar c = GetNextUnicodeFromUtf8(word_it, word.end());
			int w = tLX->cFont.GetCharacterWidth(c) + tLX->cFont.GetSpacing();

			// Draw selection
			if (InSelection(tPureText.size(), current_column)) {
				DrawRectFill(bmpDest, curX, curY, curX + w, curY + tLX->cFont.GetHeight(), tLX->clSelection);
			}

			// Draw the character
			tLX->cFont.DrawGlyph(bmpDest, curX, curY, fmt.color, c);

			// Bold
			if (fmt.bold)
				tLX->cFont.DrawGlyph(bmpDest, curX + 1, curY, fmt.color, c);

			// Underline
			if (fmt.underline)
				DrawHLine(bmpDest, curX, curX + w, curY + tLX->cFont.GetHeight() - 2, fmt.color);

			// Cursor
			if (tPureText.size() == iCursorLine && current_column == iCursorColumn)
				DrawVLine(tDestSurface, curY, curY + tLX->cFont.GetHeight(), curX, tLX->clTextboxCursor);

			curX += w;
			++current_column;
		}


		was_space = ((*word.rbegin()) == ' ');  // HINT: word.size() != 0 here
	}
}

////////////////////
// Returns true if the given position is inside the selection
bool CBrowser::InSelection(size_t line, size_t column)
{
	if (line >= iSelectionStartLine && line <= iSelectionEndLine)  {
		if (line == iSelectionStartLine && line == iSelectionEndLine)
			return column >= iSelectionStartColumn && column < iSelectionEndColumn;
		if (line == iSelectionStartLine)
			return column >= iSelectionStartColumn;
		if (line == iSelectionEndLine)
			return column < iSelectionEndColumn;
		return true;
	}
	return false;
}

////////////////////
// Get text of the current selection
std::string CBrowser::GetSelectedText()
{
	// One line selection
	if (iSelectionStartLine == iSelectionEndLine)  {
		if (iSelectionStartLine < tPureText.size() && iSelectionStartColumn < iSelectionEndColumn)
			if (iSelectionStartColumn < tPureText[iSelectionStartLine].size() && iSelectionEndColumn <= tPureText[iSelectionStartLine].size())
				return Utf8SubStr(tPureText[iSelectionStartLine], iSelectionStartColumn, iSelectionEndColumn - iSelectionStartColumn);

	// More lines
	} else {
		// Safety checks
		if (iSelectionStartLine > iSelectionEndLine || iSelectionStartLine >= tPureText.size() ||
			iSelectionEndLine >= tPureText.size())
			return "";

		std::string res;

		// First line
		if (iSelectionStartColumn < tPureText[iSelectionStartLine].size())  {
			res += Utf8SubStr(tPureText[iSelectionStartLine], iSelectionStartColumn);
			res += '\n';
		}

		// All other lines
		int diff = iSelectionEndLine - iSelectionStartLine;
		for (int i = 1; i < diff; ++i)  {
			res += tPureText[iSelectionStartLine + i] + "\n";
		}

		// Last line
		if (iSelectionEndColumn < tPureText[iSelectionEndLine].size())
			res += Utf8SubStr(tPureText[iSelectionEndLine], 0, iSelectionEndColumn);

		return res;
	}

	return "";
}


//////////////////////
// Destroys any selection
void CBrowser::ClearSelection()
{
	iSelectionStartLine = iSelectionEndLine = iSelectionGrabLine = iCursorLine;
	iSelectionStartColumn = iSelectionEndColumn = iSelectionGrabColumn = iCursorColumn;
}

///////////////////////
// Returns true if no text is selected
bool CBrowser::IsSelectionEmpty()
{
	return iSelectionStartLine == iSelectionEndLine && iSelectionStartColumn == iSelectionEndColumn;
}

////////////////////////
// Makes sure that the line with the cursor is visible
void CBrowser::AdjustScrollbar(bool mouse)
{
	if (!bUseScroll)
		return;

	// If scrolling using mouse, don't scroll so fast
	if (mouse && tLX->fCurTime - fLastMouseScroll <= 0.1f)
		return;

	// Scroll down/up if necessary
	if ((int)iCursorLine >= cScrollbar.getValue() + cScrollbar.getItemsperbox())
		cScrollbar.setValue((int)iCursorLine - cScrollbar.getItemsperbox() + 1);
	else if ((int)iCursorLine < cScrollbar.getValue())
		cScrollbar.setValue(iCursorLine);

	if (mouse)
		fLastMouseScroll = tLX->fCurTime;
}


//////////////////////
// Helper function for TraverseNodes below
void CBrowser::BrowseChildren(xmlNodePtr node)
{
	// Process the children
	if (node->children)  {
		xmlNodePtr child = node->children;
		while (child)  {
			TraverseNodes(child);
			child = child->next;
		}
	}
}


////////////////////////
// Recursivelly walks through the nodes and renders them
void CBrowser::TraverseNodes(xmlNodePtr node)
{
	if (!node)
		return;

	const int maxX = iX + iWidth - BORDER_SIZE - 16;

	if (!xmlStrcasecmp(node->name, (xmlChar *)"style")) {}
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"script")) {}
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"embed")) {}
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"object")) {}

	// Bold
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"b") || !xmlStrcasecmp(node->name, (xmlChar *)"strong"))  {
		tFormatStack.push(tCurrentFormat);
		tCurrentFormat.bold = true;
		BrowseChildren(node);
		tCurrentFormat = tFormatStack.top();
		tFormatStack.pop();
		return;
	}

	// Underline
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"u"))  {
		tFormatStack.push(tCurrentFormat);
		tCurrentFormat.underline = true;
		BrowseChildren(node);
		tCurrentFormat = tFormatStack.top();
		tFormatStack.pop();
		return;
	}

	// Color
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"font"))  {
		tFormatStack.push(tCurrentFormat);
		tCurrentFormat.color = xmlGetColour(node, "color");
		BrowseChildren(node);
		tCurrentFormat = tFormatStack.top();
		tFormatStack.pop();
		return;
	}

	// Newline
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"br"))  {
		EndLine();
		return;
	}

	// Common block elements
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"p") || !xmlStrcasecmp(node->name, (xmlChar *)"div")
		|| !xmlStrcasecmp(node->name, (xmlChar *)"ul") || !xmlStrcasecmp(node->name, (xmlChar *)"ol"))  {
		EndLine();
		BrowseChildren(node);
		return;
	}

	// List item
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"li"))  {
		EndLine();
		curX += LIST_SPACING;
		tLX->cFont.DrawGlyph(tDestSurface, curX, curY, tCurrentFormat.color, 0xA4);
		tLX->cFont.DrawGlyph(tDestSurface, curX + 1, curY, tCurrentFormat.color, 0xA4);
		curX += tLX->cFont.GetCharacterWidth(0xA4) + LIST_SPACING;
		BrowseChildren(node);
		return;
	}

	// Headings (h1 - h6)
	else if (xmlStrlen(node->name) == 2 && node->name[0] == 'h' && node->name[1] >= '1' && node->name[1] <= '6')  {
		tFormatStack.push(tCurrentFormat);
		tCurrentFormat.bold = true;
		EndLine();
		BrowseChildren(node);
		tCurrentFormat = tFormatStack.top();
		tFormatStack.pop();
		return;
	}

	// Link (<a href= ...)
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"a"))  {

		std::string url = xmlGetString(node, "href");
		if (url.size() && !bInLink)  {	
			// Setup the format (blue underlined text by default)
			tFormatStack.push(tCurrentFormat);
			tCurrentFormat.color = Color(0, 0, 255);
			tCurrentFormat.underline = true;

			StartLink(url);
		}

		// Get the content of the link
		BrowseChildren(node);

		// End the link
		if (bInLink)
			EndLink();

		// Restore the old format
		tCurrentFormat = tFormatStack.top();
		tFormatStack.pop();

		return;
	}

	// Horizontal rule
	else if (!xmlStrcasecmp(node->name, (xmlChar *)"hr"))  {
		EndLine();
		DrawHLine(tDestSurface, curX + 5, iX + iWidth - BORDER_SIZE - 5, curY + tLX->cFont.GetHeight() / 2, Color(0, 0, 0));
		EndLine();
		return;
	}

	else if (!xmlStrcasecmp(node->name, (xmlChar *)"text") && node->content)
		RenderText(tDestSurface, tCurrentFormat, curX, curY, maxX, (const char *)node->content);

	BrowseChildren(node);
}

//////////////////
// Renders the textual content
void CBrowser::RenderContent(SDL_Surface * bmpDest)
{
	// Nothing to draw
	if (!tHtmlDocument || !tRootNode)
	{
		// Backgorund (transparent by default)
		//DrawRectFill(bmpDest, iX + BORDER_SIZE, iY + BORDER_SIZE, iX + iWidth, iY + iHeight, tLX->clChatBoxBackground);
		return;
	}
	
	tPureText.clear();
	tActiveAreas.clear();
	
	curX = iX + BORDER_SIZE;
	curY = iY + BORDER_SIZE - cScrollbar.getValue() * tLX->cFont.GetHeight();
	tDestSurface = bmpDest;
	tCurrentFormat.bold = false; 
	tCurrentFormat.underline = false;
	tCurrentFormat.color = xmlGetColour(tRootNode, "text", tLX->clNormalText);
	tBgColor = xmlGetColour(tRootNode, "bgcolor", tLX->clChatBoxBackground);

	// Background
	if( tBgColor.a != SDL_ALPHA_TRANSPARENT )
		DrawRectFill(bmpDest, iX + BORDER_SIZE, iY + BORDER_SIZE, iX + iWidth, iY + iHeight, tBgColor);
	
	while (tFormatStack.size()) tFormatStack.pop();  // Free any previous stack

	// Setup the clipping
	SDL_Rect clip = {iX + BORDER_SIZE, iY + BORDER_SIZE, iWidth - BORDER_SIZE, iHeight - BORDER_SIZE};
	SDL_SetClipRect(bmpDest, &clip);

	// Go through the document
	sCurLine.clear();
	TraverseNodes(tRootNode);
	EndLine(); // Add the last parsed line

	int linesperbox = iHeight / tLX->cFont.GetHeight();
	if ((int)tPureText.size() >= linesperbox)  {
		cScrollbar.setMax(tPureText.size());
		cScrollbar.setItemsperbox(linesperbox - 1);
		bUseScroll = true;
	} else
		bUseScroll = false;

	tDestSurface = NULL;

	// Restore clipping
	SDL_SetClipRect(bmpDest, NULL);
}

//
// Chatbox routines
//

static const char * txtTypeNames[] = {
	"CHAT",
	"NORMAL",
	"NOTICE",
	"IMPORTANT",
	"NETWORK",
	"PRIVATE",
	"TEAMPM"
};

//////////////////////////
// Initialize as a chatbox
void CBrowser::InitializeChatBox( const std::string & initText )
{
	tData = initText;
	if( tData == "" )
		tData = "<html><body bgcolor=\"transparent\"></body></html>"; // Transparent background
	Parse();
}

///////////////////////////////
// Add a new line to the browser
void CBrowser::AddChatBoxLine(const std::string & text, Color color, TXT_TYPE textType, bool bold, bool underline)
{
	if (!tHtmlDocument || !tRootNode || textType < TXT_CHAT || textType > TXT_TEAMPM)
		return;
		
	xmlNodePtr line = xmlNewChild( tRootNode, NULL, (const xmlChar *)"div", NULL );

	xmlNewProp( line, (const xmlChar *)"class", (const xmlChar *)txtTypeNames[textType] );

	line = xmlNewChild( line, NULL, (const xmlChar *)"font", NULL );

	char t[20];
	sprintf(t, "#%02X%02X%02X", (unsigned)color.r, (unsigned)color.g, (unsigned)color.b);
	xmlNewProp( line, (const xmlChar *)"color", (const xmlChar *)t );

	if( bold )
		line = xmlNewChild( line, NULL, (const xmlChar *)"b", NULL );
	if( underline )
		line = xmlNewChild( line, NULL, (const xmlChar *)"u", NULL );
		
	xmlNodeAddContent( line, (const xmlChar *)text.c_str() );
}

///////////////////////////////
// Add a new line to the browser (compatible version, guessing textType from color
void CBrowser::AddChatBoxLine(const std::string & text, Color color, bool bold, bool underline)
{
	TXT_TYPE textType = TXT_CHAT;
	if( color.get() == tLX->clChatText )
		textType = TXT_CHAT;
	else if( color.get() == tLX->clNormalText )
		textType = TXT_NORMAL;
	else if( color.get() == tLX->clNotice )
		textType = TXT_NOTICE;
	else if( color.get() == tLX->clNetworkText )
		textType = TXT_NETWORK;
	else if( color.get() == tLX->clPrivateText )
		textType = TXT_PRIVATE;
	else if( 	color.get() == tLX->clTeamColors[0] ||
				color.get() == tLX->clTeamColors[1] ||
				color.get() == tLX->clTeamColors[2] ||
				color.get() == tLX->clTeamColors[3] )
		textType = TXT_TEAMPM;
	
	AddChatBoxLine(text, color, textType, bold, underline);
}

//////////////////////
// Get all the text from the chatbox browser
std::string CBrowser::GetChatBoxText()
{
	if (!tHtmlDocument || !tRootNode)
		return "";

	xmlChar *xmlbuff = NULL;
	int buffersize = 0;
	xmlDocDumpFormatMemory(tHtmlDocument, &xmlbuff, &buffersize, 1);
	std::string ret( (const char *) xmlbuff );
	xmlFree(xmlbuff);

	return ret;
}

//////////////////////
// Remove all network messages from chatbox, and remove extra lines if more than 127 lines
void CBrowser::CleanUpChatBox( const std::vector<TXT_TYPE> & removedText, int maxLineCount )
{
	if (!tHtmlDocument || !tRootNode)
		return;
	
	int lineCount = 0;
	for( xmlNodePtr node = tRootNode->children; node != NULL; )
	{
		xmlNodePtr nextNode = node->next;
		lineCount ++;
		if( xmlStrcasecmp(node->name, (xmlChar *)"div") != 0 )
		{
			xmlUnlinkNode(node);
			xmlFreeNode(node);
			lineCount --;
		}
		else
		{
			std::string divClass = xmlGetString( node, "class", "CHAT" );
			for( size_t i = 0; i < removedText.size(); i++ )
				if( txtTypeNames[removedText[i]] == divClass )
				{
					xmlUnlinkNode(node);
					xmlFreeNode(node);
					lineCount --;
					break;
				}
		}
	
		node = nextNode;
	}
	
	for( xmlNodePtr node = tRootNode->children; node != NULL && lineCount > maxLineCount; lineCount -- )
	{
		xmlUnlinkNode(node);
		xmlFreeNode(node);
	};
}

/////////////////////////
// A callback function that gets called when the user clicks on a link
void CBrowser::LinkClickHandler(CBrowser::CActiveArea *area)
{
	SetGameCursor(CURSOR_HAND);
	OpenLinkInExternBrowser( area->getStringData() );
}

/////////////////////////
// A callback function that gets called when the user moves a mouse over a link
void CBrowser::LinkMouseMoveHandler(CBrowser::CActiveArea *area)
{
	SetGameCursor(CURSOR_HAND);
}

//
// CActiveArea class
//

///////////////////
// Click on the active area
void CBrowser::CActiveArea::DoClick(int x, int y)
{
	if (tClickFunc)
		(tParent->*tClickFunc)(this);
}

///////////////////
// Mouse moving over the area
void CBrowser::CActiveArea::DoMouseMove(int x, int y)
{
	if (tMouseMoveFunc)
		(tParent->*tMouseMoveFunc)(this);
}


}; // namespace DeprecatedGUI
