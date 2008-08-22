
#ifndef __CTABCONTROL_H__SKINNED_GUI__
#define __CTABCONTROL_H__SKINNED_GUI__

#include <vector>
#include "SkinnedGUI/CGuiSkinnedLayout.h"
#include "SkinnedGUI/CButton.h"


namespace SkinnedGUI {

// TODO: this better
typedef CButton CTab;

// Tab control class
class CTabControl : public CGuiSkinnedLayout  {
public:
	CTabControl(COMMON_PARAMS);

private:
	std::vector<CTab *> cTabs;
	std::vector<CGuiSkinnedLayout *> cPages;
	int iTabsHeight;
	int iSelected;

	void	AddTab(CTab *tab);
	void	UpdateSizes();

public:
	CGuiSkinnedLayout	*AddBlankPage(CTab *tab);
	void				AddPage(CGuiSkinnedLayout *page, CTab *tab);
	void				RemovePage(int index);
	CGuiSkinnedLayout	*getPage(int index);
	int					getSelectedIndex()			{ return iSelected; }
	void				setSelectedIndex(int _s);
	size_t				getPageCount()			{ return cTabs.size(); }

	static const std::string tagName()		{ return "tabcontrol"; }
	const std::string getTagName()			{ return CTabControl::tagName(); }

	void				ApplyTag(xmlNodePtr node);
};

}; // namespace SkinnedGUI

#endif // __CTABCONTROL_H__SKINNED_GUI__

