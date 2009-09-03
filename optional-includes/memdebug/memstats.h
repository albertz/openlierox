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

// these are needed here because they don't like our #define new/delete
#include <new>
#include <ext/new_allocator.h>
#include <memory>
#include <string>
// ----------------------------------------

#define MEMSTATS

struct dmalloc_t {};

void * operator new (size_t size, dmalloc_t, const char* file, int line);
void * operator new [] (size_t size, dmalloc_t, const char* file, int line);
void operator delete (void * p);
void operator delete [] (void * p);

#define new ::new (dmalloc_t(), __FILE__, __LINE__)
#define delete ::delete

void printMemStats();
