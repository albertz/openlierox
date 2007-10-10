// Dialog class

#include "CBasicContainer.h"

class CDialog : public CBasicContainer  {
public:
	CDialog() : 
		bGrabbed(false),
		bTitlebarVisible(true),
		iTitlebarHeight(20),
		iBgColor(0),
		iTitleBgColor(0),
		iTitleTextColor(0)
		{ 
			AddChild((CBasicContainer *)&cCloseButton);
		}

private:
	bool			bGrabbed;

protected:
	void			Repaint();

private:
	// Events
	void			DoMouseClick(int x, int y, MouseButtons buttons, KeyState keys);
	void			DoMouseDown(int x, int y, MouseButtons buttons, KeyState keys);
	void			DoMouseMove(int x, int y, MouseButtons buttons, KeyState keys);

private:
	// Settings
	bool			bTitlebarVisible;
	int				iTitlebarHeight;

	// Images
	CSurface		bmpBackground;
	CSurface		bmpTitleLeft;
	CSurface		bmpTitleMain;
	CSurface		bmpTitleRight;

	// Colors
	Color			iBgColor;
	Color			iTitleBgColor;
	Color			iTitleTextColor;

	// Close button
	CButton			cCloseButton;

	// Border
	CBorder			cBorder;

public:
	void			(*OnClose) (CDialog *, bool &);

public:
	// Properties
	void			setTitlebarVisible(bool _v)  { bTitlebarVisible = _v; bRepaintRequired = true; }
	bool			getTitlebarVisible()		 { return bTitlebarVisible; }
	void			setTitlebarHeight(bool _h)	 { iTitlebarHeight = _h; bRepaintRequired = true; }
	int				getTitlebarHeight()			 { return iTitlebarHeight; }	
	void			setBackground(CSurface& _b)	 { bmpBackground = _b; bRepaintRequired = true; }
	CSurface&		getBackground()				 { return bmpBackground; }	
	void			setTitleLeft(CSurface& _t)	 { bmpTitleLeft = _t; bRepaintRequired = true; iTitlebarHeight = MAX(iTitlebarHeight, _t.getHeight()); }
	CSurface&		getTitleLeft()				 { return bmpTitleLeft; }	
	void			setTitleMain(CSurface& _t)	 { bmpTitleMain = _t; bRepaintRequired = true; iTitlebarHeight = MAX(iTitlebarHeight, _t.getHeight()); }
	CSurface&		getTitleMain()				 { return bmpTitleMain; }	
	void			setTitleRight(CSurface& _t)	 { bmpTitleRight = _t; bRepaintRequired = true; iTitlebarHeight = MAX(iTitlebarHeight, _t.getHeight()); }
	CSurface&		getTitleRight()				 { return bmpTitleRight; }
	void			setBgColor(Color _c)		 { iBgColor = _c; bRepaintRequired = true;}
	Color			getBgColor()				 { return iBgColor; }	
	void			setTitleBgColor(Color _c)	 { iTitleBgColor = _c; bRepaintRequired = true;}
	Color			getTitleBgColor()			 { return iTitleBgColor; }	
	void			setTitleTextColor(Color _c)	 { iTitleTextColor = _c; bRepaintRequired = true;}
	Color			getTitleTextColor()			 { return iTitleTextColor; }	
	CButton&		getCloseButton()			 { return cCloseButton; }
	CBorder&		getBorder()					 { return cBorder; }

	// Methods
	void			Draw(CSurface &dest);
	void			Setup(int x, int y, int w, int h);
};