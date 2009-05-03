#include "sprites.h"
#include "level.h"
#include "gfx.h"
#include <string>

class spritelist *sprites;

spritelist::spritelist()
{
	start=new sprite;
	end = start;
	start->next = start->prev = NULL;
};

spritelist::~spritelist()
{
  class sprite *curr;
	curr=end;
	
	while (curr->prev!=NULL)
	{
    curr=curr->prev;
		delete curr->next;
	};
  delete curr;
};

sprite::~sprite()
{
  int i;
  if(prev!=NULL) prev->next=next;
  if(next!=NULL) next->prev=prev;
  else sprites->end=prev;
  for(i=0;i<framenum;i++)
  {
    if (img[i] != NULL)
			destroy_bitmap(img[i]);
  }
};

//resize load for weapon HUD
class sprite* spritelist::load_sprite(const char* sprite_name,int frames,char* folder,int v_depth, bool resize, int width, int height)
{
	class sprite *curr;
	std::string tmp3;
	BITMAP* tmp_bmp;
	
	curr=start;
	
	while (curr->next!=NULL)
	{
		curr=curr->next;
		if (strcmp(curr->sprite_name,sprite_name)==0)
		{
			return curr;
		};
	};

	end->next=new sprite;
	curr=end->next;
	curr->prev=end;
	curr->next=NULL;
	end=curr;
  end->framenum=frames;
	strcpy(end->sprite_name,sprite_name);
  
  set_color_conversion(COLORCONV_TOTAL | COLORCONV_KEEP_TRANS);
  tmp3=map->path;
  tmp3+="/sprites/";
  tmp3+=curr->sprite_name;
  //set_color_depth(32);
  tmp_bmp=loadImage(tmp3.c_str(),0);
  if (tmp_bmp==NULL)
  {
    tmp3=folder;
    tmp3+="/sprites/";
    tmp3+=curr->sprite_name;
    //set_color_depth(32);
    tmp_bmp=loadImage(tmp3.c_str(),0);
    if (tmp_bmp==NULL)
    {
      tmp3="default/sprites/";
      tmp3+=curr->sprite_name;
      tmp_bmp=loadImage(tmp3.c_str(),0);
    };
  };
  set_color_conversion(COLORCONV_TOTAL);
	if (tmp_bmp!=NULL)
	{
		int i;
		for(i=0;i<end->framenum;i++)
		{
			if (!resize)
			{
				end->img[i]=create_bitmap(tmp_bmp->w/frames,tmp_bmp->h);
				blit(tmp_bmp,end->img[i],(tmp_bmp->w/frames)*i,0,0,0,tmp_bmp->w/frames,tmp_bmp->h);
			}
			else
			{
				end->img[i]=create_bitmap(width,height);
				stretch_blit(tmp_bmp,end->img[i],width*i,0,tmp_bmp->w,tmp_bmp->h,0,0,width,height);
			}
		};
    destroy_bitmap(tmp_bmp);
	}
	else
	{
		//can be better ?
		for(int i=0;i<end->framenum;i++)
			end->img[i]=NULL;
		delete end;
		return NULL;
	}
	return end;
};
