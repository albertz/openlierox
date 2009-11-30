#include "console.h"
#include "sprites.h"
#include "network.h"
#include "player.h"
#include <allegro.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
#include "engine.h"
#include "netstream.h"
#include <stdio.h>
#include <string.h>


#define vmajor 0
#define vminor 8



int main(int argc, char **argv)
{
	int i;
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
	sprintf(tmp,"VERMES v%d.%d",vmajor,vminor);
	con->log.create_msg(tmp);
	con->log.create_msg("HTTP://VERMES.VZE.COM");
	con->log.create_msg("");


	game->quitgame = false; //quitgame
	if (game->sync_mode==0)
	{
		while (!game->quitgame){ //quitgame
			while (speed_counter > 0)
			{
				game->input();
				game->calcphysics();
				speed_counter--;
			};
			game->render();
			#ifdef WINDOWS
			Sleep(0);
			#endif
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
		while (!game->quitgame) //quitgame
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
		while (!game->quitgame) //quitgame
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

	delete_players();
	delete sprites;
	delete srv;
	delete cli;
	delete zcom;
	free(game);

	return(0);
}
END_OF_MAIN();