// Sheet for tab control

#define TAB_SPACING 10

class CSheet : public CBasicContainer  {
public:
	CSheet() :
		iTabHeight(20),
		iTabX(0),
		iIndex(0)	{}

	CSheet(int tabheight, int w, int h, int tabx, const std::string& title) :
		iTabHeight(tabheight),
		iWidth(w),
		iHeight(h),
		sTitle(title),
		iTabX(tabx)
		{
			iTabWidth = cFont->GetWidth(title) + TAB_SPACING;
			cPanel.Setup(0, tabheight, w, h - tabheight);
			RecalculateTabWidth();
		}

protected:
	void			Repaint();

private:
	void			RecalculateTabWidth();

private:
	CSurface		bmpActiveLeft;
	CSurface		bmpActiveMain;
	CSurface		bmpActiveRight;

	CSurface		bmpInactiveLeft;
	CSurface		bmpInactiveMain;
	CSurface		bmpInactiveRight;

	std::string		sTitle;
	int				iTabHeight;
	int				iTabX;
	int				iIndex;
	int				iTabWidth;
	CPanel			cPanel;

public:
	void			setTitle(const std::string& _t)		{ sTitle = _t; RecalculateTabWidth(); }
	std::string&	getTitle()							{ return sTitle; }
	void			setTabHeight(int _h)				{ iTabHeight = _h; }
	int				getTabHeight()						{ return iTabHeight; }

	void			Draw(CSurface &dest);
	void			DrawTab(CSurface &dest);
	void			Setup(int x, int y, int w, int h);
};


// Tab control class

class CTabs : public CBasicContainer  {
public:
	CTabs() : 
		iTabHeight(20),
		iPage(0)
		{}

protected:
	void	Repaint();

private:
	int		iTabHeight;
	int		iPage;

public:
	void	setTabHeight(int _h)	{ iTabHeight = _h; bRepaintRequired = true; }
	int		getTabHeight()			{ return iTabHeight; }
	void	AddSheet(CSheet *sheet);
	CSheet&	getSheet(int index);
	void	setActiveTab(int _p);
	int		getActiveTab()			{ return iPage; }

	void	Draw(CSurface &dest);
	void	Setup(int x, int y, int w, int h);
};


