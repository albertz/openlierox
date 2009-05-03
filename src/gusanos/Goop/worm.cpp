#include "worm.h"

Worm::Worm() : BaseWorm()
{
}

Worm::~Worm()
{
}

void Worm::think()
{
	BaseWorm::think();
#ifndef DEDSERV
	renderPos = pos;
#endif
}
