#ifndef __OLX__2XSAI_H__
#define __OLX__2XSAI_H__

#include <stdint.h>

int Init_2xSaI(int depth);
void Super2xSaI(ALLEGRO_BITMAP * src, ALLEGRO_BITMAP * dest, int s_x, int s_y, int d_x, int d_y, int w, int h);
void Super2xSaI_ex(uint8_t *src, uint32_t src_pitch, uint8_t *unused, ALLEGRO_BITMAP *dest, uint32_t width, uint32_t height);

void SuperEagle(ALLEGRO_BITMAP * src, ALLEGRO_BITMAP * dest, int s_x, int s_y, int d_x, int d_y, int w, int h);
void SuperEagle_ex(uint8_t *src, uint32_t src_pitch, uint8_t *unused, ALLEGRO_BITMAP *dest, uint32_t width, uint32_t height);

#endif
