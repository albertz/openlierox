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
	
int Edit::classID()
{
	return Context::Edit;
}

}
