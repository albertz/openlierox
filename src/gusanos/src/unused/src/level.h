#ifndef LEVEL_H
#define LEVEL_H

#include <ctype.h>
#include <allegro.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "engine.h"
#include "lights.h"

struct s_mat
{
  bool worm_pass;
  bool particle_pass;
  bool flows;
  bool can_breath;
  bool destroyable;
  bool draw_exps;
  bool blocks_light;
  bool destroys_water;
  bool creates_water;
  part_type *chreact;
  int damage;
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
  bool has_water;
  char name[32];
  char path[1024];
  struct s_mat mat[256];
  level();
  ~level();
};

void free_map();
int load_map(char* name);
void load_map_config();
void change_level();

extern class level* map;

#endif /* LEVEL_H */
