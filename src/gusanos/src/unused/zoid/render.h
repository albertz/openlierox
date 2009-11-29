#ifndef RENDER_H
#define RENDER_H

#include <allegro.h>

extern BITMAP* screen_buffer;

bool CanBeSeen(int x, int y, int w, int h);

#endif /* RENDER_H */
