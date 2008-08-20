#ifndef LIERO_PLATFORM_HPP
#define LIERO_PLATFORM_HPP

#if defined(WIN32) || defined(_WIN32)
#define LIERO_WIN32
#else
#define LIERO_POSIX
#endif

#endif // LIERO_PLATFORM_HPP
