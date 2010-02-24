/*
 *  List.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 23.02.10.
 *  code under LGPL
 *
 */

#include <boost/bind.hpp>
#include "gui/List.h"
#include "Debug.h"

void GuiListItem::setImage(const SmartPointer<DynDrawIntf>&) {
	warnings << "GuiListItem::setImage not implemented" << endl;
}

typedef boost::function< Iterator<GuiListItem::Pt>::Ref() > DynamicListFct;
GuiList::Pt dynamicGuiList(DynamicListFct f) {
	struct DynamicList : GuiList {
		DynamicListFct f;
		DynamicList(DynamicListFct _f) : f(_f) {}
		virtual Iterator<GuiListItem::Pt>::Ref iterator() { return f(); }
	};
	return new DynamicList(f);
}

static Iterator<GuiListItem::Pt>::Ref iteratorForGuiList(std::list<GuiListItem::Pt> l) {
	typedef GuiListItem::Pt I;
	return new STLIterator<std::list<I>,I>(l);
}

GuiList::Pt dynamicGuiList(boost::function< std::list<GuiListItem::Pt>() > f) {
	return dynamicGuiList((DynamicListFct) boost::bind(&iteratorForGuiList, boost::bind(f)));
}
