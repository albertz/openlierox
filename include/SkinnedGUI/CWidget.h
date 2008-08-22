/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Widget class
// Created 30/5/02
// Jason Boettcher


#ifndef __CWIDGET_H__SKINNED_GUI__
#define __CWIDGET_H__SKINNED_GUI__

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "InputEvents.h"
#include "types.h"
#include "SkinnedGUI/CGuiSkin.h"
#include "SkinnedGUI/CWidgetEffect.h"
#include "CssParser.h"
#include "SmartPointer.h"

namespace SkinnedGUI {

class CWidget;

typedef void(CWidget::*KeyboardHandler)(CWidget *sender, UnicodeChar c, int keysym, const ModifiersState& modstate);
typedef void(CWidget::*MouseHandler)(CWidget *sender, int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
typedef void(CWidget::*MouseWheelHandler)(CWidget *sender, int x, int y, int dx, int dy, const ModifiersState& modstate);
typedef void(CWidget::*MouseEnterLeaveHandler)(CWidget *sender, int x, int y, int dx, int dy, const ModifiersState& modstate);
typedef void(CWidget::*MouseMoveHandler)(CWidget *sender, int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);

#define DECLARE_EVENT(name, handler) Delegate<handler> name
#define CLEAR_EVENT(name) name.Clear()
#define EVENT_SETGET(name, handler) virtual void set##name(const Delegate<handler>& _h)  { name = _h; } \
									Delegate<handler>& get##name() { return name; }
#define CALL_EVENT(name, params) if (name) ((name.object)->*(name.handler))params; else (void)0
#define SET_EVENT(widget, type, handler, func) (widget)->set##type(Delegate<handler>(this, (handler)(func)))

// Widget event set macros
#define SET_MOUSEENTER(widget, func)	SET_EVENT(widget, OnMouseEnter, MouseEnterLeaveHandler, func)
#define SET_MOUSELEAVE(widget, func)	SET_EVENT(widget, OnMouseLeave, MouseEnterLeaveHandler, func)
#define SET_MOUSEMOVE(widget, func)		SET_EVENT(widget, OnMouseMove, MouseMoveHandler, func)
#define SET_MOUSEUP(widget, func)		SET_EVENT(widget, OnMouseUp, MouseHandler, func)
#define SET_MOUSEDOWN(widget, func)		SET_EVENT(widget, OnMouseDown, MouseHandler, func)
#define SET_WHEELUP(widget, func)		SET_EVENT(widget, OnWheelUp, MouseHandler, func)
#define SET_WHEELDOWN(widget, func)		SET_EVENT(widget, OnWheelDown, MouseHandler, func)
#define SET_KEYDOWN(widget, func)		SET_EVENT(widget, OnKeyDown, KeyboardHandler, func)
#define SET_KEYUP(widget, func)			SET_EVENT(widget, OnKeyUp, KeyboardHandler, func)

class CWidget;

// Class for event handling
template<typename _EventHandler>
class Delegate  {
public:
	// Constructors
	Delegate() : object(NULL), handler(NULL) {}
	Delegate(CWidget *this_pointer, _EventHandler function) :
		object(this_pointer), handler(function) {}

	// Operators
	Delegate& operator=(const Delegate& d2)  {
		if (&d2 != this)  {
			object = d2.object;
			handler = d2.handler;
		}
		return *this;
	}

	operator bool() { return object != NULL && handler != NULL; }

	void Clear() { object = NULL; handler = NULL; }

	// Attributes
	CWidget *object;
	_EventHandler handler;
};


enum  {
	WID_NOT_PROCESSED = -1, // The event should be passed to other widgets
	WID_PROCESSED = 0  // The event was handled by this widget and other widgets should not get it
};

// The repaint message for SDL queue
enum { WGT_REPAINT = 1 };

// Widget types
enum WidgetType {
	wid_None=-1,
	wid_Button=0,
	wid_Label,
	wid_Listview,
	wid_Scrollbar,
	wid_Slider,
	wid_Textbox,
	wid_Titlebutton,
	wid_Imagebutton,
	wid_Togglebutton,
	wid_Textbutton,
	wid_Checkbox,
	wid_Inputbox,
	wid_Combobox,
	wid_Image,
	wid_Frame,
	wid_Animation,
	wid_GuiLayout,
	wid_Tab,
	wid_TabControl,
	wid_Minimap,
	wid_Viewport,
	wid_Skinbox,
	wid_Dialog,
	wid_MapEditor,
	wid_Marquee
};

// Name for widgets that don't have any events assigned (usually labels)
static const std::string STATIC = "Static";

// Common parameters for a widget constructor, used to save typing a bit
#define COMMON_PARAMS const std::string& name, CContainerWidget *parent
#define CALL_DEFAULT_CONSTRUCTOR CWidget(name, parent)


// This simple macro checks if bmpBuffer is not null and quits the function if it is
// Use this macro before any actions in DoRepaint!
#ifdef _MSC_VER
#define CHECK_BUFFER \
	if (bmpBuffer.get() == NULL)  {\
		printf("Warning: %s in %s:%i : bmpBuffer is NULL\n", __FUNCSIG__, __FILE__, __LINE__); \
		return; \
	} else (void)0
#else
#define CHECK_BUFFER \
	if (bmpBuffer.get() == NULL)  {\
		printf("Warning: %s in %s:%i : bmpBuffer is NULL\n", __FUNCTION__, __FILE__, __LINE__); \
		return; \
	} else (void)0
#endif

class CContainerWidget;

class CWidget {
public:
	// Constructor
	CWidget(COMMON_PARAMS);

    virtual ~CWidget() 
	{}

private:
	// Attributes

	// These are relative to parent's client area
	// Children: Always call Resize to change them!
	StyleVar<int>	iX, iY;
	StyleVar<int>	iWidth, iHeight;

protected:
	// The buffer used for repainting
	SmartPointer<SDL_Surface> bmpBuffer;

	// Size constraints
	int		iMinWidth;
	int		iMinHeight;

	bool	bFocused;
	WidgetType iType;
	std::string sName; // Identifier
	bool	bEnabled; // If false, the widget won't receive any keyboard/mouse events but will be visible
	bool	bModal; // If true, the parent widget won't process any keyboard/mouse events

	StyleVar<bool>	bVisible; // If false, the widget won't be visible nor it will receive any keyboad/mouse events
	StyleVar<int>	iOpacity;

	// Mainly for dialogs and menus, they don't necessarily have to be in parent's rectangle
	// Therefore they are not drawn on the buffer surface
	bool	bOverlap;

	CContainerWidget	*cParent; // The parent, NULL if this is the top-most widget (i.e. some GUI layout)
	int		iTag; // Reserved for user-defined data

	bool	bDestroying; // After calling DoDestroy from the parent, there can be some close effects in progress
	bool	bDestroyed; // True if all effects are processed and the widget can be destroyed
	bool	bCreated;  // After all info is loaded from the skin files
	bool	bNeedsRepaint; // True if the parent should call DoRepaint on us

	// Handy variables
	bool	bMouseOver; // True if the mouse cursor is over this widget
	bool	bMouseDown; // True if the mouse cursor is over this widget and the mouse is clicked

	// CSS things
	std::string			sCSSClass;
	CSSParser			*cLayoutCSS; // Holds a pointer to the CSS of the current layout

	// Effects
	std::list<CWidgetEffect *> tCreateEffects; // Active only when creating the widget
	std::list<CWidgetEffect *> tPersistentEffects; // Active whole widget's life (even when creating and destroying)
	std::list<CWidgetEffect *> tDestroyEffects;  // Active only when destroying the widget

	// Common events, the widget can make them public using setters and getters (EVENT_SETGET)
	DECLARE_EVENT(OnMouseEnter, MouseEnterLeaveHandler); EVENT_SETGET(OnMouseEnter, MouseEnterLeaveHandler)
	DECLARE_EVENT(OnMouseLeave, MouseEnterLeaveHandler); EVENT_SETGET(OnMouseLeave, MouseEnterLeaveHandler)
	DECLARE_EVENT(OnMouseMove, MouseMoveHandler); EVENT_SETGET(OnMouseMove, MouseMoveHandler)
	DECLARE_EVENT(OnMouseUp, MouseHandler); EVENT_SETGET(OnMouseUp, MouseHandler)
	DECLARE_EVENT(OnMouseDown, MouseHandler); EVENT_SETGET(OnMouseDown, MouseHandler)
	DECLARE_EVENT(OnWheelUp, MouseWheelHandler); EVENT_SETGET(OnWheelUp, MouseWheelHandler)
	DECLARE_EVENT(OnWheelDown, MouseWheelHandler); EVENT_SETGET(OnWheelDown, MouseWheelHandler)
	DECLARE_EVENT(OnKeyDown, KeyboardHandler); EVENT_SETGET(OnKeyDown, KeyboardHandler)
	DECLARE_EVENT(OnKeyUp, KeyboardHandler); EVENT_SETGET(OnKeyUp, KeyboardHandler)

protected:

	// Relative InBox - returns true if the given *relative* coordinates are in the widget's rect
	bool			RelInBox(int x, int y);

	void			ReallocBuffer(int w, int h);

	// These functions should return true when the widget changes its appearance when
	// the corresponding event returns (override them in your widget when necessary)
	virtual bool	changeOnActivate()  { return false; }
	virtual bool	changeOnMouseDown()	{ return false; }

	// CSS & HTML
	CSSParser::Selector getCSSSelector();
	CSSParser::Selector::Context getMyContext();
	virtual const std::string getTagName() = 0;

public:
	// Widget functions
	virtual void	Resize(int x, int y, int w, int h);
	bool			InBox(int x, int y);

	// Repaint the widget
	void Repaint();

	// Destroy the widget
	void Destroy();

	// GUI building functions
	virtual void	ApplyCSS(CSSParser& css);
	virtual void	ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
	virtual void	ApplyTag(xmlNodePtr node);

	WidgetType		getType(void)					{ return iType; }

	SDL_Rect		getRect()						{ SDL_Rect r = { getX(), getY(), getWidth(), getHeight() }; return r; }

	const std::string& getName()					{ return sName; }
	void			setName(const std::string& _n)	{ sName = _n; }

	virtual void	setFocused(bool _f)				{ bFocused = _f; }
	bool			getFocused(void)				{ return bFocused; }

	bool			getEnabled(void)				{ return bEnabled; }
	virtual void	setEnabled(bool _e)				{ bEnabled = _e; Repaint(); }

	bool			getVisible()					{ return bVisible; }
	virtual void	setVisible(bool _v)				{ bVisible.set(_v, HIGHEST_PRIORITY); Repaint(); }

	int				getOpacity()					{ return iOpacity; }
	void			setOpacity(int _o)				{ iOpacity.set(_o, HIGHEST_PRIORITY); }

	int				getMinWidth()					{ return iMinWidth; }
	int				getMinHeight()					{ return iMinHeight; }

	bool			isModal()						{ return bModal; }
	bool			isOverlapping()					{ return bOverlap; }
	bool			isDestroyed()					{ return bDestroyed; }
	bool			isDestroying()					{ return bDestroying; }
	bool			isCreated()						{ return bCreated; }
	bool			isMouseOver()					{ return bMouseOver; }
	bool			isMouseDown()					{ return bMouseDown; }
	virtual bool	needsRepaint()					{ return bNeedsRepaint; }

	int				getX()							{ return iX; }
	int				getY()							{ return iY; }
	int				getWidth()						{ return iWidth; }
	int				getHeight()						{ return iHeight; }

	int				getTag()						{ return iTag; }
	void			setTag(int _t)					{ iTag = _t; }

	CContainerWidget *getParent(void)				{ return cParent; }
	void			setParent(CContainerWidget *l);

	const std::string& getCSSClass()				{ return sCSSClass; }

public:

	virtual	int		DoMouseEnter(int x, int y, int dx, int dy, const ModifiersState& modstate);
	virtual	int		DoMouseLeave(int x, int y, int dx, int dy, const ModifiersState& modstate);
	virtual	int		DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	virtual	int		DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	virtual	int		DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	virtual	int		DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate);
	virtual	int		DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate);
	virtual	int		DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	virtual	int		DoKeyUp(UnicodeChar c, int keysym,  const ModifiersState& modstate);
	virtual int		DoDestroy(bool immediate);
	virtual void	DoRepaint();
	virtual int		DoCreate();
	virtual int		DoParentResize(int& new_parent_w, int& new_parent_h);
	virtual int		DoFocus(CWidget *prev_focused);
	virtual int		DoLoseFocus(CWidget *new_focused);
	virtual void	DoEffectFinished(CWidgetEffect *e);

	virtual	void	Draw(SDL_Surface *bmpDest, int drawX, int drawY);

	// This function is called before Draw and allows the widget to perform
	// specific actions it needs for its functionality
	// For example CGuiSkinnedLayout removes widgets here and CViewport adjusts its position
	// according to the target
	virtual void	Process() {}
};

// Base class for all widgets that contain another widgets
// For example CGuiSkinnedLayout, CListview, CCombobox, ...
class CContainerWidget : public CWidget
{
protected:
	friend class CWidget;
	CContainerWidget(COMMON_PARAMS) : CALL_DEFAULT_CONSTRUCTOR {}
	virtual void Add(CWidget *w) {} // Called by the widget's constructor/parent setter; widgets add themselves to the containers
	virtual void DeregisterWidget(CWidget *w) {} // Removes the widget from an internal list but doesn't destroy it, used when moving widget from one container to another

public:

	// Events
	virtual int DoChildResize(CWidget *child, const SDL_Rect& oldrect, const SDL_Rect& newrect) { return WID_PROCESSED; }
	virtual int DoChildNeedsRepaint(CWidget *child) { return WID_PROCESSED; }
	virtual int	DoChildDestroyed(CWidget *child)	{ return WID_PROCESSED; }
};

}; // namespace SkinnedGUI

#endif  //  __CWIDGET_H__SKINNED_GUI__
