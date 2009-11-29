#ifndef SPRITES_H
#define SPRITES_H

#include <allegro.h>

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
		//resize load for weapon HUD
		class sprite* load_sprite(const char* sprite_name,int frames,char* folder,int v_depth, bool resize=false, int width=0, int height=0);
    void destroy_sprite(class sprite*);
};
extern class spritelist *sprites;

#endif /* SPRITES_H */
