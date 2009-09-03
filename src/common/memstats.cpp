/*
 *  memstats.cpp
 *  OpenLieroX
 *
 *  memstats implementation for memstats.h
 *  
 *  Created by Albert Zeyer on 03.09.09.
 *  code under LGPL
 *
 */

#ifdef MEMSTATS

#undef new
#undef delete

#include <iostream>

void * operator new (size_t size, dmalloc_t, const char* file, int line) {
	void* p = malloc(size);
	std::cout << file << ":" << line << ": new " << size << " bytes allocated @" << p << std::endl;
	return p;
}

void * operator new [] (size_t size, dmalloc_t, const char* file, int line) {
	void* p = malloc(size);
	std::cout << file << ":" << line << ": new[] " << size << " bytes allocated @" << p << std::endl;
	return p;
}
	
void operator delete (void * p) {
	std::cout << "delete @" << p << std::endl;
	free(p);
}

void operator delete [] (void * p) {
	std::cout << "delete[] @" << p << std::endl;
	free(p);	
}

void printMemStats() {
	
}


#endif

