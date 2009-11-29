#ifndef TEXT_H
#define TEXT_H

#include <allegro.h>

struct fnt
{
	BITMAP* img;
	int chrw, chrh;
	void ld_fnt(const char* filename);
	void ld_fnt_8(const char* filename);
	void draw_string(BITMAP* bitmap, const char* str,int x,int y,bool outline);
};

char *ucase(const char *str) ;
char *lcase(const char *str);

char* strmid(const char* src, int start, int len);

void rem_spaces(const char* str);

#endif /* TEXT_H */
