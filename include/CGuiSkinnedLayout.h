/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#ifndef __CGUISKINNEDLAYOUT_H__
#define __CGUISKINNEDLAYOUT_H__

#include "CGuiLayout.h"
#include "CWidgetList.h"
#include <string>
#include <map>

// Almost exact copy of CGuiLayout but without references to global "LayoutWidgets" var and without global ID
class CGuiSkinnedLayout
{
public:
	// Constructor
	CGuiSkinnedLayout( int x = 0, int y = 0 ) {
		tEvent = new gui_event_t;
		cFocused = NULL;
		cWidgets = NULL;
		cMouseOverWidget = NULL;
		iCanFocus = true;
		iOffsetX = x;
		iOffsetY = y;
	}

	// Destructor
	~CGuiSkinnedLayout() {
		if (tEvent)
			delete tEvent;
		tEvent = NULL;
	}


private:
	// Attributes

	CWidget		*cWidgets;
	gui_event_t	*tEvent;
	CWidget		*cFocused;
	CWidget		*cMouseOverWidget;

	// Mouse button repeats
	int			nMouseButtons;
	float		fMouseNext[3];

	// Can we set focus to another widget?
	int			iCanFocus;
	
	CWidgetList	LayoutWidgets;
	int			iOffsetX, iOffsetY;	// Top-left corner of layout (just offset, may be negative)

public:
	// Methods

	void		Initialize();

	void		Add(CWidget *widget, int id, int x, int y, int w, int h);
	CWidget		*getWidget(int id);
    void        removeWidget(int id);
	int			GetIdByName(const std::string & name);
	void		Error(int ErrorCode, const char *Format, ...);
	void		SetOffset( int x, int y );

	gui_event_t	*Process(void);
	void		Draw(SDL_Surface *bmpDest);

	void		Shutdown(void);

	// Messaging
	DWORD		SendMessage(int iControl, int iMsg, DWORD Param1, DWORD Param2);
	DWORD		SendMessage(int iControl, int iMsg, const std::string& sStr, DWORD Param);
	DWORD		SendMessage(int iControl, int iMsg, std::string *sStr, DWORD Param);

};

#endif
