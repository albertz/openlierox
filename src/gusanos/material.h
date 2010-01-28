#ifndef MATERIAL_H
#define MATERIAL_H

#include "level/LXMapFlags.h"

struct Material
{
	Material();
	~Material();
	
	bool worm_pass;
	bool particle_pass;
	bool flows;
	bool can_breath;
	bool destroyable;
	bool draw_exps;
	bool blocks_light;
	bool destroys_water;
	bool creates_water;
	bool is_stagnated_water;
	int damage;
	
	// Use as read only
	unsigned char index;
	
	unsigned char toLxFlags() const {
		if(worm_pass && particle_pass) return PX_EMPTY;
		if(destroyable) return PX_DIRT;
		return PX_ROCK;
	}
};

#endif // _MATERIAL_H_
