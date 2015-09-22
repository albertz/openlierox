/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#ifndef __CGUISKINNEDLAYOUT_H__SKINNED_GUI__
#define __CGUISKINNEDLAYOUT_H__SKINNED_GUI__

#include <string>
#include <list>
#include "SkinnedGUI/CWidget.h"
#include "SkinnedGUI/CBackground.h"
#include "SkinnedGUI/CBorder.h"


namespace SkinnedGUI {

class CGuiSkin;

// Almost exact copy of CGuiLayout but without references to global "LayoutWidgets" var and without global ID
class CGuiSkinnedLayout: public CContainerWidget
{
public:
	// Constructors
	CGuiSkinnedLayout();

	CGuiSkinnedLayout(COMMON_PARAMS);

	// Destructor
	virtual ~CGuiSkinnedLayout();

private:
	// Attributes
	
	std::list<CWidget *>	cWidgets;
	
	CWidget			*cFocused;
	int				iModalsRunning; // Number of modal widgets active

	bool		bFullRepaint;

	void		FocusOnKeyPress(UnicodeChar c, int keysym, bool keyup);
	void 		FocusOnMouseClick( CWidget * w );

	void		Error(int ErrorCode, const std::string& desc);

	bool		ProcessSpecialTag(xmlNodePtr node, CGuiSkin& skin);

	void		FocusWidget(CWidget *w);

protected:
	int			iClientWidth;
	int			iClientHeight;
	int			iClientX;
	int			iClientY;

	CBackground	cBackground;
	SDL_Rect	tBgRect; // The rectangle to which the background will be drawn
	CBorder		cBorder;
	SDL_Rect	tBorderRect; // The rectangle to which the border will be drawn

	CSSParser	cCSS;
	CGuiSkin	*cSkin;	// Pointer to the current skin - can be NULL!

	std::list<SDL_Rect>	tRectsToRepaint; // List of areas we should repaint next frame

	virtual void RecalculateClientRect();

	// CWidget functions
	// These event handlers will route events to children event handlers
	friend class CGuiSkin;
	virtual	int DoMouseEnter(int x, int y, int dx, int dy, const ModifiersState& modstate);
	virtual	int DoMouseLeave(int x, int y, int dx, int dy, const ModifiersState& modstate);
	virtual	int DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	virtual	int DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	virtual	int DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	virtual	int DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate);
	virtual	int DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate);
	virtual int	DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	virtual int	DoKeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate);
	virtual void DoRepaint();
	virtual int DoCreate();
	virtual int DoChildResize(CWidget *child, const SDL_Rect& oldrect, const SDL_Rect& newrect);
	virtual int DoChildNeedsRepaint(CWidget *child);
	virtual int DoChildDestroyed(CWidget *child);

	// Self-reminders (for events that have to be processed "next frame")
	void		DoChildAddEvent(CWidget *child);
	void		DoChildDestroyEvent(CWidget *child);

	// Does the children repainting (to the internal buffer)
	virtual void DoRepaintChildren(); // TODO: why would somebody want to overload this (why virtual?)
	void DoRepaintRect(const SDL_Rect& r);

	void	Add(CWidget *widget);
	void	DeregisterWidget(CWidget *w);

public:
	// Methods

	CWidget		*getWidgetAtPoint(int x, int y);
	CWidget		*getWidgetByName(const std::string & name);
    void		removeWidget(const std::string& name);
	void		removeWidget(CWidget *widget);
	virtual void	MoveBy( int dx, int dy );
	virtual void	MoveTo( int x, int y );
	virtual void	Resize( int x, int y, int w, int h);

	SDL_Rect	getClientRect()  { SDL_Rect r = { (Sint16)iClientX, (Sint16)iClientY, (Uint16)iClientWidth, (Uint16)iClientHeight }; return r; }

	void		incModalsRunning();
	void		decModalsRunning();

	void		Process();
	virtual void Draw(SDL_Surface *bmpDest, int drawX, int drawY);
	bool		Load(const std::string& file, CGuiSkin& skin);

	static const std::string tagName()		{ return "layout"; }
	virtual const std::string getTagName()	{ return CGuiSkinnedLayout::tagName(); }

	virtual void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	virtual void	ApplyCSS(CSSParser& css);
	virtual void	ApplyTag(xmlNodePtr node);

	CGuiSkin	*getSkin()			{ return cSkin; }
};

}; // namespace SkinnedGUI

#endif
