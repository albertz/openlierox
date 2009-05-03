#ifndef WORM_H
#define WORM_H

//#include "base_object.h"
#include "base_worm.h"
//#include "sprite.h"

#include <zoidcom.h>

class BaseAnimator;
class BasePlayer;
class NinjaRope;

class Worm : public BaseWorm
{	
	public:
		
	static ZCom_ClassID  classID;
		
	Worm();
	~Worm();

	void think();
	
	private:

};

#endif  // _WORM_H_
