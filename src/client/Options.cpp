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


#include "LieroX.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "Options.h"
#include "ConfigHandler.h"


GameOptions	*tLXOptions = NULL;
NetworkTexts	*networkTexts = NULL;


bool GameOptions::Init() {
	if(tLXOptions) {
		printf("WARNING: it seems that the GameOptions are already inited\n");
		return true;
	}

	tLXOptions = new GameOptions;
	if(tLXOptions == NULL) {
		printf("ERROR: not enough mem for GameOptions\n");
		return false;
	}
	
	return tLXOptions->LoadFromDisc();
}


///////////////////
// Load the options
bool GameOptions::LoadFromDisc()
{
	printf("Loading options... \n");

    const std::string    ply_keys[] = {"Up", "Down", "Left", "Right", "Shoot", "Jump", "SelectWeapon", "Rope", "Strafe"};
    const std::string    ply_def1[] = {"up", "down", "left", "right", "lctrl", "lalt", "lshift", "x", "z"};
    const std::string    ply_def2[] = {"r",  "f",    "d",    "g",     "rctrl", "ralt", "rshift", "/", "."};
	const std::string    gen_keys[] = {"Chat", "ShowScore", "ShowHealth", "ShowSettings",  "TakeScreenshot",  "ViewportManager", "SwitchMode", "ToggleTopBar", "MediaPlayer"};
    const std::string    gen_def[]  = {"i",    "tab",		"h",		  "space",	       "F12",			  "F2",				 "F5",		   "F8",		   "F3"};
	const int	 def_widths[] = {32,180,70,80,60,150};

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

		AddToFileList(&tSearchPaths, value);
		i++;
	}

	for(searchpathlist::const_iterator p1 = basesearchpaths.begin(); p1 != basesearchpaths.end(); i++,p1++)  {
		AddToFileList(&tSearchPaths, *p1);
	}

	// print the searchpaths, this may be very usefull for the user
	printf("I have now the following searchpaths (in this direction):\n");
	for(searchpathlist::const_iterator p2 = tSearchPaths.begin(); p2 != tSearchPaths.end(); p2++) {
		std::string path = *p2;
		ReplaceFileVariables(path);
		printf("  %s\n", path.c_str());
	}
	printf(" And that's all.\n");

	for (i=0;i<sizeof(iInternetList)/sizeof(int);i++)  {
		iInternetList[i] = def_widths[i];
		iLANList[i] = def_widths[i];
		iFavouritesList[i] = def_widths[i];
	}


    // Video
#ifdef WIN32
    ReadKeyword(f, "Video", "Fullscreen",   &iFullscreen, true);
#else
	// non-windows users probably like this more
    ReadKeyword(f, "Video", "Fullscreen",   &iFullscreen, false);
#endif	
    ReadKeyword(f, "Video", "ShowFPS",      &iShowFPS, false);
    ReadKeyword(f, "Video", "OpenGL",       &bOpenGL, false);
#ifdef MACOSX	
	// this seems currently to be better for Mac OS X (16 bit looks crappy)
	// TODO: also 24bit for others?
	ReadInteger(f, "Video", "ColourDepth",	&iColourDepth, 24);
#else
	ReadInteger(f, "Video", "ColourDepth",	&iColourDepth, 16);
#endif
	
    // Network
    ReadInteger(f, "Network", "Port",       &iNetworkPort, LX_PORT);
    ReadInteger(f, "Network", "Speed",      &iNetworkSpeed, NST_MODEM);
	ReadKeyword(f, "Network", "UseIpToCountry", &bUseIpToCountry, true);
	ReadKeyword(f, "Network", "LoadDbAtStartup", &bLoadDbAtStartup, false);
    ReadString (f, "Network", "STUNServer", sSTUNServer, "stunserver.org");

    // Audio
    ReadKeyword(f, "Audio", "Enabled",      &iSoundOn, true);
    ReadInteger(f, "Audio", "Volume",       &iSoundVolume, 70);

	// Misc.
	ReadKeyword(f, "Misc", "LogConversations", &iLogConvos, true);
	ReadKeyword(f, "Misc", "ShowPing",		   &iShowPing, true);
	ReadInteger(f, "Misc", "ScreenshotFormat", &iScreenshotFormat, FMT_PNG);

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
        	sPlayerControls.push_back(controls_t());
        sPlayerControls[j-1][i] = value;

        i++;
        if(i >= 9) {
        	i = 0; j++;
        }
    }

    // General controls
    for(i=0; i<9; i++)
        ReadString(f, "GeneralControls", gen_keys[i], sGeneralControls[i], gen_def[i]);

    // Game
    ReadInteger(f, "Game", "Blood",         &iBloodAmount, 100);
    ReadKeyword(f, "Game", "Shadows",       &iShadows, true);
    ReadKeyword(f, "Game", "Particles",     &iParticles, true);
    ReadKeyword(f, "Game", "OldSkoolRope",  &iOldSkoolRope, false);
	ReadKeyword(f, "Game", "ShowWormHealth",&iShowHealth, false);
	ReadKeyword(f, "Game", "ColorizeNicks", &iColorizeNicks, false);
	ReadKeyword(f, "Game", "AutoTyping",	&iAutoTyping, true);
	ReadKeyword(f, "Game", "Antialiasing",	&bAntiAliasing, false);
    ReadKeyword(f, "Game", "MouseAiming",   &bMouseAiming, false);
	ReadKeyword(f, "Game", "AllowMouseAiming",   &bAllowMouseAiming, false);
	ReadKeyword(f, "Game", "UseNumericKeysToSwitchWeapons",   &bUseNumericKeysToSwitchWeapons, true);

	// Widget states
	ReadIntArray(f, "Widgets","InternetListCols",	&iInternetList[0],6);
	ReadIntArray(f, "Widgets","LANListCols",		&iLANList[0],6);
	ReadIntArray(f, "Widgets","FavouritesListCols",	&iFavouritesList[0],6);

	// Media player
	ReadKeyword(f, "MediaPlayer", "Repeat",		&bRepeatPlaylist, true);
	ReadKeyword(f, "MediaPlayer", "Shuffle",	&bShufflePlaylist, false);
	ReadInteger(f, "MediaPlayer", "Left",		&iMPlayerLeft, 350);
	ReadInteger(f, "MediaPlayer", "Top",		&iMPlayerTop, 240);
	ReadInteger(f, "MediaPlayer", "MusicVolume",&iMusicVolume, 50);


    // Last Game
    ReadInteger(f, "LastGame", "Lives",     &tGameinfo.iLives, 10);
    ReadInteger(f, "LastGame", "KillLimit", &tGameinfo.iKillLimit, -1);
    ReadInteger(f, "LastGame", "TimeLimit", &tGameinfo.iTimeLimit, -1);
    ReadInteger(f, "LastGame", "TagLimit",  &tGameinfo.iTagLimit, 5);
    ReadInteger(f, "LastGame", "LoadingTime",   &tGameinfo.iLoadingTime, 100);
    ReadKeyword(f, "LastGame", "Bonuses",   &tGameinfo.iBonusesOn, true);
    ReadKeyword(f, "LastGame", "BonusNames",&tGameinfo.iShowBonusName, true);
    ReadInteger(f, "LastGame", "MaxPlayers",&tGameinfo.iMaxPlayers, 8);
	ReadKeyword(f, "LastGame", "MatchLogging", &tGameinfo.bMatchLogging, true);
    ReadString (f, "LastGame", "ServerName",tGameinfo.sServerName, "LieroX Server");
	ReadString (f, "LastGame", "WelcomeMessage",tGameinfo.sWelcomeMessage, "Welcome to <server>, <player>");
    ReadString (f, "LastGame", "LevelName", tGameinfo.sMapFilename, "");
    ReadInteger(f, "LastGame", "GameType",  &tGameinfo.nGameType, GMT_DEATHMATCH);
    ReadString (f, "LastGame", "ModName",   tGameinfo.szModName, "Classic");
    ReadString (f, "LastGame", "Password",  tGameinfo.szPassword, "");
    ReadKeyword(f, "LastGame", "RegisterServer",&tGameinfo.bRegServer, true);
	ReadInteger(f, "LastGame", "LastSelectedPlayer",&tGameinfo.iLastSelectedPlayer, 0);
	ReadKeyword(f, "LastGame", "AllowWantsJoinMsg",&tGameinfo.bAllowWantsJoinMsg, true);
	ReadKeyword(f, "LastGame", "WantsToJoinFromBanned",&tGameinfo.bWantsJoinBanned, true);
	ReadKeyword(f, "LastGame", "AllowRemoteBots", &tGameinfo.bAllowRemoteBots, true);
	ReadKeyword(f, "LastGame", "TopBarVisible", &tGameinfo.bTopBarVisible, true);
	ReadKeyword(f, "LastGame", "AllowNickChange", &tGameinfo.bAllowNickChange, true);
	ReadFloat(f, "LastGame", "BonusFrequency", &tGameinfo.fBonusFreq, 30);
	ReadFloat(f, "LastGame", "BonusLife", &tGameinfo.fBonusLife, 60);

    // Advanced
    ReadInteger(f, "Advanced", "MaxFPS",    &nMaxFPS, 95);
	ReadInteger(f, "Advanced", "JpegQuality", &iJpegQuality, 80);
	ReadFloat  (f, "Advanced", "NetworkUpdatePeriod", &fUpdatePeriod,0.05f);
	ReadKeyword(f, "Advanced", "CountTeamkills", &bCountTeamkills, false);
	ReadKeyword(f, "Advanced", "ServerSideHealth", &bServerSideHealth, true);

	// Clamp the Jpeg quality
	if (iJpegQuality < 1)
		iJpegQuality = 1;
	if (iJpegQuality > 100)
		iJpegQuality = 100;

	printf("DONE loading options\n");

	return true;
}

///////////////////
// Save & shutdown the options
void ShutdownOptions(void)
{
	if(tLXOptions) {
		delete tLXOptions;
		tLXOptions = NULL;
	}
	
	if(networkTexts) {
		delete networkTexts;
		networkTexts = NULL;	
	}
}


///////////////////
// Save the options
void GameOptions::SaveToDisc()
{
    const std::string    ply_keys[] = {"Up", "Down", "Left", "Right", "Shoot", "Jump", "SelectWeapon", "Rope", "Strafe"};
    const std::string    gen_keys[] = {"Chat", "ShowScore", "ShowHealth", "ShowSettings", "TakeScreenshot", "ViewportManager", "SwitchMode", "ToggleTopBar", "MediaPlayer"};
    int     i;

    FILE *fp = OpenGameFile("cfg/options.cfg", "wt");
    if(fp == NULL)
        return;


    fprintf(fp, "# OpenLieroX Options File\n");
    fprintf(fp, "# Note: This file is automatically generated\n\n");

    fprintf(fp, "[Video]\n");
    fprintf(fp, "Fullscreen = %s\n",iFullscreen ? "true" : "false");
    fprintf(fp, "ShowFPS = %s\n",iShowFPS ? "true" : "false");
    fprintf(fp, "OpenGL = %s\n",bOpenGL ? "true" : "false");
	fprintf(fp, "ColourDepth = %d\n",iColourDepth);
    fprintf(fp, "\n");

    fprintf(fp, "[Network]\n");
    fprintf(fp, "Port = %d\n",      iNetworkPort);
    fprintf(fp, "Speed = %d\n",     iNetworkSpeed);
	fprintf(fp, "UseIpToCountry = %s\n",     bUseIpToCountry ? "true" : "false");
	fprintf(fp, "LoadDbAtStartup = %s\n",    bLoadDbAtStartup ? "true" : "false");
	fprintf(fp, "STUNServer = %s\n",    sSTUNServer.c_str() );
    fprintf(fp, "\n");

    fprintf(fp, "[Audio]\n");
    fprintf(fp, "Enabled = %s\n",   iSoundOn ? "true" : "false");
    fprintf(fp, "Volume = %d\n",    iSoundVolume);
    fprintf(fp, "\n");

	fprintf(fp,"[Misc]\n");
	fprintf(fp,"LogConversations = %s\n",	iLogConvos ? "true" : "false");
	fprintf(fp,"ShowPing = %s\n",			iShowPing ? "true" : "false");
	fprintf(fp,"ScreenshotFormat = %d\n",	iScreenshotFormat);
	fprintf(fp,"\n");

	fprintf(fp, "[FileHandling]\n");
	i=1;
	for(searchpathlist::const_iterator p = tSearchPaths.begin(); p != tSearchPaths.end(); p++, i++)
    	fprintf(fp, "SearchPath%i = %s\n", i, p->c_str());
	fprintf(fp,"\n");

	uint j = 0;
	for(;j<sPlayerControls.size();j++) {
		fprintf(fp, "[Ply%iControls]\n", j+1);
		for(i=0; i<9; i++)
        	fprintf(fp, "%s = %s\n", ply_keys[i].c_str(), sPlayerControls[j][i].c_str());
	    fprintf(fp, "\n");
	}

    fprintf(fp, "[GeneralControls]\n");
    for(i=0; i<9; i++)
        fprintf(fp, "%s = %s\n", gen_keys[i].c_str(), sGeneralControls[i].c_str());
    fprintf(fp, "\n");

    fprintf(fp, "[Game]\n");
    fprintf(fp, "Blood = %d\n",     iBloodAmount);
    fprintf(fp, "Shadows = %s\n",   iShadows ? "true" : "false");
    fprintf(fp, "Particles = %s\n", iParticles ? "true" : "false");
    fprintf(fp, "OldSkoolRope = %s\n", iOldSkoolRope ? "true" : "false");
	fprintf(fp, "ShowWormHealth = %s\n", iShowHealth ? "true" : "false");
	fprintf(fp, "ColorizeNicks = %s\n", iColorizeNicks ? "true" : "false");
	fprintf(fp, "AutoTyping = %s\n", iAutoTyping ? "true" : "false");
	fprintf(fp, "Antialiasing = %s\n", bAntiAliasing ? "true" : "false");
	fprintf(fp, "MouseAiming = %s\n", bMouseAiming ? "true" : "false");
	fprintf(fp, "AllowMouseAiming = %s\n", bAllowMouseAiming ? "true" : "false");
	fprintf(fp, "UseNumericKeysToSwitchWeapons = %s\n", bUseNumericKeysToSwitchWeapons ? "true" : "false");
    fprintf(fp, "\n");

	fprintf(fp, "[Widgets]\n");
	fprintf(fp, "InternetListCols = ");
	for (i=0;i<5;i++)
		fprintf(fp, "%i,",iInternetList[i]);
	fprintf(fp, "%i\n",iInternetList[5]);
	fprintf(fp, "LANListCols = ");
	for (i=0;i<5;i++)
		fprintf(fp, "%i,",iLANList[i]);
	fprintf(fp, "%i\n",iLANList[5]);
	fprintf(fp, "FavouritesListCols = ");
	for (i=0;i<5;i++)
		fprintf(fp, "%i,",iFavouritesList[i]);
	fprintf(fp, "%i\n",iFavouritesList[5]);
	fprintf(fp, "\n");

    fprintf(fp, "[MediaPlayer]\n");
    fprintf(fp, "Repeat = %s\n",   bRepeatPlaylist ? "true" : "false");
    fprintf(fp, "Shuffle = %s\n", bShufflePlaylist ? "true" : "false");
    fprintf(fp, "Left = %d\n", iMPlayerLeft);
    fprintf(fp, "Top = %d\n", iMPlayerTop);
    fprintf(fp, "MusicVolume = %d\n", iMusicVolume);
    fprintf(fp, "\n");

    fprintf(fp, "[LastGame]\n");
    fprintf(fp, "Lives = %d\n",     tGameinfo.iLives);
    fprintf(fp, "KillLimit = %d\n", tGameinfo.iKillLimit);
    fprintf(fp, "TimeLimit = %d\n", tGameinfo.iTimeLimit);
    fprintf(fp, "TagLimit = %d\n",  tGameinfo.iTagLimit);
    fprintf(fp, "LoadingTime = %d\n",tGameinfo.iLoadingTime);
    fprintf(fp, "Bonuses = %s\n",   tGameinfo.iBonusesOn ? "true" : "false");
    fprintf(fp, "BonusNames = %s\n",tGameinfo.iShowBonusName ? "true" : "false");
    fprintf(fp, "MaxPlayers = %d\n",tGameinfo.iMaxPlayers);
	fprintf(fp, "MatchLogging = %s\n",tGameinfo.bMatchLogging ? "true" : "false");
    fprintf(fp, "ServerName = %s\n",tGameinfo.sServerName.c_str());
	fprintf(fp, "WelcomeMessage = %s\n",tGameinfo.sWelcomeMessage.c_str());
    fprintf(fp, "LevelName = %s\n", tGameinfo.sMapFilename.c_str());
    fprintf(fp, "GameType = %d\n",  tGameinfo.nGameType);
    fprintf(fp, "ModName = %s\n",   tGameinfo.szModName.c_str());
    fprintf(fp, "Password = %s\n",  tGameinfo.szPassword.c_str());
    fprintf(fp, "RegisterServer = %s\n",tGameinfo.bRegServer ? "true" : "false");
	fprintf(fp, "LastSelectedPlayer = %d\n",tGameinfo.iLastSelectedPlayer);
	fprintf(fp, "AllowWantsJoinMsg = %s\n",tGameinfo.bAllowWantsJoinMsg ? "true" : "false");
	fprintf(fp, "WantsToJoinFromBanned = %s\n",tGameinfo.bWantsJoinBanned ? "true" : "false");
	fprintf(fp, "AllowRemoteBots = %s\n",tGameinfo.bAllowRemoteBots ? "true" : "false");
	fprintf(fp, "TopBarVisible = %s\n",tGameinfo.bTopBarVisible ? "true" : "false");
	fprintf(fp, "AllowNickChange = %s\n",tGameinfo.bAllowNickChange ? "true" : "false");
	fprintf(fp, "BonusFrequency = %f\n",tGameinfo.fBonusFreq);
	fprintf(fp, "BonusLife = %f\n",tGameinfo.fBonusLife);
    fprintf(fp, "\n");

    fprintf(fp, "[Advanced]\n");
    fprintf(fp, "MaxFPS = %d\n",    nMaxFPS);
	fprintf(fp, "JpegQuality = %d\n", iJpegQuality);
	fprintf(fp, "NetworkUpdatePeriod = %f\n", fUpdatePeriod);
	fprintf(fp, "CountTeamkills = %s\n", bCountTeamkills ? "true" : "false");
	fprintf(fp, "ServerSideHealth = %s\n", bServerSideHealth ? "true" : "false");

    fclose(fp);
}

bool NetworkTexts::Init() {
	if(networkTexts) {
		printf("WARNING: networktexts are already inited; ignoring ...\n");
		return true;
	}

	networkTexts = new NetworkTexts;
	if(!networkTexts) {
		printf("ERROR: not enough mem for networktexts\n");	
		return false;
	}

	return networkTexts->LoadFromDisc();
}

////////////////////
// Loads the texts used by server
bool NetworkTexts::LoadFromDisc()
{
	printf("Loading network texts... ");
	
	const std::string f = "cfg/network.txt";
	ReadString (f, "NetworkTexts", "HasConnected",    sHasConnected,	"<player> has connected");
	ReadString (f, "NetworkTexts", "HasLeft",	      sHasLeft,			"<player> has left");
	ReadString (f, "NetworkTexts", "HasTimedOut",     sHasTimedOut,		"<player> has timed out");

	ReadString (f, "NetworkTexts", "HasBeenKicked",   sHasBeenKicked,	"<player> has been kicked out");
	ReadString (f, "NetworkTexts", "HasBeenKickedReason", sHasBeenKickedReason, "<player> has been kicked out because <reason>");
	ReadString (f, "NetworkTexts", "HasBeenBanned",   sHasBeenBanned,	"<player> has been banned");
	ReadString (f, "NetworkTexts", "HasBeenMuted",    sHasBeenMuted,	"<player> has been muted");
	ReadString (f, "NetworkTexts", "HasBeenUnmuted",  sHasBeenUnmuted,	"<player> has been unmuted");
	ReadString (f, "NetworkTexts", "KickedYou",		  sKickedYou,		"You have been kicked");
	ReadString (f, "NetworkTexts", "KickedYouReason", sKickedYouReason, "You have been kicked because <reason>");
	ReadString (f, "NetworkTexts", "BannedYou",		  sBannedYou,		"You have been banned");
	ReadString (f, "NetworkTexts", "YouQuit",		  sYouQuit,			"You have quit");
	ReadString (f, "NetworkTexts", "YouTimed",		  sYouTimed,		"You timed out");

	ReadString (f, "NetworkTexts", "Killed",	      sKilled,			"<killer> killed <victim>");
	ReadString (f, "NetworkTexts", "CommitedSuicide", sCommitedSuicide,	"<player> commited suicide");
	ReadString (f, "NetworkTexts", "FirstBlood",	  sFirstBlood,		"<player> drew first blood");
	ReadString (f, "NetworkTexts", "TeamKill",		  sTeamkill,		"<player> is an ugly teamkiller");
	ReadString (f, "NetworkTexts", "CTFScore",		  sHasScored,		"<player> has scored");

	ReadString (f, "NetworkTexts", "PlayerOut",		  sPlayerOut,		"<player> is out of the game");
	ReadString (f, "NetworkTexts", "TeamOut",		  sTeamOut,			"The <team> team is out of the game");
	ReadString (f, "NetworkTexts", "PlayerHasWon",	  sPlayerHasWon,	"<player> has won the match");
	ReadString (f, "NetworkTexts", "TeamHasWon",	  sTeamHasWon,		"The <team> team has won the match");

	ReadString (f, "NetworkTexts", "WormIsIt",		  sWormIsIt,		"<player> is IT!");

	ReadString (f, "NetworkTexts", "Spree1",		  sSpree1,			"<player> is on a killing spree!");
	ReadString (f, "NetworkTexts", "Spree2",		  sSpree2,			"<player> is on a rampage!");
	ReadString (f, "NetworkTexts", "Spree3",		  sSpree3,			"<player> is dominating!");
	ReadString (f, "NetworkTexts", "Spree4",		  sSpree4,			"<player> is unstoppable!");
	ReadString (f, "NetworkTexts", "Spree5",		  sSpree5,			"<player> is GODLIKE!");

	ReadString (f, "NetworkTexts", "DyingSpree1",	  sDSpree1,			"<player> is on a dying spree!");
	ReadString (f, "NetworkTexts", "DyingSpree2",	  sDSpree2,			"<player> is a target!");
	ReadString (f, "NetworkTexts", "DyingSpree3",	  sDSpree3,			"<player> is a sitting duck!");
	ReadString (f, "NetworkTexts", "DyingSpree4",	  sDSpree4,			"<player> is free kills!");
	ReadString (f, "NetworkTexts", "DyingSpree5",	  sDSpree5,			"<player> is WORTHLESS!");

	ReadString (f, "NetworkTexts", "ServerFull",	  sServerFull,		"Server is full");
	ReadString (f, "NetworkTexts", "NoEmptySlots",	  sNoEmptySlots,	"The server has no emtpy slots");
	ReadString (f, "NetworkTexts", "WrongProtocol",	  sWrongProtocol,	"Wrong protocol version. Server protocol version is <version>.");
	ReadString (f, "NetworkTexts", "BadVerification", sBadVerification,	"Bad connection verification");
	ReadString (f, "NetworkTexts", "NoIpVerification",sNoIpVerification,"No verification for address");
	ReadString (f, "NetworkTexts", "GameInProgress",  sGameInProgress,	"Cannot join, the game is currently in progress");
	ReadString (f, "NetworkTexts", "YouAreBanned",	  sYouAreBanned,	"You are banned on this server");
	ReadString (f, "NetworkTexts", "BotsNotAllowed",  sBotsNotAllowed,	"Sorry, bots are not allowed on this server");
	ReadString (f, "NetworkTexts", "WantsJoin",		  sWantsJoin,		"<player> wants to join the server");

	ReadString (f, "NetworkTexts", "KnownAs",		  sKnownAs,			"<oldname> is now known as <newname>");

	printf("DONE\n");
	return true;
}

