#ifndef LIERO_BONUS_HPP
#define LIERO_BONUS_HPP

#include "math.hpp"
#include "objectList.hpp"

struct Bonus : ObjectListBase
{
	Bonus()
	: frame(-1)
	{
	}
	
	fixed x;
	fixed y;
	fixed velY;
	int frame;
	int timer;
	int weapon;
		
	void process();
};

#endif // LIERO_BONUS_HPP
