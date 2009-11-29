#ifndef SPRITES_H
#define SPRITES_H
#include <allegro.h>
#include <string.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
//#include "engine.h"

class sprite
{
  public:
    
	BITMAP* img[255];
	int framenum;
	char sprite_name[512];
	class sprite* next;
	class sprite* prev;
  ~sprite();
};

class spritelist
{
  public:
    
    class sprite *start, *end;
    spritelist();
    ~spritelist();
    class sprite* load_sprite(char* sprite_name,int frames,char* folder,int v_depth);
    void destroy_sprite(class sprite*);
};
extern class spritelist *sprites;

#endif /* SPRITES_H */
