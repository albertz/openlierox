#ifndef PLAYER_H
#define PLAYER_H

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
#include <zoidcom.h>

extern ZCom_ClassID  player_classid;

class worm
{
  public:
  worm();
  ~worm();
  char name[32];
	int x,y;
	int xspd,yspd;
	int ropex,ropey;
	int ropexspd,ropeyspd;
	int rope_length;
	char ropestate;
	int xview,yview;
	int crosshx,crosshy;
	int health,deaths;
  int air;
	int crossr;
	int aim,aim_speed,aim_recoil_speed;
	unsigned int curr_frame;
	char dir;
  sprite *skin,*mask;
  sprite* crosshair;
  sprite* curr_firecone;
  int firecone_time;
	BITMAP* view;
	struct s_playerweap *weap;
  int curr_weap;
	bool active,islocal,deleteme;
	bool flag,flag2,flag3,flagleft,flagright,ropeflag,selecting_weaps,fireing;
	void shootrope();
	void destroyrope();
	void applyropeforce();
	struct KEYS *keys;
  int color;
  int local_slot;
  ZCom_Node *node;
  ZCom_ConnID id;
  void registerClass(ZCom_Control *_cont);
  void checkevents();
  void sendmsg(char* msg);
  void shooteventsend();
  void deatheventsend();
  void render(BITMAP* where, int frame, int x, int y);
  void render_flip(BITMAP* where, int frame, int x, int y);
};

struct KEYS
{
	bool left,right,up,down,fire,jump,change,zoom;
};

struct s_player_options
{
  int* sensitivity;
  int* aim_acceleration;
  int* aim_friction;
  int* aim_maxspeed;
  int *HEALTH_DRAW_MODE;
  char name[32];
};


extern struct s_player_options pl_options[2];
extern worm* player[32];
extern int local_player[2];
extern int player_count;
extern int local_players;

void init_players();
void delete_players();

void change_nick(int pl);
  
void renderrope(worm *player);
void calcrope(worm *player);
void respawn_player(worm* player);
void calcrope(worm *player);

void pl1_moveright();
void pl1_moveleft();
void pl1_aimup();
void pl1_aimdown();
void pl1_fire();
void pl1_jump();
void pl1_change();
void pl1_reload();

void pl0_moveright();
void pl0_moveleft();
void pl0_aimup();
void pl0_aimdown();
void pl0_fire();
void pl0_jump();
void pl0_change();
void pl0_reload();
void pl0_zoom();

void pl0_color();
void pl1_color();

void pl0_nick();
void pl1_nick();

void send_msg();

#endif /* PLAYER_H */
