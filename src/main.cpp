// OpenLieroX

// Main entry point
// Created 28/6/02
// Jason Boettcher

// code under LGPL

#include <iostream>
#include <assert.h>

#include "defs.h"
#include "LieroX.h"
#include "CClient.h"
#include "CServer.h"
#include "Graphics.h"
#include "Menu.h"
#include "console.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "InputEvents.h"
#include "StringUtils.h"

#ifndef WIN32
#include <sys/dir.h>
#include <sys/stat.h>
#endif

// TODO: i hate globals ...
// we have to create a basic class CGame or something

lierox_t	*tLX = NULL;
game_t		tGameInfo;
CInput		cTakeScreenshot;
CInput		cSwitchMode;
CInput		cToggleMediaPlayer;

CMediaPlayer cMediaPlayer;

int         nDisableSound = false;

keyboard_t	*kb;
SDL_Surface	*Screen;

CVec		vGravity = CVec(0,4);

UCString	binary_dir; // given by argv[0]


void print_binary_string(const UCString& txt) {
	for(UCString::const_iterator it = txt.begin(); it != txt.end(); it++) {
		std::cout << std::hex << (ushort)(uchar)(*it) << " ";
	}
}

void test_Unicode_UTF8_Conversion() {
	UCString tmp;
	UCString::const_iterator tmpbegin;
	for(UnicodeChar c = 1; c != 0; c++) {
		//if(c % 0x100000 == 0) std::cout << std::hex << c << " ..." << std::endl;
//		std::cout << std::hex << c << " -> ";
		tmp = GetUtf8FromUnicode(c);
//		print_binary_string(tmp);
		tmpbegin = tmp.begin();
//		std::cout << " -> " << std::hex << GetNextUnicodeFromUtf8(tmpbegin, tmp) << std::endl;
		if(GetNextUnicodeFromUtf8(tmpbegin, tmp.end()) != c) {
			//std::cout << std::hex << c << " -> ";
			print_binary_string(tmp);
			//std::cout << " -> " << std::hex << GetNextUnicodeFromUtf8(tmpbegin, tmp.end()) << std::endl;
		}
	}
}


///////////////////
// Main entry point
int main(int argc, char *argv[])
{
#ifdef _MSC_VER
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif // _DEBUG
	
	InstallExceptionFilter();
	nameThread(-1,"Main game thread");
#else
	// TODO: same/similar for other systems
#endif // _MSC_VER

	printf("OpenLieroX " LX_VERSION " is starting ...\n");

	// sadly, these sizeof are directly used in CGameScript.cpp/CMap.cpp
	// TODO: fix this issue
	assert(sizeof(char) == 1);
	assert(sizeof(bool) == 1); // CBytestream
	assert(sizeof(short) == 2);
	assert(sizeof(int) == 4);
	assert(sizeof(float) == 4);
	
    bool startgame = false;

	binary_dir = argv[0];
	size_t slashpos = findLastPathSep(binary_dir);
	if(slashpos != UCString::npos)
		binary_dir.erase(slashpos);
	else
		binary_dir = "."; // TODO get exact path of binary

	// Load options and other settings
	if(!GameOptions::Init())  {
		SystemError("Could not load options");
		return -1;
	}

	if(!NetworkTexts::Init())  {
		SystemError("Could not load network strings.");
		return -1;
	}

    // Parse the arguments
    ParseArguments(argc, argv);

	// Initialize LX
	if(!InitializeLieroX())
		return -1;
	kb = GetKeyboard();
	Screen = SDL_GetVideoSurface();

	// Initialize menu
	if(!Menu_Initialize(&startgame)) {
        SystemError("Error: Could not initialize the menu system.\nError when loading graphics files");
		ShutdownLieroX();
		return -1;
	}

	// Initialize the music system
	InitializeMusic();

	// Initialize the media player
	if (!cMediaPlayer.Initialize())  {
		printf("Warning: Media Player could not be initialized");
		// Don't quit, we can quite well play without music
	}

	// Load the playlist
	cMediaPlayer.LoadPlaylistFromFile("cfg/playlist.dat");

	// TODO: abstract the logging, make an uniform message system
	// Log the game start
	if (tLXOptions->iLogConvos)  {
		FILE *f;

		f = OpenGameFile("Conversations.log","a");
		if (f)  {	
			UCString cTime = GetTime();
			fputs("<game starttime=\"",f);
			fputs(cTime.c_str(),f);
			fputs("\">\r\n",f);
			fclose(f);
		}
	}

	// Setup the global keys
	cTakeScreenshot.Setup(tLXOptions->sGeneralControls[SIN_SCREENSHOTS]);
	cSwitchMode.Setup(tLXOptions->sGeneralControls[SIN_SWITCHMODE]);
	cToggleMediaPlayer.Setup(tLXOptions->sGeneralControls[SIN_MEDIAPLAYER]);

	while(!tLX->iQuitGame) {

		startgame = false;

		// Start the menu
		Menu_Start();
		
		if(startgame) {
			// Start the game
			// this means, start the local server and connect to it
			StartGame();

		} else {
			// Quit
			ShutdownLieroX();
			return 0;
		}

		// Pre-game initialization
		Screen = SDL_GetVideoSurface();
		float oldtime = GetMilliSeconds();
		
		ClearEntities();

		ProcessEvents();
		tLX->iQuitEngine = false;
		printf("MaxFPS is %i\n", tLXOptions->nMaxFPS);
		float fMaxFrameTime = 1.0f / (float)tLXOptions->nMaxFPS;

        //
        // Main game loop
        //
		while(!tLX->iQuitEngine) {

            tLX->fCurTime = GetMilliSeconds();

            // Cap the FPS
            if(tLX->fCurTime - oldtime < fMaxFrameTime) {
				SDL_Delay((int)((fMaxFrameTime - tLX->fCurTime + oldtime)*1000));
                continue;
			}

			// Timing
			tLX->fDeltaTime = tLX->fCurTime - oldtime;
			oldtime = tLX->fCurTime;
						
			// cap the delta
			tLX->fDeltaTime = MIN(tLX->fDeltaTime, 0.5f);

		            ProcessEvents();

			// Main frame
			GameLoop();

			FlipScreen(Screen);
		}
	}


	ShutdownLieroX();

	printf("Good Bye and enjoy your day...\n");
	return 0;
}


///////////////////
// Parse the arguments
void ParseArguments(int argc, char *argv[])
{
    // Parameters passed to OpenLieroX overwrite the loaded options
    char *a;
    for(int i=1; i<argc; i++) {
        a = argv[i];

        // -opengl
        // Turns OpenGL on
        if( stricmp(a, "-opengl") == 0 ) {
            tLXOptions->iOpenGL = true;
        } else

        // -noopengl
        // Turns OpenGL off
        if( stricmp(a, "-noopengl") == 0 ) {
            tLXOptions->iOpenGL = false;
        } else

        // -nosound
        // Turns off the sound
        if( stricmp(a, "-nosound") == 0 ) {
            nDisableSound = true;
            tLXOptions->iSoundOn = false;
        } else

        // -window
        // Turns fullscreen off
        if( stricmp(a, "-window") == 0 ) {
            tLXOptions->iFullscreen = false;
        } else

        // -fullscreen
        // Turns fullscreen on
        if( stricmp(a, "-fullscreen") == 0 ) {
            tLXOptions->iFullscreen = true;
        } else
        
        if( !stricmp(a, "-h") || !stricmp(a, "-help") || !stricmp(a, "--help") ) {
        	printf("available parameters:\n");
     		printf("   -opengl\n");   	
     		printf("   -noopengl\n");   	
     		printf("   -nosound\n");   	
     		printf("   -window\n");   	
     		printf("   -fullscreen\n");
     		exit(0); // uuuh!
        }
    }
}


///////////////////
// Initialize the game
int InitializeLieroX(void)
{
	printf("Hello there, I am initializing me now...\n");
	
	// Initialize the aux library
	if(!InitializeAuxLib("OpenLieroX","config.cfg",16,0)) {
        SystemError("strange problems with the aux library");	
		return false;
	}

	// Initialize the network
    if(!InitNetworkSystem()) {
        SystemError("Error: Failed to initialize the network library");
		return false;
    }

	// Allocate the client & server
	cClient = new CClient;
    if(cClient == NULL) {
        SystemError("Error: InitializeLieroX() Out of memory");
		return false;
    }
	
	cClient->Clear();

	cServer = new GameServer;
    if(cServer == NULL) {
        SystemError("Error: InitializeLieroX() Out of memory on creating GameServer");
		return false;
    }

	// Initialize the entities
    if(!InitializeEntities()) {
        SystemError("Error: InitializeEntities() Out of memory on initializing the entities");
		return false;
    }


	// Initialize the LieroX structure
	tLX = new lierox_t;
    if(tLX == NULL) {
        SystemError("Error: InitializeLieroX() Out of memory on creating lierox_t");
		return false;
    }
	tLX->iQuitGame = false;
	tLX->debug_string = "";
	// TODO: more initiation of the values
	tLX->fCurTime = 0;
	tLX->fDeltaTime = 0;
	// Init these special colours
	tLX->clPink = MakeColour(255,0,255);
	tLX->clWhite = MakeColour(255,255,255);

	// Load the graphics
    if(!LoadGraphics()) {
        SystemError("Error: Error loading graphics");
		return false;
    }

    // Initialize the console
    if(!Con_Initialize()) {
        SystemError("Error: Could not initialize the console");
        return false;
    }

    // Add some console commands
    Cmd_AddCommand("kick", Cmd_Kick);
	Cmd_AddCommand("ban", Cmd_Ban);
	Cmd_AddCommand("mute", Cmd_Mute);
	Cmd_AddCommand("unmute", Cmd_Unmute);
    Cmd_AddCommand("kickid", Cmd_KickId);
	Cmd_AddCommand("banid", Cmd_BanId);
	Cmd_AddCommand("muteid", Cmd_MuteId);
	Cmd_AddCommand("unmuteid", Cmd_UnmuteId);
	Cmd_AddCommand("crash", Cmd_Crash);
	Cmd_AddCommand("suicide", Cmd_Suicide);
	Cmd_AddCommand("unstuck", Cmd_Unstuck);
	Cmd_AddCommand("wantsjoin", Cmd_WantsJoin);
	Cmd_AddCommand("servername", Cmd_RenameServer);
	Cmd_AddCommand("help", Cmd_Help);
	Cmd_AddCommand("version", Cmd_About);
	Cmd_AddCommand("about", Cmd_About);
	Cmd_AddCommand("fuck", Cmd_BadWord);
	Cmd_AddCommand("ass", Cmd_BadWord);
	Cmd_AddCommand("bitch", Cmd_BadWord);
	Cmd_AddCommand("sex", Cmd_BadWord);
	Cmd_AddCommand("quit", Cmd_Quit);
	Cmd_AddCommand("exit", Cmd_Quit);
	Cmd_AddCommand("volume", Cmd_Volume);
	Cmd_AddCommand("sound", Cmd_Sound);
	
	// Load the sounds
	LoadSounds();


	// Load the profiles
	LoadProfiles();

    // Initialize the game info structure
	tGameInfo.iNumPlayers = 0;
    tGameInfo.sMapRandom.psObjects = NULL;

	printf("Initializing ready\n");

	return true;
}


///////////////////
// Start the game
void StartGame(void)
{
    // Clear the screen
    DrawRectFill(SDL_GetVideoSurface(), 0,0, 640,480, 0);

	// Local game
	if(tGameInfo.iGameType == GME_LOCAL) {

		// TODO: uniform message system

		// Start the server
		if(!cServer->StartServer( "local", tLXOptions->iNetworkPort, 8, false )) {
			// ERROR
			//MessageBox(NULL, "Error: Could not start server", "OpenLieroX Error", MB_OK);
			printf("Error: Could not start server\n");
			return;
		}

		// Setup the client
		if(!cClient->Initialize()) {
			// ERROR
			//MessageBox(NULL, "Error: Could not initialize client", "OpenLieroX Error", MB_OK);
			printf("Error: Could not initialize client\n");
			return;
		}

		// Tell the client to connect to the server
		cClient->Connect("127.0.0.1");
	}
}


///////////////////
// Game loop
void GameLoop(void)
{
    if(tLX->iQuitEngine)
        return;

	// Media player
	cMediaPlayer.Frame();

	// Local
	switch (tGameInfo.iGameType)  {
	case GME_LOCAL:
		cClient->Frame();
		cServer->Frame();

		// If we are connected, just start the game straight away (bypass lobby in local)
		if(cClient->getStatus() == NET_CONNECTED) {
			if(cServer->getState() == SVS_LOBBY)
				cServer->StartGame();
		}

		cClient->Draw(Screen);
		break;


	// Hosting
	case GME_HOST:
		cClient->Frame();
		cServer->Frame();

		cClient->Draw(Screen);
		break;

	// Joined
	case GME_JOIN:
		cClient->Frame();
		cClient->Draw(Screen);
		break;


	} // SWITCH

	// Switch between window and fullscreen mode
	if( cSwitchMode.isDown() )  {
		// Set to fullscreen
		tLXOptions->iFullscreen = !tLXOptions->iFullscreen;

		// Set the new video mode
		SetVideoMode();

		// Update both menu and game screens
		Screen = SDL_GetVideoSurface();
		tMenu->bmpScreen = SDL_GetVideoSurface();
	}

	// We put it here, so the mouse never displays
    SDL_ShowCursor(SDL_DISABLE);

}


///////////////////
// Quit back to the menu
void QuittoMenu(void)
{
	tLX->iQuitEngine = true;
    Menu_SetSkipStart(false);
	cClient->Disconnect();
}


///////////////////
// Shutdown the game
void ShutdownLieroX(void)
{
	printf("Shutting me down...\n");	
	
	if (tLXOptions->iLogConvos)  {
		FILE *f;

		f = OpenGameFile("Conversations.log","a");
		if (f)  { 
			fputs("</game>\r\n",f);
			fclose(f);
		}
	}

	ShutdownMusic();
	cMediaPlayer.SavePlaylistToFile("cfg/playlist.dat");
	cMediaPlayer.Shutdown();

    Con_Shutdown();

	ShutdownGraphics();

	Menu_Shutdown();
	ShutdownProfiles();

    // Free the game info structure
    if(tGameInfo.sMapRandom.psObjects)
        delete[] tGameInfo.sMapRandom.psObjects;

    tGameInfo.sMapRandom.psObjects = NULL;

	// Free the client & server
	if(cClient) {
		cClient->Shutdown();
		delete cClient;
		cClient = NULL;
	}

	if(cServer) {
		cServer->Shutdown();
		delete cServer;
		cServer = NULL;
	}

	// Free the bots
	/*if (cBots)  {
		for (int i=0;i<tGameInfo.iNumBots;i++)
			cBots[i].Shutdown();
		delete[] cBots;
		cBots = NULL;
	}*/

	ShutdownEntities();
	
	if(tLX) {
		delete tLX;
		tLX = NULL;
	}

	Cmd_Free();

	QuitNetworkSystem();
	
	ShutdownAuxLib();

	ShutdownOptions();

	printf("Everything was shut down\n");
	
}
