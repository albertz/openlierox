#include "input.h"
#include "player.h"
#include "weapons.h"
#include "engine.h"
#include "sounds.h"
#include "console.h"
#include "network.h"
#include "particles.h"
#include "ai.h"

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
	int i;
	
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
#ifdef WORMAI
    //Check if AI player
    if (i == 1)
    {
	((wormai*)player[local_player[i]])->update();
    }
#endif
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
      player[i]->walk(1,*ACELERATION,*MAX_SPEED);
    };
    //if(player[i]->islocal);
    if (player[i]->keys->left && !player[i]->keys->right && !player[i]->keys->change)
    {
      player[i]->flagleft=true;
      player[i]->walk(-1,*ACELERATION,*MAX_SPEED);
    };
    
    if(!cli)
    if (player[i]->keys->left && player[i]->keys->right && !player[i]->keys->change && !player[i]->flag3)
    {
      player[i]->dig(worm_hole);
      if (srv) player[i]->send_dig();
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
      //allegro_message("%d",player[i]->local_slot);
    };
    
    if(player[i]->islocal)
    if (player[i]->keys->down && (player[i]->ropestate==0||!player[i]->keys->change) && player[i]->aim_speed>-*pl_options[player[i]->local_slot].aim_maxspeed)
    {
      player[i]->aim_speed-=*pl_options[player[i]->local_slot].aim_acceleration;
    };

    if(!cli)
    if (player[i]->keys->jump && !player[i]->keys->change)
    {
      player[i]->jump(*WORM_JUMP_FORCE);
      if (!player[i]->ropeflag)
      player[i]->ropestate=0;
    };
    if(!cli)
    if (player[i]->keys->fire && !player[i]->keys->change)
    {
      player[i]->shoot();
    }else
    {
      if(player[i]->fireing && weaps->num[player[i]->weap[player[i]->curr_weap].weap]->create_on_release!=NULL && player[i]->weap[player[i]->curr_weap].start_delay==0)
        partlist.create_part(player[i]->x,player[i]->y-4000,0,0,-1,weaps->num[player[i]->weap[player[i]->curr_weap].weap]->create_on_release);
      player[i]->fireing=false;
    };
    /*if (player[i]->aim>128000) player[i]->aim=128000;
    if (player[i]->aim<24000) player[i]->aim=24000;*/
    
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
  } else if (player[i]->keys->jump && player[i]->health>=*START_HEALTH)
  {
    respawn_player(player[i]);
    player[i]->keys->jump=false;
  };

};

	

