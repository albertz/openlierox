#include "explosions.h"
#include "level.h"
#include "particles.h"
#include "sounds.h"
#include "player.h"
#include "water.h"
#include "render.h"
#include "engine.h"
#include "sprites.h"
#include "lights.h"
#ifdef AAFBLEND
  #include <fblend.h>
#endif


#include <fstream>
using std::ifstream;

class exp_type_list *exp_types;
class exp_list *exps;

exp_type_list::exp_type_list()
{
	start=(class exp_type*)malloc(sizeof(class exp_type));
	end=start;
	start->next=start->prev=NULL;
};

exp_type_list::~exp_type_list()
{
  class exp_type *curr;
	curr=end;
	
	while (curr->prev!=NULL)
	{
    curr=curr->prev;
		delete curr->next;
	};
  delete curr;
};

exp_type::exp_type()
{
	wormshootnum=0;
	wormshootobj=NULL;
	wormshootspeed=0;
	wormshootspeedrnd=2;
	detect_range=0;
	sprt=NULL;
	damage=0;
	color=0;
	framenum=1;
	snd=NULL;
	bright_variation=0;
	blow_away=0;
	hole=NULL;
	affect_worm=1;
  affect_particles=0;
  next=prev=NULL;
  light_effect=0;
  light_color=makecol(255,255,255);
  light_fadeness=200;
  spd_multiply=1000;
  flash=0;
  flash_radius=0;
	//mat weapon
	hole_mat=-1;
	hole_strength=70;
	draw_sprite=NULL;
};

exp_type::~exp_type()
{
  if(prev!=NULL) prev->next=next;
  if(next!=NULL) next->prev=prev;
  else exp_types->end=prev;
};

exp_list::exp_list()
{
	start = end = new explosion;
	start->prev = start->next = NULL;
};

exp_list::~exp_list()
{
  class explosion *curr;
	curr=end;
	
	while (curr->prev!=NULL)
	{
    curr=curr->prev;
		delete curr->next;
	};
  delete curr;
};

explosion::explosion()
{
  light=NULL;
};

explosion::~explosion()
{
  if (light!=NULL) destroy_bitmap(light);
  if(prev!=NULL) prev->next=next;
  if(next!=NULL) next->prev=prev;
  else exps->end=prev;
};

class exp_type* load_exp(const char* exp_name)
{
	ifstream fbuf;
	class exp_type *curr;
	std::string tmp1,tmp2,tmp3;
	std::string var,val;
	int i;
	
	
	//check for the requested type in the loaded types list
	curr=exp_types->start;
	
	while (curr->next!=NULL)
	{
		curr=curr->next;
		if (strcmp(curr->name,exp_name)==0)
		{
			return curr;
		};
	};
	
	//if it was not found create the new type
	exp_types->end->next=new exp_type;
	curr=exp_types->end->next;
	curr->prev=exp_types->end;
	curr->next=NULL;
	exp_types->end=curr;
	
	//initizalize to default
  strcpy(curr->name,exp_name);

  //open the configuration file
  tmp3=map->path;
  tmp3+="/objects/explosions/";
  tmp3+=curr->name;
  if (!exists(tmp3.c_str()))
  {
    tmp3=game->mod;
    tmp3+="/objects/explosions/";
    tmp3+=curr->name;
    if (!exists(tmp3.c_str()))
    {
      tmp3="default/objects/explosions/";
      tmp3+=curr->name;
    };
  };
  fbuf.open(tmp3.c_str());
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
					
					if ("timeout"==var) curr->timeout=atoi(val.c_str());
					else if ("blow_away_on_hit"==var) curr->blow_away=atoi(val.c_str());
					else if ("bright_variation"==var) curr->bright_variation=atoi(val.c_str());
					else if ("worm_detect_range"==var) curr->detect_range=atoi(val.c_str());
					else if ("shoot_number_on_worm"==var) curr->wormshootnum=atoi(val.c_str());
					else if ("damage"==var) curr->damage=atoi(val.c_str());
					else if ("color"==var)
					{
						curr->color=makecol(atoi(val.substr(0,3).c_str()),atoi(val.substr(3,3).c_str()),atoi(val.substr(6,3).c_str()));
					}
          else if ("light_color"==var)
					{
						curr->light_color=makecol(atoi(val.substr(0,3).c_str()),atoi(val.substr(3,3).c_str()),atoi(val.substr(6,3).c_str()));
					}
					else if ("worm_shoot_obj_speed_variation"==var) curr->wormshootspeedrnd=atoi(val.c_str());
					else if ("shoot_object_on_worm"==var && "null"!=val) curr->wormshootobj=load_part(val.c_str());
					else if ("number_of_frames"==var) curr->framenum=atoi(val.c_str());
					else if ("sprite"==var && "null"!=val) curr->sprt=sprites->load_sprite(val.c_str(),curr->framenum,game->mod,game->v_depth);
					else if ("hole"==var && "null"!=val) curr->hole=sprites->load_sprite(val.c_str(),1,game->mod,game->v_depth);
					else if ("sound"==var && "null"!=val) curr->snd=sounds->load(val.c_str());
					else if ("affect_worm"==var) curr->affect_worm=atoi(val.c_str());
          else if ("affect_particles"==var) curr->affect_particles=atoi(val.c_str());
          else if ("light_effect"==var) curr->light_effect=atoi(val.c_str());
          else if ("light_fadeness"==var) curr->light_fadeness=atoi(val.c_str());
          else if ("speed_multiply"==var) curr->spd_multiply=atoi(val.c_str());
          else if ("flash"==var) curr->flash=atoi(val.c_str());
          else if ("flash_radius"==var) curr->flash_radius=atoi(val.c_str());
					//mat weapon
					else if ("hole_material"==var) curr->hole_mat=atoi(val.c_str());
					else if ("hole_strength"==var) curr->hole_strength=atoi(val.c_str());					
					else if ("draw_sprite"==var && "null"!=val) curr->draw_sprite=sprites->load_sprite(val.c_str(),1,game->mod,game->v_depth);					
				};
			};
		};
		fbuf.close();
	};
	return curr;
};

void create_exp(int x,int y,class exp_type *type)
{
	int i,c;
	int dx,dy,m;

	struct particle *tmp;
	exps->end->next=new explosion;

	if (exps->end->next!=NULL)
	{
		//insert the new particle at the end of the list
		exps->end->next->prev=exps->end;
		exps->end->next->next=NULL;
		exps->end = exps->end->next;

		//initialize properties
		exps->end->type=type;
		exps->end->x=x;
		exps->end->y=y;
		exps->end->timeout=type->timeout;
		exps->end->currframe=0;
		
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
			
			exps->end->color=makecol(r,g,b);
		}
		else exps->end->color=type->color;
		
		exps->end->time=0;
		
		if (type->hole!=NULL) 
			//mat weapon
			dig_hole(type->hole->img[0],exps->end->x/1000-type->hole->img[0]->w/2,exps->end->y/1000-type->hole->img[0]->h/2, exps->end->type->hole_mat,exps->end->type->hole_strength, exps->end->type->draw_sprite);
		
		for (i=0;i<player_count;i++)
		{
			if (player[i]->active && exps->end->type->affect_worm==1){
				dx= player[i]->x-exps->end->x;
				dy=(player[i]->y-4000)-exps->end->y;
				if (abs(dx) < exps->end->type->detect_range)
				if (abs(dy) < exps->end->type->detect_range)
				{
					m=fixtoi(fixhypot(itofix(dx),itofix(dy)));;
					if(m < exps->end->type->detect_range && m != 0)
					{
						if (exps->end->type->wormshootobj!=NULL)
							for(c=0;c<exps->end->type->wormshootnum;c++)
							{
								int spd_rnd;
								if (exps->end->type->wormshootspeedrnd!=0)
									spd_rnd=(rand()%exps->end->type->wormshootspeedrnd)-exps->end->type->wormshootspeedrnd/2;
								else spd_rnd=0;
								partlist.shoot_part(((rand()%1000)*256),exps->end->type->wormshootspeed-spd_rnd,1,player[i]->x,player[i]->y-4000,0,0,i,exps->end->type->wormshootobj);
							}
						if (exps->end->type->blow_away!=0)
						{
							player[i]->yspd+=(exps->end->type->blow_away*dy)/m;
							player[i]->xspd+=(exps->end->type->blow_away*dx)/m;
						};
						player[i]->health-=exps->end->type->damage;
					};
				};
			};
		};
    if (type->affect_particles==1)
    {
      tmp=partlist.start;
      while (tmp->next!=NULL)
      {
        tmp=tmp->next;
        if (tmp->type->affected_by_explosions!=0)
        {
          dx=tmp->x-exps->end->x;
          dy=tmp->y-exps->end->y;
          if (abs(dx) < exps->end->type->detect_range)
          if (abs(dy) < exps->end->type->detect_range)
          {
            m=fixtoi(fixhypot(itofix(dx),itofix(dy)));;
            if(m < exps->end->type->detect_range && m!=0)
            {
              if (exps->end->type->blow_away!=0)
              {
                tmp->yspd+=((((exps->end->type->blow_away*dy)/m)*tmp->type->affected_by_explosions)/tmp->type->laser_type)/1000;
                tmp->yspd=(tmp->yspd*exps->end->type->spd_multiply)/1000;
                tmp->xspd+=((((exps->end->type->blow_away*dx)/m)*tmp->type->affected_by_explosions)/tmp->type->laser_type)/1000;
                tmp->xspd=(tmp->xspd*exps->end->type->spd_multiply)/1000;
              };
            };
          };
        };
      };
		};
    if (exps->end->type->flash>0 && exps->end->type->flash_radius>0)
    {
      int r;
      for (i=0;i<player_count;i++)
      {
        if (player[i]->active){
          dx=(player[i]->x-exps->end->x)/1000;
          dy=((player[i]->y-4000)-exps->end->y)/1000;
          if (abs(dx) < exps->end->type->flash_radius)
          if (abs(dy) < exps->end->type->flash_radius)
          {
            r=fixtoi(fixhypot(itofix(dx),itofix(dy)));
            if (r<exps->end->type->flash_radius)
            if (!obs_line ( map->material, exps->end->x/1000 , exps->end->y/1000 , player[i]->x/1000 , player[i]->y/1000 -4))
            {
              player[i]->flash+=exps->end->type->flash-(exps->end->type->flash * r) / exps->end->type->flash_radius;
            };
          };
        };
      };
    };
    if (exps->end->type->light_effect!=0)
    {
      exps->end->light=NULL;
      if (getpixel(map->material,exps->end->x/1000,exps->end->y/1000)!=-1)
      {
        int c;
        c=51200/exps->end->type->light_fadeness+1;
        exps->end->light=create_bitmap(c,c);
        clear_to_color(exps->end->light,0);
        if(exps->end->type->light_effect==1)
          render_exp_light(exps->end->x/1000,exps->end->y/1000,exps->end->type->light_color,exps->end->type->light_fadeness,5,bitmap_mask_color(map->buffer),exps->end->light ,map->material);  
        else
          render_exp_light(exps->end->x/1000,exps->end->y/1000,exps->end->type->light_color,exps->end->type->light_fadeness,5,-1,exps->end->light ,map->material);  
      };
    };
	};
};

void dest_exp(class explosion* tmp)
{
	if(tmp!=exps->start)
	{
    delete tmp;
	};
};

void calc_exps()
{
	class explosion *tmp,*tmp2;
	tmp=exps->start;
	while (tmp->next!=NULL)
	{
		tmp=tmp->next;
		tmp2=tmp;

		
		if (tmp->type->framenum>1 && tmp->type->timeout>0)
			tmp->currframe=tmp->time/(tmp->type->timeout/tmp->type->framenum);
    if (tmp->currframe>=tmp->type->framenum) tmp->currframe=tmp->type->framenum-1;
    tmp->time++;
		if (tmp->time>=tmp->type->timeout)
		{
			tmp2=tmp->prev;
			dest_exp(tmp);
			tmp=tmp2;
		};
	};
};

//mat weapon
void dig_hole(BITMAP* image,int x,int y, int mat, int hole_strength, sprite *draw_sprite)
{
	int y2,x2,col,g,m;
	col=makecol(100,100,100);
  if (game->v_depth!=8)
  {
    for (y2=0;y2<image->h;y2++)
    for (x2=0;x2<image->w;x2++)
    {
      g=getpixel(image,x2,y2);
			m=getpixel(map->material,x+x2,y+y2)+1;
      if (g==makecol(0,0,0))
			{
				if (map->mat[m].strength<hole_strength /*&& ((map->mat[m].destroyed_into+1 != m)||(mat>-1))*/)
				{
					putpixel(map->material,x+x2,y+y2,mat==-1 ? map->mat[m].destroyed_into: mat);
					if (map->has_water) check_hole_sides(x+x2,y+y2);
					if (draw_sprite)
						putpixel(map->mapimg,x+x2,y+y2,getpixel(draw_sprite->img[0],x2,y2));
					else
						if (!map->mat[map->mat[m].destroyed_into+1].flows)
							putpixel(map->mapimg,x+x2,y+y2,getpixel(map->background,x+x2,y+y2));					
					//check_sunlight(x+x2,y+y2);
				}
			}
      else if (g==makecol(255,255,255))
			{
				// (map->mat[m].chreact!=NULL) needed? chreact materials that are destroyed into themself will loop?
        if ((map->mat[m].strength<hole_strength) /*&& ((map->mat[m].destroyed_into+1 != m)||(mat>-1)||(map->mat[m].chreact!=NULL))*/)
        {
          putpixel(map->material,x+x2,y+y2,mat==-1 ? map->mat[m].destroyed_into: mat);
          if (map->has_water) check_hole_sides(x+x2,y+y2);
					if (draw_sprite)
						putpixel(map->mapimg,x+x2,y+y2,getpixel(draw_sprite->img[0],x2,y2));
					else
						if (!map->mat[map->mat[m].destroyed_into+1].flows)
							putpixel(map->mapimg,x+x2,y+y2,getpixel(map->background,x+x2,y+y2));					
          if (map->mat[m].chreact!=NULL) partlist.create_part((x+x2)*1000+500,(y+y2)*1000+500,0, 0, -1,map->mat[m].chreact);
          //check_sunlight(x+x2,y+y2);
        }
			}
      else if (g==makecol(128,0,0))
			{
				if (map->mat[m].strength<hole_strength /*&& ((map->mat[m].destroyed_into+1 != m)||(mat>-1))*/)
				{
					if (!getpixel(map->background,x+x2,y+y2) == makecol(255,0,255))
					{
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
						set_trans_blender(0, 0, 0, 64);
						if (draw_sprite)
							putpixel(map->mapimg,x+x2,y+y2,getpixel(draw_sprite->img[0],x2,y2));
						else
							if (!map->mat[map->mat[m].destroyed_into+1].flows)
								putpixel(map->mapimg,x+x2,y+y2,getpixel(map->background,x+x2,y+y2));
						solid_mode();
					}
				};
			}
    };
  } else
  {
    for (y2=0;y2<image->h;y2++)
    for (x2=0;x2<image->w;x2++)
    {
      if (getpixel(image,x2,y2)!=0)
			{
				m= getpixel(map->material,x+x2,y+y2)+1;
				if (map->mat[m].strength<hole_strength/* && ((map->mat[m].destroyed_into+1 != m)||(mat>-1))*/)
				{
	        putpixel(map->material,x+x2,y+y2,mat==-1 ? map->mat[m].destroyed_into: mat);
					putpixel(map->mapimg,x+x2,y+y2,col);
					solid_mode();
				};
			}
    };    
  };
};

void draw_explosion(BITMAP* image,int x,int y)
{
	int y2,x2,i,o;
  int MASK_COLOR;
  MASK_COLOR=bitmap_mask_color(image);

  if (game->v_depth!=8)
  {
    for (y2=0;y2<image->h;y2++)
    for (x2=0;x2<image->w;x2++)
    {
      o=getpixel(image,x2,y2);
      if (getpixel(image,x2,y2)!=MASK_COLOR)
      {
        i=getpixel(map->material,x+x2,y+y2);
        if (map->mat[i+1].draw_exps) putpixel(map->buffer,x+x2,y+y2,o);
      };
    };
  } else
  {
    for (y2=0;y2<image->h;y2++)
    for (x2=0;x2<image->w;x2++)
    {
      o=getpixel(image,x2,y2);
      if (o!=0)
      {
        i=getpixel(map->material,x+x2,y+y2);
        if (map->mat[i+1].draw_exps) putpixel(map->buffer,x+x2,y+y2,o);
      };
    };
  };
};

void render_exps()
{
	class explosion* tmp;
	tmp=exps->start;
	while (tmp->next!=NULL)
	{
		tmp=tmp->next;
		if (tmp->type->sprt==NULL)
		{
			putpixel(map->buffer,tmp->x/ 1000,tmp->y/ 1000,tmp->color);
		}
		else
		{

        if(CanBeSeen(tmp->x/1000,tmp->y/1000,tmp->type->sprt->img[0]->w,tmp->type->sprt->img[0]->h))
				draw_explosion(tmp->type->sprt->img[tmp->currframe],tmp->x/1000-tmp->type->sprt->img[0]->w/2,tmp->y/1000-tmp->type->sprt->img[0]->h/2);

			//}
			//else
			//	draw_explosion(tmp->type->sprt->image,tmp->x/1000-tmp->type->sprt->imgbuf->w/2,tmp->y/1000-tmp->type->sprt->imgbuf->h/2);
		};
    if (tmp->type->light_effect==1 && tmp->light)
    {
      if(CanBeSeen(tmp->x/1000,tmp->y/1000,tmp->light->w,tmp->light->h))
      if(game->v_depth==32)
      {
        #ifdef AAFBLEND
        fblend_add(tmp->light,map->buffer,tmp->x/1000-tmp->light->w/2,tmp->y/1000-tmp->light->h/2,255-(tmp->time*255)/tmp->type->timeout);
        #endif
      }
      else
      {
        drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
        set_add_blender(0,0,0,255-(tmp->time*255)/tmp->type->timeout);
        draw_trans_sprite(map->buffer,tmp->light,tmp->x/1000-tmp->light->w/2,tmp->y/1000-tmp->light->h/2);
        solid_mode();
      };
    };
	};
};

void render_paralax_lights(BITMAP* where, int _player, struct s_viewport viewport)
{
	class explosion* tmp;
	tmp=exps->start;
  worm *p=player[local_player[_player]];
	while (tmp->next!=NULL)
	{
    tmp=tmp->next;
    if (tmp->type->light_effect==2 && tmp->light)
    {
      if(CanBeSeen(tmp->x/1000,tmp->y/1000,tmp->light->w,tmp->light->h))
      {
        //fblend_add(tmp->light,where,viewport.x+(tmp->x/1000-p->xview)-tmp->light->w/2,viewport.y+tmp->y/1000-p->yview-tmp->light->h/2,255-(tmp->time*255)/tmp->type->timeout);
        drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
        set_add_blender(0,0,0,255-(tmp->time*255)/tmp->type->timeout);
        draw_trans_sprite(where,tmp->light,viewport.x+(tmp->x/1000-p->xview)-tmp->light->w/2,viewport.y+tmp->y/1000-p->yview-tmp->light->h/2);
        solid_mode();
      };
    };
	};
};
