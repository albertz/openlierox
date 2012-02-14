#ifndef OLX__RANDOM_H
#define OLX__RANDOM_H

#include "CVec.h"

float	GetRandomNum(); // get a random float from [-1,1]
float	GetRandomPosNum(); // get a random float from [0,1]
int		GetRandomInt(int max); // get a random int from [0,max]

CVec	GetRandomVec();


#endif // OLX__RANDOM_H
