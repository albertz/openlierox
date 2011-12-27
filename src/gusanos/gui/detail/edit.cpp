#include "edit.h"
#include "gusanos/allegro.h" //TEMP!!!!!!!!!!!

#include <algorithm>

#include <iostream>
using std::cerr;
using std::endl;

namespace OmfgGUI
{

LuaReference Edit::metaTable;

bool Edit::render()
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
		
	//std::pair<int, int> dim = m_font->getDimensions(m_text.begin(), m_text.begin() + m_caretPos);
	
	assertCaretVisibility(renderer);
	
	int xoff = 5 - m_hscroll;
	
	if(m_active)
	{
		
		std::pair<int, int> dim = renderer->getTextDimensions(*m_font, m_text.begin(), m_text.begin() + m_caretPos);
		
		if(m_selTo != m_caretPos)
		{
			std::pair<int, int> dim2 = renderer->getTextDimensions(*m_font, m_text.begin(), m_text.begin() + m_selTo);
			
			int x1 = m_rect.x1 + xoff;
			int x2 = x1;
			if(dim2.first < dim.first)
				x1 += dim2.first, x2 += dim.first;
			else
				x1 += dim.first, x2 += dim2.first;
				
			RGB color(50, 50, 120);
				
			renderer->drawBox(Rect(x1, m_rect.y1 + 2, x2, m_rect.y2 - 2), color, color, color, color, color);
		}
		
		renderer->drawVLine(m_rect.x1 + xoff + dim.first, m_rect.y1 + 2, m_rect.y2 - 2, RGB(255, 255, 255));
	}
	
	if(m_font)
		renderer->drawText(*m_font, m_text, BaseFont::CenterV, m_rect.x1 + xoff, m_rect.centerY(), m_formatting.fontColor);
	
	return true;
}
	
void Edit::process()
{
	if(m_caretPos > m_text.size())
		m_caretPos = m_text.size();
}

void Edit::setText(std::string const& aStr)
{
	Wnd::setText(aStr);
	
	m_caretPos = std::min(m_caretPos, (ulong)m_text.size());
	m_selTo = std::min(m_selTo, (ulong)m_text.size());
}

void Edit::assertCaretVisibility(Renderer* renderer)
{
	std::pair<int, int> dim = renderer->getTextDimensions(*m_font, m_text.begin(), m_text.begin() + m_caretPos);
	
	int relCaretPos = dim.first - m_hscroll;
	if(relCaretPos > getRect().getWidth())
	{
		m_hscroll += relCaretPos - getRect().getWidth() + 5;
	}
	else if(relCaretPos < 0)
	{
		m_hscroll += relCaretPos;
	}
}
	
/*
bool Edit::keyUp(int key)
{
	return
}*/

int Edit::classID()
{
	return Context::Edit;
}

// TODO: Editing handlers

}
