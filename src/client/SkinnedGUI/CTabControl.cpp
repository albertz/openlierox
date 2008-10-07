/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Tab control
// Created 17/4/08
// Karel Petránek

#include "SkinnedGUI/CTabControl.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "XMLutils.h"
#include "debug.h"


namespace SkinnedGUI {

///////////////////
// Create
CTabControl::CTabControl(COMMON_PARAMS) : CGuiSkinnedLayout(name, parent)
{
	iSelected = -1;
	iTabsHeight = 0;
}

///////////////////
// Add a blank page to the tab control
CGuiSkinnedLayout *CTabControl::AddBlankPage(CButton *tab)
{
	// Move the button to this tab control
	tab->setParent(this);

	// Update the tabs height
	if (iTabsHeight < tab->getHeight())
		iTabsHeight = tab->getHeight();

	// Create a new layout
	CGuiSkinnedLayout *newpage = new CGuiSkinnedLayout(sName + "_page_" + itoa(cPages.size()), this);
	cPages.push_back(newpage);

	// The tabs height could changes, update the page sizes
	UpdateSizes();

	// Add the tab
	AddTab(tab);

	// If this is the first tab, make it selected
	if (cTabs.size() == 1)
		setSelectedIndex(0);

	Repaint();

	return newpage;
}

////////////////////
// Add a tab, internal function
void CTabControl::AddTab(CTab *tab)
{
	// Add the tab to the list
	cTabs.push_back(tab);

	// Get the sum of all tabs width
	int tabs_width = 0;
	for (std::vector<CTab *>::iterator it = cTabs.begin(); it != cTabs.end(); it++)  {
		tabs_width += (*it)->getWidth();
	}

	// Get spacing between the tabs
	int spacing = (getWidth() - tabs_width) / (cTabs.size());

	// Update the positions
	int x = cBorder.getLeftW();
	for (std::vector<CTab *>::iterator it = cTabs.begin(); it != cTabs.end(); it++)  {
		(*it)->Resize(x, cBorder.getTopW(), (*it)->getWidth(), (*it)->getHeight());
		x += (*it)->getWidth() + spacing;
	}
}

/////////////////////
// Update the page sizes, internal function
void CTabControl::UpdateSizes()
{
	for (std::vector<CGuiSkinnedLayout *>::iterator it = cPages.begin(); it != cPages.end(); it++)
		(*it)->Resize(cBorder.getLeftW(), cBorder.getTopW() + iTabsHeight,
			getWidth() - cBorder.getLeftW() - cBorder.getRightW(),
			getHeight() - iTabsHeight - cBorder.getTopW() - cBorder.getBottomW());
}

////////////////////
// Add a page to the tab control
void CTabControl::AddPage(CGuiSkinnedLayout *page, CTab *tab)
{
	// Move the page to this tab control
	page->setParent(this);

	// Update the tabs height
	if (iTabsHeight < tab->getHeight())
		iTabsHeight = tab->getHeight();

	// Add the page
	cPages.push_back(page);

	// The tabs height could change, resize all the layouts accordingly
	UpdateSizes();

	// Add the tab
	AddTab(tab);

	// If this is the first tab, make it selected
	if (cTabs.size() == 1)
		setSelectedIndex(0);

	Repaint();
}

/////////////////////
// Remove a page from the tab control
void CTabControl::RemovePage(int index)
{
	assert(cPages.size() == cTabs.size());

	// Remove the page from the lists and from the layout
	if (index < 0 || index >= (int)cPages.size())  {
		removeWidget(*(cPages.begin() + index));
		removeWidget(*(cTabs.begin() + index));
		cPages.erase(cPages.begin() + index);
		cTabs.erase(cTabs.begin() + index);
	}

	// Update the selected index
	if (index == iSelected)
		iSelected = 0;
	if (cTabs.size() == 0)
		iSelected = -1;

	Repaint();
}

//////////////////////
// Get a page based on the index
CGuiSkinnedLayout *CTabControl::getPage(int index)
{
	if (index >= 0 && index < (int)cPages.size())
		return cPages[index];
	else
		return NULL;
}

///////////////////////
// Change the selected page
void CTabControl::setSelectedIndex(int _s)
{
	if (_s < 0 || _s >= (int)cPages.size())
		return;

	// Unselect the previous tab
	if (iSelected >= 0 && iSelected < (int)cTabs.size())
		cTabs[iSelected]->setActive(false);

	// Select the new tab
	cTabs[_s]->setActive(true);
	iSelected = _s;

	Repaint();
}

///////////////////
// Apply a tag to the tabcontrol
void CTabControl::ApplyTag(xmlNodePtr node)
{
	// Get the tabs
	xmlNodePtr child = node->children;
	while (child)  {
		if (!xmlStrcasecmp(child->name, (const xmlChar *)"tab"))  {
			std::string name = xmlGetString(child, "id", STATIC);
			CTab *tab = new CTab(name, this);
			if (cLayoutCSS)
				tab->ApplyCSS(*cLayoutCSS);
			tab->ApplyTag(child); // Apply the tag to the tab

			// Create & build the page
			CGuiSkinnedLayout *newpage = AddBlankPage(tab);
			if (newpage)  {
				newpage->ApplyTag(child);
				if (cLayoutCSS)
					newpage->ApplyCSS(*cLayoutCSS);
			}	
		}
	}

	if (cTabs.size() > 0)
		iSelected = CLAMP(xmlGetInt(node, "selected", iSelected), 0, (int)cTabs.size() - 1);

	// TODO: is this really needed?
	CGuiSkinnedLayout::ApplyTag(node);
}

}; // namespace SkinnedGUI
