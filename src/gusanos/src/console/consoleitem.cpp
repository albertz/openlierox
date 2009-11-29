#include "consoleitem.h"

ConsoleItem::ConsoleItem(bool locked)
: temp(false), m_locked(locked), m_owner(0)
{
	
}

bool ConsoleItem::isLocked()
{
	return m_locked;
}

