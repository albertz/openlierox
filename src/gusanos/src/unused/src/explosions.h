#ifndef EXPLOSIONS_H
#define EXPLOSIONS_H

#include <ctype.h>
#include <allegro.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "particles.h"
#include "engine.h"
#include "sounds.h"


class exp_type
{
  public:
	char name[255];
	int damage,bright_variation,blow_away;
	int color,wormshootnum,wormshootspeed,wormshootspeedrnd,detect_range,framenum,timeout;
  int light_fadeness, light_color,spd_multiply;
	char affect_worm,affect_particles, light_effect;
	 sprite *sprt,*hole;
	struct part_type *wormshootobj;
	 class sound *snd;
	 exp_type* next;
	 exp_type* prev;
  exp_type();
  ~exp_type();
};

class exp_type_list
{
  public:
	class exp_type* start;
	class exp_type* end;
	exp_type_list();
  ~exp_type_list();
	void destroy();
};

class explosion
{
  public:
	int x,y;
	int time,currframe,framecount,color,timeout;
  BITMAP* light;
	class exp_type* type;
	class explosion* next;
	class explosion* prev;
  explosion();
  ~explosion();
};

class exp_list
{
  public:
	class explosion* start;
	class explosion* end;
	exp_list();
  ~exp_list();
};

extern class exp_list *exps;
extern class exp_type_list *exp_types;
  
void calc_exps();
void dig_hole(BITMAP* image,int x, int y);
class exp_type* load_exp(const char* exp_name);
void render_exps();
void draw_explosion(BITMAP* image,int x, int y);
void dest_exp(class explosion* tmp);
void create_exp(int x,int y,class exp_type *type);

#endif /* EXPLOSIONS_H */
