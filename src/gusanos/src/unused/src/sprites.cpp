#include "sprites.h"

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
    destroy_bitmap(img[i]);
  };
};

class sprite* spritelist::load_sprite(char* sprite_name,int frames,char* folder,int v_depth)
{
	class sprite *curr;
	char tmp3[1024];
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
  strcpy(tmp3,folder);
	strcat(tmp3,"/sprites/");
	strcat(tmp3,curr->sprite_name);
  set_color_depth(32);
	tmp_bmp=load_bmp(tmp3,0);
  if (tmp_bmp==NULL)
  {
    strcpy(tmp3,"default/sprites/");
    strcat(tmp3,curr->sprite_name);
    tmp_bmp=load_bmp(tmp3,0);
  };
	if (tmp_bmp!=NULL)
	{
		int i,x2,y2;
    BITMAP* tmp_bmp2;
    set_color_depth(v_depth);
    tmp_bmp2=create_bitmap(tmp_bmp->w,tmp_bmp->h);
    if(v_depth==8)
    {
      blit(tmp_bmp,tmp_bmp2,0,0,0,0,tmp_bmp->w,tmp_bmp->h);
      for (y2=0;y2<tmp_bmp->h;y2++)
      for (x2=0;x2<tmp_bmp->w;x2++)
      {
        if (getpixel(tmp_bmp,x2,y2)==MASK_COLOR_32)
        {
          putpixel(tmp_bmp2,x2,y2,0);
        };
      };
    }else
    blit(tmp_bmp,tmp_bmp2,0,0,0,0,tmp_bmp->w,tmp_bmp->h);
		for(i=0;i<end->framenum;i++)
		{
			end->img[i]=create_bitmap(tmp_bmp2->w/frames,tmp_bmp2->h);
			blit(tmp_bmp2,end->img[i],(tmp_bmp2->w/frames)*i,0,0,0,tmp_bmp2->w/frames,tmp_bmp2->h);
		};
    destroy_bitmap(tmp_bmp);
    destroy_bitmap(tmp_bmp2);
		
	};
	return end;
};
