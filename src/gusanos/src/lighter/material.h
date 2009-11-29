#ifndef MATERIAL_H
#define MATERIAL_H

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
	int damage;
};

#endif // _MATERIAL_H_
