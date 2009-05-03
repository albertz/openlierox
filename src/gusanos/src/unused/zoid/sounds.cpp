#include "sounds.h"
#include "level.h"
#include "engine.h"
#include <string>

class sound_list *sounds;

sound::~sound()
{
  if(prev!=NULL) prev->next=next;
  if(next!=NULL) next->prev=prev;
  else sounds->end=prev;
  destroy_sample(snd);
};

sound_list::sound_list()
{
	start=new sound;
	end = start;
	start->next = start->prev = NULL;
};

sound_list::~sound_list()
{

};

class sound* sound_list::load(const char* sound_name)
{
	class sound *curr;
	std::string tmp3;
	
	curr=start;
	while (curr->next!=NULL)
	{
		curr=curr->next;
		if (strcmp(curr->name,sound_name)==0)
		{
			return curr;
		};
	};
	
	end->next=new sound;
	curr=end->next;
	curr->prev=end;
	curr->next=NULL;
	end=curr;

  strcpy(end->name,sound_name);
  
  tmp3=map->path;
  tmp3+="/sounds/";
  tmp3+=sound_name;
  end->snd=load_sample(tmp3.c_str());
  if (end->snd==NULL)
  {
    tmp3=game->mod;
    tmp3+="/sounds/";
    tmp3+=sound_name;
    end->snd=load_sample(tmp3.c_str());
    if (end->snd==NULL)
    {
      tmp3="default/sounds/";
      tmp3+=sound_name;
      end->snd=load_sample(tmp3.c_str());
    };
  };
	return end;
};
