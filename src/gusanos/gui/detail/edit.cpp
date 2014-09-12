#include "edit.h"
#include "gusanos/allegro.h" //TEMP!!!!!!!!!!!

#include <algorithm>

namespace OmfgGUI
{

LuaReference Edit::metaTable;

void Edit::setText(std::string const& aStr)
{
	Wnd::setText(aStr);
	
	m_caretPos = std::min(m_caretPos, (ulong)m_text.size());
	m_selTo = std::min(m_selTo, (ulong)m_text.size());
}

void Edit::assertCaretVisibility(Renderer* renderer)
{
	std::pair<int, int> dim = renderer->getTextDimensions(*m_font, m_text.begin(), m_text.begin() + m_caretPos);
	
	int relCaretPos = dim.first - (int)m_hscroll;
	if(relCaretPos > getRect().getWidth())
	{
		m_hscroll += relCaretPos - getRect().getWidth() + 5;
	}
	else if(relCaretPos < 0)
	{
		m_hscroll += relCaretPos;
	}
}
	
int Edit::classID()
{
	return Context::Edit;
}

}
