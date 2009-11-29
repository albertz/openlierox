#include "particles.h"
#include "sprites.h"
#include "level.h"
#include "sounds.h"
#include "explosions.h"
#include "weapons.h"
#include "player.h"
#include "lights.h"
#include "engine.h"
#include <cmath>

#include <fstream>
using std::ifstream;


struct particles partlist;
struct type_list *types;

void type_list::init()
{
	start=(struct part_type*)malloc(sizeof(struct part_type));
	end=start;
	start->next=start->prev=NULL;
};

void particles::init()
{
  int i;
	start = end = new struct particle;
	start->prev = start->next = NULL;
  for (i=0;i<4;i++)
  {
    layer_start[i] = layer_end[i] = new struct particle;
    layer_start[i]->layer_prev = layer_start[i]->layer_next = NULL;
  }
};

struct particle* particles::create_part(int x,int y,int xspd, int yspd,int owner,struct part_type *type)
{
	struct particle* tmp;
	
	tmp=(struct particle*) malloc(sizeof(struct particle));
	
	if (tmp!=NULL)
	{
		 //insert the new particle at the end of the list
		tmp->prev=end;
		tmp->next=NULL;
		end->next = tmp;
    end = tmp;

    tmp->layer_prev=layer_end[type->layer];
    tmp->layer_next=NULL;
    layer_end[type->layer]->layer_next = tmp;
    layer_end[type->layer] = tmp;
		
    //initialize properties
		end->type=type;
		end->x=x;
		end->y=y;
		end->xspd=xspd;
		end->yspd=yspd;
		end->owner=owner;
		//homing
		end->target=-1;
		end->framecount=0;
		end->currframe=0;
		end->obj_trail_time=0;
		end->exp_trail_time=0;
    end->ang=0;
    if (end->xspd>0) end->dir=1;
    else end->dir=-1;
		if (type->timeout_variation!=0)
			end->timeout=type->exptime-rand()%type->timeout_variation+type->timeout_variation/2;
		else end->timeout=type->exptime;
		
		if (type->bright_variation!=0)
		{
			int r,g,b,rnd;
			rnd=rand()%abs(type->bright_variation);
			if (type->bright_variation>0)
			{
				r=getr(type->color)+ rnd;
				g=getg(type->color)+ rnd;
				b=getb(type->color)+ rnd;
				
				if (r>255) r=255;
				if (g>255) g=255;
				if (b>255) b=255;
			};
			
			if (type->bright_variation<0)
			{
				r=getr(type->color)- rnd;
				g=getg(type->color)- rnd;
				b=getb(type->color)- rnd;
				
				if (r<0) r=0;
				if (g<0) g=0;
				if (b<0) b=0;
			};
			
			end->color=makecol(r,g,b);
		}
		else end->color=type->color;
		
		end->time=0;
		
		return end;
	}
	return NULL;
};

struct particle* particles::create_directional_part(int x,int y,int xspd, int yspd,int dir,int ang,int owner,struct part_type *type)
{
	struct particle* tmp;
	
	tmp=(struct particle*) malloc(sizeof(struct particle));
	
	if (tmp!=NULL)
	{
		 //insert the new particle at the end of the list
		tmp->prev=end;
		tmp->next=NULL;
		end->next = tmp;
    end=tmp;
    
    tmp->layer_prev=layer_end[type->layer];
    tmp->layer_next=NULL;
    layer_end[type->layer]->layer_next = tmp;
    layer_end[type->layer] = tmp;
		
		//initialize properties
		end->type=type;
		end->x=x;
		end->y=y;
		end->xspd=xspd;
		end->yspd=yspd;
		end->framecount=0;
		end->currframe=0;
		end->obj_trail_time=0;
		end->exp_trail_time=0;
		if (type->timeout_variation!=0)
			end->timeout=type->exptime-rand()%type->timeout_variation+type->timeout_variation/2;
		else end->timeout=type->exptime;
		end->time=0;
		end->dir=dir;
		end->ang=ang;
    end->owner=owner;
		//homing
		end->target=-1;
		return end;
	}
	return NULL;
};

void particles::shoot_part(int ang,int spd,int dir,int x,int y,int xspdadd,int yspdadd,int owner,struct part_type *type)
{
	int xspd,yspd;
	xspd=fixtoi(fixsin(ftofix(ang/1000.))*spd)*dir+xspdadd;
	yspd=fixtoi(fixcos(ftofix(ang/1000.))*spd)+yspdadd;
	if (type->directional==1)
		create_directional_part(x,y,xspd,yspd,dir,ang,owner,type);
	else create_part(x,y,xspd,yspd,owner,type);
};

void particles::render_particles(BITMAP* where,int layer)
{
	struct particle* tmp;
	tmp=layer_start[layer];
	while (tmp->layer_next!=NULL)
	{
		tmp=tmp->layer_next;
		if (tmp->type->visible==1)
		{
			if (tmp->type->sprt==NULL)
			{
				putpixel(where,tmp->x/1000,tmp->y/1000,tmp->color);
			}
			else
			{
				if (tmp->type->directional!=1)
				{
          if (abs(tmp->type->autorotate_speed)>0)
          {
            if (tmp->dir==1)
            pivot_sprite(where,tmp->type->sprt->img[tmp->currframe], tmp->x/1000, tmp->y/1000,tmp->type->sprt->img[0]->w/2, tmp->type->sprt->img[0]->h/2, itofix(tmp->ang));
            else
            pivot_sprite_v_flip(where,tmp->type->sprt->img[tmp->currframe], tmp->x/1000, tmp->y/1000,tmp->type->sprt->img[0]->w/2, tmp->type->sprt->img[0]->h/2, itofix(tmp->ang));
          }else
            draw_sprite(where,tmp->type->sprt->img[tmp->currframe],tmp->x/1000-tmp->type->sprt->img[0]->w/2,tmp->y/1000-tmp->type->sprt->img[0]->h / 2);
				} else
				{
					if (tmp->ang>127000)tmp->ang=127000;
					if (tmp->dir==1)
						draw_sprite(where,tmp->type->sprt->img[((((tmp->ang/1000))*tmp->type->framenum)/128)],tmp->x/1000-tmp->type->sprt->img[0]->w/2,tmp->y/1000-tmp->type->sprt->img[0]->h/2);
					else
						draw_sprite_h_flip(where,tmp->type->sprt->img[((((tmp->ang/1000))*tmp->type->framenum)/128)],tmp->x/1000-tmp->type->sprt->img[0]->w/2,tmp->y/1000-tmp->type->sprt->img[0]->h/2);  
				};
			};
		};
	};
};

void destroy_particles()
{
	struct particle *curr;
	curr=partlist.end;
	while (curr!=partlist.start)
	{
		curr=curr->prev;
		rem_part(curr->next);
	};
};

struct part_type* load_part(const char* type_name)
{
	ifstream fbuf;
	struct part_type *curr;
	std::string tmp1,tmp2,tmp3;
	std::string var,val;
	int i;
	
	//check for the requested type in the loaded types list
	curr=types->start;
	
	while (curr->next!=NULL)
	{
		curr=curr->next;
		if (strcmp(curr->name,type_name)==0)
		{
			return curr;
		};
	};
  
	//if it was not found create the new type
	types->end->next=(struct part_type*) malloc(sizeof(struct part_type));
	curr=types->end->next;
	curr->prev=types->end;
	curr->next=NULL;
	types->end=curr;
  
	//initizalize to default
	strcpy(curr->name,type_name);
	curr->shootnum=0;
	curr->wormshootnum=0;
	curr->shootnumtrail=0;
	curr->shootobj=NULL;
	curr->wormshootobj=NULL;
	curr->shootobjtrail=NULL;
	curr->shootspeed=8000;
	curr->wormshootspeed=0;
	curr->shootspeedtrail=0;
	curr->shootspeedrnd=2;
	curr->wormshootspeedrnd=2;
	curr->shootspeedrndtrail=2;
	curr->detect_range=0;
	curr->bounce=1000;
	curr->sprt=NULL;
	curr->gravity=0;
	curr->damage=0;
	curr->color=0;
	curr->exptime=0;
	curr->framenum=1;
	curr->framedelay=0;
	curr->traildelay=10;
	curr->expsnd=NULL;
	curr->animonground=0;
	curr->timeout_variation=0;
	curr->bright_variation=0;
	curr->laser_type=1;
	curr->blow_away=0;
	curr->destroy_exp=NULL;
	curr->exp_trail=NULL;
	curr->exptraildelay=0;
	curr->directional=0;
	curr->drawonmap=0;
	curr->visible=1;	
	curr->affected_by_motion=0;
	curr->affected_by_explosions=1000;
	curr->alpha=255;
  curr->remgnd=0;
  curr->autorotate_speed=0;
  curr->lens_radius=0;
  curr->layer=3;
  //Crate
  curr->give_weapon=-1;
  //homing
	curr->homing=0;
	curr->home_on_owner=0;
	curr->homing_detect_range=60;
	curr->max_homing_speed=1000;
	curr->homing_acceleration=20;
	curr->catch_target_sound=NULL;
	curr->shoot_keep_angle=0;

	
  //open the configuration file
  tmp3=map->path;
  tmp3+="/objects/";
  tmp3+=curr->name;
 	if (!exists(tmp3.c_str()))
	{
    tmp3= game->mod;
    tmp3+="/objects/";
    tmp3+=curr->name;
    //fbuf.open(tmp3.c_str());
    if (!exists(tmp3.c_str()))
    {
      tmp3="default/objects/";
      tmp3+=curr->name;
      //fbuf.open(tmp3.c_str());
    };
  };
  
  fbuf.open(tmp3.c_str());
  //allegro_message(tmp3.c_str());
	//if there were no errors...
	if (fbuf.is_open())
	{
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
						
					if ("gravity"==var) curr->gravity=atoi(val.c_str());
					else if ("timeout"==var) curr->exptime=atoi(val.c_str());
					else if ("laser_type"==var) curr->laser_type=atoi(val.c_str());
					else if ("directional"==var) curr->directional=atoi(val.c_str());
					else if ("blow_away_on_hit"==var) curr->blow_away=atoi(val.c_str());
					else if ("timeout_variation"==var) curr->timeout_variation=atoi(val.c_str());
					else if ("bright_variation"==var) curr->bright_variation=atoi(val.c_str());
					else if ("worm_detect_range"==var) curr->detect_range=atoi(val.c_str());
					else if ("shoot_number"==var) curr->shootnum=atoi(val.c_str());
					else if ("shoot_number_trail"==var) curr->shootnumtrail=atoi(val.c_str());
					else if ("shoot_number_on_worm"==var) curr->wormshootnum=atoi(val.c_str());
					else if ("exp_on_ground"==var) curr->expgnd=atoi(val.c_str());
					else if ("exp_on_worm"==var) curr->expworm=atoi(val.c_str());
					else if ("visible"==var) curr->visible=atoi(val.c_str());
					else if ("remove_on_worm"==var) curr->remworm=atoi(val.c_str());
          else if ("remove_on_ground"==var) curr->remgnd=atoi(val.c_str());
					else if ("bounce"==var) curr->bounce=atoi(val.c_str());
					else if ("animate_on_ground"==var) curr->animonground=atoi(val.c_str());
					else if ("damage"==var) curr->damage=atoi(val.c_str());
					else if ("shoot_speed"==var) curr->shootspeed=atoi(val.c_str());
					else if ("worm_obj_shoot_speed"==var) curr->wormshootspeed=atoi(val.c_str());
					else if ("shoot_speed_trail"==var) curr->shootspeedtrail=atoi(val.c_str());
					else if ("delay_between_trail_objects"==var) curr->traildelay=atoi(val.c_str());
					else if ("delay_between_trail_explosions"==var) curr->exptraildelay=atoi(val.c_str());
					else if ("color"==var)
					{
            set_color_depth(game->v_depth);
						curr->color=makecol(atoi(val.substr(0,3).c_str()),atoi(val.substr(3,3).c_str()),atoi(val.substr(6,3).c_str()));
					}
					else if ("worm_shoot_obj_speed_variation"==var) curr->wormshootspeedrnd=atoi(val.c_str());
					else if ("speed_variation_trail"==var) curr->shootspeedrndtrail=atoi(val.c_str());
					else if ("speed_variation"==var) curr->shootspeedrnd=atoi(val.c_str());
					else if ("shoot_object"==var && "null"!=val) curr->shootobj=load_part(val.c_str());
					else if ("shoot_object_trail"==var && "null"!=val) curr->shootobjtrail=load_part(val.c_str());
					else if ("shoot_object_on_worm"==var && "null"!=val) curr->wormshootobj=load_part(val.c_str());
					else if ("sprite"==var && "null"!=val) curr->sprt=sprites->load_sprite(val.c_str(),curr->framenum,game->mod,game->v_depth);
					else if ("number_of_frames"==var) curr->framenum=atoi(val.c_str());
					else if ("delay_between_frames"==var) curr->framedelay=atoi(val.c_str());
					else if ("draw_on_map"==var) curr->drawonmap=atoi(val.c_str());
					else if ("explosion_sound"==var && "null"!=val) curr->expsnd=sounds->load(val.c_str());
					else if ("explosion"==var && "null"!=val) curr->destroy_exp=load_exp(val.c_str());
					else if ("explosion_trail"==var && "null"!=val) curr->exp_trail=load_exp(val.c_str());
					else if ("affected_by_motion"==var) curr->affected_by_motion=atoi(val.c_str());
					else if ("affected_by_explosions"==var) curr->affected_by_explosions=atoi(val.c_str());
					else if ("alpha"==var) curr->alpha=atoi(val.c_str());
					else if ("autorotate_speed"==var) curr->autorotate_speed=atoi(val.c_str());
					else if ("lens_radius"==var) curr->lens_radius=atoi(val.c_str());
					else if ("layer"==var) curr->layer=atoi(val.c_str());
					//Crate
					else if ("give_weapon"==var) 
					  {
					    int j;
					    if ("random" == val)
					      curr->give_weapon = -2;
					    else
					      for (j=0; j < weaps->weap_count; j++)
					        if (val == weaps->num[j]->name)
					          curr->give_weapon=j;
					  }
					//homing
					else if ("homing"==var) curr->homing=atoi(val.c_str());
					else if ("homing_detect_range"==var) curr->homing_detect_range=atoi(val.c_str());
					else if ("home_on_owner"==var) curr->home_on_owner=atoi(val.c_str());
					else if ("max_homing_speed"==var) curr->max_homing_speed=atoi(val.c_str());
					else if ("catch_target_sound"==var && "null"!=val) curr->catch_target_sound=sounds->load(val.c_str());
					else if ("homing_acceleration"==var) curr->homing_acceleration=atoi(val.c_str());
					else if ("shoot_keep_angle"==var) curr->shoot_keep_angle=atoi(val.c_str());
				};
			};
		};
		fbuf.close();
	};
  if (curr->lens_radius!=0)
  {
    set_color_depth(game->v_depth);
    curr->effect_buf=create_bitmap(curr->lens_radius*2,curr->lens_radius*2);
  };
	return curr;
};

void dest_part(struct particle* tmp)
{
	unsigned int i;
	
	if(tmp!=partlist.start)
	{
		//check if this particle type shoots other particles when destroyed and act
		if (tmp->type->shootobj!=NULL)
		for(i=0;i<tmp->type->shootnum;i++)
			partlist.shoot_part((!tmp->type->shoot_keep_angle)*((rand()%1000)*256)+(tmp->type->shoot_keep_angle)*(tmp->ang),tmp->type->shootspeed-rand()%(tmp->type->shootspeedrnd)+tmp->type->shootspeedrnd/2,1,tmp->x,tmp->y,(tmp->xspd*tmp->type->affected_by_motion)/1000,(tmp->yspd*tmp->type->affected_by_motion)/1000,tmp->owner,tmp->type->shootobj);
		if (tmp->type->expsnd!=NULL)
			play_sample(tmp->type->expsnd->snd, *game->VOLUME, 127, 1000, 0);
		if (tmp->type->destroy_exp!=NULL)
		create_exp(tmp->x,tmp->y,tmp->type->destroy_exp);
		rem_part(tmp);
	};
};

void rem_part(struct particle* tmp)
{
	if(tmp!=partlist.start)
	{
		//fix the gap created in the chain(link list)
		tmp->prev->next=tmp->next;
    tmp->layer_prev->layer_next=tmp->layer_next;
		//check if we are deletting the last particle in the chain
		if(tmp->next!=NULL) tmp->next->prev=tmp->prev;
		else partlist.end=tmp->prev;
    if(tmp->layer_next!=NULL) tmp->layer_next->layer_prev=tmp->layer_prev;
		else partlist.layer_end[tmp->type->layer]=tmp->layer_prev;
		free(tmp);
	};
};

//Crate
bool check_position (int x1, int y1)
{
  int x2, y2;
  for (x2 = x1-2; x2 <= x1+2; x2++)
    for (y2 = y1-2; y2 <= y1+2; y2++)
      if (!(map->mat[map->material->line[y2][x2] + 1].particle_pass))
	      return false;
  return true;
}

void summon_bonus(struct part_type *item, int chance)
{
  if (rand()%10000 < chance)
    {
      int x, y, num = 0;
      do
     	{
	      num++;
	      x = (rand() % (map->material->w - 10) + 5);
	      y = (rand() % (map->material->h - 10) + 5);
	      if (check_position (x,y))
	      {
	        partlist.create_part(1000 * x, 1000 * y, 0, 0, -1, item);
          return;
  	    }			
	    }
      while (num < 5000);
    }
}

void summon_bonuses ()
{
  summon_bonus (game->weapon_box, *game->WEAPON_CHANCE);
  summon_bonus (game->health_box, *game->HEALTH_CHANCE);
}
//

void calc_particles()
{
	int cycles,g;
	bool destroyed;
	struct particle *tmp;
	
	tmp=partlist.start;
	while (tmp->next!=NULL)
	{
		tmp=tmp->next;
		destroyed=false;
		for (cycles=0;cycles<tmp->type->laser_type;cycles++)
		{
	
			int i,c;
      bool friendly;
      bool ffire=!game->teamplay || *game->FRIENDLYFIRE>=0;
			for (i=0;i<player_count;i++)
      {
      friendly=tmp->owner>=0 && player[tmp->owner] && player[i]->team==player[tmp->owner]->team;
			if (player[i]->active && (ffire || !friendly))
			{
				int dx,dy;
				dx= abs(tmp->x+tmp->xspd-player[i]->x);
				dy= abs((tmp->y+tmp->yspd)-(player[i]->y-4000));
				if (dx<tmp->type->detect_range)
				if (dy<tmp->type->detect_range)
				if(fixhypot(dx,dy)<tmp->type->detect_range)
				{
					if (tmp->type->wormshootobj!=NULL)
						for(c=0;c<tmp->type->wormshootnum;c++)
						{
							int spd_rnd;
							if (tmp->type->wormshootspeedrnd!=0)
								spd_rnd=(rand()%tmp->type->wormshootspeedrnd)-tmp->type->wormshootspeedrnd/2;
							else spd_rnd=0;
							partlist.shoot_part(((rand()%1000)*256),spd_rnd,1,player[i]->x,player[i]->y-4000,0,0,tmp->owner,tmp->type->wormshootobj);
						}
					if (tmp->type->blow_away!=0)
					{
						player[i]->yspd+=(int) (tmp->yspd*(tmp->type->blow_away/1000.));
						player[i]->xspd+=(int) (tmp->xspd*(tmp->type->blow_away/1000.));
					};
					//Crate
					if (tmp->type->give_weapon != -1)
					{
					  if (tmp->type->give_weapon == -2)
					    player[i]->weap[player[i]->curr_weap].weap = rand() % (game->weap_count);
					  else
					    player[i]->weap[player[i]->curr_weap].weap = tmp->type->give_weapon;
					  player[i]->weap[player[i]->curr_weap].shoot_time=0;
					  player[i]->weap[player[i]->curr_weap].ammo=weaps->num[player[i]->weap[player[i]->curr_weap].weap]->ammo;
					  player[i]->weap[player[i]->curr_weap].reloading=false;
					  player[i]->weap[player[i]->curr_weap].reload_time=0;
					};
					//
					if (player[i]->health>0 && player[i]->health<=*game->MAX_HEALTH)
          {
            if (ffire && friendly)
              player[i]->health-=(tmp->type->damage * *game->FRIENDLYFIRE)/1000;
            else
              player[i]->health-=tmp->type->damage;
            if (player[i]->health<=0)
              player[i]->killed_by=tmp->owner;
            if (player[i]->health>=*game->MAX_HEALTH)
              player[i]->health=*game->MAX_HEALTH;
          };
					if (tmp->type->expworm==1)
					{
						tmp=tmp->prev;
						dest_part(tmp->next);
						destroyed=true;
						break;
					}
					else if (tmp->type->remworm==1)
					{
						tmp=tmp->prev;
						rem_part(tmp->next);
						destroyed=true;
						break;
					};
				};
			};
      };
			if (destroyed) break;

			//homing
			if (tmp->type->homing)
			{
				if (tmp->type->homing==1 || (tmp->type->homing==2 && tmp->target==-1))
				//look for a target
				{
					int score=-1, lastscore=-1;
					int i, r;
					int dx, dy;
					bool detectable;
					for (i=0;i<player_count;i++)
					{
						score=-1;
						if (player[i]->active && (tmp->type->home_on_owner || i != tmp->owner)){
							dx=(player[i]->x-tmp->x)/1000;
							dy=((player[i]->y-4000)-tmp->y)/1000;
							if (abs(dx) < tmp->type->homing_detect_range)
							if (abs(dy) < tmp->type->homing_detect_range)
							{
		            r=fixtoi(fixhypot(itofix(dx),itofix(dy)));
								if (r<tmp->type->homing_detect_range)
								{
									score=r/*+rand()%3*/;
									if(!obs_line (map->material, tmp->x/1000 , tmp->y/1000 , player[i]->x/1000 , player[i]->y/1000 -4))
										score= score/3;
								};
							};
						};
						if (score!=-1 && (score<lastscore || lastscore==-1))
						{
							tmp->target=i;
							if (tmp->type->homing==2 && tmp->type->catch_target_sound!=NULL)
								play_sample(tmp->type->catch_target_sound->snd, *game->VOLUME, 127, 1000, 0);
						}
						lastscore=score;
					};
				}
				
				if (tmp->target!= -1)
				//move towards target
				{
					long long dx = player[tmp->target]->x - tmp->x, dy = player[tmp->target]->y-4000 - tmp->y;
					long long len = (long long) sqrt((double)(dx*dx + dy*dy));
					if((tmp->type->max_homing_speed == -1) || (tmp->xspd * dx + tmp->yspd * dy < tmp->type->max_homing_speed * len))
					{
						tmp->xspd += (dx * tmp->type->homing_acceleration) / len;
						tmp->yspd += (dy * tmp->type->homing_acceleration) / len;
					}
				}
				/*
				{
					int dx=(player[tmp->target]->x-tmp->x);
					int dy=((player[tmp->target]->y-4000)-tmp->y);
					if (abs(dx)>abs(dy))
					{
						if (dx>0)
							tmp->xspd+=tmp->type->homing_acceleration;
						else
							tmp->xspd-=tmp->type->homing_acceleration;
						if (tmp->yspd>0)
							tmp->yspd-=tmp->type->homing_acceleration;
						else
							tmp->yspd+=tmp->type->homing_acceleration;
					}
					else
					{
						if (dy>0)
							tmp->yspd+=tmp->type->homing_acceleration;
						else
							tmp->yspd-=tmp->type->homing_acceleration;
						if (tmp->xspd>0)
							tmp->xspd-=tmp->type->homing_acceleration;
						else
							tmp->xspd+=tmp->type->homing_acceleration;
					}
				}
				*/
			};

			tmp->yspd+=tmp->type->gravity;
			tmp->time++;
      
			if (tmp->framecount>tmp->type->framedelay)
			{
				tmp->currframe++;
				tmp->framecount=0;
				if (tmp->currframe>=tmp->type->framenum)
				{
				 tmp->currframe=0;
				};
			};
      
      tmp->ang+= tmp->dir * tmp->type->autorotate_speed;
      
			tmp->framecount++;
		
			if (tmp->obj_trail_time>tmp->type->traildelay)
			{
				int i;
				if (tmp->type->shootobjtrail!=NULL)
					for(i=0;i<tmp->type->shootnumtrail;i++)
					{
						int spd_rnd;
						if (tmp->type->shootspeedrndtrail!=0)
							spd_rnd=(rand()%tmp->type->shootspeedrndtrail)-tmp->type->shootspeedrndtrail/2;
						else spd_rnd=0;
						partlist.shoot_part(((rand()%1000)*256),tmp->type->shootspeedtrail-spd_rnd,1,tmp->x,tmp->y,0,0,tmp->owner,tmp->type->shootobjtrail);
					}
				tmp->obj_trail_time=0;
			};
			tmp->obj_trail_time++;
		
			if (tmp->type->exp_trail!=NULL)
			{
        if (tmp->exp_trail_time>tmp->type->exptraildelay)
        {
          create_exp(tmp->x,tmp->y,tmp->type->exp_trail);
          tmp->exp_trail_time=0;
        };
        tmp->exp_trail_time++;
			};
	
			//check if the particle's life has expired
			if (tmp->time>tmp->timeout && tmp->type->exptime!=-1)
			{
				tmp=tmp->prev;
				dest_part(tmp->next);
				break;
			};
		
			//'if not, check for collitions
			if (tmp->type->expgnd==1)
			{
				g=getpixel(map->material,(tmp->x+tmp->xspd)/1000,(tmp->y+tmp->yspd)/1000);
				if (!map->mat[g+1].particle_pass)
				{
					if (tmp->type->drawonmap==1)
						putpixel(map->mapimg,(tmp->x+tmp->xspd)/1000,(tmp->y+tmp->yspd)/1000,tmp->color);
					tmp=tmp->prev;
					dest_part(tmp->next);
					break;
				};
			}
			else if (tmp->type->remgnd==1)
			{
				g=getpixel(map->material,(tmp->x+tmp->xspd)/1000,(tmp->y+tmp->yspd)/1000);
				if (!map->mat[g+1].particle_pass)
				{
					if (tmp->type->drawonmap==1)
						putpixel(map->mapimg,(tmp->x+tmp->xspd)/1000,(tmp->y+tmp->yspd)/1000,tmp->color);
					tmp=tmp->prev;
					rem_part(tmp->next);
					break;
				};
			}
			else
			{
				g=getpixel(map->material,(tmp->x+tmp->xspd)/1000,tmp->y/1000);
				if (!map->mat[g+1].particle_pass)
				{
					tmp->xspd=(tmp->xspd*tmp->type->bounce)/-1000;
				};
				g=getpixel(map->material,tmp->x/1000,(tmp->y+tmp->yspd)/1000);
				if (!map->mat[g+1].particle_pass)
				{
					tmp->yspd=( tmp->yspd * tmp->type->bounce ) / -1000;
					tmp->xspd=( tmp->xspd * abs(tmp->type->bounce) ) / 1000;
					if (tmp->type->animonground!=1)
						tmp->framecount=0;
				};
			};
      tmp->x+=tmp->xspd;
			tmp->y+=tmp->yspd;
		};
  };
};

void particles::r_lens(BITMAP* where)
{
	struct particle* tmp;
	tmp=start;
	while (tmp->next!=NULL)
	{
		tmp=tmp->next;
		if (tmp->type->lens_radius!=0)
		{
      render_lens(tmp->x / 1000 , tmp->y / 1000,tmp->type->lens_radius,where,tmp->type->effect_buf);
		};
	};
};

void particles::player_removed(int n, bool remove_target_only)
{
	struct particle *tmp;
	
	tmp=partlist.start;
	if (remove_target_only)
		while (tmp->next!=NULL)
		{
			tmp=tmp->next;
			if (tmp->target==n)
				tmp->target= -1;
		}
	else
		while (tmp->next!=NULL)
		{
			tmp=tmp->next;
			if (tmp->owner==n)
				tmp->owner= -1;
			if (tmp->target==n)
				tmp->target= -1;
		}
};