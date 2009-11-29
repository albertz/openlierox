#ifndef ENGINE_H
#define ENGINE_H

#include <allegro.h>
#include <zoidcom.h>
#ifdef AA2XSAI
extern "C" {
#include "2xsai.h"
}
#endif

class worm;
class exp_type;
class sound;
class level;

extern volatile int speed_counter;
extern volatile int t;

extern ZCom_ClassID  game_classid;


void recharge_weapons( worm* player);

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
  BITMAP* buffer;
	int *MAX_SPEED;
	int *ACELERATION;
	int *FRICTION;
	int *AIR_FRICTION;
	int *GRAVITY;
	int *WORM_BOUNCINESS;
	int WORM_BOUNCE_LIMIT;
  int *FLASHLIGHT;
	int *WORM_JUMP_FORCE;
	int *START_HEALTH;
	//health
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
  int *TEAMPLAY;
  int *FRIENDLYFIRE;
  //Crate
  int *WEAPON_CHANCE;
  int *HEALTH_CHANCE;
  //
  //Minimap
  int *MINIMAP;
	int *MINIMAP_TYPE;
	int *SHOW_FPS;
	//reload_multiplier
	int *RELOAD_MULTIPLIER;
	//weapon HUD
	int *WEAPON_HUD;

  bool teamplay;
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
	//talking
	class sprite* talk;
	class sprite* death_img;
  class sprite *ammo;
	class sprite *hook;
  class sprite *firecone;
	struct part_type *tmppart;
	struct part_type *gore;
  //Crate
  struct part_type *weapon_box;
  struct part_type *health_box;
  //
	sound *death,*respawn,*throwrope,*bump,*gstart,*menu_move,*menu_select,*breath;
	bool selecting;
	exp_type *exp1;
	exp_type *worm_hole;
	int w,sync_mode;
	char mod[1024],level[1024];
  bool host,client,split_screen;
  ZCom_Node *node;
  void init_node(ZCom_Control *_cont, bool is_server);
	void weaponHUD(BITMAP* where, worm* player, struct s_viewport viewport, int position); //Draw weapon HUD
  void minimap(); //Draw minimap
  void scoreboard(); //Draw scoreboard
	bool quitgame; //quitgame
};

extern struct engine* game;
extern PALETTE pal;


#endif
