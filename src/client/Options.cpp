/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Options
// Created 21/7/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


options_t	     *tLXOptions = NULL;
networktexts_t   *NetworkTexts = NULL;


///////////////////
// Load the options
int LoadOptions(void)
{
	printf("Loading options... \n");

	tLXOptions = new options_t;
	if(tLXOptions == NULL) {
		printf("ERROR: could not init options\n");
		return false;
	}	
	
    static const std::string    ply_keys[] = {"Up", "Down", "Left", "Right", "Shoot", "Jump", "SelectWeapon", "Rope"};
    static const std::string    ply_def1[] = {"up", "down", "left", "right", "lctrl", "lalt", "lshift", "z"};
    static const std::string    ply_def2[] = {"r",  "f",    "d",    "g",     "rctrl", "ralt", "rshift", "/"};
    static const std::string    gen_keys[] = {"Chat", "ShowScore", "ShowHealth", "ShowSettings",  "TakeScreenshot",  "ViewportManager", "SwitchMode", "MediaPlayer"};
    static const std::string    gen_def[]  = {"i",    "tab",	"h",	"space",   "F12",    "F2",  "F5"};
	static const int	 def_widths[] = {32,180,70,80,60,150};

    unsigned int     i;

	static const std::string f = "cfg/options.cfg";

	AddKeyword("true",true);
	AddKeyword("false",false);

	// File handling
	// read this first, because perhaps we will have new searchpaths
	InitBaseSearchPaths();
	std::string value;
	std::string item;
	i = 1;
	while(true) {
		item = "SearchPath"; item += itoa(i,10);
		if(!ReadString(f, "FileHandling", item, value, ""))
			break;
		
		AddToFileList(&tLXOptions->tSearchPaths, value);
		i++;
	}

	for(searchpathlist::const_iterator p1 = basesearchpaths.begin(); p1 != basesearchpaths.end(); i++,p1++)  {
		AddToFileList(&tLXOptions->tSearchPaths, *p1);
	}

	printf("I have now the following searchpaths (in this direction):\n");
	for(searchpathlist::const_iterator p2 = tLXOptions->tSearchPaths.begin(); p2 != tLXOptions->tSearchPaths.end(); p2++) {
		printf("  %s\n", p2->c_str());
	}
	printf(" And that's all.\n");

	for (i=0;i<sizeof(tLXOptions->iInternetList)/sizeof(int);i++)  {
		tLXOptions->iInternetList[i] = def_widths[i];
		tLXOptions->iLANList[i] = def_widths[i];
		tLXOptions->iFavouritesList[i] = def_widths[i];
	}


    // Video
    ReadKeyword(f, "Video", "Fullscreen",   &tLXOptions->iFullscreen, true);
    ReadKeyword(f, "Video", "ShowFPS",      &tLXOptions->iShowFPS, false);
    ReadKeyword(f, "Video", "OpenGL",       &tLXOptions->iOpenGL, false);

    // Network
    ReadInteger(f, "Network", "Port",       &tLXOptions->iNetworkPort, LX_PORT);
    ReadInteger(f, "Network", "Speed",      &tLXOptions->iNetworkSpeed, NST_MODEM);

    // Audio
    ReadKeyword(f, "Audio", "Enabled",      &tLXOptions->iSoundOn, true);
    ReadInteger(f, "Audio", "Volume",       &tLXOptions->iSoundVolume, 70);

	// Misc.
	ReadKeyword(f, "Misc", "LogConversations", &tLXOptions->iLogConvos, true);
	ReadKeyword(f, "Misc", "ShowPing",		   &tLXOptions->iShowPing, true);
	ReadInteger(f, "Misc", "ScreenshotFormat", &tLXOptions->iScreenshotFormat, FMT_PNG);

    // Player controls
    int j;
    std::string def;
    i = 0; j = 1;
    while(true) {
        item = "Ply"; item += itoa(j); item += "Controls";
        if(j == 1) def = ply_def1[i];
        else if(j == 2) def = ply_def2[i];
        else def = "";
        if(!ReadString(f, item, ply_keys[i], value, def) && i == 0 && j > 2)
        	break;
        if(i == 0)
        	tLXOptions->sPlayerControls.push_back(controls_t());
        tLXOptions->sPlayerControls[j-1][i] = value;
        
        i++;
        if(i >= 8) {
        	i = 0; j++;
        }
    }

    // General controls
    for(i=0; i<8; i++)
        ReadString(f, "GeneralControls", gen_keys[i], tLXOptions->sGeneralControls[i], gen_def[i]);

    // Game
    ReadInteger(f, "Game", "Blood",         &tLXOptions->iBloodAmount, 100);
    ReadKeyword(f, "Game", "Shadows",       &tLXOptions->iShadows, true);
    ReadKeyword(f, "Game", "Particles",     &tLXOptions->iParticles, true);
    ReadKeyword(f, "Game", "OldSkoolRope",  &tLXOptions->iOldSkoolRope, false);
	ReadKeyword(f, "Game", "ShowWormHealth",&tLXOptions->iShowHealth, false);
	ReadKeyword(f, "Game", "ColorizeNicks", &tLXOptions->iColorizeNicks, false);
	ReadKeyword(f, "Game", "AutoTyping",	&tLXOptions->iAutoTyping, true);

	// Widget states
	ReadIntArray(f, "Widgets","InternetListCols",	&tLXOptions->iInternetList[0],6);
	ReadIntArray(f, "Widgets","LANListCols",		&tLXOptions->iLANList[0],6);
	ReadIntArray(f, "Widgets","FavouritesListCols",	&tLXOptions->iFavouritesList[0],6);

	// Media player
	ReadKeyword(f, "MediaPlayer", "Repeat",		&tLXOptions->bRepeatPlaylist, true);
	ReadKeyword(f, "MediaPlayer", "Shuffle",	&tLXOptions->bShufflePlaylist, false);
	ReadInteger(f, "MediaPlayer", "Left",		&tLXOptions->iMPlayerLeft, 350);
	ReadInteger(f, "MediaPlayer", "Top",		&tLXOptions->iMPlayerTop, 240);
	ReadInteger(f, "MediaPlayer", "MusicVolume",&tLXOptions->iMusicVolume, 50);


    // Last Game
    ReadInteger(f, "LastGame", "Lives",     &tLXOptions->tGameinfo.iLives, 10);
    ReadInteger(f, "LastGame", "KillLimit", &tLXOptions->tGameinfo.iKillLimit, -1);
    ReadInteger(f, "LastGame", "TimeLimit", &tLXOptions->tGameinfo.iTimeLimit, -1);
    ReadInteger(f, "LastGame", "TagLimit",  &tLXOptions->tGameinfo.iTagLimit, 5);
    ReadInteger(f, "LastGame", "LoadingTime",   &tLXOptions->tGameinfo.iLoadingTime, 100);
    ReadKeyword(f, "LastGame", "Bonuses",   &tLXOptions->tGameinfo.iBonusesOn, true);
    ReadKeyword(f, "LastGame", "BonusNames",&tLXOptions->tGameinfo.iShowBonusName, true);
    ReadInteger(f, "LastGame", "MaxPlayers",&tLXOptions->tGameinfo.iMaxPlayers, 8);
	ReadKeyword(f, "LastGame", "Tournament", &tLXOptions->tGameinfo.bTournament, true);
    ReadString (f, "LastGame", "ServerName",tLXOptions->tGameinfo.sServerName, "LieroX Server");
	ReadString (f, "LastGame", "WelcomeMessage",tLXOptions->tGameinfo.sWelcomeMessage, "Welcome to <server>, <player>");
    ReadString (f, "LastGame", "LevelName", tLXOptions->tGameinfo.sMapName, "");
    ReadInteger(f, "LastGame", "GameType",  &tLXOptions->tGameinfo.nGameType, GMT_DEATHMATCH);
    ReadString (f, "LastGame", "ModName",   tLXOptions->tGameinfo.szModName, "Classic");
    ReadString (f, "LastGame", "Password",  tLXOptions->tGameinfo.szPassword, "");
    ReadKeyword(f, "LastGame", "RegisterServer",&tLXOptions->tGameinfo.bRegServer, true);
	ReadInteger(f, "LastGame", "LastSelectedPlayer",&tLXOptions->tGameinfo.iLastSelectedPlayer, 0);
	ReadKeyword(f, "LastGame", "AllowWantsJoinMsg",&tLXOptions->tGameinfo.bAllowWantsJoinMsg, true);
	ReadKeyword(f, "LastGame", "AllowRemoteBots", &tLXOptions->tGameinfo.bAllowRemoteBots, true);

    // Advanced
    ReadInteger(f, "Advanced", "MaxFPS",    &tLXOptions->nMaxFPS, 95);
	ReadInteger(f, "Advanced", "JpegQuality", &tLXOptions->iJpegQuality, 80);
	ReadFloat  (f, "Advanced", "NetworkUpdatePeriod", &tLXOptions->fUpdatePeriod,0.05f);

	// Clamp the Jpeg quality
	if (tLXOptions->iJpegQuality < 1)
		tLXOptions->iJpegQuality = 1;
	if (tLXOptions->iJpegQuality > 100)
		tLXOptions->iJpegQuality = 100;

	printf("DONE loading options\n");

	return true;
}

///////////////////
// Save & shutdown the options
void ShutdownOptions(void)
{
	if(tLXOptions == NULL)
		return;

    SaveOptions();

	// Free the structure
	assert(tLXOptions);
	delete tLXOptions;

	if (NetworkTexts)
		delete NetworkTexts;
	NetworkTexts = NULL;
}


///////////////////
// Save the options
void SaveOptions(void)
{
    static const std::string    ply_keys[] = {"Up", "Down", "Left", "Right", "Shoot", "Jump", "SelectWeapon", "Rope"};
    static const std::string    gen_keys[] = {"Chat", "ShowScore", "ShowHealth", "ShowSettings", "TakeScreenshot", "ViewportManager", "SwitchMode", "MediaPlayer"};
    int     i;

    if(tLXOptions == NULL)
		return;

    FILE *fp = OpenGameFile("cfg/options.cfg", "wt");
    if(fp == NULL)
        return;


    fprintf(fp, "# OpenLieroX Options File\n");
    fprintf(fp, "# Note: This file is automatically generated\n\n");

    fprintf(fp, "[Video]\n");
    fprintf(fp, "Fullscreen = %s\n",tLXOptions->iFullscreen ? "true" : "false");
    fprintf(fp, "ShowFPS = %s\n",tLXOptions->iShowFPS ? "true" : "false");
    fprintf(fp, "OpenGL = %s\n",tLXOptions->iOpenGL ? "true" : "false");
    fprintf(fp, "\n");

    fprintf(fp, "[Network]\n");
    fprintf(fp, "Port = %d\n",      tLXOptions->iNetworkPort);
    fprintf(fp, "Speed = %d\n",     tLXOptions->iNetworkSpeed);
    fprintf(fp, "\n");

    fprintf(fp, "[Audio]\n");
    fprintf(fp, "Enabled = %s\n",   tLXOptions->iSoundOn ? "true" : "false");
    fprintf(fp, "Volume = %d\n",    tLXOptions->iSoundVolume);
    fprintf(fp, "\n");

	fprintf(fp,"[Misc]\n");
	fprintf(fp,"LogConversations = %s\n",	tLXOptions->iLogConvos ? "true" : "false");
	fprintf(fp,"ShowPing = %s\n",			tLXOptions->iShowPing ? "true" : "false");
	fprintf(fp,"ScreenshotFormat = %d\n",	tLXOptions->iScreenshotFormat);
	fprintf(fp,"\n");

	fprintf(fp, "[FileHandling]\n");
	i=1;
	for(searchpathlist::const_iterator p = tLXOptions->tSearchPaths.begin(); p != tLXOptions->tSearchPaths.end(); p++, i++)
    	fprintf(fp, "SearchPath%i = %s\n", i, p->c_str());
	fprintf(fp,"\n");

	int j = 0;
	for(;j<tLXOptions->sPlayerControls.size();j++) {
		fprintf(fp, "[Ply%iControls]\n", j+1);
		for(i=0; i<8; i++)
        	fprintf(fp, "%s = %s\n", ply_keys[i].c_str(), tLXOptions->sPlayerControls[j][i].c_str());
	    fprintf(fp, "\n");
	}

    fprintf(fp, "[GeneralControls]\n");
    for(i=0; i<8; i++)
        fprintf(fp, "%s = %s\n", gen_keys[i].c_str(), tLXOptions->sGeneralControls[i].c_str());
    fprintf(fp, "\n");

    fprintf(fp, "[Game]\n");
    fprintf(fp, "Blood = %d\n",     tLXOptions->iBloodAmount);
    fprintf(fp, "Shadows = %s\n",   tLXOptions->iShadows ? "true" : "false");
    fprintf(fp, "Particles = %s\n", tLXOptions->iParticles ? "true" : "false");
    fprintf(fp, "OldSkoolRope = %s\n", tLXOptions->iOldSkoolRope ? "true" : "false");
	fprintf(fp, "ShowWormHealth = %s\n", tLXOptions->iShowHealth ? "true" : "false");
	fprintf(fp, "ColorizeNicks = %s\n", tLXOptions->iColorizeNicks ? "true" : "false");
	fprintf(fp, "AutoTyping = %s\n", tLXOptions->iAutoTyping ? "true" : "false");
    fprintf(fp, "\n");

	// TODO: these arrays never got intialized!
	fprintf(fp, "[Widgets]\n");
	fprintf(fp, "InternetListCols = ");
	for (i=0;i<5;i++)
		fprintf(fp, "%i,",tLXOptions->iInternetList[i]);
	fprintf(fp, "%i\n",tLXOptions->iInternetList[5]);
	fprintf(fp, "LANListCols = ");
	for (i=0;i<5;i++)
		fprintf(fp, "%i,",tLXOptions->iLANList[i]);
	fprintf(fp, "%i\n",tLXOptions->iLANList[5]);
	fprintf(fp, "FavouritesListCols = ");
	for (i=0;i<5;i++)
		fprintf(fp, "%i,",tLXOptions->iFavouritesList[i]);
	fprintf(fp, "%i\n",tLXOptions->iFavouritesList[5]);
	fprintf(fp, "\n");

    fprintf(fp, "[MediaPlayer]\n");
    fprintf(fp, "Repeat = %s\n",   tLXOptions->bRepeatPlaylist ? "true" : "false");
    fprintf(fp, "Shuffle = %s\n", tLXOptions->bShufflePlaylist ? "true" : "false");
    fprintf(fp, "Left = %d\n", tLXOptions->iMPlayerLeft);
    fprintf(fp, "Top = %d\n", tLXOptions->iMPlayerTop);
    fprintf(fp, "MusicVolume = %d\n", tLXOptions->iMusicVolume);
    fprintf(fp, "\n");

    fprintf(fp, "[LastGame]\n");
    fprintf(fp, "Lives = %d\n",     tLXOptions->tGameinfo.iLives);
    fprintf(fp, "KillLimit = %d\n", tLXOptions->tGameinfo.iKillLimit);
    fprintf(fp, "TimeLimit = %d\n", tLXOptions->tGameinfo.iTimeLimit);
    fprintf(fp, "TagLimit = %d\n",  tLXOptions->tGameinfo.iTagLimit);
    fprintf(fp, "LoadingTime = %d\n",tLXOptions->tGameinfo.iLoadingTime);
    fprintf(fp, "Bonuses = %s\n",   tLXOptions->tGameinfo.iBonusesOn ? "true" : "false");
    fprintf(fp, "BonusNames = %s\n",tLXOptions->tGameinfo.iShowBonusName ? "true" : "false");
    fprintf(fp, "MaxPlayers = %d\n",tLXOptions->tGameinfo.iMaxPlayers);
	fprintf(fp, "Tournament = %s\n",tLXOptions->tGameinfo.bTournament ? "true" : "false");
    fprintf(fp, "ServerName = %s\n",tLXOptions->tGameinfo.sServerName.c_str());
	fprintf(fp, "WelcomeMessage = %s\n",tLXOptions->tGameinfo.sWelcomeMessage.c_str());
    fprintf(fp, "LevelName = %s\n", tLXOptions->tGameinfo.sMapName.c_str());
    fprintf(fp, "GameType = %d\n",  tLXOptions->tGameinfo.nGameType);
    fprintf(fp, "ModName = %s\n",   tLXOptions->tGameinfo.szModName.c_str());
    fprintf(fp, "Password = %s\n",  tLXOptions->tGameinfo.szPassword.c_str());
    fprintf(fp, "RegisterServer = %s\n",tLXOptions->tGameinfo.bRegServer ? "true" : "false");
	fprintf(fp, "LastSelectedPlayer = %d\n",tLXOptions->tGameinfo.iLastSelectedPlayer);
	fprintf(fp, "AllowWantsJoinMsg = %s\n",tLXOptions->tGameinfo.bAllowWantsJoinMsg ? "true" : "false");
	fprintf(fp, "AllowRemoteBots = %s\n",tLXOptions->tGameinfo.bAllowRemoteBots ? "true" : "false");
    fprintf(fp, "\n");

    fprintf(fp, "[Advanced]\n");
    fprintf(fp, "MaxFPS = %d\n",    tLXOptions->nMaxFPS);
	fprintf(fp, "JpegQuality = %d\n", tLXOptions->iJpegQuality);
	fprintf(fp, "NetworkUpdatePeriod = %f\n", tLXOptions->fUpdatePeriod);

    fclose(fp);
}

////////////////////
// Loads the texts used by server
bool LoadNetworkStrings(void)
{
	printf("Loading network texts... ");
	NetworkTexts = new networktexts_t;
	if (!NetworkTexts)
		return false;
	const std::string f = "cfg/network.txt";
	ReadString (f, "NetworkTexts", "HasConnected",    NetworkTexts->sHasConnected,	"<player> has connected");
	ReadString (f, "NetworkTexts", "HasLeft",	      NetworkTexts->sHasLeft,		"<player> has left");
	ReadString (f, "NetworkTexts", "HasTimedOut",     NetworkTexts->sHasTimedOut,	"<player> has timed out");

	ReadString (f, "NetworkTexts", "HasBeenKicked",   NetworkTexts->sHasBeenKicked,	"<player> has been kicked out");
	ReadString (f, "NetworkTexts", "HasBeenBanned",   NetworkTexts->sHasBeenBanned,	"<player> has been banned");
	ReadString (f, "NetworkTexts", "HasBeenMuted",    NetworkTexts->sHasBeenMuted,	"<player> has been muted");
	ReadString (f, "NetworkTexts", "HasBeenUnmuted",  NetworkTexts->sHasBeenUnmuted,"<player> has been unmuted");
	ReadString (f, "NetworkTexts", "KickedYou",		  NetworkTexts->sKickedYou,		"You have been kicked");
	ReadString (f, "NetworkTexts", "BannedYou",		  NetworkTexts->sBannedYou,		"You have been banned");
	ReadString (f, "NetworkTexts", "YouQuit",		  NetworkTexts->sYouQuit,		"You have quit");
	ReadString (f, "NetworkTexts", "YouTimed",		  NetworkTexts->sYouTimed,		"You timed out");

	ReadString (f, "NetworkTexts", "Killed",	      NetworkTexts->sKilled,		"<killer> killed <victim>");
	ReadString (f, "NetworkTexts", "CommitedSuicide", NetworkTexts->sCommitedSuicide,"<player> commited suicide");
	ReadString (f, "NetworkTexts", "FirstBlood",	  NetworkTexts->sFirstBlood,	"<player> drew first blood");
	ReadString (f, "NetworkTexts", "TeamKill",		  NetworkTexts->sTeamkill,		"<player> is an ugly teamkiller");

	ReadString (f, "NetworkTexts", "PlayerOut",		  NetworkTexts->sPlayerOut,		"<player> is out of the game");
	ReadString (f, "NetworkTexts", "TeamOut",		  NetworkTexts->sTeamOut,		"The <team> team is out of the game");
	ReadString (f, "NetworkTexts", "PlayerHasWon",	  NetworkTexts->sPlayerHasWon,	"<player> has won the match");
	ReadString (f, "NetworkTexts", "TeamHasWon",	  NetworkTexts->sTeamHasWon,	"The <team> team has won the match");

	ReadString (f, "NetworkTexts", "WormIsIt",		  NetworkTexts->sWormIsIt,		"<player> is IT!");

	ReadString (f, "NetworkTexts", "Spree1",		  NetworkTexts->sSpree1,		"<player> is on a killing spree!");
	ReadString (f, "NetworkTexts", "Spree2",		  NetworkTexts->sSpree2,		"<player> is on a rampage!");
	ReadString (f, "NetworkTexts", "Spree3",		  NetworkTexts->sSpree3,		"<player> is dominating!");
	ReadString (f, "NetworkTexts", "Spree4",		  NetworkTexts->sSpree4,		"<player> is unstoppable!");
	ReadString (f, "NetworkTexts", "Spree5",		  NetworkTexts->sSpree5,		"<player> is GODLIKE!");

	ReadString (f, "NetworkTexts", "ServerFull",	  NetworkTexts->sServerFull,		"Server is full");
	ReadString (f, "NetworkTexts", "NoEmptySlots",	  NetworkTexts->sNoEmptySlots,		"The server has no emtpy slots");
	ReadString (f, "NetworkTexts", "WrongProtocol",	  NetworkTexts->sWrongProtocol,		"Wrong protocol version. Server protocol version is <version>.");
	ReadString (f, "NetworkTexts", "BadVerification", NetworkTexts->sBadVerification,	"Bad connection verification");
	ReadString (f, "NetworkTexts", "NoIpVerification",NetworkTexts->sNoIpVerification,	"No verification for address");
	ReadString (f, "NetworkTexts", "GameInProgress",  NetworkTexts->sGameInProgress,	"Cannot join, the game is currently in progress");
	ReadString (f, "NetworkTexts", "YouAreBanned",	  NetworkTexts->sYouAreBanned,		"You are banned on this server");
	ReadString (f, "NetworkTexts", "BotsNotAllowed",  NetworkTexts->sBotsNotAllowed,	"Sorry, bots are not allowed on this server");
	ReadString (f, "NetworkTexts", "WantsJoin",		  NetworkTexts->sWantsJoin,			"<player> wants join the server");

	printf("DONE\n");
	return true;
}

