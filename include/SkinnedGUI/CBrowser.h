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


#ifndef __CBROWSER_H__SKINNED_GUI__
#define __CBROWSER_H__SKINNED_GUI__

#include "SkinnedGUI/CWidget.h"
#include "CScrollbar.h"


namespace SkinnedGUI {

// Browser class
class CBrowser : public CContainerWidget {
public:
	CBrowser(COMMON_PARAMS);

	~CBrowser()  {
		delete cScrollbar;
	}

private:
	// Attributes
	CHttp					cHttp;
	std::list<std::string>	tLines;
	bool					bFinished;
	int						iClientWidth;
	int						iClientHeight;

	// Window attributes
	CScrollbar				*cScrollbar;
	bool					bUseScroll;

	// Methods
	void					Parse();
	void					ParseTag(std::string::const_iterator& it, std::string::const_iterator& last, std::string& cur_line);
	void					RenderContent();

	// Events
	int DoMouseMove(int x, int y, int dx, int dy, bool down, MouseButton button, const ModifiersState& modstate);
	int	DoMouseWheelDown(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int	DoMouseWheelUp(int x, int y, int dx, int dy, const ModifiersState& modstate);
	int	DoMouseDown(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int	DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate);
	int	DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);

	void DoRepaint();

public:
	// Methods

	void	LoadStyle() {}

	void	Load(const std::string& url);
	void	ProcessHTTP();
};

}; // namespace SkinnedGUI

#endif  //  __CBROWSER_H__SKINNED_GUI__
