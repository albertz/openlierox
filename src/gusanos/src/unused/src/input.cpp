#include "input.h"


void randomize_weap(struct worm *player,int o)
{
  player->weap[o].weap=rand()%weaps->weap_count;
};

void weap_selection_input()
{
  int i;
  worm *p;
  for(i=0;i<local_players;i++)
  {
    p=player[local_player[i]];
    if (p->keys->down && !p->flag && p->selecting_weaps)
    {
      if (p->curr_weap<4)
      {
        p->curr_weap++;
      } else p->curr_weap=0;
      p->flag=true;
      play_sample(game->menu_move->snd, *game->VOLUME, 127, 1000, 0);
    };
    if (p->keys->up && !p->flag && p->selecting_weaps)
    {
      if (p->curr_weap>0)
      {
        p->curr_weap--;
      } else p->curr_weap=4;
      p->flag=true;
      play_sample(game->menu_move->snd, *game->VOLUME, 127, 1000, 0);
    };
    if (p->keys->right && !p->flag && p->selecting_weaps)
    {
      if (p->weap[p->curr_weap].weap<weaps->weap_count-1)
      {
        p->weap[p->curr_weap].weap++;
      } else p->weap[p->curr_weap].weap=0;
      p->flag=true;
    };
    if (p->keys->left && !p->flag && p->selecting_weaps)
    {
      if (p->weap[p->curr_weap].weap>0)
      {
        p->weap[p->curr_weap].weap--;
      } else p->weap[p->curr_weap].weap=weaps->weap_count-1; 
      p->flag=true;
    };
    if (p->keys->fire && p->selecting_weaps)
    {
      p->selecting_weaps=false;
      play_sample(game->menu_select->snd, *game->VOLUME, 127, 1000, 0);
      recharge_weapons(p);
    };
    if (p->keys->change && p->selecting_weaps)
    {
      int o,u;
      bool repeated=true;
      for(o=0;o<5;o++)
      {
        if(game->weap_count>4)
        {
          repeated=true;
          while(repeated)
          {
            randomize_weap(p,o);
            repeated=false;
            for(u=0;u<o;u++)
            {
              if (p->weap[u].weap==p->weap[o].weap)
              repeated=true;
            };
          };
        }
        else randomize_weap(p,o);
      };
    };
    if (!p->keys->up && !p->keys->down && !p->keys->left && !p->keys->right)
		{
			p->flag=false;
		};
  };
};

void engine::input()
{
	int h,i;
	
	if (key[KEY_TILDE] && !con->flag2 && con->flag==1)
	{
		con->flag2=true;
		con->flag=0;
		clear_keybuf();
	};
	
	if (key[KEY_TILDE] && !con->flag2 && con->flag==0)
	{
		con->flag2=true;
		con->flag=1;
		strcpy(con->textbuf,"");
		con->tmp_com=con->log.start;
		clear_keybuf();
	};
	
	if (!key[KEY_TILDE]) con->flag2=false;
  for (i=0;i<local_players;i++)
  {
    player[local_player[i]]->keys->up=false;
    player[local_player[i]]->keys->down=false;
    player[local_player[i]]->keys->right=false;
    player[local_player[i]]->keys->left=false;
    player[local_player[i]]->keys->jump=false;
    player[local_player[i]]->keys->fire=false;
    player[local_player[i]]->keys->change=false;
  };
  
  con->input();
  
  if (game->selecting)
    weap_selection_input();
  else
  for (i=0;i<player_count;i++)
  if (player[i]->active) {
    //if(player[i]->islocal)
    if (player[i]->keys->right && !player[i]->keys->left && !player[i]->keys->change)
    {
      player[i]->flagright=true;
      player[i]->dir=1;
      if (player[i]->xspd<*MAX_SPEED)
      {
        player[i]->xspd+=+*ACELERATION;
      };
      player[i]->curr_frame+=120;
    };
    //if(player[i]->islocal);
    if (player[i]->keys->left && !player[i]->keys->right && !player[i]->keys->change)
    {
      player[i]->flagleft=true;
      player[i]->dir=-1;
      if (player[i]->xspd>-*MAX_SPEED)
      {
        player[i]->xspd-=*ACELERATION;
      };
      player[i]->curr_frame+=120;
    };
    if (player[i]->curr_frame>=3000) player[i]->curr_frame=0;
    
    if(!cli)
    if (player[i]->keys->left && player[i]->keys->right && !player[i]->keys->change && !player[i]->flag3)
    {
      int x2,y2;
      x2=fixtof(fixsin(ftofix(player[i]->aim/1000.)))*2000*player[i]->dir;
      y2=fixtof(fixcos(ftofix(player[i]->aim/1000.)))*2000;
      create_exp(player[i]->x+x2,player[i]->y-4000+y2,worm_hole);
      x2=fixtof(fixsin(ftofix(player[i]->aim/1000.)))*4000*player[i]->dir;
      y2=fixtof(fixcos(ftofix(player[i]->aim/1000.)))*4000;
      create_exp(player[i]->x+x2,player[i]->y-4000+y2,worm_hole);
      x2=fixtof(fixsin(ftofix(player[i]->aim/1000.)))*6000*player[i]->dir;
      y2=fixtof(fixcos(ftofix(player[i]->aim/1000.)))*6000;
      create_exp(player[i]->x+x2,player[i]->y-4000+y2,worm_hole);
      
      player[i]->flag3=true;
    };
    
    if (!player[i]->keys->left && !player[i]->keys->right)
    {
      player[i]->curr_frame=1;
      player[i]->flagleft=false;
      player[i]->flagright=false;
    };
    
    if(!cli && !player[i]->keys->jump)
    {
      player[i]->ropeflag=false;
    };
    
    if (!player[i]->keys->left || !player[i]->keys->right)
    {
      player[i]->flag3=false;
    };
    
    if(player[i]->islocal)
    if (player[i]->keys->up && (player[i]->ropestate==0||!player[i]->keys->change) && player[i]->aim_speed<*pl_options[player[i]->local_slot].aim_maxspeed)
    {
      player[i]->aim_speed+=*pl_options[player[i]->local_slot].aim_acceleration;
    };
    
    if(player[i]->islocal)
    if (player[i]->keys->down && (player[i]->ropestate==0||!player[i]->keys->change) && player[i]->aim_speed>-*pl_options[player[i]->local_slot].aim_maxspeed)
    {
      player[i]->aim_speed-=*pl_options[player[i]->local_slot].aim_acceleration;
    };
    if (player[i]->aim>128000) 
    {
      player[i]->aim=128000;
      player[i]->aim_speed=0;
    };
    if (player[i]->aim<24000) 
    {
      player[i]->aim=24000;
      player[i]->aim_speed=0;
    };
    if(!cli)
    if (player[i]->keys->jump && !player[i]->keys->change)
    {
      int g; 
      
      if (getpixel(map->material,player[i]->x/1000,(player[i]->y+player[i]->yspd-4500)/1000)==3)
      {
        player[i]->yspd+=47;
      }else
      {
        g=getpixel(map->material,player[i]->x/1000,(player[i]->y+player[i]->yspd)/1000+1);
        if (!map->mat[g+1].worm_pass)
        {
          player[i]->yspd-= *WORM_JUMP_FORCE;
          //allegro_message("%d",player[i]->yspd);
        };
      };
      if (!player[i]->ropeflag)
      player[i]->ropestate=0;
    };
    if(!cli)
    if (player[i]->keys->fire && !player[i]->keys->change)
    {
      if (player[i]->weap[player[i]->curr_weap].shoot_time==0)//>weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_times)
      if (player[i]->weap[player[i]->curr_weap].ammo>0 || weaps->num[player[i]->weap[player[i]->curr_weap].weap]->ammo==0)
      if ((!player[i]->fireing && weaps->num[player[i]->weap[player[i]->curr_weap].weap]->autofire!=1) || weaps->num[player[i]->weap[player[i]->curr_weap].weap]->autofire==1)
      {
        if (!player[i]->fireing)
        {
          player[i]->weap[player[i]->curr_weap].start_delay=weaps->num[player[i]->weap[player[i]->curr_weap].weap]->start_delay;
          if(weaps->num[player[i]->weap[player[i]->curr_weap].weap]->start_sound!=NULL)play_sample(weaps->num[player[i]->weap[player[i]->curr_weap].weap]->start_sound->snd, *VOLUME, 127, 1000, 0);
        };
        if (player[i]->weap[player[i]->curr_weap].start_delay>0)player[i]->weap[player[i]->curr_weap].start_delay--;
        player[i]->fireing=true;        
        if (player[i]->weap[player[i]->curr_weap].start_delay==0)
        {
          if(srv) player[i]->shooteventsend();
          player[i]->weap[player[i]->curr_weap].ammo--;
          if (weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_num!=0)
          {
            int dist,spd_rnd,xof,yof;
            
            for (h=0;h<weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_num;h++)
            {
              dist=((rand()%1000)*weaps->num[player[i]->weap[player[i]->curr_weap].weap]->distribution)-weaps->num[player[i]->weap[player[i]->curr_weap].weap]->distribution/2*1000;
              if (weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_spd_rnd!=0)
                spd_rnd=(rand()%weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_spd_rnd)-weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_spd_rnd/2;
              else spd_rnd=0;
              xof=fixtof(fixsin(ftofix((player[i]->aim-dist)/1000.)))*(int)(weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_obj->detect_range+1000)*player[i]->dir;
              yof=fixtof(fixcos(ftofix((player[i]->aim-dist)/1000.)))*(int)(weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_obj->detect_range+1000);
              partlist.shoot_part(player[i]->aim-dist,weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_spd-spd_rnd,player[i]->dir,player[i]->x+xof,player[i]->y-4000+yof,player[i]->xspd*(weaps->num[player[i]->weap[player[i]->curr_weap].weap]->affected_motion/1000.),player[i]->yspd*(weaps->num[player[i]->weap[player[i]->curr_weap].weap]->affected_motion/1000.),player[i],weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_obj);
            };
            if (weaps->num[player[i]->weap[player[i]->curr_weap].weap]->aim_recoil!=0)
              player[i]->aim_recoil_speed+=(100*weaps->num[player[i]->weap[player[i]->curr_weap].weap]->aim_recoil);/*-weaps->num[player[i]->weap[player[i]->curr_weap].weap]->aim_recoil/2*1000;*/
            if (weaps->num[player[i]->weap[player[i]->curr_weap].weap]->recoil!=0)
            {
              player[i]->xspd = player[i]->xspd + -fixtof(fixsin(ftofix(player[i]->aim/1000.)))*weaps->num[player[i]->weap[player[i]->curr_weap].weap]->recoil*player[i]->dir;
              player[i]->yspd = player[i]->yspd + -fixtof(fixcos(ftofix(player[i]->aim/1000.)))*weaps->num[player[i]->weap[player[i]->curr_weap].weap]->recoil;
            };
          };
          player[i]->weap[player[i]->curr_weap].shoot_time=weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_times+1;
          player[i]->firecone_time=weaps->num[player[i]->weap[player[i]->curr_weap].weap]->firecone_timeout;
          player[i]->curr_firecone=weaps->num[player[i]->weap[player[i]->curr_weap].weap]->firecone;
          if (weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_sound!=NULL)
          {
            //if (player[i]->weap->loop_sound!=1)
            play_sample(weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_sound->snd, *VOLUME, 127, 1000, 0);
            /*else if (!player[i]->sound_loop)
            {
            play_sample(player[i]->weap->shoot_sound->snd, 255, 127, 1000, 1);
            player[i]->sound_loop=true;
            };*/
          };
        };
      };
      if (player[i]->weap[player[i]->curr_weap].ammo==0 && !player[i]->fireing)
      {
        player[i]->fireing=true;
        if(weaps->num[player[i]->weap[player[i]->curr_weap].weap]->noammo_sound!=NULL)
          play_sample(weaps->num[player[i]->weap[player[i]->curr_weap].weap]->noammo_sound->snd, *VOLUME, 127, 1000, 0);
        /*if (player[i]->weap[player[i]->curr_weap].shoot_time>weaps->num[player[i]->weap[player[i]->curr_weap].weap]->shoot_times)
        {
          player[i]->weap[player[i]->curr_weap].shoot_time=0;
        };*/
          
      };
    }else
    {
      if(player[i]->fireing && weaps->num[player[i]->weap[player[i]->curr_weap].weap]->create_on_release!=NULL && player[i]->weap[player[i]->curr_weap].start_delay==0)
        partlist.create_part(player[i]->x,player[i]->y-4000,0,0,NULL,weaps->num[player[i]->weap[player[i]->curr_weap].weap]->create_on_release);
      player[i]->fireing=false;
    };
    if (player[i]->aim>128000) player[i]->aim=128000;
    if (player[i]->aim<24000) player[i]->aim=24000;
    
    /*if (!key[player[i]->keys->fire] && player[i]->sound_loop)
    {
      player[i]->sound_loop=false;
      stop_sample(player[i]->weap->shoot_sound->snd);
    };*/
    
    if (player[i]->keys->change)
    {
      player[i]->flag2=true;
      
      if(!cli)
      if (player[i]->keys->jump && !player[i]->ropeflag)
      {
        player[i]->shootrope();
        play_sample(throwrope->snd, *VOLUME, 127, 1000, 0);
        player[i]->ropeflag=true;
      };
      if(player[i]->islocal)
      if (player[i]->keys->up && player[i]->rope_length>6000)
      {
        player[i]->rope_length-=1000;
      };
      if(player[i]->islocal)
      if (player[i]->keys->down)
      {
        player[i]->rope_length+=1000;
      };
      if(player[i]->islocal)
      if (player[i]->keys->right && !player[i]->flagright)
      {
        if (player[i]->curr_weap<4)
        {
          player[i]->curr_weap++;
        } else player[i]->curr_weap=0;
        player[i]->flagright=true;
      };
      if(player[i]->islocal)
      if (player[i]->keys->left && !player[i]->flagleft)
      {
        if (player[i]->curr_weap>0)
        {
          player[i]->curr_weap--;
        } else player[i]->curr_weap=4;
        player[i]->flagleft=true;
      };
    } else player[i]->flag2=false;
  } else if (player[i]->keys->jump && player[i]->health>=*MAX_HEALTH)
  {
    respawn_player(player[i]);
    player[i]->keys->jump=false;
  };

};

	

