#include "group.h"

using std::cout;
using std::cerr;
using std::endl;

namespace OmfgGUI
{

LuaReference Group::metaTable;

bool Group::render()
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
		
	if(m_formatting.background.spriteSet)
	{
		renderer->drawSprite(*m_formatting.background.spriteSet, 0, getRect().centerX(), getRect().centerY());	
	}
	
	/*
	if(m_font)
		renderer->drawText(*m_font, m_text, BaseFont::CenterV | BaseFont::CenterH, m_rect.centerX(), m_rect.centerY(), m_formatting.fontColor);
	*/
	return true;
}

void Group::process()
{
}
/*
//Sends a mouse button down event
bool Group::mouseDown(ulong newX, ulong newY, Context::MouseKey::type button)
{
	return false;
}

//Sends a mouse button up event
bool Group::mouseUp(ulong newX, ulong newY, Context::MouseKey::type button)
{
	return false;
}*/

int Group::classID()
{
	return Context::Group;
}

}
