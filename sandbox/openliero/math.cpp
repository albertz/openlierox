#include "math.hpp"
#include "reader.hpp"
#include <cmath>

fixed sinTable[128];
fixed cosTable[128];

int vectorLength(int x, int y)
{
	return int(std::sqrt(double(x*x) + double(y*y))); // TODO: Figure out how liero does it exactly, or at least make it machine independent
}

void loadTablesFromEXE()
{
	FILE* exe = openLieroEXE();
	
	fseek(exe, 0x1C41E, SEEK_SET);
	
	for(int i = 0; i < 128; ++i)
	{
		cosTable[i] = readSint32(exe);
		sinTable[i] = readSint32(exe);
	}
}
