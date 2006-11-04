/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Custom browser class
// Created 7/6/02
// Jason Boettcher


#ifndef __CWIDGETLIST_H__
#define __CWIDGETLIST_H__


// Widget list item structure
typedef struct widget_item_s {
	int				iID;
	char			*sName;
	widget_item_s	*tNext;
} widget_item_t;



// Widget list class
class CWidgetList {
public:
	CWidgetList() {
		tItems = NULL;
		iCount = 0;
	}

private:
	// Attributes
	widget_item_t	*tItems;
	int				iCount;

public:
	// Methods
	int		getCount(void)	{return iCount; }
	int		Add(char *Name);
	char	*getName(int ID);
	int		getID(const char *Name);
	void	Shutdown(void);
};

#endif  //  __CWIDGETLIST_H__
