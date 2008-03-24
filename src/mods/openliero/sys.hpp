#ifndef LIERO_SYS_HPP
#define LIERO_SYS_HPP

#include "platform.hpp"

#ifdef LIERO_WIN32

namespace Win32
{
int getFreeMemory();
}

#endif

#endif // LIERO_SYS_HPP
