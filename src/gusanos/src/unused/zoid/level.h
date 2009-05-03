#ifndef LEVEL_H
#define LEVEL_H

#include <allegro.h>
#include "loadpng.h"

class part_type;

struct s_mat
{
  bool worm_pass;
  bool particle_pass;
  bool flows;
  bool can_breath;
  bool draw_exps;
  bool blocks_light;
  bool destroys_water;
  bool creates_water;
  part_type *chreact;
  int damage;
	//mat weapon
	int strength;
	int destroyed_into;
};

struct s_spawnpoint
{
  int x,y;
  int team;
};

class level
{
  public:
	BITMAP* mapimg;
	BITMAP* buffer;
	BITMAP* material;
  BITMAP* background;
	BITMAP* layer;
  BITMAP* paralax;
  BITMAP* water_buffer;
  BITMAP* light_layer;
  bool has_water;
  char name[32];
  char path[1024];
  struct s_mat mat[256];
  struct s_spawnpoint spawnpoint[32];
  int spawnpoint_count;
  level();
  ~level();
  void init_materials();
};

void free_map();
int load_map(char* name);
void load_map_config();
void change_level();

extern class level* map;

#endif /* LEVEL_H */
