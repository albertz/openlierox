#include "level.h"
#include "particles.h"
#include "engine.h"
#include "text.h"
#include "lights.h"
#include "console.h"
#include "network.h"
#include "player.h"
#include "explosions.h"
#include "water.h"
#include "gfx.h"
#include <iostream>
#include <string>

class level* map;

/*void antialias()
{
  int x,y;
  for (x=0;x<map->material->w;x++)
  for (y=0;y<map->material->w;y++)
  {
    if (map->mat[1+getpixel(map->material,x,y)].destroyable)
    {
      if(!map->mat[1+getpixel(map->material,x+1,y)].destroyable)
      {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
        set_trans_blender(0, 0, 0, 100);
        putpixel(map->mapimg,x,y,getpixel(map->background,x,y));
        solid_mode();
      };
      if(!map->mat[1+getpixel(map->material,x-1,y)].destroyable)
      {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
        set_trans_blender(0, 0, 0, 100);
        putpixel(map->mapimg,x,y,getpixel(map->background,x,y));
        solid_mode();
      };
      if(!map->mat[1+getpixel(map->material,x,y+1)].destroyable)
      {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
        set_trans_blender(0, 0, 0, 100);
        putpixel(map->mapimg,x,y,getpixel(map->background,x,y));
        solid_mode();
      };
      if(!map->mat[1+getpixel(map->material,x,y+1)].destroyable)
      {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
        set_trans_blender(0, 0, 0, 100);
        putpixel(map->mapimg,x,y,getpixel(map->background,x,y));
        solid_mode();
      };
    };
  };
};*/

level::level()
{
  mapimg=NULL;
	layer=NULL;
	material=NULL;
	buffer=NULL;
  background=NULL;
  water_buffer=NULL; 
  paralax=NULL;
  light_layer=NULL;
  has_water=false;
  spawnpoint_count=0;
  strcpy(this->path," ");
  int i;
  for(i=0;i<32;i++)
  {
    spawnpoint[i].x=spawnpoint[i].y=spawnpoint[i].team=0;
  };
	//mat weapon
  for(i=0;i<256;i++)
  {
    mat[i].worm_pass=true;
    mat[i].particle_pass=true;
    mat[i].flows=false;
    mat[i].can_breath=true;
    mat[i].draw_exps=true;
    mat[i].blocks_light=false;
    mat[i].destroys_water=false;
    mat[i].creates_water=false;
    mat[i].chreact=NULL;
    mat[i].damage=0;
		mat[i].strength=100;
		mat[i].destroyed_into=1;
  };
  
};

//mat weapon
void level::init_materials()
{
	//level boundaries
  mat[-1+1].worm_pass=false;
  mat[-1+1].particle_pass=false;
  mat[-1+1].flows=false;
  mat[-1+1].can_breath=true;
  mat[-1+1].blocks_light=true;
  mat[-1+1].damage=0;
	mat[-1+1].strength=100;
	mat[-1+1].destroyed_into=-1;
  
	//rock
  mat[0+1].worm_pass=false;
  mat[0+1].particle_pass=false;
  mat[0+1].flows=false;
  mat[0+1].can_breath=true;
  mat[0+1].draw_exps=false;
  mat[0+1].blocks_light=true;
  mat[0+1].damage=0;
	mat[0+1].strength=100;
	mat[0+1].destroyed_into=1;
  
	//air
  mat[1+1].worm_pass=true;
  mat[1+1].particle_pass=true;
  mat[1+1].flows=false;
  mat[1+1].can_breath=true;
  mat[1+1].damage=0;
	mat[1+1].strength=10;
	mat[1+1].destroyed_into=1;
  
	//dirt
  mat[2+1].worm_pass=false;
  mat[2+1].particle_pass=false;
  mat[2+1].flows=false;
  mat[2+1].can_breath=true;
  mat[2+1].draw_exps=false;
  mat[2+1].blocks_light=true;
  mat[2+1].damage=0;
	mat[2+1].strength=40;
	mat[2+1].destroyed_into=1;
  
	//water
  mat[3+1].worm_pass=true;
  mat[3+1].particle_pass=true;
  mat[3+1].flows=true;
  mat[3+1].can_breath=false;
  mat[3+1].damage=0;
	mat[3+1].strength=30;
	mat[3+1].destroyed_into=3;
  
	//glass
  mat[4+1].worm_pass=false;
  mat[4+1].particle_pass=false;
  mat[4+1].flows=false;
  mat[4+1].can_breath=true;
  mat[4+1].draw_exps=false;
  mat[4+1].blocks_light=false;
  mat[4+1].chreact=load_part("chreact.obj");
  mat[4+1].damage=0;
	mat[4+1].strength=40;
	mat[4+1].destroyed_into=1;
  
	//Water spawner
  mat[5+1].creates_water=true;
	mat[5+1].strength=100;
	mat[5+1].destroyed_into=1;
	//Water destroyer
  mat[6+1].destroys_water=true;
	mat[6+1].strength=100;
	mat[6+1].destroyed_into=1;
  
	//Special Rock
  mat[7+1].worm_pass=false;
  mat[7+1].particle_pass=true;
  mat[7+1].flows=false;
  mat[7+1].can_breath=true;
  mat[7+1].draw_exps=false;
  mat[7+1].damage=0;
	mat[7+1].strength=100;
	mat[7+1].destroyed_into=1;
  
	//Explosive
  mat[8+1].worm_pass=false;
  mat[8+1].particle_pass=false;
  mat[8+1].flows=false;
  mat[8+1].can_breath=true;
  mat[8+1].draw_exps=false;
  mat[8+1].blocks_light=true;
  mat[8+1].chreact=load_part("chreact2.obj");
  mat[8+1].damage=0;
	mat[8+1].strength=50;
	mat[8+1].destroyed_into=1;
  
	//Special rock 2
  mat[9+1].worm_pass=true;
  mat[9+1].particle_pass=false;
  mat[9+1].flows=false;
  mat[9+1].can_breath=true;
  mat[9+1].draw_exps=false;
  mat[9+1].blocks_light=true;
  mat[9+1].damage=0;
	mat[9+1].strength=100;
	mat[9+1].destroyed_into=1;
};

level::~level()
{
  if (map->mapimg!=NULL)destroy_bitmap(map->mapimg);
	if (map->buffer!=NULL)destroy_bitmap(map->buffer);
	if (map->material!=NULL)destroy_bitmap(map->material);
	if (map->layer!=NULL)destroy_bitmap(map->layer);
  if (map->background!=NULL)destroy_bitmap(map->background);
  if (map->water_buffer!=NULL)destroy_bitmap(map->water_buffer);
  if (map->paralax!=NULL)destroy_bitmap(map->paralax);
  if (map->light_layer!=NULL)destroy_bitmap(map->light_layer);
  spawnpoint_count=0;
};

void free_map()
{
	if (map->mapimg!=NULL)destroy_bitmap(map->mapimg);
	if (map->buffer!=NULL)destroy_bitmap(map->buffer);
	if (map->material!=NULL)destroy_bitmap(map->material);
	if (map->layer!=NULL)destroy_bitmap(map->layer);
  if (map->background!=NULL)destroy_bitmap(map->background);
  if (map->water_buffer!=NULL)destroy_bitmap(map->water_buffer);
  if (map->paralax!=NULL)destroy_bitmap(map->paralax);
  map->has_water=false;
};

int load_map(char* name)
{
	BITMAP *tmp_mat;
	char tmp[1024],tmp2[1024];
	strcpy(tmp2, game->mod);
	strcat(tmp2, "/maps/");
	strcat(tmp2, name);
  strcpy(tmp,tmp2);
	strcat(tmp,"/material");
	set_color_depth(8);
	tmp_mat = loadImage(tmp, NULL);
	set_color_depth(game->v_depth);
  if (tmp_mat==NULL)
  {
    strcpy(tmp2, "default/maps/");
    strcat(tmp2, name);
    strcpy(tmp,tmp2);
    strcat(tmp,"/material");
    set_color_depth(8);
    tmp_mat = loadImage(tmp,NULL);
    set_color_depth(game->v_depth);
  };
	if (tmp_mat!=NULL)
	{
		delete map;
    map=new level;
    map->init_materials();
    strcpy(map->path,tmp2);
    strcpy(map->name,name);
		strcpy(tmp, tmp2);
		strcat(tmp,"/level");
		map->mapimg = loadImage(tmp,NULL);
		//should do more then return 1
		if (map->mapimg == NULL)
			return 1;
		strcpy(tmp, tmp2);
		strcat(tmp,"/layer");
		map->layer = loadImage(tmp,NULL);	
    strcpy(tmp, tmp2);
		strcat(tmp,"/paralax");
		map->paralax = loadImage(tmp,NULL);
    map->material = tmp_mat;
    set_color_depth(8);
    map->water_buffer = create_bitmap(tmp_mat->w,tmp_mat->h);
    rectfill(map->water_buffer,0,0,map->water_buffer->w,map->water_buffer->h,1);
    set_color_depth(game->v_depth);
    strcpy(tmp, tmp2);
		strcat(tmp,"/background");
		map->background = loadImage(tmp,NULL);
    if (map->background==NULL)
    {
      int x,y,col,g;
      map->background=create_bitmap(map->mapimg->w,map->mapimg->h);
      blit(map->mapimg,map->background,0,0,0,0,map->mapimg->w,map->mapimg->h);
     	col=makecol(100,100,100);
      if (game->v_depth!=8)
      {
        for (y=0;y<map->background->h;y++)
        for (x=0;x<map->background->w;x++)
        {
          g=getpixel(map->material,x,y);
					//mat weapon
					if (map->mat[g+1].destroyed_into!=g)
          {
            drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
            set_multiply_blender(0, 0, 0, 255);
            putpixel(map->background,x,y,col);
            solid_mode();
          }else if (map->mat[g+1].flows)
          {
            drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
            set_multiply_blender(0, 0, 0, 255);
            putpixel(map->background,x,y,0);
            solid_mode();
          };
        };
      } else
      {
        for (y=0;y<map->background->h;y++)
        for (x=0;x<map->background->w;x++)
        {
          g=getpixel(map->material,x,y);
          if (g==2)
          {
            putpixel(map->background,x,y,col);
          }else if (g==3)
          {
            putpixel(map->background,x,y,makecol(0,0,0));
          };
        }; 
      };
    };
		map->buffer=create_bitmap(map->mapimg->w,map->mapimg->h);
		return 0;
	};
  
	return 1;
};

void load_map_config()
{
  char tmp[1024],*tmp1,tmp2[1024];
  char *var,*val;
  FILE *fbuf;
  int i;
  //render_sunlight( map->mapimg, map->material);
  //antialias();
	strcpy(tmp, map->path);
  strcat(tmp, "/config.cfg");
  fbuf=fopen(tmp,"rt");
  if (fbuf!=NULL)
	{
		//...parse the file
		while (!feof(fbuf))
		{
			tmp1=fgets(tmp2, sizeof(tmp2), fbuf);
			if (tmp1!=NULL)
			{
				i=0;
        while (tmp1[i]!=' ') i++;
        //split it
        var=strmid(tmp1,0,i);
        if (tmp1[strlen(tmp1)-1]=='\n') tmp1[strlen(tmp1)-1]='\0';
        val=strmid(tmp1,i+1,strlen(tmp1)-i);
        //rem_spaces(var);
        //rem_spaces(val);
        if (strcmp("light",var)==0)
        {
          int j,x,X,Y,R,G,B,F,N,P;
          x=0;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          X=atoi(strmid(val,x,j-x));
          x=j+1;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          Y=atoi(strmid(val,x,j-x));
          x=j+1;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          R=atoi(strmid(val,x,j-x));
          x=j+1;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          G=atoi(strmid(val,x,j-x));
          x=j+1;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          B=atoi(strmid(val,x,j-x));
          x=j+1;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          F=atoi(strmid(val,x,j-x));
          x=j+1;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          N=atoi(strmid(val,x,j-x));
          x=j+1;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          P=atoi(strmid(val,x,j-x));
          x=j+1;
          
          if(P!=1)
          {
            render_light(X,Y,R,G,B,F,N,map->mapimg ,map->material);
            render_light(X,Y,R,G,B,F,N,map->background ,map->material);
          } else
          {
            if (!map->light_layer)
            {
              map->light_layer=create_bitmap(map->material->w,map->material->h);
              clear_to_color(map->light_layer,0);
            }
            render_light(X,Y,R,G,B,F,N,map->light_layer ,map->material);
          };
        }
        else if (strcmp("obj",var)==0)
        {
          struct part_type *tmppart;
          int j,x,X,Y;
          x=0;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          tmppart=load_part(strmid(val,x,j-x));
          x=j+1;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          //allegro_message(strmid(val,x,j-x));
          X=atoi(strmid(val,x,j-x));
          x=j+1;
          for(j=x;(val[j]!=',') && (j < strlen(val));j++);
          Y=atoi(strmid(val,x,j-x));
          x=j+1;
          partlist.create_part(X*1000,Y*1000,0,0,-1,tmppart);
        }
        else if (strcmp("spawnpoint",var)==0)
        {
          if (map->spawnpoint_count<32)
          {
            int j,x;
            x=0;
            for(j=x;(val[j]!=',') && (j < strlen(val));j++);
            map->spawnpoint[map->spawnpoint_count].x=atoi(strmid(val,x,j-x));
            x=j+1;
            for(j=x;(val[j]!=',') && (j < strlen(val));j++);
            //allegro_message(strmid(val,x,j-x));
            map->spawnpoint[map->spawnpoint_count].y=atoi(strmid(val,x,j-x));
            x=j+1;
            for(j=x;(val[j]!=',') && (j < strlen(val));j++);
            map->spawnpoint[map->spawnpoint_count].team=atoi(strmid(val,x,j-x));
            map->spawnpoint_count++;
          };
        };
			};
		};
		fclose(fbuf);
	};
}

void change_level()
{
	char tmp[1024];
	if (load_map(con->arg)!=0)
	{
		char *cptr = lcase(con->arg);
		if (load_map(cptr)!=0)
		{
			sprintf(tmp,"%s%s%c","COULD NOT FIND MAP \"",con->arg,'\"');
			con->log.create_msg(tmp);
			free(cptr);
			return;
		}
		free(cptr);
	}
  if(*game->HOST==1)
  {
    srv = new Server( 1, 9898 );
    if(!srv || !srv->is_ok) {
      allegro_message("Couldnt start the server.\n");
      exit(-1);
    }else game->host=true;
    game->init_node(srv,true);
    delete_players();
    if (*game->TEAMPLAY==1)
      game->teamplay=true;
    else game->teamplay=false;
    player[0]=new worm;
    player[0]->init_node(true);
    player[0]->islocal=true;
    player[0]->local_slot=local_players;
    local_player[local_players]=player_count;
    change_nick(local_players);
    player_count++;
    local_players++;
    if(*game->SPLIT_SCREEN==1)
    {
      player[1]=new worm;
      player[1]->init_node(true);
      player[1]->islocal=true;
      player[1]->local_slot=local_players;
      local_player[local_players]=player_count;
      change_nick(local_players);
      player_count++;
      local_players++;
    };
  };

	destroy_particles();
	delete exps;
	exps=new exp_list;
	create_waterlist();
	load_map_config();
	//strcpy(game->level,con->arg);
	int i;
	for (i=0; i<local_players;i++)
	{
		player[i]->active=false;
		player[i]->deaths=0;
		player[i]->kills=0;
		player[i]->lives=0;
		player[i]->destroyrope();
		player[i]->firecone_time=0;
		player[i]->selecting_weaps=true;
		player[i]->curr_weap=0;
	};
	con->flag=0;
	game->selecting=true;
};
