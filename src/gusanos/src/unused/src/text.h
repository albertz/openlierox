#ifndef TEXT_H
#define TEXT_H

#include <allegro.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
#include "sprites.h"
#include "engine.h"

struct fnt
{
	BITMAP* img;
	int chrw, chrh;
	void ld_fnt(const char* filename);
	void ld_fnt_8(const char* filename);
	void draw_string(BITMAP* bitmap, const char* str,int x,int y,bool outline);
};

char *ucase(char *str) ;
char *lcase(char *str);

char* strmid(char* src, int start, int len);

void rem_spaces(char* str);

#endif /* TEXT_H */
