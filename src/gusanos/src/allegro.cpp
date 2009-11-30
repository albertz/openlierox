/*
 *  allegro.cpp
 *  Gusanos
 *
 *  Created by Albert Zeyer on 30.11.09.
 *  code under LGPL
 *
 */

#include <SDL.h>
#include "allegro.h"
#include "FindFile.h"

int allegro_error = 0;

void allegro_init() {}
void allegro_exit() {}

void rest(int t) { SDL_Delay(t); }
void vsync() {}


void install_timer() {}
int install_int_ex(void (*proc)(), long speed) { return 0; }



void install_mouse() {}
void remove_mouse() {}

volatile int mouse_x;
volatile int mouse_y;
volatile int mouse_z;
volatile int mouse_b;
void (*mouse_callback)(int flags);


int poll_mouse() { return 0; }



BITMAP* screen = NULL;


void acquire_screen() {}
void release_screen() {}



int set_gfx_mode(int card, int w, int h, int v_w, int v_h) { return 0; }
int SCREEN_W = 0, SCREEN_H = 0;


int set_display_switch_mode(int mode) { return 0; }



bool exists(const char* filename) { return IsFileAvailable(filename, true); }



int makecol(int r, int g, int b) { return 0; }
int makecol_depth(int color_depth, int r, int g, int b) { return 0; }


int _rgb_r_shift_15, _rgb_g_shift_15, _rgb_b_shift_15,
    _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16,
    _rgb_r_shift_24, _rgb_g_shift_24, _rgb_b_shift_24,
    _rgb_r_shift_32, _rgb_g_shift_32, _rgb_b_shift_32;

int _rgb_scale_5[32], _rgb_scale_6[64];

void rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v) {}




int getpixel(BITMAP *bmp, int x, int y) { return 0; }
void putpixel(BITMAP *bmp, int x, int y, int color) {}
void vline(BITMAP *bmp, int x, int y1, int y2, int color) {}
void hline(BITMAP *bmp, int x1, int y, int x2, int color) {}
void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {}
void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {}
void circle(BITMAP *bmp, int x, int y, int radius, int color) {}
void clear_to_color(struct BITMAP *bitmap, int color) {}
void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y) {}
void draw_sprite_h_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y) {}

void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {}
void stretch_blit(BITMAP *s, BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h) {}
void masked_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {}

void clear_bitmap(BITMAP*) {}



unsigned long bmp_write_line(BITMAP *bmp, int line) { return 0; }
void bmp_unwrite_line(BITMAP* bmp) {}




void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor) {}
void set_trans_blender(int r, int g, int b, int a) {}
void set_add_blender (int r, int g, int b, int a) {}
void solid_mode() {}

int getr(int c) { return 0; }
int getg(int c) { return 0; }
int getb(int c) { return 0; }

int get_color_conversion() { return 0; }
void set_color_conversion(int mode) {}

int get_color_depth() { return 0; }
void set_color_depth(int depth) {}


void set_clip_rect(BITMAP *bitmap, int x1, int y_1, int x2, int y2) {}
void get_clip_rect(BITMAP *bitmap, int *x1, int *y_1, int *x2, int *y2) {}








void install_keyboard() {}
void remove_keyboard() {}
bool keypressed() { return false; }
int readkey() { return 0; }
int key[KEY_MAX];

void clear_keybuf() {}


