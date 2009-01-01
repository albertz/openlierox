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


#ifndef __CWIDGETLIST_H__DEPRECATED_GUI__
#define __CWIDGETLIST_H__DEPRECATED_GUI__

// TODO: remove this and substitute with std::list

namespace DeprecatedGUI {

// Widget list item structure
class widget_item_t { public:
	int				iID;
	std::string		sName;
	widget_item_t	*tNext;
};



// Widget list class
class CWidgetList {
public:
	CWidgetList() : tItems(NULL), iCount(0) {}

private:
	// Attributes
	widget_item_t	*tItems;
	int				iCount;

public:
	// Methods
	int		getCount(void)	{return iCount; }
	int		Add(const std::string& Name);
	std::string	getName(int ID);
	int		getID(const std::string& Name);
	void	Shutdown(void);
};

} // namespace DeprecatedGUI

#endif  //  __CWIDGETLIST_H__DEPRECATED_GUI__
