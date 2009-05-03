#ifndef LIGHT_H
#define LIGHT_H

#include <allegro.h>

struct s_seg_border
{
  float x,y,yinc,xinc;
};

struct s_segment
{
  struct s_seg_border b1,b2;
};

class c_segments
{
  public:
  s_segment seg[100];
  int segcount,xorigin,yorigin;
  void create_segment(float x1,float x2,float y);
  void remove_segment(int index);
  void move_segmentsup();
};

void do_collision(BITMAP *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(BITMAP *, int, int, int));

void check_obstacles(BITMAP *where, int x, int y, int d);

void render_light( int x, int y, int r, int g, int b, int fade, int noise,BITMAP *where, BITMAP *material);

void render_exp_light(int x, int y, int _color, int fade, int noise,int mask,BITMAP *where, BITMAP *material);

void render_sunlight( BITMAP *where, BITMAP *material);

void render_flashlight( int x, int y, int angle, int dir, BITMAP *where, BITMAP *material);

void render_seglight( int x, int y, int angle, int dir, BITMAP *where, BITMAP *material);

void check_sunlight( int x, int y);

void render_lens(int x, int y, int radius, BITMAP* where, BITMAP *buf);

bool obs_line ( BITMAP*where, int x1 , int y1 , int x2 , int y2 );

#endif
