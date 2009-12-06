#include "label.h"

namespace OmfgGUI
{
	
LuaReference Label::metaTable;

bool Label::render()
{
	Renderer* renderer = context()->renderer();

/*
	if(m_formatting.background.skin)
	{
		renderer->drawSkinnedBox(*m_formatting.background.skin, getRect(), m_formatting.background.color);
	}
	else if(!m_formatting.background.invisible)
	{
		renderer->drawBox(
			getRect(), m_formatting.background.color,
			m_formatting.borders[0].color,
			m_formatting.borders[1].color,
			m_formatting.borders[2].color,
			m_formatting.borders[3].color);
	}
*/

	if(m_formatting.background.spriteSet)
	{
		renderer->drawSprite(*m_formatting.background.spriteSet, 0, getRect().centerX(), getRect().centerY());
	}
	
	if(m_font)
		renderer->drawText(*m_font, m_text, BaseFont::CenterV, getRect().x1 + 4, getRect().centerY(), m_formatting.fontColor);
	
	return true;
}

void Label::process()
{
}

// Block events but don't act on them
bool Label::mouseDown(ulong newX, ulong newY, Context::MouseKey::type button)
{
	return false;
}

bool Label::mouseUp(ulong newX, ulong newY, Context::MouseKey::type button)
{
	return false;
}

bool Label::keyDown(int key)
{
	return false;
}

}

