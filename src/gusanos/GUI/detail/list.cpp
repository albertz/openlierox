#include "list.h"
#include <cassert>
#include "util/macros.h"
#include "luaapi/context.h"
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

namespace OmfgGUI
{

LuaReference List::metaTable;
LuaReference ListNode::metaTable;

bool List::LuaLT::operator()(ListNode* a, ListNode* b)
{
	if((context.call(comp, 1), a->luaReference, b->luaReference)() == 1)
	{
		bool v = lua_toboolean(context, -1);
		context.pop(1);
		return v;
	}
	return false;
}

void ListNode::resizeColumns(size_t s)
{
	columns.resize(s);
	
	foreach_bool(i, children)
	{
		i->resizeColumns(s);
	}
}

void ListNode::changeChildrenCount(long change)
{
	if(expanded)
	{
		if(parent)
			parent->changeChildrenCount(change);
		else
			list->m_visibleChildren += change;
	}
	visibleChildren += change;
}

void ListNode::render(Renderer* renderer, long& y)
{
	long halfRowHeight = List::rowHeight/2;
	
	if(selected || list->m_MainSel == node_iter_t(this))
	{
		Rect r(list->getRect().x1 + 1, y, list->getRect().x2 - 1, y + List::rowHeight - 1);
		if(selected)
		{
			renderer->drawBox(
				r,
				list->m_listFormatting.selectionColor);
		}
		
		if(list->m_MainSel == node_iter_t(this))
		{
			renderer->drawFrame(
				r,
				list->m_listFormatting.selectionFrameColor);
		}
		
		
	}

	//renderer->drawText(*list.m_font, text, BaseFont::CenterV, list.m_rect.x1 + 3 + level * 5, y + halfRowHeight, RGB(0, 0, 0));
	
	double x = list->getRect().x1 + list->m_listFormatting.indent + level * 5.0;
	double w = list->getRect().getWidth() / list->m_totalWidthFactor;
	
	assert(columns.size() == list->m_columnHeaders.size());
	
	std::vector<std::string>::const_iterator i = columns.begin();
	std::vector<List::ColumnHeader>::const_iterator h = list->m_columnHeaders.begin();
	
	for(;
		i != columns.end();
		++i, ++h)
	{
		renderer->drawText(*list->m_font, *i, BaseFont::CenterV, long(x), y + halfRowHeight, list->m_formatting.fontColor);
		x += h->widthFactor * w;
	}
	
	//renderChildren(aRenderer, y, list);
}
/*
void ListNode::renderChildren(Renderer* aRenderer, long& y, List& list)
{
	if(expanded)
	{
		for(node_iter_t i = children.begin(); y < list.m_Rect.y2 && i != children.end(); ++i)
		{
			y += rowHeight;
			i->render(i, aRenderer, y, r, list);
		}
	}
}*/

void ListNode::renderFrom(Renderer* renderer, long& y)
{
	node_iter_t i(this);
	//bool        hasParent = i->hasParent;
	//list_t*     parentList = i->parentList;
	//assert(parentList);
	
	//Only the root element has parentList == 0, and we can't even obtain an iterator
	//to the root element, thus it's safe to assume that parentList is a valid pointer.
	
	while(i && y < list->getRect().y2)
	{
		i->render(renderer, y);
			
		y += List::rowHeight;
		
		if(i->expanded)
		{
			i->children.getFirst()->renderFrom(renderer, y);
			
			if(y >= list->getRect().y2)
				break;
		}
		
		++i;
		
		/*
		while(i == 0)
		{
			if(!hasParent)
				return;
				
			i = parent;
			//parentList = i->parentList;
			//assert(parentList);
			hasParent = i->hasParent;
			parent = i->parent;
			++i;
		}*/
	}
}

List::node_iter_t ListNode::getPrevVisible(node_iter_t i)
{
	node_iter_t parent = i->parent;
	
	--i;
	if(!i)
	{
		if(!parent)
			return node_iter_t();
		
		i = parent;
		parent = i->parent;
	}
	else if(i->expanded)
	{
		if(i->children.last())
			i = i->children.last();
	}
	
	return i;
}

List::node_iter_t ListNode::getNextVisible(node_iter_t i)
{
	node_iter_t parent = i->parent;
	
	if(i->expanded)
	{
		parent = i;
		i = i->children.begin();
	}
	else
		++i;
		
	while(!i)
	{
		if(!parent)
			return node_iter_t();
			
		i = parent;
		parent = i->parent;
		++i;
	}
	
	return i;
}

int ListNode::findOffsetTo(node_iter_t i, node_iter_t to)
{
	int offs = 0;
	
	for(; i != to; ++offs)
	{
		if(!(i = getNextVisible(i)))
			return -1;
	}

	return offs;
}

ListNode::node_iter_t ListNode::findRelative(node_iter_t i, int aIdx)
{
	node_iter_t parent = i->parent;

	if(aIdx < 0)
	{
		while(++aIdx <= 0)
		{
			node_iter_t lastValid = i;

			if(!(i = getPrevVisible(i)))
				return lastValid;
		}
	}
	else if(aIdx > 0)
	{
		while(--aIdx >= 0)
		{
			node_iter_t lastValid = i;

			if(!(i = getNextVisible(i)))
				return lastValid;
		}
	}
	
	return i;
}

void ListNode::clearSelections()
{
	selected = false;
	for(node_iter_t i = children.begin(); i; ++i)
	{
		i->clearSelections();
	}
}

bool List::render()
{
	Renderer* renderer = context()->renderer();
	
	if(m_formatting.background.skin)
	{
		renderer->drawSkinnedBox(*m_formatting.background.skin, getRect(), m_formatting.background.color);
	}
	else
	{
		renderer->drawBox(
			getRect(), m_formatting.background.color,
			m_formatting.borders[0].color,
			m_formatting.borders[1].color,
			m_formatting.borders[2].color,
			m_formatting.borders[3].color);
	}
	
	long halfRowHeight = rowHeight/2;
	
	long y = getRect().y1;
	
	if(true) //TODO: Check view column headers parameter
	{
		renderer->drawBox(
			Rect(getRect().x1 + 1, getRect().y1 + 1, getRect().x2 - 1, getRect().y1 + rowHeight - 1),
			m_listFormatting.headerColor);
			
		// Render column headers
		double x = m_listFormatting.indent + getRect().x1;
		
		double w = getRect().getWidth() / m_totalWidthFactor;
		for(std::vector<ColumnHeader>::const_iterator i = m_columnHeaders.begin();
			i != m_columnHeaders.end();
			++i)
		{
			renderer->drawText(*m_font, i->name, BaseFont::CenterV, long(x), y + halfRowHeight, RGB(0, 0, 0));
			x += i->widthFactor * w;
		}
		
		y += rowHeight;
	}
	
	//node_iter_t cur = m_Base;
	
	if(m_Base)
	{
		//m_Base->renderFrom(renderer, y, *this);
		for(node_iter_t i = m_Base; i && y < getRect().y2; i = ListNode::getNextVisible(i))
		{
			i->render(renderer, y);
			y += rowHeight;
		}
	}
	
	/*
	while(isValid(cur) && y < m_Rect.y2)
	{
		cur->render(cur, aRenderer, y, m_Rect, *this);

		++cur;
		
		y += rowHeight;
	}
*/
	return true;
}

void List::addColumn(ColumnHeader const& column)
{
	m_columnHeaders.push_back(column);
	
	m_RootNode.resizeColumns(m_columnHeaders.size());
	m_totalWidthFactor += column.widthFactor;
}

void List::setMainSel(node_iter_t iter)
{
	if(!iter)
		return;
	m_MainSel = iter;
	
	int offs = ListNode::findOffsetTo(m_RootNode.children.begin(), iter);

	if(offs < m_basePos)
		setBasePos(offs);
	else if(offs >= m_basePos + visibleRows())
	{
		setBasePos(offs - visibleRows() + 1);
	}
}

void List::scrollBottom()
{
	setBasePos(m_visibleChildren - visibleRows() + 1);
}

bool List::checkSelection()
{
	if(!m_MainSel)
	{
		if(m_RootNode.children.begin())
			setMainSel(m_RootNode.children.begin());
		return false;
	}
	
	return true;
}

bool List::verify_(node_iter_t i, node_iter_t n)
{
	if(i == n)
		return true;
	
	foreach(c, n->children)
	{
		if(verify_(i, c))
			return true;
	}
	
	return false;
}

List::node_iter_t List::verify(node_iter_t i)
{
	if(verify_(i, node_iter_t(&m_RootNode)))
		return i;
	else
		return node_iter_t();
}


bool List::keyDown(int key)
{
	if(m_active)
	{
		switch(key)
		{
/*
			case KEY_LCONTROL: case KEY_RCONTROL:
				m_multiSelect = true;
			break;
*/
			
			case KEY_ENTER:
				if(!doAction())
					doSetActivation(false);
			break;
			
			case KEY_DOWN:
				if(checkSelection())
					setMainSel(ListNode::getNextVisible(m_MainSel));
			break;
			
			case KEY_UP:
				if(checkSelection())
					setMainSel(ListNode::getPrevVisible(m_MainSel));
			break;
			
			case KEY_PGDN:
				if(checkSelection())
					setMainSel(ListNode::findRelative(m_MainSel, visibleRows()));
			break;
			
			case KEY_PGUP:
				if(checkSelection())
					setMainSel(ListNode::findRelative(m_MainSel, -visibleRows()));
			break;
			
			case KEY_RIGHT:
				if(checkSelection());
					expand(m_MainSel);
			break;
			
			case KEY_SPACE:
				if(checkSelection())
					m_MainSel->selected = !m_MainSel->selected;
			break;
		}
		
		return false;
	}
	else
	{
		if(key == KEY_ENTER)
		{
			doSetActivation(true);
			return false;
		}
	}
	
	return true;
}

bool List::mouseDown(ulong x, ulong y, Context::MouseKey::type button)
{
	if(button == Context::MouseKey::Left)
	{
		bool select = context()->keyState(KEY_LCONTROL) || context()->keyState(KEY_RCONTROL);
		bool range = context()->keyState(KEY_LSHIFT) || context()->keyState(KEY_RSHIFT);
		
		focus();
		
		if(!m_active)
			doSetActivation(true);
		
		int y2 = y - getRect().y1 - rowHeight;
		
		if(m_Base)
		{
			node_iter_t newSel = ListNode::findRelative(m_Base, y2 / rowHeight);
			
			if(!select)
			{
				m_RootNode.clearSelections();
			}
			
			if(range && m_MainSel && m_MainSel != newSel
			&& m_MainSel->parent == newSel->parent)
			{
				bool toggle = false;
				ListNode* parent = m_MainSel->parent;
				if(!parent)
					parent = &m_RootNode;
				for(node_iter_t i = parent->children.begin(); i; ++i)
				{
					if(i == m_MainSel || i == newSel)
					{
						if(!toggle)
							toggle = true;
						else
						{
							i->selected = true;
							break;
						}
					}
					
					if(toggle)
						i->selected = true;
				}
			}
			else
			{
				setMainSel(newSel);
				
				if(select)
					newSel->selected = !newSel->selected;
				else
				{
					newSel->selected = true;
					expand(newSel);
				}
			}
		}
			
		return false;
	}
	return true;
}

bool List::mouseScroll(ulong newX, ulong newY, int offs)
{
	setBasePos(m_basePos + offs*2);
	return false;
}


/*
bool Edit::mouseMove(ulong x, ulong y)
{
	if(m_drag)
	{
		focus();
		
		if(!m_active)
			doSetActivation(true);
		
		int xoff = m_hscroll - 5 - m_rect.x1;
		Renderer* renderer = context()->renderer();
		m_selTo = renderer->getTextCoordToIndex(*m_font, m_text.begin(), m_text.end(), x + xoff);
		
		return false;
	}
	return true;
}*/

void List::applyFormatting(Context::GSSpropertyMap const& f)
{
	Wnd::applyFormatting(f);

	const_foreach(i, f)
	{
		if(i->first == "header-color")
		{
			const_foreach(v, i->second)
			{
				readColor(m_listFormatting.headerColor, *v);
			}
		}
		else if(i->first == "selection-color")
		{
			const_foreach(v, i->second)
			{
				readColor(m_listFormatting.selectionColor, *v);
			}
		}
		else if(i->first == "selection-frame-color")
		{
			const_foreach(v, i->second)
			{
				readColor(m_listFormatting.selectionFrameColor, *v);
			}
		}
		else if(i->first == "indent")
		{
			const_foreach(v, i->second)
			{
				m_listFormatting.indent = lexical_cast<double>(*v);
			}
		}
		
	}
}

void List::sortLua(LuaReference comparer)
{
	LuaLT criteria(lua, comparer);
	m_RootNode.children.sort(criteria);
	
	m_basePos = 0;
	m_Base = m_RootNode.children.begin();
}

int List::classID()
{
	return Context::List;
}

}
