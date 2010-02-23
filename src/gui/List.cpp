/*
 *  List.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 23.02.10.
 *  code under LGPL
 *
 */

#include "gui/List.h"
#include "Debug.h"

void GuiListItem::setImage(const SmartPointer<DynDrawIntf>&) {
	warnings << "GuiListItem::setImage not implemented" << endl;
}

GuiList::Pt dynamicGuiList(boost::function< std::list<GuiListItem::Pt>() > f) {
	
}
