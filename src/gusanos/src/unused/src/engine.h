#ifndef ENGINE_H
#define ENGINE_H
//#define AA2XSAI

#include <ctype.h>
#include <allegro.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "console.h"
#include "water.h"
#include "sprites.h"
#include "sounds.h"
#include "particles.h"
#include "explosions.h"
#include "weapons.h"
#include "level.h"
#include "player.h"
#include "network.h"
#include "render.h"
#include <zoidcom.h>
#ifdef AA2XSAI
extern "C" {
#include "2xsai.h"
}
#endif


extern volatile int speed_counter;
extern volatile int t;

class exp_type;

void recharge_weapons( worm* player);

struct s_playerweap
{
  int weap;
  int ammo,shoot_time,reload_time,start_delay;
  bool reloading;
};

/*struct s_water
{
	int x,y,dir,time;
	int color;
};*/

struct s_viewport
{
  int x,y,w,h;
};

struct engine
{
	int *MAX_SPEED;
	int *ACELERATION;
	int *FRICTION;
	int *AIR_FRICTION;
	int *GRAVITY;
	int WORM_HEIGHT;
	int *WORM_BOUNCINESS;
	int WORM_BOUNCE_LIMIT;
  int *FLASHLIGHT;
	int *WORM_JUMP_FORCE;
	int *MAX_HEALTH;
	int *MAP_SHOW_MODE;
	int *RENDER_LAYERS;
	int *VOLUME;
	int *DAMAGE_SPEED;
	int *FALL_DAMAGE;
  int *RESPAWN_RELOAD;
  int *ROPE_STRENTH;
  int *ROPE_LENGHT;
  int *AIM_RECOIL_FRICTION;
  int *HOST;
  int *AIR_CAPACITY;
  int *ROPE_GRAVITY;
  int *VIDEO_FILTER;
  int *SPLIT_SCREEN;
  struct s_viewport viewport[2];
  int v_width,v_height,v_depth;
	int frame_count, fps;
  int weap_count;
  int water_count;
	struct fnt* fonty;
	void input();
	void render();
	void calcphysics();
	void init_game();
	class sprite* health;
	class sprite* death_img;
  class sprite *ammo;
	class sprite *hook;
  class sprite *firecone;
	BITMAP* buffer;
	struct al_ffblk* file;
	struct part_type *tmppart;
	struct part_type *gore;
  struct part_type *chreact;
	sound *death,*respawn,*throwrope,*bump,*gstart,*menu_move,*menu_select,*breath;
	bool selecting;
	 exp_type *exp1;
	 exp_type *worm_hole;
	int w,sync_mode;
	char mod[1024],level[1024];
  bool host,client,split_screen;
};

extern struct engine* game;
extern PALETTE pal;


#endif
