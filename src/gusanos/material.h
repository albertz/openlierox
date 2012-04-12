#ifndef MATERIAL_H
#define MATERIAL_H

/* An array of Material structs is CMap::m_materialList.
  This gets initialized by CMap::gusInit() in level.cpp.
  */

#include "level/LXMapFlags.h"

// see CMap::gusInit()
#define MATINDEX_SOLID 0
#define MATINDEX_BG 1
#define MATINDEX_DIRT 2
#define MATINDEX_DEATH 5
#define MATINDEX_NOHOOK 6

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
	bool can_hook;

	// Use as read only
	unsigned char index;
	
	unsigned char toLxFlags() const {
		if(worm_pass && particle_pass) return PX_EMPTY;
		if(destroyable) return PX_DIRT;
		return PX_ROCK;
	}
	
	static unsigned char indexFromLxFlag(unsigned char lxflag) {
		unsigned char f = 1; // background
		if(lxflag & PX_DIRT) f = 2;
		if(lxflag & PX_ROCK) f = 0;
		return f;
	}
};

#endif // _MATERIAL_H_
