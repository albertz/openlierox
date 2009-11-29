#ifndef WEAPONS_H
#define WEAPONS_H

#include <allegro.h>
#include <fstream>

class sprite;
class sound;

//forward
class sound;

class weapon
{
  public:
  weapon();
  ~weapon();

	int shoot_spd,shoot_num,distribution,shoot_times,shoot_spd_rnd,aim_recoil,recoil,affected_motion;
  int firecone_timeout,ammo,reload_time,lsight_intensity,lsight_fade,start_delay;
  char autofire;
	struct part_type *shoot_obj,*create_on_release;
  sprite *firecone;
	//weapon HUD
	sprite *image;
	sound *shoot_sound,*reload_sound,*noammo_sound,*start_sound;
	char name[512],filename[512];
	class weapon *next,*prev;
};

class weap_list
{
  public:
	class weapon *start,*end;
  class weapon *num[100];
  int weap_count;
	weap_list();
  ~weap_list();
	void destroy();
	void sort();
};

class s_playerweap
{
  public:
  int weap;
  int ammo,shoot_time,reload_time,start_delay;
  bool reloading;
  void shoot( int x, int y, int xspd, int yspd, int aim, int dir, int owner );
};

extern class weap_list *weaps;

class weapon* load_weap(const char* weap_name);
void scanWeapsDir();

#endif /* WEAPONS_H */
