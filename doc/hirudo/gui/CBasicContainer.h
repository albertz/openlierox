// Basic container class
// This class is the preceder of all GUI classes

#include <string>
#include <list>
#include "Utils.h"
#include "CSurface.h"

class CBasicContainer;
typedef std::list<CBasicContainer *> ContainerList;

class EffectList {};
class CButton { public: void Draw(CSurface &dest) {} };
class CBorder { public: void Draw(CSurface &dest) {} };

class MouseButtons  { public:
	MouseButtons() : Left(false), Middle(false), Right(false) {}
	bool	Left;
	bool	Middle;
	bool	Right;
};

class KeyState  { public:
	KeyState() : Shift(false), Ctrl(false), Alt(false) {}
	bool	Shift;
	bool	Ctrl;
	bool	Alt;
};

// TODO: remove these after putting them in OLX's code
typedef int UnicodeChar;
typedef unsigned long DWORD;

class CBasicContainer  {
public:
	// Constructor
	CBasicContainer() : 
		tParent(NULL),
		tFocusedChild(NULL),
		bModalRunning(false),
		bRepaintRequired(true),
		bFocused(false),
		bEnabled(false),
		bVisible(false),
		bFreeAutomatically(false),
		bWasMouseOver(false),
		iX(0),
		iY(0),
		iWidth(0),
		iHeight(0),
		OnClick(NULL),
		OnMouseDown(NULL),
		OnMouseOver(NULL),
		OnMouseOut(NULL),
		OnMouseMove(NULL),
		OnKeyDown(NULL),
		OnKeyUp(NULL),
		OnCreate(NULL),
		OnDestroy(NULL)
		{}

	~CBasicContainer();

protected:
	CSurface		tBuffer;

	CBasicContainer	*tParent;
	ContainerList	tChildren;
	CBasicContainer	*tFocusedChild;

	bool			bModalRunning;
	bool			bRepaintRequired;
	bool			bWasMouseOver;

	bool			bFocused;
	bool			bEnabled;
	bool			bVisible;
	bool			bFreeAutomatically;
	std::string		sID;

	int				iX;
	int				iY;
	int				iWidth;
	int				iHeight;

protected:
	// Methods
	ContainerList::iterator FindChild(const std::string& id);
	void			DrawChildren(CSurface &dest);
	bool			ChildrenNeedRepaint();
	virtual void	Repaint() = 0;

public:
	// Special effects
	EffectList		Effects;

	// Events
	void			(*OnClick) ( CBasicContainer *, int, int, MouseButtons, KeyState );
	void			(*OnMouseDown) ( CBasicContainer *, int, int, MouseButtons, KeyState );
	void			(*OnMouseOver) ( CBasicContainer * );
	void			(*OnMouseOut) ( CBasicContainer * );
	void			(*OnMouseMove) ( CBasicContainer *, int, int, MouseButtons, KeyState );
	void			(*OnMouseWheelUp) ( CBasicContainer *, int, int, MouseButtons, KeyState);
	void			(*OnMouseWheelDown) ( CBasicContainer *, int, int, MouseButtons, KeyState);
	void			(*OnKeyDown) ( CBasicContainer *, UnicodeChar cKey, KeyState );
	void			(*OnKeyUp) ( CBasicContainer *, UnicodeChar cKey, KeyState );
	void			(*OnPaint) ( CBasicContainer *, CSurface *);
	void			(*OnCreate) ( CBasicContainer * );
	void			(*OnDestroy) ( CBasicContainer * );

protected:
	// Events
	virtual void	DoMouseClick(int x, int y, MouseButtons buttons, KeyState keys);
	virtual void	DoMouseDown(int x, int y, MouseButtons buttons, KeyState keys);
	virtual void	DoMouseOver() {}
	virtual void	DoMouseOut() {}
	virtual void	DoMouseMove(int x, int y, MouseButtons buttons, KeyState keys);
	virtual void	DoMouseWheelUp(int x, int y, MouseButtons buttons, KeyState keys);
	virtual void	DoMouseWheelDown(int x, int y, MouseButtons buttons, KeyState keys);
	virtual void	DoKeyDown(UnicodeChar c, KeyState keys);
	virtual void	DoKeyUp(UnicodeChar c, KeyState keys);

	bool			ProcessMouseClick(int x, int y, MouseButtons buttons, KeyState keys);
	bool			ProcessMouseDown(int x, int y, MouseButtons buttons, KeyState keys);
	bool			ProcessMouseMove(int x, int y, MouseButtons buttons, KeyState keys);
	bool			ProcessMouseWheelUp(int x, int y, MouseButtons buttons, KeyState keys);
	bool			ProcessMouseWheelDown(int x, int y, MouseButtons buttons, KeyState keys);
	bool			ProcessKeyDown(UnicodeChar c, KeyState keys);
	bool			ProcessKeyUp(UnicodeChar c, KeyState keys);

public:
	// Properties
	void			setFocused(bool _f)			{ bFocused = _f; bRepaintRequired = true; }
	bool			getFocused()				{ return bFocused; }
	void			setEnabled(bool _e)			{ bEnabled = _e; bRepaintRequired = true; }
	bool			getEnabled()				{ return bEnabled; }
	void			setVisible(bool _v)			{ bVisible = _v;}
	bool			getVisible()				{ return bVisible; }
	void			setAutoFree(bool _f)		{ bFreeAutomatically = _f; }
	bool			getAutoFree()				{ return bFreeAutomatically; }
	bool			getModalRunning()			{ return bModalRunning; }

	void			setX(int _x)		 { iX = _x; if (tParent) tParent->bRepaintRequired = true; }
	int				getX()				 { return iX; }
	void			setY(int _y)		 { iY = _y; if (tParent) tParent->bRepaintRequired = true; }
	int				getY()				 { return iY; }
	void			setWidth(int _w)	 { iWidth = _w; bRepaintRequired = true; if (tParent) tParent->bRepaintRequired = true; }
	int				getWidth()			 { return iWidth; }
	void			setHeight(int _h)	 { iHeight = _h; bRepaintRequired = true; if (tParent) tParent->bRepaintRequired = true; }
	int				getHeight()			 { return iHeight; }

	// Methods
	bool			InBox(int x, int y)  { return x >= iX && x <= (iX + iWidth) && y >= iY && y <= (iY + iHeight); }
	void			AddChild(CBasicContainer *child);
	CBasicContainer *GetChild(const std::string& id);
	void			RemoveChild(const std::string& id);

	virtual void	Setup(int x, int y, int w, int h) = 0;
	virtual void	Draw(CSurface &dest) = 0;
};
