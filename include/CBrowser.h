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


#ifndef __CBROWSER_H__
#define __CBROWSER_H__


#include "InputEvents.h"


// Browser messages
enum {
	BRW_NONE = -1
};


// Browser class
class CBrowser : public CWidget {
public:

private:
	// Attributes
	CHttp					cHttp;
	std::list<std::string>	tLines;
	bool					bFinished;
	int						iClientWidth;
	int						iClientHeight;

	// Window attributes
	CScrollbar				cScrollbar;
	bool					bUseScroll;

	// Methods
	void					Parse();
	void					ParseTag(std::string::const_iterator& it, std::string::const_iterator& last, std::string& cur_line);
	void					RenderContent(const SmartPointer<SDL_Surface> & bmpDest);


public:
	// Methods


	void	Create(void);
	void	Destroy(void) {}

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse);
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return BRW_NONE; }
	int		MouseWheelDown(mouse_t *tMouse);
	int		MouseWheelUp(mouse_t *tMouse);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate) { return BRW_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2) { return 0; }
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	void	Draw(const SmartPointer<SDL_Surface> & bmpDest);
	void	LoadStyle(void) {}

	void	Load(const std::string& url);
	void	ProcessHTTP();
};


#endif  //  __CBROWSER_H__
