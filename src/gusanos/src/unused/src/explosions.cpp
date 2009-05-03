#include "explosions.h"

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
	FILE *fbuf;
	class exp_type *curr;
	char *tmp1,tmp2[1024],tmp3[1024];
	char *var,*val;
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
  strcpy(tmp3,game->mod);
	strcat(tmp3,"/objects/explosions/");
	strcat(tmp3,curr->name);
	fbuf=fopen(tmp3,"rt");
	if (fbuf==NULL)
  {
    strcpy(tmp3,"default/objects/explosions/");
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
					val=strmid(tmp1,i+1,strlen(tmp1)-i-2);
					
					if (strcmp("timeout",var)==0) curr->timeout=atoi(val);
					else if (strcmp("blow_away_on_hit",var)==0) curr->blow_away=atoi(val);
					else if (strcmp("bright_variation",var)==0) curr->bright_variation=atoi(val);
					else if (strcmp("worm_detect_range",var)==0) curr->detect_range=atoi(val);
					else if (strcmp("shoot_number_on_worm",var)==0) curr->wormshootnum=atoi(val);
					else if (strcmp("damage",var)==0) curr->damage=atoi(val);
					else if (strcmp("color",var)==0)
					{
						curr->color=makecol(atoi(strmid(val,0,3)),atoi(strmid(val,3,3)),atoi(strmid(val,6,3)));
					}
          else if (strcmp("light_color",var)==0)
					{
						curr->light_color=makecol(atoi(strmid(val,0,3)),atoi(strmid(val,3,3)),atoi(strmid(val,6,3)));
					}
					else if (strcmp("worm_shoot_obj_speed_variation",var)==0) curr->wormshootspeedrnd=atoi(val);
					else if (strcmp("shoot_object_on_worm",var)==0 && strcmp("null",val)!=0) curr->wormshootobj=load_part(val);
					else if (strcmp("number_of_frames",var)==0) curr->framenum=atoi(val);
					else if (strcmp("sprite",var)==0 && strcmp("null",val)!=0) curr->sprt=sprites->load_sprite(val,curr->framenum,game->mod,game->v_depth);
					else if (strcmp("hole",var)==0 && strcmp("null",val)!=0) curr->hole=sprites->load_sprite(val,1,game->mod,game->v_depth);
					else if (strcmp("sound",var)==0 && strcmp("null",val)!=0) curr->snd=sounds->load(val);
					else if (strcmp("affect_worm",var)==0) curr->affect_worm=atoi(val);
          else if (strcmp("affect_particles",var)==0) curr->affect_particles=atoi(val);
          else if (strcmp("light_effect",var)==0) curr->light_effect=atoi(val);
          else if (strcmp("light_fadeness",var)==0) curr->light_fadeness=atoi(val);
          else if (strcmp("speed_multiply",var)==0) curr->spd_multiply=atoi(val);
				};
			};
		};
		fclose(fbuf);
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
			dig_hole(type->hole->img[0],exps->end->x/1000-type->hole->img[0]->w/2,exps->end->y/1000-type->hole->img[0]->h/2);
		
		for (i=0;i<player_count;i++)
		{
			if (player[i]->active && exps->end->type->affect_worm==1){
				dx= player[i]->x-exps->end->x;
				dy=(player[i]->y-4000)-exps->end->y;
				if (abs(dx) < exps->end->type->detect_range)
				if (abs(dy) < exps->end->type->detect_range)
				{
					m=fixhypot(dx,dy);
					if(m < exps->end->type->detect_range)
					{
						if (exps->end->type->wormshootobj!=NULL)
							for(c=0;c<exps->end->type->wormshootnum;c++) partlist.shoot_part(((rand()%1000)*256),exps->end->type->wormshootspeed-rand()%(exps->end->type->wormshootspeedrnd)+exps->end->type->wormshootspeedrnd/2,1,player[i]->x,player[i]->y-4000,0,0,player[i],exps->end->type->wormshootobj);
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
          dy=(tmp->y)-exps->end->y;
          if (abs(dx) < exps->end->type->detect_range)
          if (abs(dy) < exps->end->type->detect_range)
          {
            m=fixhypot(dx,dy);
            if(m < exps->end->type->detect_range)
            {
              if (exps->end->type->blow_away!=0)
              {
                tmp->yspd+=((((exps->end->type->blow_away*dy)/m)*tmp->type->affected_by_explosions)/tmp->type->laser_type)/1000;
                tmp->yspd*=exps->end->type->spd_multiply/1000.;
                tmp->xspd+=((((exps->end->type->blow_away*dx)/m)*tmp->type->affected_by_explosions)/tmp->type->laser_type)/1000;
                tmp->xspd*=exps->end->type->spd_multiply/1000.;
              };
            };
          };
        };
      };
		};
    if (exps->end->type->light_effect==1)
    {
      exps->end->light=NULL;
      if (getpixel(map->material,exps->end->x/1000,exps->end->y/1000)!=-1)
      {
        int c;
        c=512/(exps->end->type->light_fadeness/100.)+1;
        exps->end->light=create_bitmap(c,c);
        clear_to_color(exps->end->light,0);
        render_exp_light(exps->end->x/1000,exps->end->y/1000,exps->end->type->light_color,exps->end->type->light_fadeness,5,exps->end->light ,map->material);  
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

void dig_hole(BITMAP* image,int x,int y)
{
	int y2,x2,col,g,m;
	col=makecol(100,100,100);
  if (game->v_depth!=8)
  {
    for (y2=0;y2<image->h;y2++)
    for (x2=0;x2<image->w;x2++)
    {
      g=_getpixel16(image,x2,y2);
      if (g==makecol(0,0,0))
      if (map->mat[getpixel(map->material,x+x2,y+y2)+1].destroyable)
      {
        putpixel(map->material,x+x2,y+y2,1);
        if(map->has_water) check_hole_sides(x+x2,y+y2);
        putpixel(map->mapimg,x+x2,y+y2,getpixel(map->background,x+x2,y+y2));
        //check_sunlight(x+x2,y+y2);
      };
      if (g==makecol(255,255,255))
      {
        m=getpixel(map->material,x+x2,y+y2)+1;
        if (map->mat[m].chreact!=NULL)
        {
          putpixel(map->material,x+x2,y+y2,1);
          if(map->has_water) check_hole_sides(x+x2,y+y2);
          putpixel(map->mapimg,x+x2,y+y2,getpixel(map->background,x+x2,y+y2));
          partlist.create_part((x+x2)*1000+500,(y+y2)*1000+500,0, 0, NULL,map->mat[m].chreact);
          //check_sunlight(x+x2,y+y2);
        };
        if (map->mat[m].destroyable)
        {
          putpixel(map->material,x+x2,y+y2,1);
          if(map->has_water) check_hole_sides(x+x2,y+y2);
          putpixel(map->mapimg,x+x2,y+y2,getpixel(map->background,x+x2,y+y2));
          //check_sunlight(x+x2,y+y2);
        };
      };
      if (g==makecol(128,0,0))
      if (map->mat[getpixel(map->material,x+x2,y+y2)+1].destroyable)
      {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
        set_trans_blender(0, 0, 0, 64);
        putpixel(map->mapimg,x+x2,y+y2,getpixel(map->background,x+x2,y+y2));
        solid_mode();
      };
    };
  } else
  {
    for (y2=0;y2<image->h;y2++)
    for (x2=0;x2<image->w;x2++)
    {
      if (getpixel(image,x2,y2)!=0)
      if (map->mat[getpixel(map->material,x+x2,y+y2)+1].destroyable)
      {
        putpixel(map->material,x+x2,y+y2,1);
        putpixel(map->mapimg,x+x2,y+y2,col);
        solid_mode();
      };
    };    
  };
};

void draw_explosion(BITMAP* image,int x,int y)
{
	int y2,x2,i,o;
	bool is_on_player_view = false;
	for(i=0;i<local_players;i++)
	{
		if (x+image->w/2>player[local_player[i]]->xview && x-image->w/2<player[local_player[i]]->xview+game->viewport[i].w)
		if (y+image->h/2>player[local_player[i]]->yview && y-image->h/2<player[local_player[i]]->yview+game->viewport[i].h)
			is_on_player_view=true;
	};
	if(is_on_player_view)
  if (game->v_depth!=8)
  {
    for (y2=0;y2<image->h;y2++)
    for (x2=0;x2<image->w;x2++)
    {
      o=getpixel(image,x2,y2);
      if (getpixel(image,x2,y2)!=MASK_COLOR_16)
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
			//if (tmp->type->framenum>1)
			//{
				//blit(tmp->type->sprt->image,tmp->type->sprt->imgbuf,(tmp->type->sprt->image->w/tmp->type->framenum)*(tmp->currframe),0,0,0,tmp->type->sprt->imgbuf->w,tmp->type->sprt->imgbuf->h);
				//draw_sprite(map->buffer,tmp->type->sprt->imgbuf,tmp->x/1000-tmp->type->sprt->imgbuf->w/2,tmp->y/1000-tmp->type->sprt->imgbuf->h/2);
				draw_explosion(tmp->type->sprt->img[tmp->currframe],tmp->x/1000-tmp->type->sprt->img[0]->w/2,tmp->y/1000-tmp->type->sprt->img[0]->h/2);

			//}
			//else
			//	draw_explosion(tmp->type->sprt->image,tmp->x/1000-tmp->type->sprt->imgbuf->w/2,tmp->y/1000-tmp->type->sprt->imgbuf->h/2);
		};
    if (tmp->type->light_effect==1 && tmp->light)
    {
      drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
      set_add_blender(0,0,0,255-(tmp->time*255)/tmp->type->timeout);
      draw_trans_sprite(map->buffer,tmp->light,tmp->x/1000-tmp->light->w/2,tmp->y/1000-tmp->light->h/2);
      solid_mode();
    };
	};
};
