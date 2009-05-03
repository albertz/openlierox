#ifndef PARTICLES_H
#define PARTICLES_H

#include <ctype.h>
#include <allegro.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include "console.h"
//#include "water.h"
#include "sprites.h"
#include "sounds.h"
#include "explosions.h"
#include "player.h"

class worm;

struct part_type
{
	char name[255];
	int gravity,damage,bright_variation,blow_away,bounce;
	int color,laser_type,alpha,exptime,timeout_variation,shootnum,wormshootnum,shootnumtrail,shootspeed,wormshootspeed,shootspeedtrail;
	int shootspeedrnd,wormshootspeedrnd,shootspeedrndtrail,detect_range,framenum,framedelay,traildelay,exptraildelay,affected_by_motion,affected_by_explosions;
  int autorotate_speed, lens_radius;
	char expgnd,expworm,drawonmap,animonground,remworm,remgnd,directional,visible;
	sprite* sprt;
	part_type *shootobj,*wormshootobj,*shootobjtrail;
	exp_type *destroy_exp,*exp_trail;
	sound *expsnd;
	part_type *next,*next_lens;
	part_type *prev,*prev_lens;
};

struct type_list
{
	struct part_type* start;
	struct part_type* end;
	void init();
	void destroy();
};

struct particle
{
	int x,y,xspd,yspd,dir,ang;
	int time,currframe,framecount,obj_trail_time,exp_trail_time,timeout,color;
	worm *player;
	part_type* type;
	particle* next;
	particle* prev;
};

struct particles
{
	 particle* start;
	 particle* end;
	void init();
	 particle* create_part(int x,int y,int xspd,int yspd,worm *player,struct part_type *type);
	 particle* create_directional_part(int x,int y,int xspd,int yspd,int dir,int ang,worm *player,struct part_type *type);
	void render_particles(BITMAP* where);
  void r_lens(BITMAP* where);
	void shoot_part(int ang,int spd,int dir,int x,int y,int xspdadd,int yspdadd,worm *player,struct part_type *type);
};

void destroy_particles();
struct part_type* load_part(const char* type_name);
void calc_particles();
void dest_part(struct particle* tmp);
void rem_part(struct particle* tmp);
  
extern struct particles partlist;
extern struct type_list *types;


#endif /* PARTICLES_H */
