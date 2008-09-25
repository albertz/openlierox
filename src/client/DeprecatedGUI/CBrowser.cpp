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
void CBrowser::Draw(SDL_Surface * bmpDest)
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

static const float boldCoeff = 1.1f;

int CBrowser::TextW(const std::string& text, FontFormat& fmt)
{
	if (fmt.bold)
		return (int)ceil((float)tLX->cFont.GetWidth(text) * boldCoeff);
	else
		return tLX->cFont.GetWidth(text);
}

void CBrowser::RenderText(SDL_Surface *bmpDest, FontFormat& fmt, int& curX, int& curY, int maxX, const std::string& text, std::string& line)
{
	if (!text.size())
		return;

	// Text
	for (std::string::const_iterator it = text.begin(); it != text.end();)  {
		std::string word = GetNextWord(it, text);
		if (curX + TextW(word, fmt) >= maxX)  {
			curY += tLX->cFont.GetHeight();
			tPureText.push_back(line);
			line.clear();
			curX = iX + BORDER_SIZE;
		}
		line += word;
		it += word.size();

		word += " ";
		int width = TextW(word, fmt);
		tLX->cFont.Draw(bmpDest, curX, curY, fmt.color.get(bmpDest->format), word);
		if (fmt.underline)
			DrawHLine(bmpDest, curX, curX + width, curY + tLX->cFont.GetHeight() - 3, fmt.color);
		if (fmt.bold)  {
			tLX->cFont.Draw(bmpDest, curX + 1, curY, fmt.color.get(bmpDest->format), word);
		}
		curX += width;
	}
}

//////////////////
// Renders the textual content
void CBrowser::RenderContent(SDL_Surface * bmpDest)
{
	// Nothing to draw
	if (!tHtmlDocument || !tRootNode)
		return;

	int curX = iX + BORDER_SIZE;
	int curY = iY + BORDER_SIZE - cScrollbar.getValue() * tLX->cFont.GetHeight();
	int maxX = iX + iWidth - BORDER_SIZE - 16;
	FontFormat currentFormat ={ false, false, Color() };
	std::stack<FontFormat> formatStack;
	int lines = 0;

	// Setup the clipping
	SDL_Rect clip = {iX + BORDER_SIZE, iY + BORDER_SIZE, iWidth - BORDER_SIZE, iHeight - BORDER_SIZE};
	SDL_SetClipRect(bmpDest, &clip);

	// Go through the document
	std::string line;
	xmlNodePtr node = tRootNode->children;
	while (node)  {
		if (!xmlStrcmp(node->name, (xmlChar *)"style"))
			continue;
		if (!xmlStrcmp(node->name, (xmlChar *)"script"))
			continue;
		if (!xmlStrcmp(node->name, (xmlChar *)"embed"))
			continue;
		if (!xmlStrcmp(node->name, (xmlChar *)"object"))
			continue;

		// Bold
		if (!xmlStrcmp(node->name, (xmlChar *)"b"))  {
			formatStack.push(currentFormat);
			currentFormat.bold = true;
			RenderText(bmpDest, currentFormat, curX, curY, maxX, xmlNodeText(node, ""), line);
			currentFormat = formatStack.top();
			formatStack.pop();
		}

		// Underline
		if (!xmlStrcmp(node->name, (xmlChar *)"u"))  {
			formatStack.push(currentFormat);
			currentFormat.underline = true;
			RenderText(bmpDest, currentFormat, curX, curY, maxX, xmlNodeText(node, ""), line);
			currentFormat = formatStack.top();
			formatStack.pop();
		}

		// Color
		if (!xmlStrcmp(node->name, (xmlChar *)"font"))  {
			formatStack.push(currentFormat);
			currentFormat.color = xmlGetColour(node, "color");
			RenderText(bmpDest, currentFormat, curX, curY, maxX, xmlNodeText(node, ""), line);
			currentFormat = formatStack.top();
			formatStack.pop();
		}

		if (!xmlStrcmp(node->name, (xmlChar *)"br"))  {
			tPureText.push_back(line);
			line.clear();
			curY += tLX->cFont.GetHeight();
			curX = iX + BORDER_SIZE;
		}

		RenderText(bmpDest, currentFormat, curX, curY, maxX, xmlNodeText(node, ""), line);

		// Skip to next node
		if (node->children)  {
			node = node->children;
		} else if (node->next)  {
			node = node->next;
		} else if (node->parent)  {
			if (node->parent->next)
				node = node->parent->next;
			else
				break;  // End of the file
		} else {
			break; // End of the file
		}

	}

	// Restore clipping
	SDL_SetClipRect(bmpDest, NULL);
}

}; // namespace DeprecatedGUI
