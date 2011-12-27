#include "button.h"

using std::cout;
using std::cerr;
using std::endl;

namespace OmfgGUI
{
	
LuaReference Button::metaTable;

bool Button::render()
{
	//Draw a flat, grey box with the text by default
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
		
	if(m_formatting.background.spriteSet)
	{
		//cerr << "Rendering: " << m_formatting.background.spriteSet << endl;
		//cerr << "Pos: (" << getRect().centerX() << ", " << getRect().centerY() << ")" << endl;
		renderer->drawSprite(*m_formatting.background.spriteSet, 0, getRect().centerX(), getRect().centerY());
		
		//delete m_formatting.background.spriteSet;
		//m_formatting.background.spriteSet = 0;
	}
	
	if(m_font)
		renderer->drawText(*m_font, m_text, BaseFont::CenterV | BaseFont::CenterH, m_rect.centerX(), m_rect.centerY(), m_formatting.fontColor);
	
	return true;
}

void Button::process()
{
}


}
