#ifndef WATER_H
#define WATER_H

#include <allegro.h>


struct s_water
{
	int x,y;
  char dir,time,matunder;
	int color;
};

struct s_water_spawn
{
  int x,y;
};

void stagnate_water(int index);

void create_water(int x,int y,int dir);

void check_water_sides(int index);

void check_hole_sides(int x,int y);

void calc_water();

void create_waterlist();

extern struct s_water *water;

#endif /* WATER_H */
