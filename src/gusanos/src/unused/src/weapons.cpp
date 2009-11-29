#include "weapons.h"

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

class weapon* load_weap(const char* weap_name)
{
	FILE *fbuf;
	char *tmp1,tmp2[1024],tmp3[1024];
	char *var,*val;
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
  strcpy(tmp3,game->mod);
	strcat(tmp3,"/weapons/");
	strcat(tmp3,curr->filename);
	fbuf=fopen(tmp3,"rt");
	
	//if there were no errors...
	if (fbuf!=NULL)
	{
		fprintf(stderr, "Loading weapon \"%s\"\n", tmp3);
		//...parse the file
		while (!feof(fbuf))
		{
			tmp1=fgets(tmp2, sizeof(tmp2), fbuf);
			if (tmp1!=NULL)
			{
				i=0;
				if (tmp1[0]==' ')
				{
					//find an equal sign in the current line
					while (tmp1[i]!='=') i++;
					
					//split it
					var=strmid(tmp1,1,i-1);
					val=strmid(tmp1,i+1,strlen(tmp1)-i-2);
					
					if (strcmp("shoot_number",var)==0) curr->shoot_num=atoi(val);
					else if (strcmp("shoot_speed",var)==0) curr->shoot_spd=atoi(val);
					else if (strcmp("speed_variation",var)==0) curr->shoot_spd_rnd=atoi(val);
					else if (strcmp("delay_between_shots",var)==0) curr->shoot_times=atoi(val);
					else if (strcmp("distribution",var)==0) curr->distribution=atoi(val);
					else if (strcmp("aim_recoil",var)==0) curr->aim_recoil=atoi(val);
          else if (strcmp("firecone_timeout",var)==0) curr->firecone_timeout=atoi(val);
					else if (strcmp("recoil",var)==0) curr->recoil=atoi(val);
					else if (strcmp("affected_by_motion",var)==0) curr->affected_motion=atoi(val);
					else if (strcmp("name",var)==0) strcpy(curr->name,val);
					else if (strcmp("shoot_sound",var)==0 && strcmp("null",val)!=0) curr->shoot_sound=sounds->load(val);
          else if (strcmp("start_sound",var)==0 && strcmp("null",val)!=0) curr->start_sound=sounds->load(val);
          else if (strcmp("reload_sound",var)==0 && strcmp("null",val)!=0) curr->reload_sound=sounds->load(val);
          else if (strcmp("noammo_sound",var)==0 && strcmp("null",val)!=0) curr->noammo_sound=sounds->load(val);
          else if (strcmp("firecone",var)==0 && strcmp("null",val)!=0) curr->firecone=sprites->load_sprite(val,7,game->mod,game->v_depth);
					else if (strcmp("shoot_object",var)==0 && strcmp("null",val)!=0) curr->shoot_obj=load_part(val);
          else if (strcmp("ammo",var)==0) curr->ammo=atoi(val);
          else if (strcmp("reload_time",var)==0) curr->reload_time=atoi(val);
          else if (strcmp("autofire",var)==0) curr->autofire=atoi(val);
          else if (strcmp("lsight_intensity",var)==0) curr->lsight_intensity=atoi(val);
          else if (strcmp("lsight_fade",var)==0) curr->lsight_fade=atoi(val);
          else if (strcmp("start_delay",var)==0) curr->start_delay=atoi(val);
          else if (strcmp("create_on_release",var)==0) curr->create_on_release=load_part(val);
				};
			};
		};
		fclose(fbuf);
	} else
	{
		fprintf(stderr, "Could not find weapon \"%s\"\n", tmp3);
	}
	return curr;
};
