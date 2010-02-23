/*
 *  List.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 23.02.10.
 *  code under LGPL
 *
 */

/*
 This list is for GUI widgets. It differes from normal lists
 that it provides additional information (like tooltip).
 */

#ifndef __OLX_GUI_LIST_H__
#define __OLX_GUI_LIST_H__

#include <string>
#include <list>
#include <boost/function.hpp>

#include "Iterator.h"
#include "Ref.h"
#include "SmartPointer.h"
#include "DynDraw.h"

class GuiListItem {
public:
	typedef SmartPointer<GuiListItem> Pt;
	virtual ~GuiListItem() {}
	
	virtual std::string caption() = 0;
	virtual std::string tooltip() { return ""; } // empty string -> no tooltip
	virtual SmartPointer<DynDrawIntf> image() { return NULL; }
	
	virtual std::string index() { return ""; }
	virtual int tag() { return 0; }
	
	virtual void setImage(const SmartPointer<DynDrawIntf>&);
};

class GuiList {
public:
	typedef SmartPointer<GuiList> Pt;
	virtual ~GuiList() {}
	virtual Iterator<GuiListItem::Pt>::Ref iterator() = 0;
};

GuiList::Pt dynamicGuiList(boost::function< std::list<GuiListItem::Pt>() >);

#endif

