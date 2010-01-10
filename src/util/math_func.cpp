#include "math_func.h"

boost::mt19937 rndgen;
boost::uniform_01<boost::mt19937> rnd(rndgen);
boost::uniform_real<> miduni_dist(-0.5,0.5);
boost::variate_generator<boost::mt19937, boost::uniform_real<> > midrnd(rndgen, miduni_dist);
