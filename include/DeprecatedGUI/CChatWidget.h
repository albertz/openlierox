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

	void	EnableChat();
	void	DisableChat();
	
	static CChatWidget * getInstance();
};

} // namespace DeprecatedGUI

#endif
