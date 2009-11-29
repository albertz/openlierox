#include "render.h"
#include "lights.h"

int pepo=0;

int video_filter=0;
BITMAP* screen_buffer;

void render_lasersight(worm *player)
{
  int xadd,yadd,x,y,i,i2,g;
	xadd=fixtof(fixsin(ftofix(player->aim/1000.))) * 1000 * player->dir;
	yadd=fixtof(fixcos(ftofix(player->aim/1000.))) * 1000;
  x=player->x;
  y=(player->y/1000);
  y=y*1000-3500;
  g=getpixel(map->material,x/1000,y/1000);
  i2=weaps->num[player->weap[player->curr_weap].weap]->lsight_intensity;
	while (!map->mat[g+1].blocks_light)
  {
    i=rand()%1000;
    i2-=weaps->num[player->weap[player->curr_weap].weap]->lsight_fade;
    if(i2>1000)i2=1000;
    if (i<i2) putpixel(map->buffer,x/1000,y/1000,makecol(255,0,0));
    x+=xadd;
    y+=yadd;
    g=getpixel(map->material,x/1000,y/1000);
  };
};

void renderrope(worm *player)
{
	 line(map->buffer,player->x/1000,player->y/1000-5,player->ropex/1000,player->ropey/1000,makecol(200,140,10));
	 draw_sprite(map->buffer, game->hook->img[0],player->ropex/1000-game->hook->img[0]->w/2,player->ropey/1000-game->hook->img[0]->h/2);
};

void draw_bar(BITMAP* where,int w,int h, int x, int y, int max_value, int value, int color)
{
	if (value > 0)
	rectfill(where,x,y,(value*w)/max_value+x,y+h,color);
};

void render_weapon_selection_menu(BITMAP *where)
{
  int i,j;
  worm *p;
  struct s_viewport *vp;
  char str[255];
  //set_clip(where,0,0,320,240);
  for (j=0;j<local_players;j++)
  {
    p=player[local_player[j]];
    vp=&game->viewport[j];
    if (p->selecting_weaps)
    {
      rectfill(where,vp->x+vp->w/2-70,28,vp->x+vp->w/2+70,69,makecol(0,0,0));
      rect(where,vp->x+vp->w/2-71,28,vp->x+vp->w/2+71,69,makecol(100,100,100));
      rectfill(where,vp->x+vp->w/2-70,30+p->curr_weap*8,vp->x+vp->w/2+70,35+p->curr_weap*8,makecol(60,60,60));
      for(i=0;i<5;i++)
      {
        game->fonty->draw_string(where,weaps->num[p->weap[i].weap]->name,vp->x+vp->w/2-69,30+i*8,false);
      };
      rectfill(where,vp->x+vp->w/2-70,8,vp->x+vp->w/2+70,25,makecol(0,0,0));
      rect(where,vp->x+vp->w/2-71,8,vp->x+vp->w/2+71,25,makecol(100,100,100));
      game->fonty->draw_string(where,"[PRESS FIRE BUTTON WHEN DONE]",vp->x+vp->w/2-69,10,false);
      game->fonty->draw_string(where,"[PRESS CHANGE BUTTON TO RANDOMIZE]",vp->x+vp->w/2-69,18,false);
      rectfill(where,vp->x+vp->w/2-70,72,vp->x+vp->w/2+70,97,makecol(0,0,0));
      rect(where,vp->x+vp->w/2-71,72,vp->x+vp->w/2+71,97,makecol(100,100,100));
      sprintf(str,"MOD NAME: %s",ucase(game->mod));
      game->fonty->draw_string(where,str,vp->x+vp->w/2-69,74,false);
      sprintf(str,"MAP NAME: %s",ucase(map->name));
      game->fonty->draw_string(where,str,vp->x+vp->w/2-69,74+8,false);
      sprintf(str,"NUMBER OF WEAPONS: %d",game->weap_count);
      game->fonty->draw_string(where,str,vp->x+vp->w/2-69,74+16,false);
    };
  };
};

void draw_hud(BITMAP* where, int _player, struct s_viewport viewport)
{
    char ints[40];
    int b;
    worm *p=player[local_player[_player]];

    draw_sprite(where,game->health->img[0],viewport.x+4,viewport.y+4);
    masked_blit(game->death_img->img[0],where,0,0,viewport.x+4,viewport.y+11,game->death_img->img[0]->w,game->death_img->img[0]->h);
    if (*pl_options->HEALTH_DRAW_MODE==0)
    {
      int c;
      c=((p->health*1.)/ *game->MAX_HEALTH)*255;
      draw_bar(where,30,2, viewport.x+11, viewport.y+5, *game->MAX_HEALTH, p->health, makecol(255-c,c,0));
    }else 
    {
      sprintf(ints, "%d",p->health);
      game->fonty->draw_string(where,ints,viewport.x+10,viewport.y+4,false);
    };
    draw_sprite(where,game->ammo->img[0],viewport.x+46,viewport.y+4);
    if (p->weap[p->curr_weap].reloading)
    {
      b=((p->weap[p->curr_weap].reload_time*1.)/ weaps->num[p->weap[p->curr_weap].weap]->reload_time)*255;
      draw_bar(where,30,2, viewport.x+53, viewport.y+5, weaps->num[p->weap[p->curr_weap].weap]->reload_time, weaps->num[p->weap[p->curr_weap].weap]->reload_time-p->weap[p->curr_weap].reload_time, makecol(b,255-b,0));
      game->fonty->draw_string(where,"RELOADING",viewport.x+47,viewport.y+9,false);
    }else
    {
      b=((p->weap[p->curr_weap].ammo*1.)/ weaps->num[p->weap[p->curr_weap].weap]->ammo)*255;
      draw_bar(where,30,2, viewport.x+53, viewport.y+5, weaps->num[p->weap[p->curr_weap].weap]->ammo, p->weap[p->curr_weap].ammo, makecol(255-b,b,0));
    };
    sprintf(ints, "%d",p->deaths);
    game->fonty->draw_string(where,ints,viewport.x+10,viewport.y+11,false);
    if (!p->active && p->health>=*game->MAX_HEALTH)  game->fonty->draw_string(where,"[PRESS JUMP KEY TO RESPAWN]",viewport.x+20,viewport.y+120,true);
    if (p->active && p->flag2)  game->fonty->draw_string(where,weaps->num[p->weap[p->curr_weap].weap]->name,viewport.x+(p->x/1000-p->xview)-(strlen(weaps->num[p->weap[p->curr_weap].weap]->name)*2),viewport.y+p->y/1000-p->yview-16,true);
    if (p->active && p->air<*game->AIR_CAPACITY-*game->AIR_CAPACITY/6) draw_bar(where,16,1, viewport.x+(p->x/1000-p->xview)-8, viewport.y+p->y/1000-p->yview+2, *game->AIR_CAPACITY-*game->AIR_CAPACITY/6, p->air, makecol(255,255,255));

    if (p->active){
      p->crosshx=(p->x/1000)+fixtof(fixsin(ftofix(p->aim/1000.)))*p->crossr*p->dir-p->xview;
      p->crosshy=(p->y/1000)-4+fixtof(fixcos(ftofix(p->aim/1000.)))*p->crossr-p->yview;
      masked_blit(p->crosshair->img[0],where,0,0,viewport.x+p->crosshx-p->crosshair->img[0]->w/2,viewport.y+p->crosshy-p->crosshair->img[0]->h/2,p->crosshair->img[0]->w,p->crosshair->img[0]->h);
    };

};

void engine::render()
{
	int i;
	bool smallmap=false;
	
	char fpsstr[50];
	char ints[50];
  worm *p;
	
	sprintf(fpsstr, "FPS: %d", fps);

  if(local_players==1)
  {
    viewport[0].w=320;
    viewport[0].h=240;
    viewport[0].x=0;
    viewport[0].y=0;
  }else 
  {
    viewport[0].w=320/2-1;
    viewport[0].h=240;
    viewport[0].x=0;
    viewport[0].y=0;
  };
    
  printf("b");
  
	if (map->mapimg->w<320 && map->mapimg->h<240) smallmap=true;
		
	if (*MAP_SHOW_MODE==0 || !smallmap)
	{
    for (i=0;i<local_players;i++)
    {
      p=player[local_player[i]];
      if (map->mapimg->w>viewport[i].w)
      {
				p->xview=(p->x/1000)-viewport[i].w/2;
				if (p->xview<0)
				{
					p->xview=0;
				};
				if (p->xview>map->mapimg->w-viewport[i].w)
				{
					p->xview=map->mapimg->w-viewport[i].w;
				};
			}else
      {
        p->xview=map->mapimg->w/2-viewport[i].w/2;
      };
  
      
      if (map->mapimg->h>viewport[i].h)
      {
        p->yview=(p->y/1000)-viewport[i].h/2;
        if (p->yview<0)
        {
          p->yview=0;
        };
        if (p->yview>map->mapimg->h-viewport[i].h)
        {
          p->yview=map->mapimg->h-viewport[i].h;
        };
      } else
      {
        p->yview=map->mapimg->h/2-viewport[i].h/2;
      };
    };
	}else
	for (i=0;i<local_players;i++)
	{
    p=player[local_player[i]];
		p->xview=viewport[i].x-1-(159-map->mapimg->w/2);
		p->yview=map->mapimg->h/2-viewport[i].h/2;
	};
	
 
	if (*MAP_SHOW_MODE==1 && smallmap)
	blit(map->mapimg,map->buffer,0,0,0,0,map->mapimg->w,map->mapimg->h);
	else
	for (i=0;i<local_players;i++){
    p=player[local_player[i]];
		blit(map->mapimg,map->buffer,p->xview,p->yview,p->xview,p->yview,viewport[i].w,viewport[i].h);
	};
	

	
	for (i=0;i<player_count;i++)
	{
		if (player[i]->ropestate!=0)
		renderrope(player[i]);
	};
	
  
	for (i=0;i<player_count;i++)
	{
		if (player[i]->active)
		{
      render_lasersight(player[i]);
      if(*FLASHLIGHT==1)render_flashlight(player[i]->x/1000,player[i]->y/1000-4,player[i]->aim-64000,player[i]->dir,map->buffer,map->material);
      if (!player[i]->islocal) game->fonty->draw_string(map->buffer,player[i]->name,player[i]->x/1000-(strlen(player[i]->name)*2),player[i]->y/1000-16,true);
			if (player[i]->dir==-1)
			{
				//draw_sprite_h_flip(map->buffer,player[i]->skin->img[(((player[i]->aim/1000)-32+8)/(96/6))+(player[i]->curr_frame/1000)*7],player[i]->x / 1000 - 9,(player[i]->y / 1000)-8);
        player[i]->render_flip(map->buffer, (((player[i]->aim/1000)-32+8)/(96/6))+(player[i]->curr_frame/1000)*7, player[i]->x / 1000 - 9, (player[i]->y / 1000)-8);
        if(player[i]->curr_firecone!=NULL && player[i]->firecone_time>0)
        draw_sprite_h_flip(map->buffer,player[i]->curr_firecone->img[(((player[i]->aim/1000)-32+8)/(96/6))],player[i]->x / 1000 + fixtof(fixsin(ftofix(player[i]->aim/1000.)))*-5 - player[i]->curr_firecone->img[0]->w/2,(player[i]->y / 1000)+fixtof(fixcos(ftofix(player[i]->aim/1000.)))*5 -4 - player[i]->curr_firecone->img[0]->h/2);
			}
			else
			{
				//draw_sprite(map->buffer,player[i]->skin->img[(((player[i]->aim/1000)-32+8)/(96/6))+(player[i]->curr_frame/1000)*7],player[i]->x / 1000 - 6,(player[i]->y / 1000)-8);
        player[i]->render(map->buffer, (((player[i]->aim/1000)-32+8)/(96/6))+(player[i]->curr_frame/1000)*7, player[i]->x / 1000 - 6, (player[i]->y / 1000)-8);
        if(player[i]->curr_firecone!=NULL && player[i]->firecone_time>0)
        draw_sprite(map->buffer,player[i]->curr_firecone->img[(((player[i]->aim/1000)-32+8)/(96/6))],player[i]->x / 1000 + fixtof(fixsin(ftofix(player[i]->aim/1000.)))*5 - player[i]->curr_firecone->img[0]->w/2,(player[i]->y / 1000)+fixtof(fixcos(ftofix(player[i]->aim/1000.)))*5 -4 - player[i]->curr_firecone->img[0]->h/2);
			};
		};
	};
  
	partlist.render_particles(map->buffer);
  
  render_exps();
  
  partlist.r_lens(map->buffer);
	
	if (map->mapimg->h<viewport[0].h || map->mapimg->w<viewport[0].w) clear_bitmap(buffer);

	if (*MAP_SHOW_MODE==1 && smallmap)
		blit(map->buffer,buffer,0,0,160-map->mapimg->w/2,120-map->mapimg->h/2,map->mapimg->w,map->mapimg->h);//160-map->mapimg->w/2,120-map->mapimg->h/2,160+map->mapimg->w/2,120-map->mapimg->h/2
	
    
  pepo++;
  for (i=0;i<local_players;i++)
  {
    p=player[local_player[i]];
    if (*MAP_SHOW_MODE!=1 || !smallmap)
    {
      set_clip(buffer,viewport[i].x,viewport[i].y,viewport[i].x+viewport[i].w,viewport[i].y+viewport[i].h);
      if(map->paralax!=NULL)
      {
        blit(map->paralax,buffer,(p->xview*(map->paralax->w-viewport[i].w))/(map->mapimg->w-viewport[i].w),(p->yview*(map->paralax->h-viewport[i].h))/(map->mapimg->h-viewport[i].h),viewport[i].x,viewport[i].y,viewport[i].w,viewport[i].h);
        masked_blit(map->buffer,buffer,p->xview,p->yview,viewport[i].x,viewport[i].y,viewport[i].w,viewport[i].h);
      }
      else 
      {
        /*int j,o,f;
        for (j=0;j<viewport[i].h;j++)
        {
          o=fixtof(fixsin(ftofix(j*2+pepo/2.)))*20;
          blit(map->buffer,buffer,p->xview+o,p->yview+j,viewport[i].x,viewport[i].y+j,viewport[i].w,1);
        };*/
        blit(map->buffer,buffer,p->xview,p->yview,viewport[i].x,viewport[i].y,viewport[i].w,viewport[i].h);
      };
      if (map->layer!=NULL && *RENDER_LAYERS==1)
      masked_blit(map->layer,buffer,p->xview,p->yview,viewport[i].x,viewport[i].y,viewport[i].w,viewport[i].h);
    }
    if (!game->selecting)
    draw_hud(buffer,i,viewport[i]);
  };
	set_clip(buffer,0,0,320,240);
	/******HUD*******/
  if (game->selecting)
    render_weapon_selection_menu(buffer);
  
  printf("a");

  
	fonty->draw_string(buffer,fpsstr,280,4,false);
	//  sprintf(ints, "%d",caca);
	if (*MAP_SHOW_MODE==0 || !smallmap && local_players==2)
	{
		line(buffer,viewport[0].w,0,viewport[0].w,240,makecol(0,0,0));
		line(buffer,viewport[0].w+1,0,viewport[0].w+1,240,makecol(0,0,0));
	};
  
	/*  if (con->pos>0)
	{
	rectfill(buffer,0,0,319,con->pos-2,0);
	sprintf(tmp, "%s%c",caca,'_');
	fonty->draw_string(buffer,tmp,3,con->pos-8);
	line(buffer,0,con->pos-1,319,con->pos-1,makecol(120,120,120));
	};*/
	con->render(buffer);
  if(v_width==320)
	blit(buffer,screen,0,0,0,0,320,240);
  else
  {
    int o;
    acquire_screen();
    if (*game->VIDEO_FILTER!=video_filter)
    {
      video_filter=*game->VIDEO_FILTER;
      clear_bitmap(screen);
    };
    switch(*game->VIDEO_FILTER)
   	{
      case 0: stretch_blit(buffer, screen, 0, 0, 320, 240, 0, 0, v_width, v_height); break;
      case 1: {
        for (i=0;i<v_height-1;i+=2)
        {
          stretch_blit(buffer, screen, 0, i/2, 320, 1, 0, i, v_width, 1);
        };
        break;
      };
      case 2: {
        #ifdef AA2XSAI
        Super2xSaI(buffer, screen_buffer, 0, 0, 0, 0, 320,240);
        #endif
        blit(screen_buffer, screen, 0, 0, 0, 0, v_width, v_height);
        break;
      };
   	};
    release_screen();
  };

};


