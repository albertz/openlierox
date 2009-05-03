#include "weapons.h"
#include "sounds.h"
#include "sprites.h"
#include "particles.h"
#include "level.h"
#include "engine.h"
#include "player.h"

#include <fstream>
#include <string>

using std::ifstream;

class weap_list *weaps;
  
weapon::weapon()
{
  shoot_num=0;
	shoot_obj=NULL;
	shoot_spd=0;
  distribution=0;
  shoot_times=0;
  shoot_spd_rnd=0;
  aim_recoil=0;
  recoil=0;
	affected_motion=0;
	shoot_sound=NULL;
  firecone=NULL;
  firecone_timeout=0;
  reload_sound=NULL;
  noammo_sound=NULL;
  start_sound=NULL;
  reload_time=0;
  ammo=0;
  aim_recoil=0;
  autofire=1;
  lsight_intensity=0;
  lsight_fade=0;
  start_delay=0;
  create_on_release=0;
  next=NULL;
  prev=NULL;
	//weapon HUD
	image=NULL;
};

weapon::~weapon()
{
  if(prev!=NULL) prev->next=next;
  if(next!=NULL) next->prev=prev;
  else weaps->end=prev;
};

weap_list::weap_list()
{
  weap_count=0;
	start=new weapon;
	end=start;
	start->next=start->prev=NULL;
};

weap_list::~weap_list()
{
  class weapon *curr;
	curr=end;
	
	while (curr->prev!=NULL)
	{
    curr=curr->prev;
		delete curr->next;
	};
  delete curr;
};

//Sort weapons in alphabetical order using selection sort
void weap_list::sort()
{
	int elements = weap_count;

	int low = 0;
	int t;
	for (int o = 0; o < elements; o++)
	{
		low = o;
		for (int i = o; i < elements; i++)
		{
			if (strcmp(num[i]->name, num[low]->name) < 0)
			{
				low = i;
			}
		}
		weapon *temp;
		temp = num[o];
		num[o] = num[low];
		num[low] = temp;
	}
}

class weapon* load_weap(const char* weap_name)
{
	ifstream fbuf;
	std::string tmp1,tmp2,tmp3;
	std::string var,val;
	int i;
	class weapon *curr;
	
	//check for the requested weapon in the loaded weapons list
	//curr=weaps->start;
  i=0;
  /*for(i=0;i<weaps->weap_count;i++)
  {
    if (strcmp(weaps->num[i]->filename,weap_name)==0)
		{
			return weaps->num[i];
		};
  };*/
	/*while (curr->next!=NULL)
	{
		curr=curr->next;
		if (strcmp(curr->filename,weap_name)==0)
		{
			return curr;
		};
	};*/
	
	//if it was not found create the weapon
	//weaps->end->next = curr = new weapon;
  weaps->weap_count++;
  //weaps->num=(weapon**)realloc(weaps->num,weaps->weap_count*sizeof(weapon*));
  weaps->num[weaps->weap_count-1]= new weapon; 
  curr=weaps->num[weaps->weap_count-1];
	//curr->prev=weaps->end;
	//curr->next=NULL;
	//weaps->end=curr;
	game->weap_count++;
  //weaps->weap_count++;
	//initizalize to default
	
	strcpy(curr->filename,weap_name);
	strcpy(curr->name,curr->filename);
	
	//open the configuration file
  tmp3=game->mod;
	tmp3+="/weapons/";
	tmp3+=curr->filename;
	fbuf.open(tmp3.c_str());
	
	//if there were no errors...
	if (fbuf.is_open())
	{
		//...parse the file
		while (!fbuf.eof())
		{
      getline(fbuf, tmp1);
			if (!tmp1.empty())
			{
				i=0;
				if (tmp1.c_str()[0]==' ')
				{
					//find an equal sign in the current line
          i=tmp1.find_first_of('=');
					//split it
					var=tmp1.substr(1,i-1);
          val=tmp1.substr(i+1);
					
					if ("shoot_number"==var) curr->shoot_num=atoi(val.c_str());
					else if ("shoot_speed"==var) curr->shoot_spd=atoi(val.c_str());
					else if ("speed_variation"==var) curr->shoot_spd_rnd=atoi(val.c_str());
					else if ("delay_between_shots"==var) curr->shoot_times=atoi(val.c_str());
					else if ("distribution"==var) curr->distribution=atoi(val.c_str());
					else if ("aim_recoil"==var) curr->aim_recoil=atoi(val.c_str());
          else if ("firecone_timeout"==var) curr->firecone_timeout=atoi(val.c_str());
					else if ("recoil"==var) curr->recoil=atoi(val.c_str());
					else if ("affected_by_motion"==var) curr->affected_motion=atoi(val.c_str());
					else if ("name"==var) strcpy(curr->name,val.c_str());
					else if ("shoot_sound"==var && "null"!=val) curr->shoot_sound=sounds->load(val.c_str());
          else if ("start_sound"==var && "null"!=val) curr->start_sound=sounds->load(val.c_str());
          else if ("reload_sound"==var && "null"!=val) curr->reload_sound=sounds->load(val.c_str());
          else if ("noammo_sound"==var && "null"!=val) curr->noammo_sound=sounds->load(val.c_str());
          else if ("firecone"==var && "null"!=val) curr->firecone=sprites->load_sprite(val.c_str(),7,game->mod,game->v_depth);
					else if ("shoot_object"==var && "null"!=val) curr->shoot_obj=load_part(val.c_str());
          else if ("ammo"==var) curr->ammo=atoi(val.c_str());
					//reload_multiplier
          else if ("reload_time"==var) curr->reload_time=100*atoi(val.c_str());
          else if ("autofire"==var) curr->autofire=atoi(val.c_str());
          else if ("lsight_intensity"==var) curr->lsight_intensity=atoi(val.c_str());
          else if ("lsight_fade"==var) curr->lsight_fade=atoi(val.c_str());
          else if ("start_delay"==var) curr->start_delay=atoi(val.c_str());
          else if ("create_on_release"==var) curr->create_on_release=load_part(val.c_str());
					//weapon HUD
					else if ("image"==var && "null"!=val) curr->image=sprites->load_sprite(("weapons/"+val).c_str(),1,game->mod,game->v_depth,true,90,60);
				};
			};
		};
		fbuf.close();
	};
	return curr;
};

void s_playerweap::shoot(int x, int y, int xspd, int yspd, int aim, int dir, int owner)
{
  shoot_time=weaps->num[weap]->shoot_times+1;
  ammo--;
  if (weaps->num[weap]->shoot_num!=0)
  {
    int dist,spd_rnd,xof,yof;
    
    for (int i=0;i<weaps->num[weap]->shoot_num;i++)
    {
      dist=((rand()%1000)*weaps->num[weap]->distribution)-weaps->num[weap]->distribution/2*1000;
      if (weaps->num[weap]->shoot_spd_rnd!=0)
        spd_rnd=(rand()%weaps->num[weap]->shoot_spd_rnd)-weaps->num[weap]->shoot_spd_rnd/2;
      else spd_rnd=0;
      xof=fixtoi(fixsin(ftofix((aim-dist)/1000.))*(weaps->num[weap]->shoot_obj->detect_range+1000))*dir;
      yof=fixtoi(fixcos(ftofix((aim-dist)/1000.))*(weaps->num[weap]->shoot_obj->detect_range+1000));
      partlist.shoot_part(aim-dist,weaps->num[weap]->shoot_spd-spd_rnd,dir,x+xof,y-4000+yof,(xspd*weaps->num[weap]->affected_motion)/1000,(yspd*weaps->num[weap]->affected_motion)/1000,player[owner]->local_slot,weaps->num[weap]->shoot_obj);
    }
  }
  if (weaps->num[weap]->shoot_sound!=NULL)
  {
    play_sample(weaps->num[weap]->shoot_sound->snd, *game->VOLUME, 127, 1000, 0);
  }
  if (weaps->num[weap]->aim_recoil!=0)
    player[owner]->aim_recoil_speed+=(100*weaps->num[weap]->aim_recoil);
  if (weaps->num[weap]->recoil!=0)
  {
    player[owner]->xspd += -fixtoi(fixsin(ftofix(aim/1000.))*weaps->num[weap]->recoil)*dir;
    player[owner]->yspd += -fixtoi(fixcos(ftofix(aim/1000.))*weaps->num[weap]->recoil);
  }
  player[owner]->curr_firecone=weaps->num[weap]->firecone;
  player[owner]->firecone_time=weaps->num[weap]->firecone_timeout;

};

void scanWeapsDir()
{
	std::string tmp;
	struct al_ffblk *file=new struct al_ffblk;
	
	// (weapons load only on init_game so they won't change with map)
	/*tmp=map->path;
	tmp+="/weapons/*.wpn";
	if (al_findfirst(tmp.c_str(), file, FA_ARCH) == 0)
		do
			load_weap(file->name);
		while(al_findnext(file)==0);
	al_findclose(file);*/

	tmp=game->mod;
	tmp+="/weapons/*.wpn";
	if (al_findfirst(tmp.c_str(), file, FA_ARCH) == 0)
		do
			load_weap(file->name);
		while(al_findnext(file)==0);
	al_findclose(file);
	
	delete file;
};
