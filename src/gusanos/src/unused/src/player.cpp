#include "player.h"

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
      con->log.create_msg(msg2);
      con->echolist.add_echo(msg2);
      play_sample(game->menu_select->snd, *game->VOLUME, 127, 1300, 0);
    };
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
  /*if(player[local_player[pl]] && player[local_player[pl]]->node)
  {
    ZCom_BitStream *data=ZCom_Control::ZCom_createBitStream();;
    data->addInt(4,8);
    data->addString( pl_options[pl].name);
    player[local_player[pl]]->node->sendEvent(ZCom_Node::eEventMode_ReliableOrdered, ZCOM_REPRULE_OWNER_2_AUTH, data);
  };*/
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
  data->addSignedInt((int)dir,8);
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
			con->log.create_msg("PLAYER REMOVED");
			deleteme = true;
		} else
    if (type == ZCom_Node::eEvent_User)
    {
      event=data->getInt(8);
      if(event==4)
      {
        strcpy(name,data->getStringStatic());
      }else if(event==3)
      {
        char msg[1024],msg2[1024];
        strcpy(msg,data->getStringStatic());
        sprintf(msg2,"[%s] %s",name,msg);
        //strcat(msg2,msg);
        con->log.create_msg(msg2);
        con->echolist.add_echo(msg2);
        if (node->getRole()==eZCom_RoleAuthority)
        {
          sendmsg(msg);
        };
        play_sample(game->menu_select->snd, *game->VOLUME, 127, 1300, 0);
      }else if(event==2)
      {
        int o;
        int _x=data->getInt(32);
        int _y=data->getInt(32);
        int _xspd=data->getInt(32);
        int _yspd=data->getInt(32);
        int _t=data->getInt(32);
        srand(_t);
        for (o=0;o<14;o++)
          partlist.shoot_part(rand()%1000*255,(rand()%200)+600,1,_x,_y-4000,_xspd/2,_yspd/2,this,game->gore);
        play_sample(game->death->snd, *game->VOLUME, 127, 1000, 0);
        deaths++;
        health=0;
        if (*game->RESPAWN_RELOAD==1)
        recharge_weapons(this);
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
                  xof=fixtof(fixsin(ftofix((_ang-dist)/1000.)))*(int)(weaps->num[_weap]->shoot_obj->detect_range+1000)*_dir;
                  yof=fixtof(fixcos(ftofix((_ang-dist)/1000.)))*(int)(weaps->num[_weap]->shoot_obj->detect_range+1000);
                  partlist.shoot_part(_ang-dist,weaps->num[_weap]->shoot_spd-spd_rnd,_dir,_x+xof,_y-4000+yof,_xspd*(weaps->num[_weap]->affected_motion/1000.),_yspd*(weaps->num[_weap]->affected_motion/1000.),this,weaps->num[_weap]->shoot_obj);
                };
                if (weaps->num[_weap]->aim_recoil!=0)
                  aim_recoil_speed+=(100*weaps->num[_weap]->aim_recoil);/*-weap[curr_weap].weap->aim_recoil/2*1000;*/
                if (weaps->num[_weap]->recoil!=0)
                {
                  xspd = xspd + -fixtof(fixsin(ftofix(_ang/1000.)))*weaps->num[_weap]->recoil*_dir;
                  yspd = yspd + -fixtof(fixcos(ftofix(_ang/1000.)))*weaps->num[_weap]->recoil;
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
		curr_weap=0;
    aim_speed=0;
    aim_recoil_speed=0;
    color=makecol(100,100,220);
    for(o=0;o<5;o++)
    {
      weap[o].weap=0;
      weap[o].shoot_time=0;
      weap[o].ammo=weaps->num[weap[o].weap]->ammo;
      weap[o].reloading=false;
    };
		curr_frame=2700;
		crosshair=sprites->load_sprite("crosshair.bmp",1,game->mod,game->v_depth);
		active=false;
		flag=false;
    islocal=false;
    deleteme=false;
    selecting_weaps=true;
		ropestate=0;
		ropex=1;
    ropexspd=1;
		ropey=1;
    ropeyspd=1;
    curr_firecone=NULL;
    firecone_time=0;
    //aim_acceleration=new int(100);
    //aim_friction=new int(50);
    //aim_maxspeed=new int(1200);
    skin=sprites->load_sprite("lskinb.bmp",21,game->mod,game->v_depth);
    mask=sprites->load_sprite("lskinmask.bmp",21,game->mod,game->v_depth);
    keys=new struct KEYS;
    keys->up=false;
    keys->down=false;
    keys->right=false;
    keys->left=false;
    keys->jump=false;
    keys->fire=false;
    keys->change=false;
    
    id = ZCom_Invalid_ID;
    
    int i=1;
    
    node = new ZCom_Node();
    if (!node)
    {
      con->log.create_msg("unable to create node");
    }
    
    node->beginReplicationSetup();
    node->addInterpolationInt((zS32*)&x,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,20000,NULL,-1,-1,0.2f);
    node->addInterpolationInt((zS32*)&y,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,20000,NULL,-1,-1,0.2f);
    //node->addInterpolationInt(&yspd,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,1000,NULL,-1,-1,0);
    //node->addInterpolationInt(&xspd,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,1000,NULL,-1,-1,0);
    node->addReplicationInt((zS32*)&ropestate,8,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,-1,-1);
    node->addReplicationInt((zS32*)&ropex,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,-1,-1);
    node->addReplicationInt((zS32*)&ropey,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,-1,-1);
    node->addReplicationInt((zS32*)&ropexspd,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,-1,-1);
    node->addReplicationInt((zS32*)&ropeyspd,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,-1,-1);
    node->addReplicationInt((zS32*)&aim,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY,99,-1,-1);
    node->addReplicationInt((zS32*)&dir,8,true,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,99,-1,-1);
    node->addReplicationBool(&keys->fire,ZCOM_REPFLAG_RARELYCHANGED,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->left,ZCOM_REPFLAG_RARELYCHANGED,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->right,ZCOM_REPFLAG_RARELYCHANGED,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->up,ZCOM_REPFLAG_RARELYCHANGED,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->down,ZCOM_REPFLAG_RARELYCHANGED,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->jump,ZCOM_REPFLAG_RARELYCHANGED,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&keys->change,ZCOM_REPFLAG_RARELYCHANGED,ZCOM_REPRULE_OWNER_2_AUTH,false,-1,-1 );
    node->addReplicationBool(&active,ZCOM_REPFLAG_RARELYCHANGED,ZCOM_REPRULE_AUTH_2_ALL,false,-1,-1 );
    node->addReplicationInt((zS32*)&curr_weap,8,true,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY,0,-1,-1);
    //node->addReplicationInt((zS32*)&weap[0].ammo,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,0,-1,-1 );
    node->addReplicationInt((zS32*)&health,32,false,ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_AUTH_2_ALL,0,-1,-1 );
    node->addReplicationInt((zS32*)&color,32,false,ZCOM_REPFLAG_RARELYCHANGED|ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY,0,-1,-1 );
    for(o=0;o<5;o++)
      node->addReplicationInt((zS32*)&weap[o].weap,32,false,ZCOM_REPFLAG_RARELYCHANGED|ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY,0,-1,-1 );
    node->addReplicationString(name,32,ZCOM_REPFLAG_RARELYCHANGED|ZCOM_REPFLAG_MOSTRECENT,ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY," ",-1,-1 );
    
    node->endReplicationSetup();

    if(srv)
    {
      if(!node->registerNodeDynamic(player_classid, srv))
      allegro_message("unable to register player node");
    };
    if(cli)
    {
      if(!node->registerRequestedNode(player_classid, cli))
      allegro_message("was unable to create the player node");
    };
      
      node->applyForZoidLevel(2);
};

worm::~worm()
{
  delete node;
  free (keys);
  free (weap);
};

void worm::render(BITMAP* where, int frame, int _x, int _y)
{
  int r,g,c,R,G,B,i;
  float h1,s1,v1,h,s,v;
  for (i=0;i<skin->img[frame]->w;i++)
  {
    for (r=0;r<skin->img[frame]->h;r++)
    {
      g=getpixel(skin->img[frame],i,r);
      c=getpixel(mask->img[frame],i,r);
      if(g!=makecol(255,0,255))
      {
        if(c!=makecol(255,0,255))
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
  for (i=0;i<skin->img[frame]->w;i++)
  {
    for (r=0;r<skin->img[frame]->h;r++)
    {
      g=getpixel(skin->img[frame],i,r);
      c=getpixel(mask->img[frame],i,r);
      if(g!=makecol(255,0,255))
      {
        if(c!=makecol(255,0,255))
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
	ropexspd=fixtof(fixsin(ftofix(aim/1000.)))*3500*dir;
	ropeyspd=fixtof(fixcos(ftofix(aim/1000.)))*3500;
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
	double px,py;
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

void calcrope(struct worm *player)
{
	int g,dx,dy,m;
	
	if (player->ropestate!=0)
	{
		g=getpixel(map->material,player->ropex/1000,player->ropey/1000);
		if (!map->mat[g+1].particle_pass)
		{
			if(player->ropestate!=2)
			{
				player->ropestate=2;
				//dx=player->x-player->ropex;
				//dy=player->y-4000-player->ropey;
				//m=fixhypot(dx,dy);
				//player->rope_length=m;
			};
		} else player->ropestate=1;
		
		if (player->ropestate==1)
		{
			player->ropex=player->ropex+player->ropexspd;
			player->ropey=player->ropey+player->ropeyspd;
      player->ropeyspd+=*game->ROPE_GRAVITY;
		};
		
    if(!cli)
		if (player->ropestate==2)
    {
      player->applyropeforce();
      player->ropexspd=player->ropeyspd=0;
    };
	};
};

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
