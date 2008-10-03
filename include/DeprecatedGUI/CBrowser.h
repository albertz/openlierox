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
#include "LieroX.h"
#include "Timer.h"
#include "Protocol.h"


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
	  fLastMouseScroll(0),
	  curX(0),
	  curY(0),
	  tDestSurface(NULL),
	  iCursorColumn(0),
	  iCursorLine(0),
	  iSelectionStartLine(0),
	  iSelectionStartColumn(0),
	  iSelectionEndLine(0),
	  iSelectionEndColumn(0),
	  bSelectionGrabbed(false),
	  bInLink(false),
	  bmpBuffer(NULL),
	  bDrawCursor(true),
	  bNeedsRender(false)
	  {
	  	tCurrentFormat.bold = false;
	  	tCurrentFormat.underline = false;
	  	tCurrentFormat.color = Color(0,0,0);
	  	tBgColor = Color(255,255,255);
	  }

public:

	// Handler for the active (interacting) areas of the browser (for example links)
	class CActiveArea  {
	public:
		typedef void(CBrowser::*ActiveAreaHandler)(CActiveArea *);
	public:
		CActiveArea(CBrowser *parent, const SDL_Rect& rect, ActiveAreaHandler click_func, ActiveAreaHandler move_func,
			const std::string s_data = "", int i_data = 0) :
		  tRect(rect),
		  sData(s_data),
		  iData(i_data),
		  tClickFunc(click_func),
		  tMouseMoveFunc(move_func),
		  tParent(parent)
		  {}

		CActiveArea() :
		  iData(0),
		  tClickFunc(NULL),
		  tMouseMoveFunc(NULL),
		  tParent(NULL)
		  { tRect = MakeRect(0, 0, 0, 0); }

		CActiveArea(const CActiveArea& oth)  { operator=(oth); }

	private:
		friend class CBrowser;
		SDL_Rect	tRect;
		std::string	sData;
		int			iData;
		ActiveAreaHandler tClickFunc;
		ActiveAreaHandler tMouseMoveFunc;
		CBrowser	*tParent;

	public:
		bool	InBox(int x, int y) const  { return x >= tRect.x && x < (tRect.x + tRect.w) 
			&& y >= tRect.y && y < (tRect.y + tRect.h); }
		void	DoMouseMove(int x, int y);
		void	DoMouseDown(int x, int y) { /* TODO */ }
		void	DoClick(int x, int y);

		const std::string& getStringData()	{ return sData; }
		int	getIntData()					{ return iData; }
		CBrowser *getParent()				{ return tParent; }

		void Clear()  { tRect = MakeRect(0, 0, 0, 0); sData.clear();
						iData = 0; tClickFunc = NULL; tMouseMoveFunc = NULL; tParent = NULL; }

		CActiveArea& operator=(const CActiveArea& oth)  {
			if (&oth != this)  {
				tRect = oth.tRect;
				sData = oth.sData;
				iData = oth.iData;
				tClickFunc = oth.tClickFunc;
				tMouseMoveFunc = oth.tMouseMoveFunc;
				tParent = oth.tParent;
			}
			return *this;
		}

	};

private:
	// Attributes
	CHttp					cHttp;
	std::string				tData;
	std::string				sURL;
	std::string				sHostName;
	bool					bFinished;
	int						iClientWidth;
	int						iClientHeight;
	htmlDocPtr				tHtmlDocument;
	htmlNodePtr				tRootNode;
	std::vector<std::string>	tPureText;
	std::list<CActiveArea>	tActiveAreas;
	float					fLastMouseScroll;

	// Parsing temps
	FontFormat				tCurrentFormat;
	std::stack<FontFormat>	tFormatStack;
	Color					tBgColor;
	int						curX;
	int						curY;
	std::string				sCurLine;
	SDL_Surface				*tDestSurface;

	// Selection & caret
	size_t					iCursorColumn;
	size_t					iCursorLine;
	size_t					iSelectionStartLine;
	size_t					iSelectionStartColumn;
	size_t					iSelectionEndLine;
	size_t					iSelectionEndColumn;

	size_t					iSelectionGrabLine;
	size_t					iSelectionGrabColumn;
	bool					bSelectionGrabbed;
	bool					bDrawCursor;

	// Links
	bool					bInLink;
	CActiveArea				cCurrentLink;

	// Window attributes
	CScrollbar				cScrollbar;
	bool					bUseScroll;

	// Other
	SmartPointer<SDL_Surface>	bmpBuffer;
	bool					bNeedsRender;

	// Methods
	void					Parse();
	void					ParseTag(std::string::const_iterator& it, std::string::const_iterator& last, std::string& cur_line);
	void					RenderContent();
	int						TextW(const std::string& text, FontFormat& fmt);
	void					RenderText(SDL_Surface *bmpDest, FontFormat& fmt, int& curX, int& curY, int maxX, const std::string& text);
	void					TraverseNodes(xmlNodePtr node);
	void					BrowseChildren(xmlNodePtr node);
	bool					InSelection(size_t line, size_t column);
	std::string				GetSelectedText();
	bool					IsSelectionEmpty();
	void					ClearSelection();
	void					SwapSelectionEnds();
	void					EndLine();
	void					AdjustScrollbar(bool mouse = false);
	void					ReRender();
	void					DrawCursor(SDL_Surface *bmpDest);
	void					ResetScrollbar();
	
	
	// Link helper functions
	void					StartLink(const std::string& url);
	void					EndLink();
	std::string				GetFullURL(const std::string& url);
	void					LinkClickHandler(CActiveArea *area);
	void					LinkMouseMoveHandler(CActiveArea *area);


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

	// TODO: create a descendant chatbox class and move the chatbox things there, they do not belong to browser
	void	InitializeChatBox( const std::string & initText = "" );
	void	AddChatBoxLine(const std::string & text, Color color, TXT_TYPE textType, bool bold = false, bool underline = false);
	std::string GetChatBoxText();
	void	CleanUpChatBox( const std::vector<TXT_TYPE> & removedText, int maxLineCount );
	void	ScrollToLastLine(void);
	inline bool	NeedsRepaint()  { return bNeedsRender; }


	// TODO: add function for getting individual text lines & their color

};

}; // namespace DeprecatedGUI

#endif  //  __CBROWSER_H__DEPRECATED_GUI__
