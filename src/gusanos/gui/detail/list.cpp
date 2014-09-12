#include "list.h"
#include <cassert>
#include "util/macros.h"
#include "../../luaapi/context.h"
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
		bool v = lua_toboolean(context, -1) != 0;
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

void List::addColumn(ColumnHeader const& column)
{
	m_columnHeaders.push_back(column);
	
	m_RootNode.resizeColumns(m_columnHeaders.size());
	m_totalWidthFactor += column.widthFactor;
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

void List::sortLua(LuaReference comparer)
{
	LuaLT criteria(luaIngame, comparer);
	m_RootNode.children.sort(criteria);
}

int List::classID()
{
	return Context::List;
}

}
