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

namespace DeprecatedGUI {

#define BORDER_SIZE 2

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

	if (bSelectionGrabbed)  {
		iSelectionStartColumn = iSelectionGrabColumn;
		iSelectionStartLine = iSelectionGrabLine;
		iSelectionEndLine = iCursorLine;
		iSelectionEndColumn = iCursorColumn;
	} else {
		iSelectionGrabColumn = iCursorColumn;
		iSelectionGrabLine = iCursorLine;
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

	return BRW_NONE;
}


///////////////////
// Render the browser
void CBrowser::Draw(SDL_Surface * bmpDest)
{
	// 3D Box
	DrawRect(bmpDest, iX, iY, iX + iWidth, iY + iHeight, tLX->clBoxDark);
	DrawRect(bmpDest, iX + 1, iY + 1, iX + iWidth - 1, iY + iHeight - 1, tLX->clBoxLight);
	
	// Background (white)
	DrawRectFill(bmpDest, iX + BORDER_SIZE, iY + BORDER_SIZE, iX + iWidth, iY + iHeight, tLX->clWhite);

	DrawSelection(bmpDest);

	// Render the content
	RenderContent(bmpDest);

	// Draw the cursor
	int x, y;
	CursorPosToMousePos(iCursorColumn, iCursorLine, x, y);
	DrawVLine(bmpDest, y, y + tLX->cFont.GetHeight(), x, Color(255, 0, 0));

	// Scrollbar
	if (bUseScroll)
		cScrollbar.Draw(bmpDest);
}

//////////////////////
// Draws a selection
void CBrowser::DrawSelection(SDL_Surface *bmpDest)
{
	return;
	Color cl(0, 0, 255);

	size_t sel_start_line = iSelectionStartLine;
	size_t sel_end_line = iSelectionEndLine;
	if (sel_start_line > sel_end_line)  {
		size_t tmp = sel_start_line;
		sel_start_line = sel_end_line;
		sel_end_line = tmp;
	}

	if (sel_start_line >= tPureText.size() || sel_end_line >= tPureText.size())
		return;

	size_t col = iSelectionStartColumn;
	size_t lin = iSelectionStartLine;
	for (size_t i = 0; i <= sel_end_line - sel_start_line; ++i)  {
		int start_x, start_y;
		CursorPosToMousePos(col, lin + i, start_x, start_y);
		if (i != 0)
			start_x = iX + BORDER_SIZE;

		int end_x, end_y;
		if (i == sel_end_line - sel_start_line)  {
			CursorPosToMousePos(iSelectionEndColumn, iSelectionEndLine, end_x, end_y);
			end_y += tLX->cFont.GetHeight();
		} else  {
			end_x = iX + BORDER_SIZE + tLX->cFont.GetWidth(tPureText[sel_start_line + i]);
			end_y = start_y + (i + 1) * tLX->cFont.GetHeight();
		}

		DrawRectFill(bmpDest, start_x, start_y, end_x, end_y, cl);
	}
}

/////////////////////
// Converts mouse position to cursor position
void CBrowser::MousePosToCursorPos(int ms_x, int ms_y, size_t& cur_x, size_t& cur_y)
{
	cur_x = cur_y = 0;

	if (tPureText.empty())
		return;

	int x = ms_x - iX - BORDER_SIZE;
	int y = ms_y - iY - BORDER_SIZE;

	// Get the line
	size_t line = y / tLX->cFont.GetHeight();
	if (line >= tPureText.size())
		line = tPureText.size() - 1;

	// Get the column
	size_t column = 0;
	int width = 0;
	int width2 = 0;
	std::string tmp;
	std::string::iterator it = tPureText[line].begin();
	while (it != tPureText[line].end())  {
		if (width <= x && x <= width2)
			break;
		tmp += *it;
		width = tLX->cFont.GetWidth(tmp);
		it++;
		if (it == tPureText[line].end())  {
			cur_y = line;
			cur_x = tPureText[line].size();
			return;
		}
		width2 = tLX->cFont.GetWidth(tmp + *it);
		++column;
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


///////////////////////////
// Renders a chunk of text with the given style
void CBrowser::RenderText(SDL_Surface *bmpDest, FontFormat& fmt, int& curX, int& curY, int maxX, const std::string& text)
{
	if (!text.size())
		return;

	//std::string buffer = text;

	// Text
	bool was_space = false;
	for (std::string::const_iterator it = text.begin(); it != text.end();)  {
		// Handle spaces & newlines
		if (*it == '\r')  {
			it++;
			continue;
		}

		if ((*it == '\n' || *it == ' ' || *it == '\t') && sCurLine.size() != 0)  {
			if (!was_space)
				curX += tLX->cFont.GetWidth(" ");
			was_space = true;
			it++;
			continue;
		}

		was_space = false;
		
		// Ignore empty words
		std::string word = GetNextWord(it, text);
		if (word.size() == 0)  {
			it++;
			continue;
		}

		int width = TextW(word, fmt);
		if (curX + width >= maxX)  {
			curY += tLX->cFont.GetHeight();
			tPureText.push_back(sCurLine);
			sCurLine.clear();
			curX = iX + BORDER_SIZE;
		}
		sCurLine += word;
		it += word.size();
		
		Color clSelection(0, 255, 255);
		size_t charsX, charsY;
		MousePosToCursorPos( curX, curY, charsX, charsY);
		// TODO: select portions of lines, select by rect
		//if( iSelectionStartColumn <= charsX && iSelectionEndColumn >= charsX &&
		if(	iSelectionStartLine <= charsY && iSelectionEndLine > charsY )
			DrawRectFill(bmpDest, curX, curY, curX + width + was_space ? tLX->cFont.GetWidth(" ") : 0, 
							curY + tLX->cFont.GetHeight(), clSelection);
		
		tLX->cFont.Draw(bmpDest, curX, curY, fmt.color.get(bmpDest->format), word);
		if (fmt.underline)
			DrawHLine(bmpDest, curX, curX + width, curY + tLX->cFont.GetHeight() - 3, fmt.color);
		if (fmt.bold)  {
			tLX->cFont.Draw(bmpDest, curX + 1, curY, fmt.color.get(bmpDest->format), word);
		}
		curX += width;
	}
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

	if (!xmlStrcmp(node->name, (xmlChar *)"style")) {}
	else if (!xmlStrcmp(node->name, (xmlChar *)"script")) {}
	else if (!xmlStrcmp(node->name, (xmlChar *)"embed")) {}
	else if (!xmlStrcmp(node->name, (xmlChar *)"object")) {}

	// Bold
	else if (!xmlStrcmp(node->name, (xmlChar *)"b"))  {
		tFormatStack.push(tCurrentFormat);
		tCurrentFormat.bold = true;
		BrowseChildren(node);
		tCurrentFormat = tFormatStack.top();
		tFormatStack.pop();
		return;
	}

	// Underline
	else if (!xmlStrcmp(node->name, (xmlChar *)"u"))  {
		tFormatStack.push(tCurrentFormat);
		tCurrentFormat.underline = true;
		BrowseChildren(node);
		tCurrentFormat = tFormatStack.top();
		tFormatStack.pop();
		return;
	}

	// Color
	else if (!xmlStrcmp(node->name, (xmlChar *)"font"))  {
		tFormatStack.push(tCurrentFormat);
		tCurrentFormat.color = xmlGetColour(node, "color");
		BrowseChildren(node);
		tCurrentFormat = tFormatStack.top();
		tFormatStack.pop();
		return;
	}

	else if (!xmlStrcmp(node->name, (xmlChar *)"br"))  {
		tPureText.push_back(sCurLine);
		sCurLine.clear();
		curY += tLX->cFont.GetHeight();
		curX = iX + BORDER_SIZE;
		return;
	}

	else if (!xmlStrcmp(node->name, (xmlChar *)"text") && node->content)
		RenderText(tDestSurface, tCurrentFormat, curX, curY, maxX, (const char *)node->content);

	BrowseChildren(node);
}

//////////////////
// Renders the textual content
void CBrowser::RenderContent(SDL_Surface * bmpDest)
{
	// Nothing to draw
	if (!tHtmlDocument || !tRootNode)
		return;

	tPureText.clear();

	curX = iX + BORDER_SIZE;
	curY = iY + BORDER_SIZE - cScrollbar.getValue() * tLX->cFont.GetHeight();
	tDestSurface = bmpDest;
	tCurrentFormat.bold = false; tCurrentFormat.underline = false;
	while (tFormatStack.size()) tFormatStack.pop();  // Free any previous stack

	// Setup the clipping
	SDL_Rect clip = {iX + BORDER_SIZE, iY + BORDER_SIZE, iWidth - BORDER_SIZE, iHeight - BORDER_SIZE};
	SDL_SetClipRect(bmpDest, &clip);

	// Go through the document
	sCurLine.clear();
	TraverseNodes(tRootNode);

	if ((int)tPureText.size() >= iWidth / tLX->cFont.GetHeight())  {
		cScrollbar.setMax(tPureText.size());
		cScrollbar.setItemsperbox(iWidth / tLX->cFont.GetHeight());
		bUseScroll = true;
	} else
		bUseScroll = false;

	tDestSurface = NULL;

	// Restore clipping
	SDL_SetClipRect(bmpDest, NULL);
}

}; // namespace DeprecatedGUI
