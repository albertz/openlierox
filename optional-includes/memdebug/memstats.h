/*
 *  memstats.h
 *  OpenLieroX
 *
 *  Memory statistics
 *
 *  Will count how much memory is consumed by which classes.
 *  Set it as a prefix header if you want to use it.
 *
 *  Created by Albert Zeyer on 03.09.09.
 *  code under LGPL
 *
 */

#ifdef __cplusplus

#ifdef _MSC_VER
#pragma warning(disable: 4290)
#endif

// these are needed here because they don't like our #define new/delete
#include <new>
#ifndef WIN32
#include <ext/new_allocator.h>
#include <ext/malloc_allocator.h>
#endif
#include <memory>
#include <string>
// these don't make compiler problems but runtime problems
#include <map>
#include <cstdio>
#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <vector>
#include <stack>
#include <streambuf>
#include <queue>
#include <algorithm>
#include <functional>
#include <locale>
#include <valarray>
#include <typeinfo>
#include <bitset>
#include <deque>
#include <limits>
#include <utility>
// ----------------------------------------

#define MEMSTATS

struct dmalloc_t {};

void * operator new (size_t size, dmalloc_t, const char* file, int line);
void * operator new [] (size_t size, dmalloc_t, const char* file, int line);
void operator delete (void *addr, dmalloc_t, const char* file, int line)  { ::delete addr; }
void operator delete [] (void *addr, dmalloc_t, const char* file, int line)  { ::delete[] addr; }

void* operator new(std::size_t) throw (std::bad_alloc);
void* operator new[](std::size_t) throw (std::bad_alloc);
void operator delete(void*) throw();
void operator delete[](void*) throw();
void* operator new(std::size_t, const std::nothrow_t&) throw();
void* operator new[](std::size_t, const std::nothrow_t&) throw();
void operator delete(void*, const std::nothrow_t&) throw();
void operator delete[](void*, const std::nothrow_t&) throw();

#define new ::new (dmalloc_t(), __FILE__, __LINE__)
#define delete ::delete

void printMemStats();

#endif
