#ifndef LIERO_MATH_HPP
#define LIERO_MATH_HPP

typedef int fixed;

inline fixed itof(int v)
{
	return v << 16;
}

inline int ftoi(fixed v)
{
	return v >> 16;
}

extern fixed sinTable[128];
extern fixed cosTable[128];

int vectorLength(int x, int y);

inline int distanceTo(int x1, int y1, int x2, int y2)
{
	return vectorLength(x1 - x2, y1 - y2);
}

void loadTablesFromEXE();

#endif // LIERO_MATH_HPP
