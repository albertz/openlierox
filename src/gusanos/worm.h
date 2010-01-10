#ifndef WORM_H
#define WORM_H

//#include "CGameObject.h"
#include "CWorm.h"
//#include "sprite.h"

#include "netstream.h"

class BaseAnimator;
class CWormInputHandler;
class NinjaRope;

class Worm : public CWorm
{
	public:
		static Net_ClassID classID;

		Worm();
		~Worm();

		void think();
	private:
};

#endif  // _WORM_H_
