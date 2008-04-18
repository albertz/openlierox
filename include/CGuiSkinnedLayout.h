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

#include "CWidget.h"
#include "CGuiLayout.h"
#include "CWidgetList.h"
#include <string>
#include <map>

// Almost exact copy of CGuiLayout but without references to global "LayoutWidgets" var and without global ID
class CGuiSkinnedLayout: public CWidget, public CGuiLayoutBase
{
public:
	// Constructor
	CGuiSkinnedLayout( int x = 0, int y = 0 ) {
		cFocused = NULL;
		bFocusSticked = false;
		//cMouseOverWidget = NULL;
		//iCanFocus = true;
		iOffsetX = x;
		iOffsetY = y;
		iType = wid_GuiLayout;
		bExitCurrentDialog = false;
		setParent( NULL );
		cChildLayout = NULL;
	}

	// Destructor
	~CGuiSkinnedLayout() { Shutdown(); }

private:
	// Attributes

	std::list<CWidget *>	cWidgets;
	CWidget			*cFocused;
	bool			bFocusSticked;	// if user hold mouse button over some widget the focus sticks to that widget, analog to CGuiLayout::iCanFocus which is never used

	bool		bExitCurrentDialog;	// Used to exit to MainMenu - remove when only skinned GUI will exist
	CGuiSkinnedLayout	*cChildLayout;
	bool		bChildLayoutFullscreen;	// Do not redraw itself if child is fullscreen

	CWidgetList	LayoutWidgets;
	int			iOffsetX, iOffsetY;	// Top-left corner of layout (just offset, may be negative)

	void		FocusOnKeyPress(UnicodeChar c, int keysym, bool keyup);
	void 		FocusOnMouseClick( CWidget * w );

public:
	// Methods

	void		Add(CWidget *widget, int id, int x, int y, int w, int h);
	CWidget		*getWidget(int id);
	CWidget		*getWidgetAtPoint(int x, int y);
	CWidget		*getWidget(const std::string & name){ return getWidget(GetIdByName(name)); };
    void        removeWidget(int id);
	int			GetIdByName(const std::string & name);
	void		Error(int ErrorCode, const std::string& desc);
	void		SetOffset( int x, int y );

	bool		Process(void);	// Called only for main layout -dispatches messages to children, returns false on exit layout
	void		Draw(SDL_Surface * bmpDest);

	void		Shutdown(void);

	void	Create(void) { };
	void	Destroy(void) { Shutdown(); }

	// CWidget functions
	// These event handlers will route events to children event handlers
	int		MouseOver(mouse_t *tMouse);
	int		MouseUp(mouse_t *tMouse, int nDown);
	int		MouseDown(mouse_t *tMouse, int nDown);
	int		MouseWheelDown(mouse_t *tMouse);
	int		MouseWheelUp(mouse_t *tMouse);
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate);

	// Other functions are empty, because GUI layout don't do any actions itself
	DWORD	SendMessage(int iMsg, DWORD Param1, DWORD Param2) { return 0; };
	DWORD	SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; };
	DWORD	SendMessage(int iMsg, std::string *sStr, DWORD Param) { return 0; };

	void	LoadStyle(void) { };
	
	void	ProcessGuiSkinEvent(int iEvent);
	
	static CWidget * WidgetCreator( const std::vector< CScriptableVars::ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy );

	static void ExitDialog( const std::string & param, CWidget * source );
	static void ChildDialog( const std::string & param, CWidget * source );
	static void SetTab( const std::string & param, CWidget * source );
};

#endif
