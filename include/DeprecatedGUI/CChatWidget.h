/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#ifndef __CCHATWIDGET_H__DEPRECATED_GUI__
#define __CCHATWIDGET_H__DEPRECATED_GUI__

#include "DeprecatedGUI/CGuiSkinnedLayout.h"
#include "DeprecatedGUI/CBrowser.h"
#include "DeprecatedGUI/CTextbox.h"
#include "DeprecatedGUI/CListview.h"

namespace DeprecatedGUI {

// Almost exact copy of CGuiLayout but without references to global "LayoutWidgets" var and without global ID
class CChatWidget: public CGuiSkinnedLayout
{
public:
	// Constructor
	CChatWidget();

	// Destructor
	~CChatWidget();

	// Methods
	void	Create();
	void	Destroy();

	void	ProcessChildEvent(int iEvent, CWidget * child);
	void	Draw(SDL_Surface *bmpDest);

	static void EnableChat();
	static void DisableChat();
	
	// Parent dialog should do "if(CChatWidget::GlobalEnabled()) CChatWidget::GlobalShow();" each frame
	// instead of calling Process() on your own GUI layout, you should only call Draw() before that
	static bool GlobalEnabled();
	static void GlobalSetEnabled(bool enabled = true);
	// Show standalone chat window - it will draw some background, so it won't be transparent
	static void GlobalProcessAndDraw(SDL_Surface * bmpDest);
	static void GlobalDestroy();
};

} // namespace DeprecatedGUI

#endif
