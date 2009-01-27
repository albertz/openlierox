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

#include <ctype.h> // isspace


#include "LieroX.h"
#include "Debug.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "Options.h"
#include "FindFile.h"
#include "ConfigHandler.h"
#include "CScriptableVars.h"
#include "IniReader.h"
#include "Version.h"
#include "Iterator.h"




GameOptions	*tLXOptions = NULL;
NetworkTexts	*networkTexts = NULL;

static const std::string OptionsFileName = "cfg/options.cfg";

const std::string    ply_keys[] = {"Up", "Down", "Left", "Right", "Shoot", "Jump", "SelectWeapon", "Rope", "Strafe", "Weapon1", "Weapon2", "Weapon3", "Weapon4", "Weapon5" };
const std::string    ply_def1[] =
#ifdef MACOSX
	{"up", "down", "left", "right", "lalt", "lmeta", "space", "x", "c", "1", "2", "3", "4", "5" };
#else
	{"up", "down", "left", "right", "lctrl", "lalt", "lshift", "x", "z", "1", "2", "3", "4", "5" };
#endif
const std::string    ply_def2[] = {"kp 8",  "kp 5",    "kp 4",    "kp 6",     "kp +", "kp enter", "kp 0", "kp -", "kp .", "6", "7", "8", "9", "0" };
const std::string    gen_keys[] = {"Chat", "ShowScore", "ShowHealth", "ShowSettings",  "TakeScreenshot",  "ViewportManager", "SwitchMode", "ToggleTopBar", "TeamChat",	"MediaPlayer"};
const std::string    gen_def[]  = {"i",    "tab",		"h",		  "space",	       "F12",			  "F2",				 "F5",		   "F8",		   "o",			"F3"};

const char * GameInfoGroupDescriptions[GIG_Size][2] = 
{
	{"General", "General game options"},
	{"Advanced", "Advanced game options"},
	{"Scores", "Scoreboard related game options"},
	{"Weapons", "Weapons and game physics related game options"},
	{"Bonuses", "Bonuses related game options"},
	{"Other", "Other game options"},
};

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

	// TODO: don't hardcode the size here
	tLXOptions->sPlayerControls.resize(2);	// Don't change array size or we'll get segfault when vector memory allocation changes

	CScriptableVars::RegisterVars("GameOptions")
		( tLXOptions->bFullscreen, "Video.Fullscreen",
#ifdef WIN32
			true )
#else
			false )
#endif
		( tLXOptions->bShowFPS, "Video.ShowFPS", false )
		( tLXOptions->bOpenGL, "Video.OpenGL",
#ifdef MACOSX
			true )
#else
			false )
#endif
		( tLXOptions->iColourDepth, "Video.ColourDepth",
#ifndef WIN32
			32 )
#else
			16 )
#endif
		( tLXOptions->sResolution, "Video.Resolution", "" )
		( tLXOptions->sVideoPostProcessor, "Video.PostProcessor", "" )

		( tLXOptions->iNetworkPort, "Network.Port", LX_PORT )
		( tLXOptions->iNetworkSpeed, "Network.Speed", NST_LAN )
		( tLXOptions->bUseIpToCountry, "Network.UseIpToCountry", true )
		( tLXOptions->iMaxUploadBandwidth, "Network.MaxUploadBandwidth", 20000 )
		( tLXOptions->sHttpProxy, "Network.HttpProxy", "" )
		( tLXOptions->bAutoSetupHttpProxy, "Network.AutoSetupHttpProxy", true )

		( tLXOptions->bEnableChat, "Network.EnableChat", false ) // That's a shame that such marvellous feature is disabled by default :P
		( tLXOptions->sServerName, "Network.ServerName", "OpenLieroX Server" )
		( tLXOptions->sWelcomeMessage, "Network.WelcomeMessage", "Welcome to <server>, <player>" )
		( tLXOptions->sServerPassword, "Network.Password" )
		( tLXOptions->bRegServer, "Network.RegisterServer", true )
		( tLXOptions->bAllowWantsJoinMsg, "Network.AllowWantsJoinMsg", true )
		( tLXOptions->bWantsJoinBanned, "Network.WantsToJoinFromBanned", true )
		( tLXOptions->bAllowRemoteBots, "Network.AllowRemoteBots", true )

		( tLXOptions->bSoundOn, "Audio.Enabled", true )
		( tLXOptions->iSoundVolume, "Audio.Volume", 70 )
		( tLXOptions->iMusicVolume, "Audio.MusicVolume", 70 )

		( tLXOptions->iBloodAmount, "Game.Blood", 100 )
		( tLXOptions->bShadows, "Game.Shadows", true )
		( tLXOptions->bParticles, "Game.Particles", true )
		( tLXOptions->bOldSkoolRope, "Game.OldSkoolRope", false )
		( tLXOptions->bShowHealth, "Game.ShowWormHealth", false )
		( tLXOptions->bColorizeNicks, "Game.ColorizeNicks", false )
		( tLXOptions->bAutoTyping, "Game.AutoTyping", false )
		( tLXOptions->sSkinPath, "Game.SkinPath", "" )
		( tLXOptions->bNewSkinnedGUI, "Game.NewSkinnedGUI", false )
		( tLXOptions->sTheme, "Game.Theme", "" )
		( tLXOptions->bAntiAliasing, "Game.Antialiasing", false )
		( tLXOptions->bMouseAiming, "Game.MouseAiming", false ) // TODO: rename to mouse control?
		( tLXOptions->iMouseSensity, "Game.MouseSensity", 200 )
		( tLXOptions->bAntilagMovementPrediction, "Game.AntilagMovementPrediction", true )
		( tLXOptions->sLastSelectedPlayer, "Game.LastSelectedPlayer", "" )
		( tLXOptions->bTopBarVisible, "Game.TopBarVisible", true )
		( tLXOptions->bScreenShaking, "Game.ScreenShaking", true )
		( tLXOptions->bDamagePopups, "Game.DamagePopups", true )
		( tLXOptions->bColorizeDamageByWorm, "Game.ColorizeDamageByWorm", false )

		( tLXOptions->nMaxFPS, "Advanced.MaxFPS", 95 )
		( tLXOptions->iJpegQuality, "Advanced.JpegQuality", 80 )
		( tLXOptions->iMaxCachedEntries, "Advanced.MaxCachedEntries", 300 ) // Should be enough for every mod (we have 2777 .png and .wav files total now) and does not matter anyway with SmartPointer
		( tLXOptions->bMatchLogging, "Advanced.MatchLogging", true )

		( tLXOptions->bLogConvos, "Misc.LogConversations", true )
		( tLXOptions->bShowPing, "Misc.ShowPing", true )
		( tLXOptions->bShowNetRates, "Misc.ShowNetRate", false )
		( tLXOptions->iScreenshotFormat, "Misc.ScreenshotFormat", FMT_PNG )
		( tLXOptions->sDedicatedScript, "Misc.DedicatedScript", "scripts/dedicated_control" )
		( tLXOptions->iVerbosity, "Misc.Verbosity", 0 )	
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

	CScriptableVars::RegisterVars("GameOptions.GameInfo")
		( tLXOptions->tGameInfo.iLives, "Lives", 10, "Lives", "Lives", GIG_General, true )
		( tLXOptions->tGameInfo.iKillLimit, "KillLimit", -1, "Max kills", "Game ends when a player reaches the specified number of kills", GIG_General, true )
		( tLXOptions->tGameInfo.fTimeLimit, "TimeLimit", -1, "Time limit", "Time limit, in minutes", GIG_General, true )
		( tLXOptions->tGameInfo.iTagLimit, "TagLimit", 5, "Tag limit", "Tag limit, for Tag game mode", GIG_General, true )
		( tLXOptions->tGameInfo.iLoadingTime, "LoadingTime", 100, "Loading time", "Loading time, in percent", GIG_General, true, 0, 500 )
		( tLXOptions->tGameInfo.bBonusesOn, "Bonuses", true, "Bonuses", "Bonuses enabled", GIG_Bonus )
		( tLXOptions->tGameInfo.bShowBonusName, "BonusNames", true, "Show Bonus names", "Show bonus name above its image", GIG_Bonus )
		( tLXOptions->tGameInfo.iMaxPlayers, "MaxPlayers", 8, "Max players", "Max amount of players allowed on server", GIG_General, true, 1, 32 )
		( tLXOptions->tGameInfo.sMapFile, "LevelName" ) // WARNING: confusing, it is handled like the filename
		( tLXOptions->tGameInfo.iGameMode, "GameType", GMT_DEATHMATCH )
		( tLXOptions->tGameInfo.sModDir, "ModName", "Classic" ) // WARNING: confusing, it is handled like the dirname
		( tLXOptions->tGameInfo.fBonusFreq, "BonusFrequency", 30, "Bonus spawn time", "How often a new bonus will be spawned (every N seconds)", GIG_Bonus )
		( tLXOptions->tGameInfo.fBonusLife, "BonusLife", 60, "Bonus life time", "Bonus life time, in seconds", GIG_Bonus )
		( tLXOptions->tGameInfo.fRespawnTime, "RespawnTime", 2.5, "Respawn time", "Player respawn time, in seconds", GIG_Advanced, true, 0.0, 20.0 )
		( tLXOptions->tGameInfo.bRespawnGroupTeams, "RespawnGroupTeams", false, "Group teams", "Respawn player closer to its team", GIG_Advanced )
		( tLXOptions->tGameInfo.bGroupTeamScore, "GroupTeamScore", false, "Group team score", "Kill count is the same for all worms in team", GIG_Score )
		( tLXOptions->tGameInfo.bEmptyWeaponsOnRespawn, "EmptyWeaponsOnRespawn", false, "Empty weapons when respawn", "Your weapon ammo is emptied when you respawn", GIG_Weapons )
		( tLXOptions->tGameInfo.fBonusHealthToWeaponChance, "BonusHealthToWeaponChance", 0.5f, "Bonus weapon chance", "Chance of spawning a weapon bonus instead of a health bonus", GIG_Bonus, true, 0.0f, 1.0f )
		( tLXOptions->tGameInfo.bForceRandomWeapons, "ForceRandomWeapons", false, "Force random weapons", "Force all players to select random weapons", GIG_Weapons )
		( tLXOptions->tGameInfo.bSameWeaponsAsHostWorm, "SameWeaponsAsHostWorm", false, "Same weapons as host worm", "Force all players to select the same weapons as host worm", GIG_Weapons )
		( tLXOptions->tGameInfo.bAllowConnectDuringGame, "AllowConnectDuringGame", false, "Connect during game", "Allow new players to connect during game", GIG_Advanced )
		( tLXOptions->tGameInfo.bAllowNickChange, "AllowNickChange", true, "Allow name change", "Allow players to change name with /setmyname command", GIG_Other )
		( tLXOptions->tGameInfo.bAllowStrafing, "AllowStrafing", true, "Allow strafing", "Allow players to use the Strafe key", GIG_Other )
		( tLXOptions->tGameInfo.bServerSideHealth, "ServerSideHealth", false, "Server sided health", "Health is calculated on server, to prevent cheating", GIG_Other )
		( tLXOptions->tGameInfo.iWeaponSelectionMaxTime, "WeaponSelectionMaxTime", 360, "Weapon selection max time", "Max time to allow players to select weapons, in seconds", GIG_Weapons )
		;

	foreach( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		CScriptableVars::RegisterVars("GameOptions.GameInfo")
		( tLXOptions->tGameInfo.features[f->get()], f->get()->name, f->get()->defaultValue, 
				f->get()->humanReadableName, f->get()->description, f->get()->group, f->get()->minValue, f->get()->maxValue );
	}
	
	bool ret = tLXOptions->LoadFromDisc();

	/*printf( "Skinnable vars:\n%s", CGuiSkin::DumpVars().c_str() );
	printf( "Skinnable widgets:\n%s", CGuiSkin::DumpWidgets().c_str() );*/
	return ret;
}

static void InitSearchPaths() {
	// have to set to find the config at some of the default places
	InitBaseSearchPaths();

	std::string value;
	int i = 1;
	while(true) {
		if(!ReadString(OptionsFileName, "FileHandling", "SearchPath" + itoa(i,10), value, ""))
			break;

		AddToFileList(&tSearchPaths, value);
		i++;
	}

	// add the basesearchpaths to the searchpathlist as they should be saved in the end
	for(searchpathlist::const_iterator p1 = basesearchpaths.begin(); p1 != basesearchpaths.end(); i++,p1++)  {
		AddToFileList(&tSearchPaths, *p1);
	}

	// print the searchpaths, this may be very usefull for the user
	notes << "I have now the following searchpaths (in this order):\n";
	for(searchpathlist::const_iterator p2 = tSearchPaths.begin(); p2 != tSearchPaths.end(); p2++) {
		std::string path = *p2;
		ReplaceFileVariables(path);
		notes << "  " << path << "\n";
	}
	notes << " And that's all." << endl;
}

static void InitWidgetStates(GameOptions& opts) {
	// this has to be done explicitly at the moment as scriptablevars doesn't support arrays
	// TODO: add this feature

	const int	 def_widths[] = {32,180,70,80,60,150};

	for (size_t i=0; i<sizeof(opts.iInternetList)/sizeof(int); i++)  {
		opts.iInternetList[i] = def_widths[i];
		opts.iLANList[i] = def_widths[i];
		opts.iFavouritesList[i] = def_widths[i];
	}

	// Widget states
	ReadIntArray(OptionsFileName, "Widgets","InternetListCols",	&opts.iInternetList[0],6);
	ReadIntArray(OptionsFileName, "Widgets","LANListCols",		&opts.iLANList[0],6);
	ReadIntArray(OptionsFileName, "Widgets","FavouritesListCols",	&opts.iFavouritesList[0],6);
}


///////////////////
// Load the options
bool GameOptions::LoadFromDisc()
{
	notes << "Loading options..." << endl;

	additionalOptions.clear();

	// TODO: is this still needed with the new parsing?
	AddKeyword("true",true);
	AddKeyword("false",false);

	// Load all variables that should be treated specially

	// File handling
	// read this first, because perhaps we will have new searchpaths
	InitSearchPaths();

	// TODO: these use arrays which are not handled by scriptablevars
	InitWidgetStates(*this);

	// first set the standards (else the vars would be undefined if not defined in options.cfg)
	for( CScriptableVars::iterator it = CScriptableVars::begin();
			it != CScriptableVars::end(); it++ )
	{
		if( it->first.find("GameOptions.") == 0 )
		{
			if( it->second.type == SVT_BOOL )
				*(it->second.b) = it->second.bdef;
			else if( it->second.type == SVT_INT )
				*(it->second.i) = it->second.idef;
			else if( it->second.type == SVT_FLOAT )
				*(it->second.f) = it->second.fdef;
			else if( it->second.type == SVT_STRING )
				*(it->second.s) = it->second.sdef;
			else warnings << "Invalid var type " << it->second.type << " of \"" << it->first << "\" when setting default!" << endl;
		}
	}

	notes << "Reading game options from " << GetFullFileName("cfg/options.cfg") << endl;
	notes << "Will write game options to " << GetWriteFullFileName("cfg/options.cfg", true) << endl;
	
	// define parser handler
	class MyIniReader : public IniReader {
	public:
		GameOptions* opts;
		MyIniReader(const std::string& fn, GameOptions* o) : IniReader(fn), opts(o) {}

		bool OnEntry(const std::string& section, const std::string& propname, const std::string& value) {
			ScriptVarPtr_t var = CScriptableVars::GetVar("GameOptions." + section + "." + propname);
			if( var.b !=  NULL ) { // found entry
				CScriptableVars::SetVarByString(var, value);
			} else {
				if( (section == "FileHandling" && propname.find("SearchPath") == 0)
				 || (section == "Widgets") ) {
					// ignore these atm
				} else {
					opts->additionalOptions[section + "." + propname] = value;
					notes << "the option \"" << section << "." << propname << "\" defined in options.cfg is unknown" << endl;
				}
			}

			return true;
		}
	}
	iniReader("cfg/options.cfg", this);

	// parse the file now
	if( ! iniReader.Parse() ) {
		hints << "cfg/options.cfg not found, will use standards" << endl;
	}


	initSpecialSearchPathForTheme();
	if(getSpecialSearchPathForTheme()) {
		notes << "Special searchpath for the theme: " << *getSpecialSearchPathForTheme() << endl;
	} else
		notes << "Default theme is used" << endl;


	if(additionalOptions.size() > 0) {
		hints << "Unknown options were found." << endl;
	}

	// Clamp the Jpeg quality
	if (iJpegQuality < 1)
		iJpegQuality = 1;
	if (iJpegQuality > 100)
		iJpegQuality = 100;

	notes << "DONE loading options" << endl;

	return true;
}

///////////////////
// Save & shutdown the options
void ShutdownOptions(void)
{
	CScriptableVars::DeRegisterVars("GameOptions");
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
    fprintf(fp, "# Note: This file is automatically generated by ");
	// just looks better with version-info and is a good information for the user
	// this should not be used in parsing to change any behaviour
	fprintf(fp, "%s", GetFullGameName().c_str());
	fprintf(fp, "\n\n");

	// Save all variables that should be treated specially
	fprintf(fp, "[FileHandling]\n");
	i=1;
	for(searchpathlist::const_iterator p = tSearchPaths.begin(); p != tSearchPaths.end(); p++, i++)
    	fprintf(fp, "SearchPath%i = %s\n", i, p->c_str());
	fprintf(fp,"\n");

	// TODO: same issue as in InitWidgetStates()
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

	// The following part has the disadvantage that section
	// could occur multiple times in the config file
	// (most often because of additionalOptions).
	// The current parsing of a INI-file allowes this but it
	// looks not so nice.
	// TODO: make it better (but in a clean way, else just leave it!)

	// Save variables registered with CGuiSkin
	std::string currentSection;
	for( CScriptableVars::iterator it = CScriptableVars::begin();
			it != CScriptableVars::end(); it++ )
	{
		if( it->first.find("GameOptions.") == 0 )
		{
			size_t dot1 = it->first.find("."), dot2 = it->first.find( ".", dot1 + 1 );
			std::string section = it->first.substr( dot1 + 1, dot2 - dot1 - 1 );	// Between two dots
			std::string key = it->first.substr( dot2 + 1 );	// After last dot
			if( currentSection != section )
			{
			    fprintf( fp, "\n[%s]\n", section.c_str() );
				currentSection = section;
			}
			if( it->second.type == SVT_BOOL )
			    fprintf( fp, "%s = %s\n", key.c_str(), *(it->second.b) ? "true" : "false" );
			else if( it->second.type == SVT_INT )
			    fprintf( fp, "%s = %d\n", key.c_str(), *(it->second.i) );
			else if( it->second.type == SVT_FLOAT )
			    fprintf( fp, "%s = %f\n", key.c_str(), *(it->second.f) );
			else if( it->second.type == SVT_STRING )
			    fprintf( fp, "%s = %s\n", key.c_str(), it->second.s->c_str() );
			else printf("Invalid var type %i of \"%s\" when saving config!\n", it->second.type, it->first.c_str() );
		}
	}

	// save additional options
	// HINT: as this is done seperatly, some sections may be double; though the current parsing and therefore all future parsings handle this correctly
	currentSection = "";
	for( std::map< std::string, std::string > :: iterator it = additionalOptions.begin(); it != additionalOptions.end(); it++ ) {
		size_t dot = it->first.find(".");
		std::string section = it->first.substr( 0, dot ); // before the dot
		std::string key = it->first.substr( dot + 1 ); // after the dot
		if( currentSection != section )
		{
			fprintf( fp, "\n[%s]\n", section.c_str() );
			currentSection = section;
		}
		fprintf( fp, "%s = %s\n", key.c_str(), it->second.c_str() );
	}

    fclose(fp);
}

bool NetworkTexts::Init() {
	if(networkTexts) {
		warnings << "networktexts are already inited; ignoring ..." << endl;
		return true;
	}

	networkTexts = new NetworkTexts;
	if(!networkTexts) {
		errors << "not enough mem for networktexts" << endl;
		return false;
	}

	return networkTexts->LoadFromDisc();
}

////////////////////
// Loads the texts used by server
bool NetworkTexts::LoadFromDisc()
{
	notes << "Loading network texts... ";

	// TODO: use the general INI-parser here
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
	ReadString (f, "NetworkTexts", "IsSpectating",    sIsSpectating,	"<player> will only spectate this round");
	ReadString (f, "NetworkTexts", "IsPlaying",		  sIsPlaying,		"<player> will play this round!");
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
	ReadString (f, "NetworkTexts", "KilledAFK",	      sKilledAFK,		"<killer> typekilled <victim>");

	ReadString (f, "NetworkTexts", "PlayerOut",		  sPlayerOut,		"<player> is out of the game");
	ReadString (f, "NetworkTexts", "TeamOut",		  sTeamOut,			"The <team> team is out of the game");
	ReadString (f, "NetworkTexts", "PlayerHasWon",	  sPlayerHasWon,	"<player> has won the match");
	ReadString (f, "NetworkTexts", "TeamHasWon",	  sTeamHasWon,		"The <team> team has won the match");
	ReadString (f, "NetworkTexts", "TimeLimit",		  sTimeLimit,		"Timelimit has been reached");

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

	notes << "DONE" << endl;
	return true;
}

