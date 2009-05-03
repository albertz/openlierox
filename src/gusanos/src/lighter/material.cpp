#include "material.h"

using namespace std;

Material::Material()
{
	worm_pass = true;
	particle_pass = true;
	flows = false;
	can_breath = true;
	destroyable = false;
	draw_exps = true;
	blocks_light = false;
	destroys_water = false;
	creates_water = false;
	damage = 0;
}

Material::~Material()
{
}
