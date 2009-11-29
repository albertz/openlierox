#include "player.h"
#include "engine.h"
#include "console.h"
#include "sounds.h"
#include "network.h"
#include "weapons.h"
#include "explosions.h"
#include "level.h"
#include "particles.h"
#include "sprites.h"
#include "text.h"


worm* player[32];
struct s_player_options pl_options[2];
int local_player[2];
int player_count=0;
int local_players=0;
ZCom_ClassID  player_classid = ZCom_Invalid_ID;

void init_players()
{
  int i;
  for (i=0;i<32;i++)
  {
    player[i]=NULL;
  };
  player_count=0;
  local_players=0;
};

void delete_players()
{
  int i;
  for (i=0;i<32;i++)
  {
    if (player[i]){
      delete player[i];
      player[i]=NULL;
    };
  };
  player_count=0;
  local_players=0;
};

void send_msg()
{
  if(player[local_player[0]] && player[local_player[0]]->node)
  {
    player[local_player[0]]->sendmsg(con->arg);
    if(player[local_player[0]]->node->getRole()==eZCom_RoleAuthority)
    {
      char msg2[1024];
      sprintf(msg2,"[%s] %s",player[local_player[0]]->name,con->arg);
      con->echolist.add_echo(msg2);
      play_sample(game->menu_select->snd, *game->VOLUME, 127, 1300, 0);
    };
  };
};

//skins
void worm::load_skin(std::string name)
{
	skin=sprites->load_sprite(("/skins/" + name + "/image").c_str(),21,game->mod,game->v_depth);
	mask=sprites->load_sprite(("/skins/" + name + "/mask").c_str(),21,game->mod,game->v_depth);
	if (skin==NULL || mask == NULL)
	{
		con->log.create_msg(("SKIN \"" + name + "\" NOT FOUND").c_str()); 
		skin=sprites->load_sprite("/skins/default/image",21,game->mod,game->v_depth);
		mask=sprites->load_sprite("/skins/default/mask",21,game->mod,game->v_depth);
	}
}

void worm::change_team(int _team)
{
  if (!cli)
  {
    health=0;
    team=_team;
  } else
  {
    ZCom_BitStream *data=ZCom_Control::ZCom_createBitStream();
    data->addInt(5,8);
    data->addInt(_team,8);
    node->sendEvent(ZCom_Node::eEventMode_ReliableOrdered,ZCOM_REPRULE_OWNER_2_AUTH, data);
  }
};

void worm::send_dig()
{
  if(node)
  {
    ZCom_BitStream *data=ZCom_Control::ZCom_createBitStream();
    data->addInt(4,8);
    data->addInt(x,32);
    data->addInt(y,32);
    data->addInt(aim,32);
    data->addSignedInt(dir,8);
    node->sendEvent(ZCom_Node::eEventMode_ReliableOrdered,ZCOM_REPRULE_AUTH_2_ALL, data);
  };
};

void worm::sendmsg(char *msg)
{
  if(node)
  {
    ZCom_BitStream *data=ZCom_Control::ZCom_createBitStream();;
    data->addInt(3,8);
    data->addString( msg );
    node->sendEvent(ZCom_Node::eEventMode_ReliableOrdered, ZCOM_REPRULE_OWNER_2_AUTH | ZCOM_REPRULE_AUTH_2_ALL, data);
  };
};

void change_nick(int pl)
{
  strcpy(player[local_player[pl]]->name,pl_options[pl].name);
};

void worm::shooteventsend()
{
  int t=rand();
  ZCom_BitStream *data=ZCom_Control::ZCom_createBitStream();;
  data->addInt(1,8);
  data->addInt(x,32);
  data->addInt(y,32);
  data->addInt(xspd,32);
  data->addInt(yspd,32);
  data->addInt(aim,32);
  data->addSignedInt(dir,8);
  data->addInt(weap[curr_weap].weap,16);
  data->addInt(t,32);

  node->sendEvent(ZCom_Node::eEventMode_ReliableOrdered, ZCOM_REPRULE_AUTH_2_ALL, data);
  srand(t);
};

void worm::deatheventsend()
{
  int t=rand();
  ZCom_BitStream *data=ZCom_Control::ZCom_createBitStream();;
  data->addInt(2,8); 
  data->addInt(x,32);
  data->addInt(y,32);
  data->addInt(xspd,32);
  data->addInt(yspd,32);
  data->addInt(t,32);
  if (killed_by!=-1 && player[killed_by])
    data->addSignedInt(player[killed_by]->node->getNetworkID(),32);
  else data->addSignedInt(-1,32);

  node->sendEvent(ZCom_Node::eEventMode_ReliableOrdered, ZCOM_REPRULE_AUTH_2_ALL, data);
  srand(t);
};

void worm::checkevents()
{
  while (node->checkEventWaiting()) 
  {
    ZCom_Node::eEvent type;            // event type
    eZCom_NodeRole    remote_role;     // role of remote sender
    ZCom_ConnID       conn_id;         // connection id of sender
    int event;
  
    ZCom_BitStream *data = node->getNextEvent(&type, &remote_role, &conn_id);
    if (type == ZCom_Node::eEvent_AuthorityRemoved)
		{
			deleteme = true;
		} else
    if (type == ZCom_Node::eEvent_User)
    {
      event=data->getInt(8);
			//changed team
      if(event==5)
      {
        int t=data->getInt(8);
        team=t;
        health=0;
			//dig?
      }else if(event==4)
      {
        int _x=data->getInt(32);
        int _y=data->getInt(32);
        int _ang=data->getInt(32);
        int _dir=data->getSignedInt(8);
        int x2,y2;
        x2=fixtoi(fixsin(ftofix(_ang/1000.))*2000)*_dir;
        y2=fixtoi(fixcos(ftofix(_ang/1000.))*2000);
        create_exp(_x+x2,_y-4000+y2,game->worm_hole);
        x2=fixtoi(fixsin(ftofix(_ang/1000.))*4000)*_dir;
        y2=fixtoi(fixcos(ftofix(_ang/1000.))*4000);
        create_exp(_x+x2,_y-4000+y2,game->worm_hole);
        x2=fixtoi(fixsin(ftofix(_ang/1000.))*6000)*_dir;
        y2=fixtoi(fixcos(ftofix(_ang/1000.))*6000);
        create_exp(_x+x2,_y-4000+y2,game->worm_hole);
			//chat message
      }else if(event==3)
      {
        char msg[1024],msg2[1024];
        strcpy(msg,data->getStringStatic());
        sprintf(msg2,"[%s] %s",name,msg);
        //strcat(msg2,msg);
        //con->log.create_msg(msg2);
        con->echolist.add_echo(msg2);
        if (node->getRole()==eZCom_RoleAuthority)
        {
          sendmsg(msg);
        };
        play_sample(game->menu_select->snd, *game->VOLUME, 127, 1300, 0);
			//died
      }else if(event==2)
      {
        int o,i;
        int _x=data->getInt(32);
        int _y=data->getInt(32);
        int _xspd=data->getInt(32);
        int _yspd=data->getInt(32);
        int _t=data->getInt(32);
        int _id=data->getSignedInt(32);
        srand(_t);
        for (o=0;o<14;o++)
          partlist.shoot_part(rand()%1000*255,(rand()%200)+600,1,_x,_y-4000,_xspd/2,_yspd/2,local_slot,game->gore);
        play_sample(game->death->snd, *game->VOLUME, 127, 1000, 0);
				//homing
				partlist.player_removed(local_slot,true);
        deaths++;
        health=0;
        if (*game->RESPAWN_RELOAD==1)
          recharge_weapons(this);

        char tmpstr[1024];
        sprintf(tmpstr,"* %s DIED",name);
        if (node->getNetworkID()!=-1)
        {
          if (node->getNetworkID()==_id)
          {
            sprintf(tmpstr,"* %s KILLED HIMSELF, WHAT A N00B",name);
          }else
          {
            for (i=0;i<player_count;i++)
            {
              if (player[i]->node->getNetworkID()==_id)
              {
                if (!game->teamplay || team!=player[i]->team)
                  sprintf(tmpstr,"* %s KILLED %s",player[i]->name,name);
                else
                  sprintf(tmpstr,"* %s TEAM KILLED %s",player[i]->name,name);
								//talk death message
								if (talking)
									strcat(tmpstr, " WHILE HE WAS TALKING");
								//break; ?
              }              
            };
          };
        };
        con->echolist.add_echo(tmpstr);
			//shot
      }else if(event==1)
      {
        int _x=data->getInt(32);
        int _y=data->getInt(32);
        int _xspd=data->getInt(32);
        int _yspd=data->getInt(32);
        int _ang=data->getInt(32);
        int _dir=data->getSignedInt(8);
        int _weap=data->getInt(16);
        int _t=data->getInt(32);
        srand(_t);
        fireing=true;
        weap[curr_weap].ammo--;
        if (weaps->num[_weap]->shoot_num!=0)
        {
          int dist,spd_rnd,xof,yof,h;
                
          for (h=0;h<weaps->num[_weap]->shoot_num;h++)
          {
            dist=((rand()%1000)*weaps->num[_weap]->distribution)-weaps->num[_weap]->distribution/2*1000;
            if (weaps->num[_weap]->shoot_spd_rnd!=0)
              spd_rnd=(rand()%weaps->num[_weap]->shoot_spd_rnd)-weaps->num[_weap]->shoot_spd_rnd/2;
            else spd_rnd=0;
            xof=fixtoi(fixsin(ftofix((_ang-dist)/1000.))*(weaps->num[_weap]->shoot_obj->detect_range+1000))*_dir;
            yof=fixtoi(fixcos(ftofix((_ang-dist)/1000.))*(weaps->num[_weap]->shoot_obj->detect_range+1000));
            partlist.shoot_part(_ang-dist,weaps->num[_weap]->shoot_spd-spd_rnd,_dir,_x+xof,_y-4000+yof,(_xspd*weaps->num[_weap]->affected_motion)/1000,(_yspd*weaps->num[_weap]->affected_motion)/1000,local_slot,weaps->num[_weap]->shoot_obj);
          };
          if (weaps->num[_weap]->aim_recoil!=0)
            aim_recoil_speed+=(100*weaps->num[_weap]->aim_recoil);/*-weap[curr_weap].weap->aim_recoil/2*1000;*/
          if (weaps->num[_weap]->recoil!=0)
          {
            xspd = xspd + -fixtoi(fixsin(ftofix(_ang/1000.))*weaps->num[_weap]->recoil)*_dir;
            yspd = yspd + -fixtoi(fixcos(ftofix(_ang/1000.))*weaps->num[_weap]->recoil);
          };
        };
        weap[curr_weap].shoot_time=0;
        firecone_time=weaps->num[_weap]->firecone_timeout;
        curr_firecone=weaps->num[_weap]->firecone;
        if (weaps->num[_weap]->shoot_sound!=NULL)
        {
          //if (weap->loop_sound!=1)
          play_sample(weaps->num[_weap]->shoot_sound->snd, *game->VOLUME, 127, 1000, 0);
          /*else if (!sound_loop)
          {
            play_sample(weap->shoot_sound->snd, 255, 127, 1000, 1);
            sound_loop=true;
          };*/
        };
      };
    };
  }
};

worm::worm()
{
    int o;
    strcpy(name,"PLAYER");
    weap = (struct s_playerweap*) malloc(sizeof(struct s_playerweap)*5);
		x=80*1000;
		y=40*1000;
    local_slot=0;
		xspd=0;
		yspd=0;
		aim=64000;
		dir=1;
		crossr=20;
		health=1000;
		deaths=0;
		kills = 0;
		lives = 0;
		curr_weap=0;
    aim_speed=0;
    aim_recoil_speed=0;
    flash=0;
    killed_by=-1;
    team=0;
		//talking
		talking=false;
    color=makecol(100,100,220);
    for(o=0;o<5;o++)
    {
      weap[o].weap=0;
      weap[o].shoot_time=0;
      weap[o].ammo=weaps->num[weap[o].weap]->ammo;
      weap[o].reloading=false;
    };
		curr_frame=2700;
		crosshair=sprites->load_sprite("crosshair",1,game->mod,game->v_depth);
		active=false;
		flag=false;
    islocal=false;
    deleteme=false;
    selecting_weaps=true;
    local_slot=local_players;
		ropestate=0;
		ropex=1;
    ropexspd=1;
		ropey=1;
    ropeyspd=1;
    curr_firecone=NULL;
    firecone_time=0;
    skin=sprites->load_sprite("/skins/default/image",21,game->mod,game->v_depth);
    mask=sprites->load_sprite("/skins/default/mask",21,game->mod,game->v_depth);
    keys=new struct KEYS;
    keys->up=false;
    keys->down=false;
    keys->right=false;
    keys->left=false;
    keys->jump=false;
    keys->fire=false;
    keys->change=false;
    node=NULL;
    
    id = ZCom_Invalid_ID;
    
};

worm::~worm()
{
  delete node;
  delete keys;
  free (weap);
};

void worm::init_node(bool is_authority)
{
  node = new ZCom_Node();
  if (!node)
  {
    con->log.create_msg("unable to create node");
  }
  
  node->beginReplicationSetup();
    
    //Authority replication items
    node->addInterpolationInt((zS32*)&x,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,20000,NULL,-1,-1,0.2f);
    node->addInterpolationInt((zS32*)&y,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,20000,NULL,-1,-1,0.2f);
    node->addReplicationInt((zS32*)&ropestate,3,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,-1,-1);
    node->addReplicationInt((zS32*)&ropex,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,300,-1);
    node->addReplicationInt((zS32*)&ropey,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,300,-1);
    node->addReplicationInt((zS32*)&ropexspd,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,300,-1);
    node->addReplicationInt((zS32*)&ropeyspd,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,300,-1);
    node->addReplicationInt((zS32*)&dir,2,true,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_PROXY,99,-1,-1);
    node->addReplicationInt((zS32*)&health,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,0,-1,-1 );
    node->addReplicationBool(&active,ZCOM_REPFLAG_RARELYCHANGED,ZCOM_REPRULE_AUTH_2_ALL,false,-1,-1 );
    node->addReplicationInt((zS32*)&team,8,false,ZCOM_REPFLAG_RARELYCHANGED|ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,0,-1,-1 );
		//ping (maybe ZCOM_REPRULE_AUTH_2_PROXY instead)
		node->addReplicationInt((zS32*)&ping,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,1000,-1);
    //node->addReplicationInt((zS32*)&weap[0].ammo,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,0,-1,-1 );
  
    //Owner replication items
    node->addReplicationInt((zS32*)&aim,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY,99,-1,-1);
    node->addReplicationInt((zS32*)&color,32,false,ZCOM_REPFLAG_RARELYCHANGED|ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY,0,-1,-1 );
    node->addReplicationString(name,32,ZCOM_REPFLAG_RARELYCHANGED|ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY," ",-1,-1 );
    node->addReplicationBool(&talking,ZCOM_REPFLAG_RARELYCHANGED|ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY,false,-1,-1 );
    
    //Keys structure replication
    node->addReplicationBool(&keys->fire,0,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->left,0,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->right,0,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->up,0,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->down,0,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->jump,0,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->change,0,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    
    //Weapons replication
    node->addReplicationInt((zS32*)&curr_weap,8,true,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY,0,-1,-1);
    for(int i=0;i<5;i++)
      node->addReplicationInt((zS32*)&weap[i].weap,32,false,ZCOM_REPFLAG_RARELYCHANGED|ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY,0,-1,-1 );
    
  node->endReplicationSetup();

  if(is_authority)
  {
    if(!node->registerNodeDynamic(player_classid, srv))
    allegro_message("unable to register player node");
  }else
  {
    if(!node->registerRequestedNode(player_classid, cli))
    allegro_message("was unable to create the player node");
  };
    
  node->applyForZoidLevel(2);
};

void worm::walk(int direction,int acceleration, int maxspeed)
{
  dir=direction;
  if (dir*xspd<maxspeed)
  {
    xspd+=acceleration*dir;
  };
  curr_frame+=120;
  if (curr_frame>=3000) curr_frame=0;
};

void worm::dig(exp_type *hole)
{
  int x2,y2;
  x2=fixtoi(fixsin(ftofix(aim/1000.))*2000)*dir;
  y2=fixtoi(fixcos(ftofix(aim/1000.))*2000);
  create_exp(x+x2,y-4000+y2,hole);
  x2=fixtoi(fixsin(ftofix(aim/1000.))*4000)*dir;
  y2=fixtoi(fixcos(ftofix(aim/1000.))*4000);
  create_exp(x+x2,y-4000+y2,hole);
  x2=fixtoi(fixsin(ftofix(aim/1000.))*6000)*dir;
  y2=fixtoi(fixcos(ftofix(aim/1000.))*6000);
  create_exp(x+x2,y-4000+y2,hole);
  flag3=true;
};

void worm::jump(int jump_force)
{
  int g; 
  
  if (getpixel(map->material,x/1000,(y+yspd-4500)/1000)==3)
  {
    yspd+=47;
  }else
  {
    g=getpixel(map->material,x/1000,(y+yspd)/1000+1);
    if (!map->mat[g+1].worm_pass)
    {
        yspd-= jump_force;//*WORM_JUMP_FORCE;
        //allegro_message("%d",player[i]->yspd);
    };
  };
};

void worm::shoot()
{
  if (weap[curr_weap].shoot_time==0)
		if (weap[curr_weap].ammo>0 || weaps->num[weap[curr_weap].weap]->ammo==0)
			if ((!fireing && weaps->num[weap[curr_weap].weap]->autofire!=1) || weaps->num[weap[curr_weap].weap]->autofire==1)
			{
				if (!fireing)
				{
					weap[curr_weap].start_delay=weaps->num[weap[curr_weap].weap]->start_delay;
					if(weaps->num[weap[curr_weap].weap]->start_sound!=NULL)
						play_sample(weaps->num[weap[curr_weap].weap]->start_sound->snd, *game->VOLUME, 127, 1000, 0);
				};
				if (weap[curr_weap].start_delay>0)weap[curr_weap].start_delay--;
				fireing=true;        
				if (weap[curr_weap].start_delay==0)
				{
					if(srv) shooteventsend();
					weap[curr_weap].shoot(x,y,xspd,yspd,aim,dir,local_slot);
				};
			};
  if (weap[curr_weap].ammo==0 && !fireing)
  {
    fireing=true;
    if(weaps->num[weap[curr_weap].weap]->noammo_sound!=NULL)
      play_sample(weaps->num[weap[curr_weap].weap]->noammo_sound->snd, *game->VOLUME, 127, 1000, 0);
  };
};

void worm::render(BITMAP* where, int frame, int _x, int _y)
{
  int r,g,c,R,G,B,i;
  float h1,s1,v1,h,s,v;
  int MASK_COLOR;
  MASK_COLOR=bitmap_mask_color(skin->img[frame]);
  for (i=0;i<skin->img[frame]->w;i++)
  {
    for (r=0;r<skin->img[frame]->h;r++)
    {
      g=getpixel(skin->img[frame],i,r);
      c=getpixel(mask->img[frame],i,r);
      if(g!=MASK_COLOR)
      {
        if(c!=MASK_COLOR)
        {
          c=color;
          rgb_to_hsv(getr(c), getg(c), getb(c), &h1, &s1, &v1);
          
          R = getr(g);
          G = getg(g);
          B = getb(g);
          
          rgb_to_hsv(R, G, B, &h, &s, &v);
          h = h1;
          s = s1;
          v -= (1-v1)/1.4;
          if (v<0)v=0;
          
          hsv_to_rgb(h,s,v, &R, &G, &B);
          g = makecol(R,G,B);
          putpixel(where,_x+i,_y+r,g);
        }else putpixel(where,_x+i,_y+r,g);
      };
    };
  };
};

void worm::render_flip(BITMAP* where, int frame, int _x, int _y)
{
  int r,g,c,R,G,B,i;
  float h1,s1,v1,h,s,v;
  int MASK_COLOR;
  MASK_COLOR=bitmap_mask_color(skin->img[frame]);
  for (i=0;i<skin->img[frame]->w;i++)
  {
    for (r=0;r<skin->img[frame]->h;r++)
    {
      g=getpixel(skin->img[frame],i,r);
      c=getpixel(mask->img[frame],i,r);
      if(g!=MASK_COLOR)
      {
        if(c!=MASK_COLOR)
        {
          c=color;
          rgb_to_hsv(getr(c), getg(c), getb(c), &h1, &s1, &v1);
          
          R = getr(g);
          G = getg(g);
          B = getb(g);
          
          rgb_to_hsv(R, G, B, &h, &s, &v);
          h = h1;
          s = s1;
          v -= (1-v1)/1.4;
          if (v<0)v=0;
          
          hsv_to_rgb(h,s,v, &R, &G, &B);
          g = makecol(R,G,B);
          putpixel(where,_x+skin->img[frame]->w-1-i,_y+r,g);
        }else putpixel(where,_x+skin->img[frame]->w-1-i,_y+r,g);
      };
    };
  };
};

void worm::shootrope() 
{
	ropex=x;
	ropey=y-4000; 
	ropexspd=fixtoi(fixsin(ftofix(aim/1000.))*3500)*dir;
	ropeyspd=fixtoi(fixcos(ftofix(aim/1000.))*3500);
	rope_length=*game->ROPE_LENGHT;
	ropestate=1;
};

void worm::destroyrope()
{
	ropestate=0;
};

void worm::applyropeforce()
{
	int dx,dy,m;
	dx=x-ropex;
	dy=y-4000-ropey;
	m=fixhypot(dx,dy);
	if (m>rope_length)
	{
		xspd-=(dx* *game->ROPE_STRENTH)/m;
		yspd-=(dy* *game->ROPE_STRENTH)/m;
		/*px=((dx/1000.)*(rope_length/1000.))/(m/1000.);
		py=((dy/1000.)*(rope_length/1000.))/(m/1000.);
		x=ropex+px*1000;
		y=ropey+py*1000+4000;
		putpixel(map->mapimg,(ropex+dx)/1000,(ropey+dy)/1000,makecol(0,255,0));
		putpixel(map->mapimg,(x)/1000,(y)/1000,makecol(255,0,0));
		xspd-=(ropex+dx)-x;
		yspd-=(ropey+dy)-y+4000;*/
		
	};/*	else if (m<rope_length)
  {
    xspd+=(dx* *game->ROPE_STRENTH)/m;
		yspd+=(dy* *game->ROPE_STRENTH)/m;
  };*/
};

void worm::remove_player (int n)
{
	con->log.create_msg("PLAYER REMOVED");
	delete player[n];
	partlist.player_removed(n);
	player[n]=player[player_count-1];
	player[player_count-1]=NULL;
	player_count--;
};

void calcrope(struct worm *player)
{
	int g;
	
	if (player->ropestate!=0)
	{
		g=getpixel(map->material,player->ropex/1000,player->ropey/1000);
		if (!map->mat[g+1].particle_pass)
		{
			//if(player->ropestate!=2)
			//{
				//player->ropestate=2;
				//dx=player->x-player->ropex;
				//dy=player->y-4000-player->ropey;
				//m=fixhypot(dx,dy);
				//player->rope_length=m;
			//};
			if(!cli)
			{
	      player->applyropeforce();
				player->ropexspd=player->ropeyspd=0;
			};
		}
		else
		{
			//player->ropestate=1;
			player->ropex+=player->ropexspd;
			player->ropey+=player->ropeyspd;
      player->ropeyspd+=*game->ROPE_GRAVITY;
		}
	};
};

//ping
void calcping(worm *player)
{
	player->ping=srv->ZCom_getConnectionStats(player->id).avg_ping;
}

void pl1_moveright()
{
  if(player[local_player[1]] && player[local_player[1]]->islocal)
	player[local_player[1]]->keys->right=true;
};

void pl1_moveleft()
{
  if(player[local_player[1]] && player[local_player[1]]->islocal)
	player[local_player[1]]->keys->left=true;
};
void pl1_aimup()
{
  if(player[local_player[1]] && player[local_player[1]]->islocal)
	player[local_player[1]]->keys->up=true;
};
void pl1_aimdown()
{
  if(player[local_player[1]] && player[local_player[1]]->islocal)
	player[local_player[1]]->keys->down=true;
};
void pl1_fire()
{
  if(player[local_player[1]] && player[local_player[1]]->islocal)
	player[local_player[1]]->keys->fire=true;
};
void pl1_jump()
{
  if(player[local_player[1]] && player[local_player[1]]->islocal)
	player[local_player[1]]->keys->jump=true;
};
void pl1_change()
{
  if(player[local_player[1]] && player[local_player[1]]->islocal)
	player[local_player[1]]->keys->change=true;
};
void pl1_reload()
{
  if(player[local_player[1]] && player[local_player[1]]->islocal)
  if (player[local_player[1]]->weap[player[local_player[1]]->curr_weap].ammo!=weaps->num[player[local_player[1]]->weap[player[local_player[1]]->curr_weap].weap]->ammo && player[local_player[1]]->active)
	player[local_player[1]]->weap[player[local_player[1]]->curr_weap].ammo=0;
};

void pl0_moveright()
{
  if(player[local_player[0]] && player[local_player[0]]->islocal)
	player[local_player[0]]->keys->right=true;
};

void pl0_moveleft()
{
  if(player[local_player[0]] && player[local_player[0]]->islocal)
	player[local_player[0]]->keys->left=true;
};
void pl0_aimup()
{
  if(player[local_player[0]] && player[local_player[0]]->islocal)
	player[local_player[0]]->keys->up=true;
};
void pl0_aimdown()
{
  if(player[local_player[0]] && player[local_player[0]]->islocal)
	player[local_player[0]]->keys->down=true;
};
void pl0_fire()
{
  if(player[local_player[0]] && player[local_player[0]]->islocal)
	player[local_player[0]]->keys->fire=true;
};
void pl0_jump()
{
  if(player[local_player[0]] && player[local_player[0]]->islocal)
	player[local_player[0]]->keys->jump=true;
};
void pl0_change()
{
  if(player[local_player[0]] && player[local_player[0]]->islocal)
	player[local_player[0]]->keys->change=true;
};
void pl0_reload()
{
  if(player[local_player[0]] && player[local_player[0]]->islocal)
  if (player[local_player[0]]->weap[player[local_player[0]]->curr_weap].ammo!=weaps->num[player[local_player[0]]->weap[player[local_player[0]]->curr_weap].weap]->ammo && player[local_player[0]]->active)
	player[local_player[0]]->weap[player[local_player[0]]->curr_weap].ammo=0;
};

void pl0_zoom()
{
  if(player[local_player[0]] && player[local_player[0]]->islocal)
	player[local_player[0]]->keys->zoom=true;
};

void pl0_color()
{
  int j,x,R,G,B;
  if(player[local_player[0]] && player[local_player[0]]->islocal)
  {
    if (strlen(con->arg)>0)
    {
      x=0;
      for(j=x;(con->arg[j]!=',') && (j < strlen(con->arg));j++);
      R=atoi(strmid(con->arg,x,j-x));
      x=j+1;
      for(j=x;(con->arg[j]!=',') && (j < strlen(con->arg));j++);
      G=atoi(strmid(con->arg,x,j-x));
      x=j+1;
      for(j=x;(con->arg[j]!=',') && (j < strlen(con->arg));j++);
      B=atoi(strmid(con->arg,x,j-x));
      x=j+1;
      player[local_player[0]]->color=makecol(R,G,B);
    };
  };
};

void pl1_color()
{
  int j,x,R,G,B;
  if(player[local_player[1]] && player[local_player[1]]->islocal)
  {
    if (strlen(con->arg)>0)
    {
      x=0;
      for(j=x;(con->arg[j]!=',') && (j < strlen(con->arg));j++);
      R=atoi(strmid(con->arg,x,j-x));
      x=j+1;
      for(j=x;(con->arg[j]!=',') && (j < strlen(con->arg));j++);
      G=atoi(strmid(con->arg,x,j-x));
      x=j+1;
      for(j=x;(con->arg[j]!=',') && (j < strlen(con->arg));j++);
      B=atoi(strmid(con->arg,x,j-x));
      x=j+1;
      player[local_player[1]]->color=makecol(R,G,B);
    };
  };
};

void pl0_nick()
{
  strcpy(pl_options[0].name,con->arg);
  change_nick(0);
};

void pl1_nick()
{
  strcpy(pl_options[1].name,con->arg);
  change_nick(1);
};

void pl0_team()
{
  player[local_player[0]]->change_team(atoi(con->arg));
};

void pl1_team()
{
  player[local_player[1]]->change_team(atoi(con->arg));
}

//skins
void pl0_skin()
{
	player[local_player[0]]->load_skin(std::string(con->arg));
}

void pl1_skin()
{
	player[local_player[1]]->load_skin(std::string(con->arg));
}
