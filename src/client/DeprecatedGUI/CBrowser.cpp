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
	sURL = url;
	cHttp.RequestData(url, tLXOptions->sHttpProxy);
}

void CBrowser::LoadFromFile(const std::string& file)
{
	tData.clear();
	bFinished = false;
	std::ifstream *fp = OpenGameFileR(file);
	if (!fp)
		return;

	sURL = file;
	(*fp) >> tData;

	fp->close();
	delete fp;

	bFinished = true;
	Parse();
}

void CBrowser::LoadFromString(const std::string& data)
{
	sURL = "";
	tData = data;
	bFinished = true;
	Parse();
}

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

	if (!node || !node->children)  {
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
	if( sTextSelection != "" && 
		(tMouse->FirstDown & SDL_BUTTON(2) || tMouse->FirstDown & SDL_BUTTON(3)) )
	{
        copy_to_clipboard(sTextSelection);
		bSelectionGrabbed = false;
		return BRW_NONE;
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
	} else {
		iSelectionGrabColumn = iCursorColumn;
		iSelectionGrabLine = iCursorLine;

		// Destroy any previous selection
		iSelectionStartColumn = iSelectionEndColumn = iCursorColumn;
		iSelectionStartLine = iSelectionEndLine = iCursorLine;
	}

	bSelectionGrabbed = true;

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

int CBrowser::MouseUp(mouse_t *tMouse, int nDown)
{
	// Copy to clipboard with 2-nd or 3-rd mouse button
	if( bSelectionGrabbed && 
		(tMouse->Up & SDL_BUTTON(2) || tMouse->Up & SDL_BUTTON(3)) )
        copy_to_clipboard(sTextSelection);

	bSelectionGrabbed = false;

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

    // Ctrl-c or Super-c or Ctrl-Insert (copy)
    if ((modstate.bCtrl || modstate.bSuper) && keysym == SDLK_c ||
		((modstate.bCtrl || modstate.bSuper) && keysym == SDLK_INSERT )) {
        copy_to_clipboard(sTextSelection);
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
	size_t line = 0;
	if (y > 0)  {
		line = y / tLX->cFont.GetHeight() + scroll_val;
		if (line >= tPureText.size())
			line = tPureText.size() - 1;
	}

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

	// Add a newline to the selected text if necessary
	if (InSelection(tPureText.size(), cur_line_size))
		sTextSelection += '\n';

	// Add the line to the pure text and start a new line
	tPureText.push_back(sCurLine);
	sCurLine.clear();
	curY += tLX->cFont.GetHeight();
	curX = iX + BORDER_SIZE;
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
				sTextSelection += GetUtf8FromUnicode(c);
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
		// Backgorund 
		DrawRectFill(bmpDest, iX + BORDER_SIZE, iY + BORDER_SIZE, iX + iWidth, iY + iHeight, tLX->clChatBoxBackground);
		return;
	}
	
	tPureText.clear();
	sTextSelection.clear();
	
	curX = iX + BORDER_SIZE;
	curY = iY + BORDER_SIZE - cScrollbar.getValue() * tLX->cFont.GetHeight();
	tDestSurface = bmpDest;
	tCurrentFormat.bold = false; 
	tCurrentFormat.underline = false;
	tCurrentFormat.color = xmlGetColour(tRootNode, "text", tLX->clNormalText);
	tBgColor = xmlGetColour(tRootNode, "bgcolor", tLX->clChatBoxBackground);

	// Background
	DrawRectFill(bmpDest, iX + BORDER_SIZE, iY + BORDER_SIZE, iX + iWidth, iY + iHeight, tBgColor);
	
	while (tFormatStack.size()) tFormatStack.pop();  // Free any previous stack

	// Setup the clipping
	SDL_Rect clip = {iX + BORDER_SIZE, iY + BORDER_SIZE, iWidth - BORDER_SIZE, iHeight - BORDER_SIZE};
	SDL_SetClipRect(bmpDest, &clip);

	// Go through the document
	sCurLine.clear();
	TraverseNodes(tRootNode);

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

}; // namespace DeprecatedGUI
