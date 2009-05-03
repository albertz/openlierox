#include "render.h"
#include "player.h"
#include "weapons.h"
#include "text.h"
#include "particles.h"
#include "console.h"
#include "lights.h"
#include "explosions.h"
#include "engine.h"
#include "level.h"
#include "sprites.h"
#include "network.h"

#ifdef AAFBLEND
  #include <fblend.h>
#endif
#ifdef AA2XSAI
extern "C" {
  #include "2xsai.h"
}
#endif

int video_filter=0;
BITMAP* screen_buffer;

bool CanBeSeen(int x, int y, int w, int h)
{
  int i;
  for(i=0;i<local_players;i++)
	{
		if (x+w/2>player[local_player[i]]->xview && x-w/2<player[local_player[i]]->xview+game->viewport[i].w)
		if (y+h/2>player[local_player[i]]->yview && y-h/2<player[local_player[i]]->yview+game->viewport[i].h)
			return true;
	};
  return false;
}

void render_lasersight(worm *player)
{
  int xadd,yadd,x,y,i,i2,g;
	xadd=fixtoi(fixsin(ftofix(player->aim/1000.))* 1000)  * player->dir;
	yadd=fixtoi(fixcos(ftofix(player->aim/1000.))* 1000) ;
  x=player->x;
  y=(player->y/1000);
  y=y*1000-3500;
  g=getpixel(map->material,x/1000,y/1000);
  i2=weaps->num[player->weap[player->curr_weap].weap]->lsight_intensity;
	while (!map->mat[g+1].blocks_light)
  {
    i=rand()%1000;
    i2-=weaps->num[player->weap[player->curr_weap].weap]->lsight_fade;
    if (i2>1000) i2=1000;
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

void draw_random_bar(BITMAP* where,int w,int h, int x, int y, int max_value, int value, int color)
{
	if (value > 0)
  {
    int r;
    for(int _x=0;_x<=(value*w)/max_value;_x++)
			for(int _y=0;_y<=h;_y++)
			{
	      r=rand()%128;
				putpixel(where,x+_x,y+_y,makecol(128+r-30*_y,40,40));
			}
  }
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
      //
      char *cptr = ucase(game->mod);
      sprintf(str,"MOD NAME: %s",cptr);
			free(cptr);
      game->fonty->draw_string(where,str,vp->x+vp->w/2-69,74,false);			
      cptr = ucase(map->name);
      sprintf(str,"MAP NAME: %s",cptr);
			free(cptr);
      game->fonty->draw_string(where,str,vp->x+vp->w/2-69,74+8,false);
      //
      sprintf(str,"NUMBER OF WEAPONS: %d",game->weap_count);
      game->fonty->draw_string(where,str,vp->x+vp->w/2-69,74+16,false);
			//weaopn HUD
			game->weaponHUD(where, p, *vp, 0);
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
    if (*pl_options[_player].HEALTH_DRAW_MODE==0)
    {
      int c;
			if (p->health <= *game->START_HEALTH)
			{
				c=((p->health*1.)/ *game->START_HEALTH)*255;
				draw_bar(where,30,2, viewport.x+11, viewport.y+5, *game->START_HEALTH, p->health, makecol(255-c,c,0));
			}
			else
			{
				if (p->health <= *game->MAX_HEALTH)
				{
					//draw green bar
					rectfill(where,viewport.x+11, viewport.y+5, viewport.x+41, viewport.y+7, makecol(0,255,0));
					//blue to white bar on-top
					c=(((p->health-*game->START_HEALTH)*1.)/ (*game->MAX_HEALTH-*game->START_HEALTH))*255;
					draw_random_bar(where,30,2, viewport.x+11, viewport.y+5, *game->MAX_HEALTH-*game->START_HEALTH, p->health-*game->START_HEALTH, makecol(c,c,255));
				}
				else
				{
					rectfill(where,viewport.x+11, viewport.y+5, viewport.x+41, viewport.y+7, makecol(0,0,255));
					rectfill(where,viewport.x+11, viewport.y+6, viewport.x+41, viewport.y+6, makecol(255,255,255));
				}
			}		
		}
    else 
    {
      sprintf(ints, "%d",p->health);
      game->fonty->draw_string(where,ints,viewport.x+10,viewport.y+4,false);
    };
    draw_sprite(where,game->ammo->img[0],viewport.x+46,viewport.y+4);
		//checks that max ammo is not 0 (unlimited ammo)
		if (weaps->num[p->weap[p->curr_weap].weap]->ammo > 0)
			if (p->weap[p->curr_weap].reloading)
			{
	      b=((p->weap[p->curr_weap].reload_time*255)/ weaps->num[p->weap[p->curr_weap].weap]->reload_time);
				draw_bar(where,30,2, viewport.x+53, viewport.y+5, weaps->num[p->weap[p->curr_weap].weap]->reload_time, weaps->num[p->weap[p->curr_weap].weap]->reload_time-p->weap[p->curr_weap].reload_time, makecol(b,255-b,0));
				game->fonty->draw_string(where,"RELOADING",viewport.x+47,viewport.y+9,false);
			}else
			{
	      b=((p->weap[p->curr_weap].ammo*255)/ weaps->num[p->weap[p->curr_weap].weap]->ammo);
				draw_bar(where,30,2, viewport.x+53, viewport.y+5, weaps->num[p->weap[p->curr_weap].weap]->ammo, p->weap[p->curr_weap].ammo, makecol(255-b,b,0));
			}
		else
			game->fonty->draw_string(where,"NO LIMIT",viewport.x+53,viewport.y+4,false);
    sprintf(ints, "%d",p->deaths);
    game->fonty->draw_string(where,ints,viewport.x+10,viewport.y+11,false);

		if (p->active)
		{
			if (p->flag2)
			{
				game->fonty->draw_string(where,weaps->num[p->weap[p->curr_weap].weap]->name,viewport.x+(p->x/1000-p->xview)-(strlen(weaps->num[p->weap[p->curr_weap].weap]->name)*2),viewport.y+p->y/1000-p->yview-16,true);
				//weapon HUD
				game->weaponHUD(where, player[_player], viewport, 1);
			}
			if (p->air<*game->AIR_CAPACITY-*game->AIR_CAPACITY/6) draw_bar(where,16,1, viewport.x+(p->x/1000-p->xview)-8, viewport.y+p->y/1000-p->yview+2, *game->AIR_CAPACITY-*game->AIR_CAPACITY/6, p->air, makecol(255,255,255));	
			p->crosshx=(p->x/1000)+fixtoi(fixsin(ftofix(p->aim/1000.))*p->crossr)*p->dir-p->xview;
			p->crosshy=(p->y/1000)-4+fixtoi(fixcos(ftofix(p->aim/1000.))*p->crossr)-p->yview;
			masked_blit(p->crosshair->img[0],where,0,0,viewport.x+p->crosshx-p->crosshair->img[0]->w/2,viewport.y+p->crosshy-p->crosshair->img[0]->h/2,p->crosshair->img[0]->w,p->crosshair->img[0]->h);
		}
		else
			if (p->health>=*game->START_HEALTH)  game->fonty->draw_string(where,"[PRESS JUMP KEY TO RESPAWN]",viewport.x+20,viewport.y+120,true);
};

void engine::render()
{
	int i;
	bool smallmap=false;
	
  worm *p;

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
		for (i=0;i<local_players;i++)
		{
	    p=player[local_player[i]];
			blit(map->mapimg,map->buffer,p->xview,p->yview,p->xview,p->yview,viewport[i].w,viewport[i].h);
		};
	
  partlist.render_particles(map->buffer,0);
  partlist.render_particles(map->buffer,1);
	
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
      if (player[i]->talking)
        draw_sprite(map->buffer, talk->img[0], player[i]->x / 1000 - talk->img[0]->w/2, player[i]->y / 1000 - 22);
			if (player[i]->dir==-1)
			{
				//draw_sprite_h_flip(map->buffer,player[i]->skin->img[(((player[i]->aim/1000)-32+8)/(96/6))+(player[i]->curr_frame/1000)*7],player[i]->x / 1000 - 9,(player[i]->y / 1000)-8);
        player[i]->render_flip(map->buffer, (((player[i]->aim/1000)-32+8)/(96/6))+(player[i]->curr_frame/1000)*7, player[i]->x / 1000 - 9, (player[i]->y / 1000)-8);
        if(player[i]->curr_firecone!=NULL && player[i]->firecone_time>0)
        draw_sprite_h_flip(map->buffer,player[i]->curr_firecone->img[(((player[i]->aim/1000)-32+8)/(96/6))],player[i]->x / 1000 + fixtoi(fixsin(ftofix(player[i]->aim/1000.))*-5) - player[i]->curr_firecone->img[0]->w/2,(player[i]->y / 1000)+fixtoi(fixcos(ftofix(player[i]->aim/1000.))*5) -4 - player[i]->curr_firecone->img[0]->h/2);
			}
			else
			{
				//draw_sprite(map->buffer,player[i]->skin->img[(((player[i]->aim/1000)-32+8)/(96/6))+(player[i]->curr_frame/1000)*7],player[i]->x / 1000 - 6,(player[i]->y / 1000)-8);
        player[i]->render(map->buffer, (((player[i]->aim/1000)-32+8)/(96/6))+(player[i]->curr_frame/1000)*7, player[i]->x / 1000 - 6, (player[i]->y / 1000)-8);
        if(player[i]->curr_firecone!=NULL && player[i]->firecone_time>0)
        draw_sprite(map->buffer,player[i]->curr_firecone->img[(((player[i]->aim/1000)-32+8)/(96/6))],player[i]->x / 1000 + fixtoi(fixsin(ftofix(player[i]->aim/1000.))*5) - player[i]->curr_firecone->img[0]->w/2,(player[i]->y / 1000)+fixtoi(fixcos(ftofix(player[i]->aim/1000.))*5) -4 - player[i]->curr_firecone->img[0]->h/2);
			};
		};
	};
  
  //render_seglight(player[0]->x/1000,player[0]->y/1000-4,player[0]->aim-64000,player[0]->dir,map->buffer,map->material);
  
	
  partlist.render_particles(map->buffer,3);
  partlist.render_particles(map->buffer,4);
  render_exps();
  
  partlist.r_lens(map->buffer);
	
	if (map->mapimg->h<viewport[0].h || map->mapimg->w<viewport[0].w) clear_bitmap(buffer);

	if (*MAP_SHOW_MODE==1 && smallmap)
		blit(map->buffer,buffer,0,0,160-map->mapimg->w/2,120-map->mapimg->h/2,map->mapimg->w,map->mapimg->h);//160-map->mapimg->w/2,120-map->mapimg->h/2,160+map->mapimg->w/2,120-map->mapimg->h/2
	
    
  for (i=0;i<local_players;i++)
  {
    p=player[local_player[i]];
    if (*MAP_SHOW_MODE!=1 || !smallmap)
    {
      set_clip_rect(buffer,viewport[i].x,viewport[i].y,viewport[i].x+viewport[i].w,viewport[i].y+viewport[i].h);
      if (p->flash<=25500)
      {
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
        render_paralax_lights(buffer,i,viewport[i]);
				#ifdef AAFBLEND
        if (map->light_layer!=NULL && *RENDER_LAYERS==1)
        {
          BITMAP* light_layer_buffer=create_sub_bitmap(map->light_layer, p->xview,p->yview, viewport[i].w,viewport[i].h);          
          fblend_add(light_layer_buffer,buffer , viewport[i].x, viewport[i].y,255);          
          destroy_bitmap(light_layer_buffer);
        };
				#endif
        if (map->layer!=NULL && *RENDER_LAYERS==1)
          masked_blit(map->layer,buffer,p->xview,p->yview,viewport[i].x,viewport[i].y,viewport[i].w,viewport[i].h);
        if (p->flash>0)
        {
          if(v_depth==32)
          {
            #ifdef AAFBLEND
            fblend_rect_trans(buffer, viewport[i].x,viewport[i].y,viewport[i].w,viewport[i].h, makecol(255,255,255)/*p->flash/100,p->flash/100,p->flash/100)*/, p->flash/100);
            #endif
          }
          else
          {
            drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
            set_add_blender(0,0,0,p->flash/100);
            rectfill(buffer, viewport[i].x,viewport[i].y,viewport[i].x+viewport[i].w,viewport[i].y+viewport[i].h, makecol(255,255,255));
            solid_mode();
          }
        };
      }else
        rectfill(buffer, viewport[i].x,viewport[i].y,viewport[i].x+viewport[i].w,viewport[i].y+viewport[i].h, makecol(255,255,255));
    }
    if (!game->selecting)
      draw_hud(buffer,i,viewport[i]);
  };
	set_clip_rect(buffer,0,0,320,240);
	/******HUD*******/
  if (game->selecting)
    render_weapon_selection_menu(buffer);

	if (*SHOW_FPS)
	{
		char fpsstr[50];
		sprintf(fpsstr, "FPS: %d", fps);
		fonty->draw_string(buffer,fpsstr,280,4,false);
	}

	//  sprintf(ints, "%d",caca);
	if (*MAP_SHOW_MODE==0 || !smallmap && local_players==2)
	{
		line(buffer,viewport[0].w,0,viewport[0].w,240,makecol(0,0,0));
		line(buffer,viewport[0].w+1,0,viewport[0].w+1,240,makecol(0,0,0));
	};
  	
  minimap();
  scoreboard();
  
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

//weapon HUD
void engine::weaponHUD(BITMAP* where, worm* player, struct s_viewport viewport, int position)
//position: 0 for weapon selection, in the middle; 1 for mid-game, on top-right of viewport
{
	int WEAPWIDTH = 90;
	int WEAPHEIGHT = 60;
	int weapx, weapy;
	if (position==1)
	{		
		weapx= viewport.x + (viewport.w - WEAPWIDTH) - 5;
		weapy= viewport.y + 15;
	}
	else
	{		
		weapx= viewport.x + (viewport.w - WEAPWIDTH)/2;
		weapy= viewport.y + 130;
	}
	if (*WEAPON_HUD && weaps->num[player->weap[player->curr_weap].weap]->image != NULL)
	{
		//drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
		//set_trans_blender(0, 0, 0, 96);
		blit(weaps->num[player->weap[player->curr_weap].weap]->image->img[0], where, 0, 0, weapx, weapy, WEAPWIDTH, WEAPHEIGHT);
		rect(buffer, weapx - 1, weapy - 1, weapx + WEAPWIDTH, weapy + WEAPHEIGHT, makecol(100, 100, 100));
		//solid_mode();
	}
}

void engine::minimap()
{
  int MINIWIDTH = 70;
  int MINIHEIGHT = 48;
  int MINIX = (320 - MINIWIDTH) / 2 - 1;
  int MINIY = 240 - MINIHEIGHT - 1;
  if (local_players==1)
    MINIX=(320 - MINIWIDTH) - 1;

	//TAB hardcoded
  if ((*MINIMAP==1 && key[KEY_TAB]) || *MINIMAP > 1)
  {
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    set_trans_blender(0, 0, 0, 96);
    switch(*MINIMAP_TYPE)
    {
      case 0: masked_stretch_blit(map->mapimg, buffer, 0, 0, map->mapimg->w, map->mapimg->h, MINIX, MINIY, MINIWIDTH, MINIHEIGHT); break;
      case 1: rectfill(buffer, MINIX, MINIY, MINIX + MINIWIDTH, MINIY + MINIHEIGHT, makecol(0, 0, 0)); break;
    }
    rect(buffer, MINIX, MINIY, MINIX + MINIWIDTH, MINIY + MINIHEIGHT, makecol(230, 100, 100));
    solid_mode();
    //
  
    int mapwidth = map->mapimg->w;
    int mapheight = map->mapimg->h;
  
    for (int i = 0; i < player_count; i++)
    {
      if (player[i]->active)
      {
        //Calculate position
        int x = (MINIWIDTH * player[i]->x) / (mapwidth * 1000);
        int y = (MINIHEIGHT*player[i]->y) / (mapheight * 1000);
        putpixel(buffer, x + MINIX, y + MINIY, player[i]->color);
        putpixel(buffer, x + MINIX + 1, y + MINIY + 1, player[i]->color);
        putpixel(buffer, x + MINIX + 1, y + MINIY, player[i]->color);
        putpixel(buffer, x + MINIX, y + MINIY + 1, player[i]->color);
      }
    }
  }
}

//Scoreboard renderer
void engine::scoreboard()
{
  const int SBWIDTH = 280;
  const int SBHEIGHT = 200;
  const int SBX = (320 - SBWIDTH)/2 - 1;
  const int SBY = (240 - SBHEIGHT)/2 - 1;

  //Only show scoreboard when key pressed
  if (!key[KEY_F1])
    return;

  //Draw scoreboard background
  drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
  set_trans_blender(0, 0, 0, 96);

  rectfill(buffer, SBX+1, SBY+1, SBX + SBWIDTH - 1, SBY + SBHEIGHT - 1, makecol(0, 0, 0));
  rect(buffer, SBX, SBY, SBX + SBWIDTH, SBY + SBHEIGHT, makecol(230, 100, 100));

  //Draw player stats
  //NAME - 16, KILLS - 12, LIVES - 12, DEATHS - 12, PING - 4
  char info[1024] = " NAME            KILLS       LIVES       DEATHS     PING";
  rectfill(buffer, SBX+1, SBY + + 4, SBX + SBWIDTH - 1, SBY + 8, makecol(128, 0, 0));
  game->fonty->draw_string(buffer, info, SBX + 4, SBY + 4, true);

  solid_mode();

  for (int i = 0; i < player_count; i++)
    {
      if (i % 2 == 1)
        {
          drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
          set_trans_blender(0, 0, 0, 96);
          rectfill(buffer, SBX+1, SBY + (game->fonty->chrh + 2) * i + 12, SBX + SBWIDTH - 1, SBY + (game->fonty->chrh + 2) * i + 12 + 4, makecol(32, 32, 32));
          solid_mode();
        }
      //ping
      //sprintf(info, " %-16s%-12i%-12i%-12i%-4i", player[i]->name, player[i]->kills, player[i]->lives, player[i]->deaths, 0);
			if (srv)
				sprintf(info, " %-16s%-12c%-12c%-12i%-4i", player[i]->name, '-', '-', player[i]->deaths, player[i]->ping);
			else if (cli)
			{
				// change this condition to a real check of "is server" if possible
				if (player[i]->ping == 0)
					sprintf(info, " %-16s%-12c%-12c%-12i%-4i", player[i]->name, '-', '-', player[i]->deaths, cli->ZCom_getConnectionStats(player[i]->id).avg_ping);
				else
					sprintf(info, " %-16s%-12c%-12c%-12i%-4i", player[i]->name, '-', '-', player[i]->deaths, player[i]->ping);
			}
			else
				sprintf(info, " %-16s%-12c%-12c%-12i%-4c", player[i]->name, '-', '-', player[i]->deaths, '-');
      game->fonty->draw_string(buffer, info, SBX + 4, SBY + 12 + 8 * i, true);
      //Draw players color
      rectfill(buffer, SBX + 2, SBY + (game->fonty->chrh + 2) * i + 12, SBX + 6, SBY + 12 + (game->fonty->chrh + 2) * i + 4, player[i]->color);
    }
}


