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
#include "CGuiSkin.h"

GameOptions	*tLXOptions = NULL;
NetworkTexts	*networkTexts = NULL;

const std::string    ply_keys[] = {"Up", "Down", "Left", "Right", "Shoot", "Jump", "SelectWeapon", "Rope", "Strafe"};
const std::string    ply_def1[] = {"up", "down", "left", "right", "lctrl", "lalt", "lshift", "x", "z"};
const std::string    ply_def2[] = {"r",  "f",    "d",    "g",     "rctrl", "ralt", "rshift", "/", "."};
const std::string    gen_keys[] = {"Chat", "ShowScore", "ShowHealth", "ShowSettings",  "TakeScreenshot",  "ViewportManager", "SwitchMode", "ToggleTopBar", "TeamChat",	"MediaPlayer"};
const std::string    gen_def[]  = {"i",    "tab",		"h",		  "space",	       "F12",			  "F2",				 "F5",		   "F8",		   "o",			"F3"};

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
	
	tLXOptions->sPlayerControls.resize(2);	// Don't change array size or we'll get segfault when vector memory allocation changes
	
	CScriptableVars::RegisterVars("GameOptions")	
#ifdef WIN32
		( tLXOptions->bFullscreen, "Video.Fullscreen", true )
#else
		( tLXOptions->bFullscreen, "Video.Fullscreen", false )
#endif
		( tLXOptions->bShowFPS, "Video.ShowFPS", false )
#ifdef MACOSX	
		( tLXOptions->bOpenGL, "Video.OpenGL", true )
#else
		( tLXOptions->bOpenGL, "Video.OpenGL", false )
#endif
		( tLXOptions->sResolution, "Video.Resolution", "" )
#ifdef MACOSX	
		( tLXOptions->iColourDepth, "Video.ColourDepth", 24 )
#else
		( tLXOptions->iColourDepth, "Video.ColourDepth", 16 )
#endif
		( tLXOptions->iNetworkPort, "Network.Port", LX_PORT )
		( tLXOptions->iNetworkSpeed, "Network.Speed", NST_MODEM )
		( tLXOptions->fUpdatePeriod, "Advanced.NetworkUpdatePeriod", 0.05f )
		( tLXOptions->bUseIpToCountry, "Network.UseIpToCountry", true )
		( tLXOptions->bLoadDbAtStartup, "Network.LoadDbAtStartup", false )
		( tLXOptions->sSTUNServer, "Network.STUNServer", "stunserver.org" )

		( tLXOptions->bSoundOn, "Audio.Enabled", true )
		( tLXOptions->iSoundVolume, "Audio.Volume", 70 )

		( tLXOptions->iBloodAmount, "Game.Blood", 100 )
		( tLXOptions->bShadows, "Game.Shadows", true )
		( tLXOptions->bParticles, "Game.Particles", true )
		( tLXOptions->bOldSkoolRope, "Game.OldSkoolRope", false )
		( tLXOptions->bShowHealth, "Game.ShowWormHealth", false )
		( tLXOptions->bColorizeNicks, "Game.ColorizeNicks", false )
		( tLXOptions->bAutoTyping, "Game.AutoTyping", false )
		( tLXOptions->sSkinPath, "Game.SkinPath", "" )
		( tLXOptions->bAntiAliasing, "Game.Antialiasing", false )
		( tLXOptions->bMouseAiming, "Game.MouseAiming", false )
		( tLXOptions->bAllowMouseAiming, "Game.AllowMouseAiming", false )
		( tLXOptions->bUseNumericKeysToSwitchWeapons, "Game.UseNumericKeysToSwitchWeapons", true )
		( tLXOptions->bAntilagMovementPrediction, "Game.AntilagMovementPrediction", true )
		
		( tLXOptions->nMaxFPS, "Advanced.MaxFPS", 95 )
		( tLXOptions->iJpegQuality, "Advanced.JpegQuality", 80 )
		( tLXOptions->bCountTeamkills, "Advanced.CountTeamkills", false )
		( tLXOptions->bServerSideHealth, "Advanced.ServerSideHealth", false )
		( tLXOptions->bSendDirtUpdate, "Advanced.SendDirtUpdate", false )
		( tLXOptions->bAllowFileDownload, "Advanced.AllowFileDownload", false )
		( tLXOptions->iWeaponSelectionMaxTime, "Advanced.WeaponSelectionMaxTime", 40 )
		( tLXOptions->bSpectatorSmoothViewport, "Advanced.SpectatorSmoothViewport", true )
		( tLXOptions->bShowUnstableFeatures, "Advanced.ShowUnstableFeatures", false )

		( tLXOptions->bLogConvos, "Misc.LogConversations", true )
		( tLXOptions->bShowPing, "Misc.ShowPing", true )
		( tLXOptions->iScreenshotFormat, "Misc.ScreenshotFormat", FMT_PNG )
		
		( tLXOptions->bRepeatPlaylist, "MediaPlayer.Repeat", true )
		( tLXOptions->bShufflePlaylist, "MediaPlayer.Shuffle", false )
		( tLXOptions->iMPlayerLeft, "MediaPlayer.Left", 350 )
		( tLXOptions->iMPlayerTop, "MediaPlayer.Top", 240 )
		( tLXOptions->iMusicVolume, "MediaPlayer.MusicVolume", 50 )
		;

	unsigned i;
	for( i = 0; i < sizeof(ply_keys) / sizeof(ply_keys[0]) ; i ++ )
	{
		CScriptableVars::RegisterVars("GameOptions.Ply1Controls") ( tLXOptions->sPlayerControls[0][i], ply_keys[i], ply_def1[i].c_str() );
		CScriptableVars::RegisterVars("GameOptions.Ply2Controls") ( tLXOptions->sPlayerControls[1][i], ply_keys[i], ply_def2[i].c_str() );
	};
	for( i = 0; i < sizeof(gen_keys) / sizeof(gen_keys[0]) ; i ++ )
	{
		CScriptableVars::RegisterVars("GameOptions.GeneralControls") ( tLXOptions->sGeneralControls[i], gen_keys[i], gen_def[i].c_str() );
	};

	CScriptableVars::RegisterVars("GameOptions.LastGame")
		( tLXOptions->tGameinfo.iLives, "Lives", 10 )
		( tLXOptions->tGameinfo.iKillLimit, "KillLimit", -1 )
		( tLXOptions->tGameinfo.fTimeLimit, "TimeLimit", -1 )
		( tLXOptions->tGameinfo.iTagLimit, "TagLimit", 5 )
		( tLXOptions->tGameinfo.iLoadingTime, "LoadingTime", 100 )
		( tLXOptions->tGameinfo.bBonusesOn, "Bonuses", true )
		( tLXOptions->tGameinfo.bShowBonusName, "BonusNames", true )
		( tLXOptions->tGameinfo.iMaxPlayers, "MaxPlayers", 8 )
		( tLXOptions->tGameinfo.bMatchLogging, "MatchLogging", true )
		( tLXOptions->tGameinfo.sServerName, "ServerName", "LieroX Server" )
		( tLXOptions->tGameinfo.sWelcomeMessage, "WelcomeMessage", "Welcome to <server>, <player>" )
		( tLXOptions->tGameinfo.sMapFilename, "LevelName" )
		( tLXOptions->tGameinfo.nGameType, "GameType", GMT_DEATHMATCH )
		( tLXOptions->tGameinfo.szModName, "ModName", "Classic" )
		( tLXOptions->tGameinfo.szPassword, "Password" )
		( tLXOptions->tGameinfo.bRegServer, "RegisterServer", true )
		( tLXOptions->tGameinfo.sLastSelectedPlayer, "LastSelectedPlayer", "" )
		( tLXOptions->tGameinfo.bAllowWantsJoinMsg, "AllowWantsJoinMsg", true )
		( tLXOptions->tGameinfo.bWantsJoinBanned, "WantsToJoinFromBanned", true )
		( tLXOptions->tGameinfo.bAllowRemoteBots, "AllowRemoteBots", true )
		( tLXOptions->tGameinfo.bTopBarVisible, "TopBarVisible", true )
		( tLXOptions->tGameinfo.bAllowNickChange, "AllowNickChange", true )
		( tLXOptions->tGameinfo.fBonusFreq, "BonusFrequency", 30 )
		( tLXOptions->tGameinfo.fBonusLife, "BonusLife", 60 )
		( tLXOptions->tGameinfo.bAllowConnectDuringGame, "AllowConnectDuringGame", false )
		( tLXOptions->tGameinfo.iAllowConnectDuringGameLives, "AllowConnectDuringGameLives", 80 )
		( tLXOptions->tGameinfo.iAllowConnectDuringGameLivesMin, "AllowConnectDuringGameLivesMin", 3 )
		( tLXOptions->tGameinfo.fRespawnTime, "RespawnTime", 2.5 )
		( tLXOptions->tGameinfo.bRespawnInWaves, "RespawnInWaves", false )
		( tLXOptions->tGameinfo.bRespawnGroupTeams, "RespawnGroupTeams", false )
		( tLXOptions->tGameinfo.bGroupTeamScore, "GroupTeamScore", false )
		( tLXOptions->tGameinfo.bSuicideDecreasesScore, "SuicideDecreasesScore", false )
		( tLXOptions->tGameinfo.bEmptyWeaponsOnRespawn, "EmptyWeaponsOnRespawn", false )
		( tLXOptions->tGameinfo.fBonusHealthToWeaponChance, "BonusHealthToWeaponChance", 0.5f )
		;

	bool ret = tLXOptions->LoadFromDisc();

	/*printf( "Skinnable vars:\n%s", CGuiSkin::DumpVars().c_str() );
	printf( "Skinnable widgets:\n%s", CGuiSkin::DumpWidgets().c_str() );*/
	return ret;
}


///////////////////
// Load the options
bool GameOptions::LoadFromDisc()
{
	printf("Loading options... \n");

    unsigned int     i;

	static const std::string f = "cfg/options.cfg";

	AddKeyword("true",true);
	AddKeyword("false",false);

	// Load all variables that should be treated specially
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

	const int	 def_widths[] = {32,180,70,80,60,150};
	
	for (i=0;i<sizeof(iInternetList)/sizeof(int);i++)  {
		iInternetList[i] = def_widths[i];
		iLANList[i] = def_widths[i];
		iFavouritesList[i] = def_widths[i];
	}

	// Widget states
	ReadIntArray(f, "Widgets","InternetListCols",	&iInternetList[0],6);
	ReadIntArray(f, "Widgets","LANListCols",		&iLANList[0],6);
	ReadIntArray(f, "Widgets","FavouritesListCols",	&iFavouritesList[0],6);
	
	// Load variables registered with CGuiSkin
	for( std::map< std::string, CScriptableVars::ScriptVarPtr_t > :: iterator it = CScriptableVars::Vars().begin(); 
			it != CScriptableVars::Vars().end(); it++ )
	{
		if( it->first.find("GameOptions.") == 0 )
		{
			int dot1 = it->first.find("."), dot2 = it->first.find( ".", dot1 + 1 );
			std::string section = it->first.substr( dot1 + 1, dot2 - dot1 - 1 );	// Between two dots
			std::string key = it->first.substr( dot2 + 1 );	// After last dot
			if( it->second.type == CScriptableVars::SVT_BOOL )	// Some bools are actually ints in config file
			{
				std::string s = "";
				ReadString( f, section, key, s, "" );
				if( s.find_first_of("0123456789") == 0 )
				{
					int ii = 0;
					ReadInteger( f, section, key, &ii, it->second.bdef );
					*(it->second.b) = ( ii != 0 );
				}
				else ReadKeyword( f, section, key, it->second.b, it->second.bdef );
			}
			else if( it->second.type == CScriptableVars::SVT_INT )	// Some ints are actually bools in config file
			{
				std::string s = "";
				ReadString( f, section, key, s, "" );
				if( s.find_first_of("0123456789") == 0 )
					ReadInteger( f, section, key, it->second.i, it->second.idef );
				else
					ReadKeyword( f, section, key, it->second.i, it->second.idef );
			}
			else if( it->second.type == CScriptableVars::SVT_FLOAT )
				ReadFloat( f, section, key, it->second.f, it->second.fdef );
			else if( it->second.type == CScriptableVars::SVT_STRING )
				ReadString( f, section, key, *(it->second.s), it->second.sdef );
			else printf("Invalid var type %i of \"%s\" when loading config!\n", it->second.type, it->first.c_str() );
		};
	};

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
    int     i;

    FILE *fp = OpenGameFile("cfg/options.cfg", "wt");
    if(fp == NULL)
        return;

	// TODO: remove this UTF8 information here
	fprintf(fp, "%c%c%c", 0xEF, 0xBB, 0xBF);  // mark that this file is UTF-8 encoded
    fprintf(fp, "# OpenLieroX Options File\n");
    fprintf(fp, "# Note: This file is automatically generated\n\n");

	// Save all variables that should be treated specially
	fprintf(fp, "[FileHandling]\n");
	i=1;
	for(searchpathlist::const_iterator p = tSearchPaths.begin(); p != tSearchPaths.end(); p++, i++)
    	fprintf(fp, "SearchPath%i = %s\n", i, p->c_str());
	fprintf(fp,"\n");

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

	// Save variables registered with CGuiSkin
	std::string currentSection;
	for( std::map< std::string, CScriptableVars::ScriptVarPtr_t > :: iterator it = CScriptableVars::Vars().begin(); 
			it != CScriptableVars::Vars().end(); it++ )
	{
		if( it->first.find("GameOptions.") == 0 )
		{
			int dot1 = it->first.find("."), dot2 = it->first.find( ".", dot1 + 1 );
			std::string section = it->first.substr( dot1 + 1, dot2 - dot1 - 1 );	// Between two dots
			std::string key = it->first.substr( dot2 + 1 );	// After last dot
			if( currentSection != section )
			{
			    fprintf( fp, "\n[%s]\n", section.c_str() );
				currentSection = section;
			};
			if( it->second.type == CScriptableVars::SVT_BOOL )
			    fprintf( fp, "%s = %s\n", key.c_str(), *(it->second.b) ? "true" : "false" );
			else if( it->second.type == CScriptableVars::SVT_INT )
			    fprintf( fp, "%s = %d\n", key.c_str(), *(it->second.i) );
			else if( it->second.type == CScriptableVars::SVT_FLOAT )
			    fprintf( fp, "%s = %f\n", key.c_str(), *(it->second.f) );
			else if( it->second.type == CScriptableVars::SVT_STRING )
			    fprintf( fp, "%s = %s\n", key.c_str(), it->second.s->c_str() );
			else printf("Invalid var type %i of \"%s\" when saving config!\n", it->second.type, it->first.c_str() );
		};
	};

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
	ReadString (f, "NetworkTexts", "HasBeenKickedReason",	sHasBeenKickedReason,	"<player> has been kicked out because <reason>");
	ReadString (f, "NetworkTexts", "HasBeenBanned",   sHasBeenBanned,	"<player> has been banned");
	ReadString (f, "NetworkTexts", "HasBeenBannedReason",   sHasBeenBannedReason,	"<player> has been banned because <reason>");
	ReadString (f, "NetworkTexts", "HasBeenMuted",    sHasBeenMuted,	"<player> has been muted");
	ReadString (f, "NetworkTexts", "HasBeenUnmuted",  sHasBeenUnmuted,	"<player> has been unmuted");
	ReadString (f, "NetworkTexts", "KickedYou",		  sKickedYou,		"You have been kicked");
	ReadString (f, "NetworkTexts", "KickedYouReason", sKickedYouReason, "You have been kicked because <reason>");
	ReadString (f, "NetworkTexts", "BannedYou",		  sBannedYou,		"You have been banned");
	ReadString (f, "NetworkTexts", "BannedYouReason", sBannedYouReason, "You have been banned because <reason>");
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

