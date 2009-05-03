#include "particles.h"

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
	start = end = new struct particle;
	start->prev = start->next = NULL;
};

struct particle* particles::create_part(int x,int y,int xspd, int yspd,worm *player,struct part_type *type)
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
		
    //initialize properties
		end->type=type;
		end->x=x;
		end->y=y;
		end->xspd=xspd;
		end->yspd=yspd;
		end->player=player;
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

struct particle* particles::create_directional_part(int x,int y,int xspd, int yspd,int dir,int ang,struct worm *player,struct part_type *type)
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
		return end;
	}
	return NULL;
};

void particles::shoot_part(int ang,int spd,int dir,int x,int y,int xspdadd,int yspdadd,worm *player,struct part_type *type)
{
	int xspd,yspd;
	xspd=fixtof(fixsin(ftofix(ang/1000.)))*spd*dir+xspdadd;
	yspd=fixtof(fixcos(ftofix(ang/1000.)))*spd+yspdadd;
	if (type->directional==1)
		create_directional_part(x,y,xspd,yspd,dir,ang,player,type);
	else create_part(x,y,xspd,yspd,player,type);
};

void particles::render_particles(BITMAP* where)
{
	struct particle* tmp;
	tmp=start;
	while (tmp->next!=NULL)
	{
		tmp=tmp->next;
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
	FILE *fbuf;
	struct part_type *curr;
	char *tmp1,tmp2[1024],tmp3[1024];
	char *var,*val;
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

	
 //open the configuration file
  strcpy(tmp3,game->mod);
	strcat(tmp3,"/objects/");
	strcat(tmp3,curr->name);
	fbuf=fopen(tmp3,"rt");
	if (fbuf==NULL)
	{
    strcpy(tmp3,"default/objects/");
    strcat(tmp3,curr->name);
    fbuf=fopen(tmp3,"rt");
  };
	//if there were no errors...
	if (fbuf!=NULL)
	{
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
					if (tmp1[strlen(tmp1)-1]=='\n') tmp1[strlen(tmp1)-1]='\0';
					val=strmid(tmp1,i+1,strlen(tmp1)-i);
          rem_spaces(var);
          rem_spaces(val);
						
					if (strcmp("gravity",var)==0) curr->gravity=atoi(val);
					else if (strcmp("timeout",var)==0) curr->exptime=atoi(val);
					else if (strcmp("laser_type",var)==0) curr->laser_type=atoi(val);
					else if (strcmp("directional",var)==0) curr->directional=atoi(val);
					else if (strcmp("blow_away_on_hit",var)==0) curr->blow_away=atoi(val);
					else if (strcmp("timeout_variation",var)==0) curr->timeout_variation=atoi(val);
					else if (strcmp("bright_variation",var)==0) curr->bright_variation=atoi(val);
					else if (strcmp("worm_detect_range",var)==0) curr->detect_range=atoi(val);
					else if (strcmp("shoot_number",var)==0) curr->shootnum=atoi(val);
					else if (strcmp("shoot_number_trail",var)==0) curr->shootnumtrail=atoi(val);
					else if (strcmp("shoot_number_on_worm",var)==0) curr->wormshootnum=atoi(val);
					else if (strcmp("exp_on_ground",var)==0) curr->expgnd=atoi(val);
					else if (strcmp("exp_on_worm",var)==0) curr->expworm=atoi(val);
					else if (strcmp("visible",var)==0) curr->visible=atoi(val);
					else if (strcmp("remove_on_worm",var)==0) curr->remworm=atoi(val);
          else if (strcmp("remove_on_ground",var)==0) curr->remgnd=atoi(val);
					else if (strcmp("bounce",var)==0) curr->bounce=atoi(val);
					else if (strcmp("animate_on_ground",var)==0) curr->animonground=atoi(val);
					else if (strcmp("damage",var)==0) curr->damage=atoi(val);
					else if (strcmp("shoot_speed",var)==0) curr->shootspeed=atoi(val);
					else if (strcmp("worm_obj_shoot_speed",var)==0) curr->wormshootspeed=atoi(val);
					else if (strcmp("shoot_speed_trail",var)==0) curr->shootspeedtrail=atoi(val);
					else if (strcmp("delay_between_trail_objects",var)==0) curr->traildelay=atoi(val);
					else if (strcmp("delay_between_trail_explosions",var)==0) curr->exptraildelay=atoi(val);
					else if (strcmp("color",var)==0)
					{
            set_color_depth(game->v_depth);
						curr->color=makecol(atoi(strmid(val,0,3)),atoi(strmid(val,3,3)),atoi(strmid(val,6,3)));
					}
					else if (strcmp("worm_shoot_obj_speed_variation",var)==0) curr->wormshootspeedrnd=atoi(val);
					else if (strcmp("speed_variation_trail",var)==0) curr->shootspeedrndtrail=atoi(val);
					else if (strcmp("speed_variation",var)==0) curr->shootspeedrnd=atoi(val);
					else if (strcmp("shoot_object",var)==0 && strcmp("null",val)!=0) curr->shootobj=load_part(val);
					else if (strcmp("shoot_object_trail",var)==0 && strcmp("null",val)!=0) curr->shootobjtrail=load_part(val);
					else if (strcmp("shoot_object_on_worm",var)==0 && strcmp("null",val)!=0) curr->wormshootobj=load_part(val);
					else if (strcmp("sprite",var)==0 && strcmp("null",val)!=0) curr->sprt=sprites->load_sprite(val,curr->framenum,game->mod,game->v_depth);
					else if (strcmp("number_of_frames",var)==0) curr->framenum=atoi(val);
					else if (strcmp("delay_between_frames",var)==0) curr->framedelay=atoi(val);
					else if (strcmp("draw_on_map",var)==0) curr->drawonmap=atoi(val);
					else if (strcmp("explosion_sound",var)==0 && strcmp("null",val)!=0) curr->expsnd=sounds->load(val);
					else if (strcmp("explosion",var)==0 && strcmp("null",val)!=0) curr->destroy_exp=load_exp(val);
					else if (strcmp("explosion_trail",var)==0 && strcmp("null",val)!=0) curr->exp_trail=load_exp(val);
					else if (strcmp("affected_by_motion",var)==0) curr->affected_by_motion=atoi(val);
					else if (strcmp("affected_by_explosions",var)==0) curr->affected_by_explosions=atoi(val);
					else if (strcmp("alpha",var)==0) curr->alpha=atoi(val);
          else if (strcmp("autorotate_speed",var)==0) curr->autorotate_speed=atoi(val);
          else if (strcmp("lens_radius",var)==0) curr->lens_radius=atoi(val);
				};
			};
		};
		fclose(fbuf);
	} else
	{
		fprintf(stderr, "Could not find part \"%s\"\n", tmp3);
	}
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
			partlist.shoot_part(((rand()%1000)*256),tmp->type->shootspeed-rand()%(tmp->type->shootspeedrnd)+tmp->type->shootspeedrnd/2,1,tmp->x,tmp->y,tmp->xspd*(tmp->type->affected_by_motion/1000.),tmp->yspd*(tmp->type->affected_by_motion/1000.),tmp->player,tmp->type->shootobj);
		if (tmp->type->expsnd!=NULL)
			play_sample(tmp->type->expsnd->snd, *game->VOLUME, 127, 1000, 0);
		//fix the gap created in the chain(link list)
		tmp->prev->next=tmp->next;
		//check if we are deletting the last particle in the chain
		if(tmp->next!=NULL) tmp->next->prev=tmp->prev;
		else partlist.end=tmp->prev;
		if (tmp->type->destroy_exp!=NULL)
		create_exp(tmp->x,tmp->y,tmp->type->destroy_exp);
		free(tmp);
	};
};

void rem_part(struct particle* tmp)
{
	if(tmp!=partlist.start)
	{
		//fix the gap created in the chain(link list)
		tmp->prev->next=tmp->next;
		//check if we are deletting the last particle in the chain
		if(tmp->next!=NULL) tmp->next->prev=tmp->prev;
		else partlist.end=tmp->prev;
		free(tmp);
	};
};

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
			for (i=0;i<player_count;i++)
			if (player[i]->active)
			{
				int dx,dy;
				dx= abs(tmp->x+tmp->xspd-player[i]->x);
				dy= abs((tmp->y+tmp->yspd)-(player[i]->y-4000));
				if (dx<tmp->type->detect_range)
				if (dy<tmp->type->detect_range)
				if(fixhypot(dx,dy)<tmp->type->detect_range)
				{
					if (tmp->type->wormshootobj!=NULL)
						for(c=0;c<tmp->type->wormshootnum;c++) partlist.shoot_part(((rand()%1000)*256),tmp->type->wormshootspeed-rand()%(tmp->type->wormshootspeedrnd)+tmp->type->wormshootspeedrnd/2,1,player[i]->x,player[i]->y-4000,0,0,tmp->player,tmp->type->wormshootobj);
					if (tmp->type->blow_away!=0)
					{
						player[i]->yspd+=(int) (tmp->yspd*(tmp->type->blow_away/1000.));
						player[i]->xspd+=(int) (tmp->xspd*(tmp->type->blow_away/1000.));
					};
					player[i]->health-=tmp->type->damage;
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
			if (destroyed) break;

			tmp->x+=tmp->xspd;
			tmp->y+=tmp->yspd;
		
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
      
      if (tmp->dir==1) tmp->ang+=tmp->type->autorotate_speed;
      else tmp->ang-=tmp->type->autorotate_speed;
      
			tmp->framecount++;
		
			if (tmp->obj_trail_time>tmp->type->traildelay)
			{
				int i;
				if (tmp->type->shootobjtrail!=NULL)
					for(i=0;i<tmp->type->shootnumtrail;i++) partlist.shoot_part(((rand()%1000)*256),tmp->type->shootspeedtrail-rand()%(tmp->type->shootspeedrndtrail)+tmp->type->shootspeedrndtrail/2,1,tmp->x,tmp->y,0,0,tmp->player,tmp->type->shootobjtrail);
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
					{
						putpixel(map->mapimg,(tmp->x+tmp->xspd)/1000,(tmp->y+tmp->yspd)/1000,tmp->color);
					}
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
					{
						putpixel(map->mapimg,(tmp->x+tmp->xspd)/1000,(tmp->y+tmp->yspd)/1000,tmp->color);
					}
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
					tmp->xspd*=-1.*(tmp->type->bounce/1000.);
				};
				g=getpixel(map->material,tmp->x/1000,(tmp->y+tmp->yspd)/1000);
				if (!map->mat[g+1].particle_pass)
				{
					tmp->yspd*=-1.*(tmp->type->bounce/1000.);
					tmp->xspd*=(abs(tmp->type->bounce)/1000.);
					if (tmp->type->animonground!=1)
						tmp->framecount=0;
				};
			};
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
      render_lens(tmp->x / 1000 , tmp->y / 1000,tmp->type->lens_radius,where);
		};
	};
};
