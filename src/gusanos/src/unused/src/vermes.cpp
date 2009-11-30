//#define AA2XSAI
//#define WINDOWS

#include <ctype.h>
#include <allegro.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>
#include "engine.h"
#include "netstream.h"

#define major 0
#define minor 8



int main(int argc, char **argv)
{
 int i,r=0;
 char tmp[20];
 //gentlemen, start your engines :-)
 game=(struct engine*) malloc(sizeof(struct engine));
 game->w=0;
 game->sync_mode=0;
 strcpy(game->mod,"default");
 strcpy(game->level,"dm1");
 game->v_width=320;
 game->v_height=240;
 game->v_depth=16;
 
 
 
 //parse options
 for(i=1;i<argc;i++)
 {
 	if(argv[i][0]=='-')
 	{
   	switch(argv[i][1])
   	{
   	 case 'w': game->w=1; break;
   	 case 'g': strcpy(game->mod,argv[i+1]); i++; break;
   	 case 'm': strcpy(game->level,argv[i+1]); i++; break;
   	 case 'v': game->sync_mode=atoi(argv[i+1]); i++; break;
     case 'd': game->v_depth=atoi(argv[i+1]); i++; break;
     case 'r': 
     {
      game->v_width=640; game->v_height=480; break;
     };
   	}
  };
 };
 

 game->init_game();
 con->log.create_msg("");
 con->log.create_msg("");
 sprintf(tmp,"VERMES v%d.%d",major,minor);
 con->log.create_msg(tmp);
 //TODO: get a website!
 con->log.create_msg("no website yet!");
 con->log.create_msg("");
 


 if (game->sync_mode==0)
 {
    while (true){
      while (speed_counter > 0)
      {
        game->input();
        game->calcphysics();
        speed_counter--;
      };
      game->render();
      //Sleep(0);
      game->frame_count++;
      if (t > 100)
      {
        game->fps = (100 * game->frame_count) / t;
        t = 0;
        game->frame_count = 0;
      };
      
    };
 }
 else if(game->sync_mode==1)
 {
  while (true)
  {
   game->input();
   game->calcphysics();
   vsync();
   game->render();
   game->frame_count++;
   if (t > 100)
   {
	  game->fps = (100 * game->frame_count) / t;
	  t = 0;
	  game->frame_count = 0;
   };
  };
 }
 else if(game->sync_mode==2)
  while (true)
  {
   game->input();
   game->calcphysics();
   game->render();
   game->frame_count++;
   if (t > 100)
   {
	  game->fps = (100 * game->frame_count) / t;
	  t = 0;
	  game->frame_count = 0;
   };
  };
  
  delete sprites;
  if(srv) delete srv;
  if(cli) delete cli;

// free(game);
 return(0);
}
END_OF_MAIN();

