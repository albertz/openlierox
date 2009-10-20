/*
	

	main wrapper

	code under LGPL
	created 2009-10-20 by albert
*/

#include <iostream>

#include "../../../src/breakpad/ExtractInfo.h"

int usage(const char* bin) {
	printf("usage: %s <minidumpfile>\n", bin);
	return 1;
}

int main(int argc, char** argv) {
	if(argc < 2) return usage(argv[0]);

	MinidumpExtractInfo(argv[1], std::cout, std::cerr);

	return 0;
}

