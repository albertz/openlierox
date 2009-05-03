#ifndef PARTICLES_H
#define PARTICLES_H

#include <allegro.h>

class worm;
class sprite;
class exp_type;
class sound;

struct part_type
{
	char name[255];
	int gravity,damage,bright_variation,blow_away,bounce;
	int color,laser_type,alpha,exptime,timeout_variation,shootnum,wormshootnum,shootnumtrail,shootspeed,wormshootspeed,shootspeedtrail;
	int shootspeedrnd,wormshootspeedrnd,shootspeedrndtrail,detect_range,framenum,framedelay,traildelay,exptraildelay,affected_by_motion,affected_by_explosions;
	int autorotate_speed, lens_radius;
	//Crate
	int give_weapon;
	//homing
	int homing, home_on_owner, homing_detect_range, max_homing_speed, homing_acceleration;
	sound *catch_target_sound;
	int shoot_keep_angle;
	char expgnd,expworm,drawonmap,animonground,remworm,remgnd,directional,visible,layer;
	sprite* sprt;
	part_type *shootobj,*wormshootobj,*shootobjtrail;
	exp_type *destroy_exp,*exp_trail;
	sound *expsnd;
	BITMAP *effect_buf;
	part_type *next;
	part_type *prev;
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
	int owner, target;
	part_type* type;
	particle* next;
	particle* prev;
  particle* layer_next;
	particle* layer_prev;
};

struct particles
{
  particle* start;
  particle* end;
  particle  *layer_start[4];
  particle  *layer_end[4];
	void init();
  particle* create_part(int x,int y,int xspd,int yspd,int owner,struct part_type *type);
  particle* create_directional_part(int x,int y,int xspd,int yspd,int dir,int ang,int owner,struct part_type *type);
	void render_particles( BITMAP* where , int layer );
  void r_lens(BITMAP* where);
	void shoot_part(int ang,int spd,int dir,int x,int y,int xspdadd,int yspdadd,int owner,struct part_type *type);
	void player_removed(int pl, bool remove_target_only = false);
};

void destroy_particles();
struct part_type* load_part(const char* type_name);
//Crate
void summon_bonuses();
//
void calc_particles();
void dest_part(struct particle* tmp);
void rem_part(struct particle* tmp);
  
extern struct particles partlist;
extern struct type_list *types;


#endif /* PARTICLES_H */
