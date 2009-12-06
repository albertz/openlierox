#include "worm.h"

Worm::Worm() : BaseWorm()
{}

Worm::~Worm()
{}

void Worm::think()
{
	BaseWorm::think();
#ifndef DEDICATED_ONLY
	renderPos = pos;
#endif
}
