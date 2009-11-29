#ifndef LIGHT_H
#define LIGHT_H

#include <allegro.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
#include "sprites.h"
#include "engine.h"
#include "level.h"

void do_collision(BITMAP *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(BITMAP *, int, int, int));

void check_obstacles(BITMAP *where, int x, int y, int d);

void render_light( int x, int y, int r, int g, int b, int fade, int noise,BITMAP *where, BITMAP *material);

void render_exp_light( int x, int y, int color, int fade, int noise,BITMAP *where, BITMAP *material);

void render_sunlight( BITMAP *where, BITMAP *material);

void render_flashlight( int x, int y, int angle, int dir, BITMAP *where, BITMAP *material);

void check_sunlight( int x, int y);

void render_lens(int x, int y, int radius, BITMAP* where);

#endif