#include "text.h"
#include "gfx.h"
#include <string>

char *ucase(const char *str) 
{
	int i = 0;
	char *newstr = new char[strlen(str) + 1];
	while(str[i]) {
		newstr[i] = toupper(str[i]);
		i++;
	}
	newstr[strlen(str)] = '\0';
	return(newstr);
}

char *lcase(const char *str) 
{
	int i = 0;
	char *newstr = new char[strlen(str) + 1];
	while(str[i]) {
		newstr[i] = tolower(str[i]);
		i++;
	}
	newstr[strlen(str)] = '\0';
	return(newstr);
}


char* strmid(const char* src, int start, int len)
{
	char *tmp;
	unsigned int i;
	tmp=(char*)malloc(len+1);
	for(i=0;i<len;i++)
	{
		tmp[i]=src[i+start];
	};
	tmp[len]='\0';
	return tmp;
};

void rem_spaces(const char* str)
{
  int i,j;
  j=0;
  for (i=0;i<=strlen(str);i++)
  {
    if (str[i]!=' ')
    {
      //str[j]=str[i];
      j++;
    };
  };
  return;
};


void fnt::ld_fnt(const char *filename)
{
	img=loadImage(filename,0);
	chrw=img->w/256;
	chrh=img->h;
};

void fnt::ld_fnt_8(const char *filename)
{
  int x,y;
  BITMAP *tmp_bmp;
  set_color_depth(32);
  tmp_bmp=loadImage(filename,0);
  set_color_depth(8);
  img=create_bitmap(tmp_bmp->w,tmp_bmp->h);
  blit(tmp_bmp,img,0,0,0,0,tmp_bmp->w,tmp_bmp->h);
  for(x=0;x<tmp_bmp->w;x++)
  for(y=0;y<tmp_bmp->h;y++)
    if (getpixel(tmp_bmp,x,y)==MASK_COLOR_32)
      putpixel(img,x,y,0);
	chrw=img->w/256;
	chrh=img->h;
  destroy_bitmap(tmp_bmp);
};

void fnt::draw_string(BITMAP* bitmap, const char* str, int x, int y, bool outline)
{
	unsigned int i,col;
  col=bitmap_mask_color(img);
	for (i=0;i<strlen(str);i++)
  {
    int _x,_y;
		masked_blit(img, bitmap, str[i]*chrw,0,x+i*chrw,y,chrw,chrh);
    if (outline)
    for (_x=0;_x<chrw;_x++)
    for (_y=0;_y<chrh;_y++)
    {
      if (getpixel(img,str[i]*chrw+_x,_y)!=col)
      {
        if (getpixel(img,str[i]*chrw+_x+1,_y)==col)
        {
          putpixel(bitmap,x+1+_x+i*chrw,y+_y,0);
        };
        if (getpixel(img,str[i]*chrw+_x-1,_y)==col)
        {
          putpixel(bitmap,x-1+_x+i*chrw,y+_y,0);
        };
        if (getpixel(img,str[i]*chrw+_x,_y+1)==col)
        {
          putpixel(bitmap,x+_x+i*chrw,y+_y+1,0);
        };
        if (getpixel(img,str[i]*chrw+_x,_y-1)==col)
        {
          putpixel(bitmap,x+_x+i*chrw,y+_y-1,0);
        };
      };
    };
  };
};

