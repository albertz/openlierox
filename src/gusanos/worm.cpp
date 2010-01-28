#include "worm.h"

Worm::Worm() : CWorm()
{}

Worm::~Worm()
{}

void Worm::think()
{
	CWorm::think();
#ifndef DEDICATED_ONLY
	renderPos = pos();
#endif
}
