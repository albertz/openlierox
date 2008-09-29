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


#ifndef __CBROWSER_H__DEPRECATED_GUI__
#define __CBROWSER_H__DEPRECATED_GUI__


#include "InputEvents.h"
#include <libxml/HTMLparser.h>
#include <stack>
#include <string>
#include "LieroX.h"

namespace DeprecatedGUI {

// Browser messages
enum {
	BRW_NONE = -1
};

struct FontFormat  {
	bool bold;
	bool underline;
	Color color;
};


// Browser class
class CBrowser : public CWidget {
public:
	CBrowser() :
	  bFinished(false),
	  iClientWidth(0),
	  iClientHeight(0),
	  tHtmlDocument(NULL),
	  tRootNode(NULL),
	  curX(0),
	  curY(0),
	  tDestSurface(NULL),
	  iCursorColumn(0),
	  iCursorLine(0),
	  iSelectionStartLine(0),
	  iSelectionStartColumn(0),
	  iSelectionEndLine(0),
	  iSelectionEndColumn(0),
	  bSelectionGrabbed(false)
	  {
	  	tCurrentFormat.bold = false;
	  	tCurrentFormat.underline = false;
	  	tCurrentFormat.color = Color(0,0,0);
	  	tBgColor = Color(255,255,255);
	  }

private:
	// Attributes
	CHttp					cHttp;
	std::string				tData;
	std::string				sURL;
	bool					bFinished;
	int						iClientWidth;
	int						iClientHeight;
	htmlDocPtr				tHtmlDocument;
	htmlNodePtr				tRootNode;
	std::vector<std::string>	tPureText;

	// Parsing temps
	FontFormat				tCurrentFormat;
	std::stack<FontFormat>	tFormatStack;
	Color					tBgColor;
	int						curX;
	int						curY;
	std::string				sCurLine;
	SDL_Surface				*tDestSurface;
	std::string				sTextSelection;

	// Selection
	size_t					iCursorColumn;
	size_t					iCursorLine;
	size_t					iSelectionStartLine;
	size_t					iSelectionStartColumn;
	size_t					iSelectionEndLine;
	size_t					iSelectionEndColumn;

	size_t					iSelectionGrabLine;
	size_t					iSelectionGrabColumn;
	bool					bSelectionGrabbed;

	// Window attributes
	CScrollbar				cScrollbar;
	bool					bUseScroll;

	// Methods
	void					Parse();
	void					ParseTag(std::string::const_iterator& it, std::string::const_iterator& last, std::string& cur_line);
	void					RenderContent(SDL_Surface * bmpDest);
	int						TextW(const std::string& text, FontFormat& fmt);
	void					RenderText(SDL_Surface *bmpDest, FontFormat& fmt, int& curX, int& curY, int maxX, const std::string& text);
	void					TraverseNodes(xmlNodePtr node);
	void					BrowseChildren(xmlNodePtr node);
	bool					InSelection(size_t line, size_t column);
	void					EndLine();


public:
	// Methods


	void	Create(void);
	void	Destroy(void);

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse);
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse);
	int		MouseWheelUp(mouse_t *tMouse);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate) { return BRW_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2) { return 0; }
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	Draw(SDL_Surface * bmpDest);
	void	LoadStyle(void) {}

	void	MousePosToCursorPos(int ms_x, int ms_y, size_t& cur_x, size_t& cur_y);
	void	CursorPosToMousePos(size_t cur_x, size_t cur_y, int& ms_x, int& ms_y);

	void	LoadFromHTTP(const std::string& url);
	void	LoadFromFile(const std::string& file);
	void	LoadFromString(const std::string& data);
	void	AppendData(const std::string& data);
	void	ProcessHTTP();
	
	// Chatbox routines
	void	InitializeChatbox();	// Clears content and creates simple HTML header
	void	AddChatBoxLine(const std::string & text, Color color, bool bold = false, bool underline = false);
	std::string GetChatBoxText();	// Returns chatbox text in HTML form
};

}; // namespace DeprecatedGUI

#endif  //  __CBROWSER_H__DEPRECATED_GUI__
