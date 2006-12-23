/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
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
    char    *ply_keys[] = {"Up", "Down", "Left", "Right", "Shoot", "Jump", "SelectWeapon", "Rope"};
    char    *ply_def1[] = {"up", "down", "left", "right", "lctrl", "lalt", "lshift", "z"};
    char    *ply_def2[] = {"r",  "f",    "d",    "g",     "rctrl", "ralt", "rshift", "/"};
    char    *gen_keys[] = {"Chat", "ShowScore", "ShowHealth", "ShowSettings",  "TakeScreenshot",  "ViewportManager", "SwitchMode"};
    char    *gen_def[]  = {"i",    "tab",	"h",	"space",   "F12",    "F2",  "F5"};
    
    int     i;
	
	tLXOptions = new options_t;
	if(tLXOptions == NULL) {
		return false;
	}
	tLXOptions->tSearchPaths = NULL;
	tLXOptions->tSearchPaths = NULL;


    char *f = "cfg/options.cfg";

  	AddKeyword("true",true);
	AddKeyword("false",false);


	// File handling
	// read this first, because perhaps we will have new searchpaths
	InitBaseSearchPaths();
	char tmp[20], tmp2[30];
	strcpy(tmp, "SearchPath"); i = 1;
	filelist_t** spath = &tLXOptions->tSearchPaths;
    while(true) {
    	 // &tmp[10] is the end of "SearchPath"
    	strcpy(&tmp[10], itoa(i, tmp2, 10));
    	*spath = new filelist_t;
		(*spath)->next = NULL;
		(*spath)->filename[0] = '\0';
        if(!ReadString(f, "FileHandling", tmp, (*spath)->filename, NULL)) {
        	delete *spath; *spath = NULL;     	
        	break;
        }
		i++; spath = &(*spath)->next;
	}
	for(spath = &basesearchpaths; *spath != NULL; spath = &(*spath)->next)
		if(!FileListIncludes(tLXOptions->tSearchPaths, (*spath)->filename))
			AddToFileList(&tLXOptions->tSearchPaths, (*spath)->filename);


    // Video
    ReadKeyword(f, "Video", "Fullscreen",   &tLXOptions->iFullscreen, true);
    ReadKeyword(f, "Video", "ShowFPS",      &tLXOptions->iShowFPS, false);

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
    for(i=0; i<8; i++) {
        ReadString(f, "Ply1Controls", ply_keys[i], tLXOptions->sPlayer1Controls[i], ply_def1[i]);
        ReadString(f, "Ply2Controls", ply_keys[i], tLXOptions->sPlayer2Controls[i], ply_def2[i]);
    }

    // General controls
    for(i=0; i<7; i++)
        ReadString(f, "GeneralControls", gen_keys[i], tLXOptions->sGeneralControls[i], gen_def[i]);

    // Game
    ReadInteger(f, "Game", "Blood",         &tLXOptions->iBloodAmount, 100);
    ReadKeyword(f, "Game", "Shadows",       &tLXOptions->iShadows, true);
    ReadKeyword(f, "Game", "Particles",     &tLXOptions->iParticles, true);
    ReadKeyword(f, "Game", "OldSkoolRope",  &tLXOptions->iOldSkoolRope, false);
	ReadKeyword(f, "Game", "ShowWormHealth",&tLXOptions->iShowHealth, false);
	ReadKeyword(f, "Game", "ColorizeNicks", &tLXOptions->iColorizeNicks, false);
	ReadKeyword(f, "Game", "AutoTyping",	&tLXOptions->iAutoTyping, true);

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

	return true;
}

// shutdown a filelist_t structure
void ShutdownFilenameStruct(filelist_t* f) {
	if(f != NULL) {
		ShutdownFilenameStruct(f->next);
		delete f;
	}
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
	ShutdownFilenameStruct(tLXOptions->tSearchPaths);
	delete tLXOptions;
	tLXOptions = NULL;
	
	if (NetworkTexts)
		delete NetworkTexts;
	NetworkTexts = NULL;

	ShutdownFilenameStruct(basesearchpaths);
	basesearchpaths = NULL;
}


///////////////////
// Save the options
void SaveOptions(void)
{
    char    *ply_keys[] = {"Up", "Down", "Left", "Right", "Shoot", "Jump", "SelectWeapon", "Rope"};
    char    *gen_keys[] = {"Chat", "ShowScore", "ShowHealth", "ShowSettings", "TakeScreenshot", "ViewportManager", "SwitchMode"};
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
	filelist_t* spath = tLXOptions->tSearchPaths;
	for(i = 1; spath != NULL; i++, spath = spath->next)
    	fprintf(fp, "SearchPath%i = %s\n", i, spath->filename);	
	fprintf(fp,"\n");

    fprintf(fp, "[Ply1Controls]\n");
    for(i=0; i<8; i++)
        fprintf(fp, "%s = %s\n", ply_keys[i], tLXOptions->sPlayer1Controls[i]);    
    fprintf(fp, "\n");

    fprintf(fp, "[Ply2Controls]\n");
    for(i=0; i<8; i++)
        fprintf(fp, "%s = %s\n", ply_keys[i], tLXOptions->sPlayer2Controls[i]);
    fprintf(fp, "\n");

    fprintf(fp, "[GeneralControls]\n");
    for(i=0; i<7; i++)
        fprintf(fp, "%s = %s\n", gen_keys[i], tLXOptions->sGeneralControls[i]);
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
    fprintf(fp, "ServerName = %s\n",tLXOptions->tGameinfo.sServerName);
	fprintf(fp, "WelcomeMessage = %s\n",tLXOptions->tGameinfo.sWelcomeMessage);
    fprintf(fp, "LevelName = %s\n", tLXOptions->tGameinfo.sMapName);
    fprintf(fp, "GameType = %d\n",  tLXOptions->tGameinfo.nGameType);
    fprintf(fp, "ModName = %s\n",   tLXOptions->tGameinfo.szModName);
    fprintf(fp, "Password = %s\n",  tLXOptions->tGameinfo.szPassword);
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
	NetworkTexts = new networktexts_t;
	if (!NetworkTexts)
		return false;
	char *f = {"cfg/network.txt"};
	ReadString (f, "NetworkTexts", "HasConnected",    NetworkTexts->sHasConnected,   "<player> has connected");
	ReadString (f, "NetworkTexts", "HasLeft",	      NetworkTexts->sHasLeft,	     "<player> has left");
	ReadString (f, "NetworkTexts", "HasTimedOut",     NetworkTexts->sHasTimedOut,    "<player> has timed out");

	ReadString (f, "NetworkTexts", "HasBeenKicked",   NetworkTexts->sHasBeenKicked,  "<player> has been kicked out");
	ReadString (f, "NetworkTexts", "HasBeenBanned",   NetworkTexts->sHasBeenBanned,  "<player> has been banned");
	ReadString (f, "NetworkTexts", "HasBeenMuted",    NetworkTexts->sHasBeenMuted,   "<player> has been muted");
	ReadString (f, "NetworkTexts", "HasBeenUnmuted",  NetworkTexts->sHasBeenUnmuted, "<player> has been unmuted");
	ReadString (f, "NetworkTexts", "KickedYou",		  NetworkTexts->sKickedYou,      "You have been kicked");
	ReadString (f, "NetworkTexts", "BannedYou",		  NetworkTexts->sBannedYou,      "You have been banned");
	ReadString (f, "NetworkTexts", "YouQuit",		  NetworkTexts->sYouQuit,        "You have quit");
	ReadString (f, "NetworkTexts", "YouTimed",		  NetworkTexts->sYouTimed,       "You timed out");

	ReadString (f, "NetworkTexts", "Killed",	      NetworkTexts->sKilled,		 "<killer> killed <victim>");
	ReadString (f, "NetworkTexts", "CommitedSuicide", NetworkTexts->sCommitedSuicide,"<player> commited suicide");
	ReadString (f, "NetworkTexts", "FirstBlood",	  NetworkTexts->sFirstBlood,	 "<player> drew first blood");
	ReadString (f, "NetworkTexts", "TeamKill",		  NetworkTexts->sTeamkill,		 "<player> is an ugly teamkiller");

	ReadString (f, "NetworkTexts", "PlayerOut",		  NetworkTexts->sPlayerOut,		 "<player> is out of the game");
	ReadString (f, "NetworkTexts", "TeamOut",		  NetworkTexts->sTeamOut,		 "The <team> team is out of the game");
	ReadString (f, "NetworkTexts", "PlayerHasWon",	  NetworkTexts->sPlayerHasWon,	 "<player> has won the match");
	ReadString (f, "NetworkTexts", "TeamHasWon",	  NetworkTexts->sTeamHasWon,	 "The <team> team has won the match");

	ReadString (f, "NetworkTexts", "WormIsIt",		  NetworkTexts->sWormIsIt,		 "<player> is IT!");

	ReadString (f, "NetworkTexts", "ServerFull",	  NetworkTexts->sServerFull,	 "Server is full");
	ReadString (f, "NetworkTexts", "NoEmptySlots",	  NetworkTexts->sNoEmptySlots,	 "The server has no emtpy slots");
	ReadString (f, "NetworkTexts", "WrongProtocol",	  NetworkTexts->sWrongProtocol,	 "Wrong protocol version. Server protocol version is <version>.");
	ReadString (f, "NetworkTexts", "BadVerification", NetworkTexts->sBadVerification,"Bad connection verification");
	ReadString (f, "NetworkTexts", "NoIpVerification",NetworkTexts->sNoIpVerification,"No verification for address");
	ReadString (f, "NetworkTexts", "GameInProgress",  NetworkTexts->sGameInProgress, "Cannot join, the game is currently in progress");
	ReadString (f, "NetworkTexts", "YouAreBanned",	  NetworkTexts->sYouAreBanned,	 "You are banned on this server");
	ReadString (f, "NetworkTexts", "BotsNotAllowed",  NetworkTexts->sBotsNotAllowed, "Sorry, bots are not allowed on this server");
	ReadString (f, "NetworkTexts", "WantsJoin",		  NetworkTexts->sWantsJoin,		 "<player> wants join the server");

	return true;
}

