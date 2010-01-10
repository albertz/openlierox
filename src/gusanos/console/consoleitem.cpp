#include "consoleitem.h"

ConsoleItem::ConsoleItem(bool locked)
: temp(false), m_owner(0), m_locked(locked)
{
	
}

bool ConsoleItem::isLocked()
{
	return m_locked;
}

