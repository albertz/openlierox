/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Command/variable parsing
// Created 9/4/02
// Jason Boettcher



#include "LieroX.h"
#include "Debug.h"
#include "CServer.h"
#include "CClient.h"
#include "console.h"
#include "StringUtils.h"
#include "sex.h"
#include "CWorm.h"
#include "AuxLib.h"
#include "Version.h"
#include "CClientNetEngine.h"
#include "DeprecatedGUI/Menu.h"
#include "IRC.h"
#include "ProfileSystem.h"
#include "FindFile.h"
#include "DedicatedControl.h"
#include "CGameMode.h"
#include "Cache.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "CChannel.h"
#include "IpToCountryDB.h"


#define		MAX_ARGS		32
#define		MAX_ARGLENGTH	128


// Command structure
struct command_t {
	std::string		strName;
	void			(*func) ( void );
	bool			bHidden;

	command_t	*Next;
};


// Arguments
int		Cmd_GetNumArgs();
void	Cmd_AddArg(const std::string& text);
std::string Cmd_GetArg(int a);



// Command routines
command_t	*Cmd_GetCommand(const std::string& strName);
int		Cmd_AddCommand(const std::string& strName, void (*func) ( void ), bool hide = false);




// User commands
void    Cmd_Kick();
void	Cmd_Ban();
void	Cmd_KickId();
void	Cmd_BanId();
void    Cmd_Mute();
void	Cmd_MuteId();
void	Cmd_Unmute();
void	Cmd_UnmuteId();
void	Cmd_Crash();
void	Cmd_CoreDump();
void	Cmd_Suicide();
void	Cmd_Unstuck();
void	Cmd_WantsJoin();
void	Cmd_RenameServer();
void	Cmd_Help();
void	Cmd_About();
void	Cmd_BadWord();
void	Cmd_Quit();
void	Cmd_Volume();
void	Cmd_Sound();
void	Cmd_ServerSideHealth();
void	Cmd_Connect();


command_t	*Commands = NULL;

std::string	Arguments[MAX_ARGS];
int		NumArgs;


///////////////////
// Add an argument to the list
void Cmd_AddArg(const std::string& text)
{
	if(NumArgs >= MAX_ARGS)
		warnings << "too much arguments, ignoring: " << text << endl;
	else
		Arguments[NumArgs++] = text;
}


///////////////////
// Get the number of arguments
int Cmd_GetNumArgs()
{
	return NumArgs;
}


///////////////////
// Get an argument by index
std::string Cmd_GetArg(int a)
{
	if(a>=0 && a<NumArgs)
		return Arguments[a];

	return "";
}


std::vector<std::string> ParseParams(const std::string& params) {
	bool quote = false;
	std::string	token;
	std::vector<std::string> res;
	
	std::string::const_iterator i = params.begin();
	for(; i != params.end(); i++) {
		
		// Check delimeters
		if(*i == ' ' || *i == ',') {
			if(!token.empty())
				res.push_back(token);
			token = "";
			
			continue;
		}
		
		// Check comments
		std::string::const_iterator i2 = i; i2++;
		if (i2 != params.end())  {
			if(*i == '/' && *i2 == '/') {
				if(!token.empty())
					res.push_back(token);
				token = "";
				
				// Just end here
				break;
			}
		}
		
		// Check quotes
		if(*i == '"') {
			quote = true;
			
			// Read until another quote
			for(i++; i != params.end(); i++) {
				
				if(*i == '"') {
					quote = false;
					break;
				}
				
				token += *i;
			}
			if(quote) break; // if we are still in the quote, break (else we would make an addition i++ => crash)
			continue;
		}
		
		// Normal text
		token += *i;
	}
	
	// Add the last token, only if it's not in unfinished quotes
	if(!token.empty() && !quote) {
		res.push_back(token);
	}
		
	return res;
}

///////////////////
// Parse a line of text
bool Cmd_ParseLine(const std::string& text)
{
	// Clear the arguments
	NumArgs = 0;

	std::vector<std::string> params = ParseParams(text);
	for(std::vector<std::string>::iterator i = params.begin(); i != params.end(); ++i) {
		Cmd_AddArg(*i);
	}
	
	if(!NumArgs)
		return false;


	// Translate the first token

	// Check if it's a variable
	/*cvar_t *var = CV_Find(Cmd_GetArg(0));
	if(var) {
		CV_Translate(var);
		return;
	}*/


	// Check if it's a command
	command_t *cmd = Cmd_GetCommand(Cmd_GetArg(0));
	if(cmd) {
		// Run the command
		if(cmd->func)
			cmd->func();
		return true;
	}

	std::string tmp = Cmd_GetArg(0);
	Con_AddText(CNC_NOTIFY, "Unknown command '" + tmp + "'");
	return false;
}


///////////////////
// Find a command with the same name
command_t *Cmd_GetCommand(const std::string& strName)
{
	command_t *cmd;

	for(cmd=Commands ; cmd ; cmd=cmd->Next)
		if(stringcasecmp(strName, cmd->strName) == 0)
			return cmd;

	return NULL;
}


///////////////////
// Auto complete a command
int Cmd_AutoComplete(std::string& strVar)
{
	size_t len = strVar.size();
	command_t *cmd;

	if(!len)
		return false;

	// See if it's an exact match
	cmd = Cmd_GetCommand(strVar);
	if(cmd) {
		strVar = cmd->strName + " ";
		return true;
	}

	// See if it's a partial match
	for(cmd=Commands ; cmd ; cmd=cmd->Next)  {
		if (cmd->strName.size() >= len)
			if(!stringcasecmp(strVar, cmd->strName.substr(0,len))) {
				strVar = cmd->strName + " ";
				return true;
			}
	}


	return false;
}


///////////////////
// Add a command to the list
int Cmd_AddCommand(const std::string& strName, void (*func) ( void ), bool hide)
{
	// Make sure the command isn't a variable
	/*if(CV_Find(strName)) {
		Con_AddText(CNC_WARNING,"%s already used as a variable",strName);
		return false;
	}*/


	// Make sure the command isn't already used
	if(Cmd_GetCommand(strName)) {
		Con_AddText(CNC_WARNING, strName + " already defined as a command");
		return false;
	}


	// Allocate room for the new var
	command_t *cmd;

	cmd = new command_t;
	cmd->strName = strName;
	cmd->func = func;
	cmd->bHidden = hide;

	// link the command in
	cmd->Next = Commands;
	Commands = cmd;

	return true;
}


///////////////////
// Free the commands
void Cmd_Free()
{
	command_t *cmd;
	command_t *cn;

	for(cmd=Commands ; cmd ; cmd=cn) {
		cn = cmd->Next;

		if(cmd)
			delete cmd;
	}

	Commands = NULL;
}

///////////////////
// Converts ID as string to an integer, returns -1 on fail
static int atoid(const std::string& str)
{
	// Ignore any non-numerical characters before the actual number, in most cases this is #
	std::string::const_iterator it = str.begin();
	while (it != str.end())  {
		if (*it >= 48 && *it <= 57)
			break;

		it++;
	}

	// Convert the string to a number
	std::string id = std::string(it, str.end());
	if (id.size() == 0)
		return -1;

	bool fail;
	int res = from_string<int>(id, fail);

	if (fail)
		return -1;
	else
		return res;
}



/*
======================================

              Commands

======================================
*/


///////////////////
// Tell the server to kick someone
void Cmd_Kick()
{
	if(tLX->iGameType == GME_JOIN)  {
		Con_AddText(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_AddText(CNC_NORMAL, "Usage:");
        Con_AddText(CNC_NORMAL, "kick <worm_name>");
        return;
    }

    if(cServer)
        cServer->kickWorm(Cmd_GetArg(1));
}

///////////////////
// Tell the server to kick and ban someone
void Cmd_Ban()
{
	if(tLX->iGameType != GME_HOST)  {
		Con_AddText(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_AddText(CNC_NORMAL, "Usage:");
        Con_AddText(CNC_NORMAL, "ban <worm_name>");
        return;
    }

    if(cServer)
       cServer->banWorm(Cmd_GetArg(1));
}

///////////////////
// Tell the server to mute someone
void Cmd_Mute()
{
	if(tLX->iGameType != GME_HOST)  {
		Con_AddText(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_AddText(CNC_NORMAL, "Usage:");
        Con_AddText(CNC_NORMAL, "mute <worm_name>");
        return;
    }

    if(cServer)
       cServer->muteWorm(Cmd_GetArg(1));
}

///////////////////
// Tell the server to unmute someone
void Cmd_Unmute()
{
	if(tLX->iGameType != GME_HOST)  {
		Con_AddText(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_AddText(CNC_NORMAL, "Usage:");
        Con_AddText(CNC_NORMAL, "unmute <worm_name>");
        return;
    }

    if(cServer)
       cServer->unmuteWorm(Cmd_GetArg(1));
}

///////////////////
// Tell the server to kick someone by ID
void Cmd_KickId()
{
	if(tLX->iGameType == GME_JOIN)  {
		Con_AddText(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_AddText(CNC_NORMAL, "Usage:");
        Con_AddText(CNC_NORMAL, "kickid <worm_id>");
        return;
    }

	int ID = atoid(Cmd_GetArg(1));
	if (ID == -1)
		return;

    if(cServer)
       cServer->kickWorm(ID);
}

///////////////////
// Tell the server to kick and ban someone by ID
void Cmd_BanId()
{
	if(tLX->iGameType != GME_HOST)  {
		Con_AddText(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_AddText(CNC_NORMAL, "Usage:");
        Con_AddText(CNC_NORMAL, "banid <worm_id>");
        return;
    }

	int ID = atoid(Cmd_GetArg(1));
	if (ID == -1)
		return;

    if(cServer)
       cServer->banWorm(ID);
}

///////////////////
// Tell the server to mute someone by ID
void Cmd_MuteId()
{
	if(tLX->iGameType != GME_HOST)  {
		Con_AddText(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_AddText(CNC_NORMAL, "Usage:");
        Con_AddText(CNC_NORMAL, "muteid <worm_id>");
        return;
    }

	int ID = atoid(Cmd_GetArg(1));
	if (ID == -1)
		return;

    if(cServer)
       cServer->muteWorm(ID);
}

///////////////////
// Tell the server to unmute someone by ID
void Cmd_UnmuteId()
{
	if(tLX->iGameType != GME_HOST)  {
		Con_AddText(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_AddText(CNC_NORMAL, "Usage:");
        Con_AddText(CNC_NORMAL, "unmuteid <worm_id>");
        return;
    }

	int ID = atoid(Cmd_GetArg(1));
	if (ID == -1)
		return;

    if(cServer)
       cServer->unmuteWorm(ID);
}

///////////////////
// Crash
void Cmd_Crash()
{
	Con_AddText(CNC_NORMAL, "In a previous version, the game would crash now!");
	// HINT: please don't add any code, which could make the game unstable
	//		(I myself just tested this command without knowing and BANG,
	//		I got an access violation. Perhaps the hoster of an important
	//		clan war does it...)
#ifdef DEBUG
	Con_AddText(CNC_WARNING, "This debug version will crash too, though.");
	// HINT: the current simple CrashHandler does not have any problems with this, thus it can stay here for testing
	(*(int*)0x13) = 42;
	assert(false);
#endif
}


void Cmd_CoreDump() {
	Con_AddText(CNC_NORMAL, "Dumping core ...");
	doVideoFrameInMainThread();
	struct Dumper : Action {
		int handle() {
			OlxWriteCoreDump("cmd");
			Con_AddText(CNC_NORMAL, "Dumping core finished");
			return 0;
		}
	};
	doActionInMainThread(new Dumper());
}


///////////////////
// Suicide
void Cmd_Suicide()
{
	if (cClient)  {
		if(bDedicated) {
			Con_AddText(CNC_NORMAL, "Cannot suicide in dedicated mode!");
			return;
		}

		if(cClient->getStatus() != NET_PLAYING)  {
			Con_AddText(CNC_NORMAL, "Cannot suicide when not playing!");
			return;
		}

		CWorm *w = cClient->getWorm(0);
		// Without arguments, just commit one suicide
		if (Cmd_GetNumArgs() == 1)  {
			if(w->isUsed())
				cClient->getNetEngine()->SendDeath(w->getID(), w->getID());
		}
		// A number has been entered, suicide the specified number
		else  {
			// Get the number
			bool fail;
			int number = from_string<int>(Cmd_GetArg(1), fail);
			if (fail)
				number = 1;

			if (number > cClient->getGameLobby()->iLives+1)  // Safety, not needed really (should be covered in next condition)
				number = cClient->getGameLobby()->iLives+1;
			if (number > w->getLives()+1)
				number = w->getLives()+1;
			if (number < 1)
				number = 1;

			// Suicide
			if (w->isUsed())
				for (int i = 0; i < number; i++)
					cClient->getNetEngine()->SendDeath(w->getID(), w->getID());
		}
	}
}

//////////////////
// Unstuck a stucked worm
void Cmd_Unstuck()
{

	if (cClient)  {
		if(bDedicated) {
			Con_AddText(CNC_NORMAL, "Cannot unstuck in dedicated mode!");
			return;
		}

		// Not playing
		if(cClient->getStatus() != NET_PLAYING)  {
			Con_AddText(CNC_NORMAL, "Cannot unstuck when not playing!");
			return;
		}

		// Unstuck
		CWorm *w = cClient->getWorm(0);
		if (w->isUsed() && w->getAlive())
			w->setPos(cClient->FindNearestSpot(w));
	}
}

/////////////////////
// Enables or disables wants to join messages
void Cmd_WantsJoin()
{
	// Check arguments
	if (Cmd_GetNumArgs() == 1)  {
		Con_AddText(CNC_NORMAL, "Usage: wantsjoin <on/off>");
		return;
	}

	std::string arg = Cmd_GetArg(1);

	if (!stringcasecmp(arg,"on") || !stringcasecmp(arg,"true") || !stringcasecmp(arg,"1") || !stringcasecmp(arg,"yes"))  {
		tLXOptions->bAllowWantsJoinMsg = true;
		Con_AddText(CNC_NORMAL, "\"Wants to join\" messages have been enabled");
	}
	else  {
		tLXOptions->bAllowWantsJoinMsg = false;
		Con_AddText(CNC_NORMAL, "\"Wants to join\" messages have been disabled");
	}
}

void Cmd_RenameServer()
{
	// Check arguments
	if (Cmd_GetNumArgs() == 1)  {
		Con_AddText(CNC_NORMAL, "Usage: servername <new name>");
		return;
	}

	// Check if hosting
	if (tLX->iGameType != GME_HOST)  {
		Con_AddText(CNC_NORMAL, "This command is available only for host");
		return;
	}

	if (cServer)  {
		std::string name = Cmd_GetArg(1);
		for (int i=2; i<Cmd_GetNumArgs();i++)
			name += " "+Cmd_GetArg(i);
		cServer->setName(name);
	}
}

void Cmd_Help() {
	Con_AddText(CNC_NORMAL, "Available commands:");
	std::string cmd_help_buf;
	command_t* cmd;
	unsigned short count = 0;
	cmd_help_buf = "";

	for(cmd=Commands; cmd; cmd=cmd->Next) {
		if(!cmd->bHidden) {
			cmd_help_buf += cmd->strName;
			cmd_help_buf += " ";
			count++;
			if(count >= 5) {
				count = 0;
				Con_AddText(CNC_NORMAL, "  " + cmd_help_buf);
				cmd_help_buf = "";
			}
		}
	}
	if(count && cmd_help_buf != "") {
		Con_AddText(CNC_NORMAL, "  " + cmd_help_buf);
	}
}

void Cmd_About() {
	Con_AddText(CNC_NOTIFY, GetFullGameName());
}

void Cmd_BadWord() {
	Con_AddText(CNC_NOTIFY, sex(50));
}

void Cmd_Quit() {
	if(!DeprecatedGUI::tMenu || !DeprecatedGUI::tMenu->bMenuRunning) {
		if(tLX && tLX->iGameType == GME_JOIN) {
			if(cClient && cClient->getStatus() != NET_DISCONNECTED)
				cClient->Disconnect();		
		}

		SetQuitEngineFlag("Console Cmd_Quit");

	} else
		Con_AddText(CNC_NORMAL, "quit has no effect in menu");
}

///////////////////
// Set sound volume
void Cmd_Volume()  {
	if (Cmd_GetNumArgs() == 1)  {
		Con_AddText(CNC_NORMAL, "Usage: volume <0-100>");
	}

	std::string arg = Cmd_GetArg(1);
	if(arg != "")  {
		int vol = from_string<int>(arg);
		vol = MIN(vol,100);
		vol = MAX(vol,0);
		SetSoundVolume(vol);
	}
}

//////////////////
// Enable or disable sound
void Cmd_Sound()  {
	// Check arguments
	if (Cmd_GetNumArgs() == 1)  {
		Con_AddText(CNC_NORMAL, "Usage: sound <on/off>");
	}

	std::string arg = Cmd_GetArg(1);

	if (!stringcasecmp(arg,"on") || !stringcasecmp(arg,"true") || !stringcasecmp(arg,"1") || !stringcasecmp(arg,"yes"))  {
		StartSoundSystem();
		tLXOptions->bSoundOn = true;
	}
	else  {
		StopSoundSystem();
		tLXOptions->bSoundOn = false;
	}
}

/////////////////
// Turn on/off server-side health
void Cmd_ServerSideHealth()  {
/*
	// Check arguments
	if (Cmd_GetNumArgs() == 1)  {
		Con_AddText(CNC_NORMAL, "Usage: ssh <on/off>");
	} else {
		std::string arg = Cmd_GetArg(1);

		// Set the ssh
		tLXOptions->bServerSideHealth =  !stringcasecmp(arg,"on") || !stringcasecmp(arg,"true") || !stringcasecmp(arg,"1") || !stringcasecmp(arg,"yes");
	}

	Con_AddText(CNC_NORMAL, std::string("Server-side health is now ") + (tLXOptions->bServerSideHealth ? std::string("enabled.") : std::string("disabled.")));
*/

	Con_AddText(CNC_NORMAL, "Sorry, server side health has been removed for non-dedicated servers");
}

//////////////////
// Send message to IRC chat from inside game
void Cmd_SendIrcMessage()  {
	// Check arguments
	if (Cmd_GetNumArgs() == 1)  {
		Con_AddText(CNC_NORMAL, "Usage: irc your message");
	}

	std::string msg;
	for( int i = 1; i < Cmd_GetNumArgs(); i++ )
	{
		if( i > 1 )
			msg += ' ';
		msg += Cmd_GetArg(i);
	}
	
	if (GetGlobalIRC())
		GetGlobalIRC()->sendChat(msg);
}

void Cmd_Connect() {
	// Check arguments
	if (Cmd_GetNumArgs() <= 1)  {
		Con_AddText(CNC_NORMAL, "Usage: connect server[:port]");
	}

	if(cClient && cClient->getStatus() != NET_DISCONNECTED)
		cClient->Disconnect();

	DeprecatedGUI::Menu_Current_Shutdown();

	if(!DeprecatedGUI::tMenu || !DeprecatedGUI::tMenu->bMenuRunning) { // we are in game
		SetQuitEngineFlag("Cmd_Connect & in game");
	}
	
	std::string server = Cmd_GetArg(1);
	std::string player = tLXOptions->sLastSelectedPlayer;
	if(player == "" && GetProfiles()) player = GetProfiles()->sName;
	if(!JoinServer(server, server, player)) return;
	
	// goto the joining dialog
	DeprecatedGUI::Menu_SetSkipStart(true);
	DeprecatedGUI::Menu_NetInitialize(false);
	DeprecatedGUI::Menu_Net_JoinInitialize(server);
	
	// when we leave the server
	DeprecatedGUI::tMenu->iReturnTo = DeprecatedGUI::iNetMode;
}

void Cmd_Initialize() {

    // Add some console commands
    Cmd_AddCommand("kick", Cmd_Kick);
	Cmd_AddCommand("ban", Cmd_Ban);
	Cmd_AddCommand("mute", Cmd_Mute);
	Cmd_AddCommand("unmute", Cmd_Unmute);
    Cmd_AddCommand("kickid", Cmd_KickId);
	Cmd_AddCommand("banid", Cmd_BanId);
	Cmd_AddCommand("muteid", Cmd_MuteId);
	Cmd_AddCommand("unmuteid", Cmd_UnmuteId);
	Cmd_AddCommand("crash", Cmd_Crash, true);
	Cmd_AddCommand("coredump", Cmd_CoreDump, true);
	Cmd_AddCommand("suicide", Cmd_Suicide);
	Cmd_AddCommand("unstuck", Cmd_Unstuck);
	Cmd_AddCommand("wantsjoin", Cmd_WantsJoin);
	Cmd_AddCommand("servername", Cmd_RenameServer);
	Cmd_AddCommand("help", Cmd_Help);
	Cmd_AddCommand("version", Cmd_About);
	Cmd_AddCommand("about", Cmd_About);
	Cmd_AddCommand("fuck", Cmd_BadWord, true);
	Cmd_AddCommand("ass", Cmd_BadWord, true);
	Cmd_AddCommand("bitch", Cmd_BadWord, true);
	Cmd_AddCommand("sex", Cmd_BadWord, true);
	Cmd_AddCommand("quit", Cmd_Quit);
	Cmd_AddCommand("exit", Cmd_Quit);
	Cmd_AddCommand("volume", Cmd_Volume);
	Cmd_AddCommand("sound", Cmd_Sound);
	Cmd_AddCommand("ssh", Cmd_ServerSideHealth, true);
	Cmd_AddCommand("irc", Cmd_SendIrcMessage);
	Cmd_AddCommand("chat", Cmd_SendIrcMessage);
	Cmd_AddCommand("connect", Cmd_Connect);
}







// TODO: Move this
static CWorm* CheckWorm(CmdLineIntf* caller, int id, const std::string& request)
{
	if(id <0 || id >= MAX_WORMS) {
		caller->writeMsg(request + " : Faulty ID " + itoa(id));
		return NULL;
	}
	CWorm *w = cServer->getWorms() + id;
	if(!w->isUsed()) {
		caller->writeMsg(request + " : ID " + itoa(id) + " not in use");
		return NULL;
	}
	return w;
}

void Cmd_Quit(CmdLineIntf* caller) {
	*DeprecatedGUI::bGame = false; // this means if we were in menu => quit
	DeprecatedGUI::tMenu->bMenuRunning = false; // if we were in menu, quit menu

	tLX->bQuitGame = true; // quit main-main-loop
	SetQuitEngineFlag("DedicatedControl::Cmd_Quit()"); // quit main-game-loop
}

void Cmd_Message(CmdLineIntf* caller, const std::string& msg) {
	hints << "DedicatedControl: message: " << msg << endl;
}

void Cmd_Script(CmdLineIntf* caller, const std::string& script) {
	if(!DedicatedControl::Get()) {
	
	}
	
	if(IsAbsolutePath(script)) {
		caller->writeMsg("Error: absolute path names are not allowed for script command");
		return;
	}

	if(script.find("..") != std::string::npos) {
		caller->writeMsg("Error: invalid script filename: " + script);
		return;
	}

	DedicatedControl::Get()->ChangeScript(script);
}

// adds a worm to the game (By string - id is way to complicated)
void Cmd_AddBot(CmdLineIntf* caller, const std::string & params)
{
	if( cClient->getNumWorms() + 1 >= MAX_WORMS ) {
		caller->writeMsg("Too many worms!");
		return;
	}
	
	// try to find the requested worm
	if(params != "") {
		std::string localWorm = params;
		TrimSpaces(localWorm);
		StripQuotes(localWorm);
		
		profile_t *p = FindProfile(localWorm);
		if(p) {
			cClient->AddWorm(p);
			return;
		}
		
		caller->writeMsg("cannot find worm profile " + localWorm + ", using random instead"); 
	}
	
	cClient->AddRandomBot();
}

void Cmd_KickBot(CmdLineIntf* caller, const std::string& params) {
	std::string reason = params;
	if(reason == "") reason = "Dedicated command";
	int worm = cServer->getLastBot();
	if(worm < 0) {
		caller->writeMsg("there is no bot on the server");
		return;
	}
	cServer->kickWorm(worm, reason);
}

void Cmd_KillBots(CmdLineIntf* caller, const std::string & params) {
	for( int f=0; f<cClient->getNumWorms(); f++ )
		if( cClient->getWorm(f)->getType() == PRF_COMPUTER )
	{
		cServer->getWorms()[cClient->getWorm(f)->getID()].setLives(0);
		cClient->getNetEngine()->SendDeath(cClient->getWorm(f)->getID(), cClient->getWorm(f)->getID());
	}
}

// Kick and ban will both function using ID
// It's up to the control-program to supply the ID
// - if it sends a string atoi will fail at converting it to something sensible
void Cmd_KickWorm(CmdLineIntf* caller, const std::string & params)
{
	std::string reason = "";
	int id = -1;
	std::vector<std::string> sSplit = explode(params," ");

	if (sSplit.size() == 1)
		id = atoi(params);
	else if (sSplit.size() >= 2) {
		id = atoi(sSplit[0]);
		for(std::vector<std::string>::iterator it = sSplit.begin();it != sSplit.end(); it++) {
			if(it == sSplit.begin())
				continue;
			reason += *it;
			reason += " ";
		}
		TrimSpaces(reason);
	}
	else {
		caller->writeMsg("KickWorm: Wrong syntax");
		return;
	}

	if(!CheckWorm(caller, id, "KickWorm"))
		return;

	cServer->kickWorm(id,reason);
}

void Cmd_BanWorm(CmdLineIntf* caller, const std::string & params)
{
	std::string reason = "";
	int id = -1;
	std::vector<std::string> sSplit = explode(params," ");

	if (sSplit.size() == 1)
		id = atoi(params);
	else if (sSplit.size() >= 2) {
		id = atoi(sSplit[0]);
		for(std::vector<std::string>::iterator it = sSplit.begin();it != sSplit.end(); it++) {
			if(it == sSplit.begin())
				continue;
			reason += *it;
			if (it+1 != sSplit.end())
				reason += " ";
		}
	}
	else {
		caller->writeMsg("BanWorm: Wrong syntax");
		return;
	}
	if(!CheckWorm(caller, id, "BanWorm"))
		return;

	cServer->banWorm(id,reason);
}

// TODO: Add name muting, if wanted.
void Cmd_MuteWorm(CmdLineIntf* caller, const std::string & params)
{
	int id = -1;
	id = atoi(params);
	if(!CheckWorm(caller, id, "MuteWorm"))
		return;

	cServer->muteWorm(id);
}

void Cmd_SetWormTeam(CmdLineIntf* caller, const std::string & params)
{
	std::vector<std::string> param = ParseParams(params);
	if(param.size() != 2) {
		caller->writeMsg("SetWormTeam: should be 2 parameters, given are " + itoa(param.size()) + ": " + params);
		return;
	}
	
	int id = atoi(param[0]);
	int team = atoi(param[1]);

	CWorm *w = CheckWorm(caller, id,"SetWormTeam");
	if (!w) return;

	if( team < 0 || team > 3 ) {
		caller->writeMsg("SetWormTeam: invalid team number");
		return;
	}

	w->getLobby()->iTeam = team;
	w->setTeam(team);
	cServer->UpdateWorms();
	cServer->SendWormLobbyUpdate();
	cServer->RecheckGame();
}

void Cmd_SetWormSpeedFactor(CmdLineIntf* caller, const std::string& params) {
	std::vector<std::string> param = ParseParams(params);
	if(param.size() != 2) {
		caller->writeMsg("Cmd_SetWormSpeedFactor: should be 2 parameters, given are " + itoa(param.size()) + ": " + params);
		return;
	}
	
	int id = atoi(param[0]);
	float factor = atof(param[1]);
	if(!CheckWorm(caller, id, "Cmd_SetWormSpeedFactor")) return;
	
	cServer->SetWormSpeedFactor(id, factor);
}

void Cmd_SetWormDamageFactor(CmdLineIntf* caller, const std::string& params) {
	std::vector<std::string> param = ParseParams(params);
	if(param.size() != 2) {
		caller->writeMsg("Cmd_SetWormDamageFactor: should be 2 parameters, given are " + itoa(param.size()) + ": " + params);
		return;
	}
	
	int id = atoi(param[0]);
	float factor = atof(param[1]);
	if(!CheckWorm(caller, id, "Cmd_SetWormDamageFactor")) return;
	
	cServer->SetWormDamageFactor(id, factor);
}

void Cmd_SetWormCanUseNinja(CmdLineIntf* caller, const std::string& params) {
	std::vector<std::string> param = ParseParams(params);
	if(param.size() != 2) {
		caller->writeMsg("Cmd_SetWormCanUseNinja: should be 2 parameters, given are " + itoa(param.size()) + ": " + params);
		return;
	}
	
	int id = atoi(param[0]);
	bool canUse = from_string<bool>(param[1]);
	if(!CheckWorm(caller, id, "Cmd_SetWormCanUseNinja")) return;
	
	cServer->SetWormCanUseNinja(id, canUse);
}

void Cmd_SetWormCanAirJump(CmdLineIntf* caller, const std::string& params) {
	std::vector<std::string> param = ParseParams(params);
	if(param.size() != 2) {
		caller->writeMsg("Cmd_SetWormCanAirJump: should be 2 parameters, given are " + itoa(param.size()) + ": " + params);
		return;
	}
	
	int id = atoi(param[0]);
	bool canUse = from_string<bool>(param[1]);
	if(!CheckWorm(caller, id, "Cmd_SetWormCanAirJump")) return;
	
	cServer->SetWormCanAirJump(id, canUse);
}

void Cmd_AuthorizeWorm(CmdLineIntf* caller, const std::string& params) {
	if( params.find(" ") == std::string::npos ) {
		caller->writeMsg("SetVar: wrong params: " + params);
		return;
	}
	int id = -1;
	id = atoi(params);
	if(!CheckWorm(caller, id, "AuthorizeWorm"))
		return;

	cServer->authorizeWorm(id);
}

void Cmd_SetVar(CmdLineIntf* caller, const std::string& params) {
	size_t f = params.find(" ");
	if( f == std::string::npos ) {
		caller->writeMsg("SetVar: wrong params: " + params);
		return;
	}
	std::string var = params.substr( 0, f );
	std::string value = params.substr( f + 1 );
	TrimSpaces( var );
	TrimSpaces( value );
	StripQuotes( value );

	RegisteredVar* varptr = CScriptableVars::GetVar(var);
	if( varptr == NULL )
	{
		caller->writeMsg("SetVar: no var with name " + var);
		notes << "Available vars:\n" << CScriptableVars::DumpVars() << endl;
		notes << "\nFor Python ded control script:\n" << endl;
		for( CScriptableVars::const_iterator it = CScriptableVars::begin(); it != CScriptableVars::end(); it++ )
		{
			notes << "setvar( \"" << it->first << "\", ";
			if( it->second.var.type == SVT_BOOL )
				notes << (int) * it->second.var.b;
			else if( it->second.var.type == SVT_INT )
				notes << * it->second.var.i;
			else if( it->second.var.type == SVT_FLOAT )
				notes << * it->second.var.f;
			else if( it->second.var.type == SVT_STRING )
				notes << "\"" << * it->second.var.s << "\"";
			else if( it->second.var.type == SVT_COLOR )
				notes << ColToHex(*it->second.var.cl);
			else
				notes << "\"\"";
			notes << ")" << endl;
		}
		return;
	}

	if( varptr->var.type == SVT_CALLBACK ) {
		caller->writeMsg("SetVar: callbacks are not allowed");
		// If we want supoort for that, I would suggest a seperated command like "call ...".
		return;
	}
	
	CScriptableVars::SetVarByString(varptr->var, value);

	//notes << "DedicatedControl: SetVar " << var << " = " << value << endl;

	cServer->UpdateGameLobby();
}

void Cmd_GetVar(CmdLineIntf* caller, const std::string& params) {
	if( params.find(" ") != std::string::npos ) {
		caller->writeMsg("GetVar: wrong params: " + params);
		return;
	}
	std::string var = params;
	TrimSpaces( var );
	RegisteredVar* varptr = CScriptableVars::GetVar(var);
	if( varptr == NULL ) {
		caller->writeMsg("GetVar: no var with name " + var);
		return;
	}
	
	if( varptr->var.type == SVT_CALLBACK ) {
		caller->writeMsg("GetVar: callbacks are not allowed");
		// If we want supoort for that, I would suggest a seperated command like "call ...".
		return;
	}
	
	if( varptr->var.s == &tLXOptions->sServerPassword ) {
		caller->writeMsg("GetVar: this variable is restricted");
		// If you want to check if a worm is authorized, use another function for that.
		return;
	}
	
	caller->pushReturnArg(varptr->var.toString());
}

void Cmd_GetFullFileName(CmdLineIntf* caller, std::string param) {
	std::string fn = param;
	TrimSpaces(fn);
	StripQuotes(fn);
	
	caller->pushReturnArg(GetAbsolutePath(GetFullFileName(fn, NULL)));
}

void Cmd_GetWriteFullFileName(CmdLineIntf* caller, std::string param) {
	std::string fn = param;
	TrimSpaces(fn);
	StripQuotes(fn);
	
	caller->pushReturnArg(GetAbsolutePath(GetWriteFullFileName(fn)));
}

void Cmd_StartLobby(CmdLineIntf* caller, std::string param) {
	if(!DeprecatedGUI::tMenu->bMenuRunning) {
		caller->writeMsg("we cannot start the lobby in current state");
		caller->writeMsg("stop lobby/game if you want to restart it");
		return; // just ignore it and stay in current state
	}
	
	if( param != "" ) {
		int port = atoi(param);
		if( port )
			tLXOptions->iNetworkPort = port;
	}

	tLXOptions->tGameInfo.iMaxPlayers = CLAMP(tLXOptions->tGameInfo.iMaxPlayers, 2, (int)MAX_PLAYERS);

	tLX->iGameType = GME_HOST;

	cClient->Shutdown();
	cClient->Clear();

	// Start the server
	if(!cServer->StartServer()) {
		// Crappy
		caller->writeMsg("ERROR: Server wouldn't start");
		return;
	}

	// Lets connect me to the server
	if(!cClient->Initialize()) {
		// Crappy
		caller->writeMsg("ERROR: Client wouldn't initialize");
		return;
	}
	cClient->Connect("127.0.0.1:" + itoa(cServer->getPort()));

	if(tLXOptions->tGameInfo.sModDir == "")
		tLXOptions->tGameInfo.sModDir = "MW 1.0";
	if(!CGameScript::CheckFile(tLXOptions->tGameInfo.sModDir, tLXOptions->tGameInfo.sModName)) {
		caller->writeMsg("no mod for dedicated, " + tLXOptions->tGameInfo.sModDir + " not found");
		// TODO..
	}

	// Get the game type
	if(tLXOptions->tGameInfo.gameMode == NULL)
		tLXOptions->tGameInfo.gameMode = GameMode(GM_DEATHMATCH);

	if(tLXOptions->tGameInfo.sMapFile == "")
		tLXOptions->tGameInfo.sMapFile = "CastleStrike.lxl";
	tLXOptions->tGameInfo.sMapName = CMap::GetLevelName(tLXOptions->tGameInfo.sMapFile);

	if(DedicatedControl::Get())
		DedicatedControl::Get()->LobbyStarted_Signal();
}

void Cmd_StartGame(CmdLineIntf* caller) {
	if(cServer->getNumPlayers() <= 1 && !tLXOptions->tGameInfo.features[FT_AllowEmptyGames]) {
		caller->writeMsg("cannot start game, too few players");
		return;
	}

	// Start the game
	cClient->setSpectate(false); // don't spectate; if we have added some players like bots, use them
	if(!cServer->StartGame()) {
		caller->writeMsg("cannot start game, got some error");
		cCache.ClearExtraEntries(); // just to be sure
		return;
	}

	// Leave the frontend
	*DeprecatedGUI::bGame = true;
	DeprecatedGUI::tMenu->bMenuRunning = false;
	tLX->iGameType = GME_HOST;
}

void Cmd_Map(CmdLineIntf* caller, const std::string& params) {
	std::string filename = params;
	TrimSpaces(filename);
	StripQuotes(filename);
	
	if(filename == "") {
		caller->writeMsg("specify map filename");
		return;
	}
	
	if(filename.find(".") == std::string::npos)
		filename += ".lxl";
	
	// TODO: search through all levels and match name if we don't have a proper filename
	// Don't do this explicitly though, we should cache the list (and also use it
	// everywhere else where we need it).
	
	tLXOptions->tGameInfo.sMapFile = filename;
	cServer->UpdateGameLobby();
}

void Cmd_GotoLobby(CmdLineIntf* caller) {
	cServer->gotoLobby();
	*DeprecatedGUI::bGame = false;
	DeprecatedGUI::tMenu->bMenuRunning = true;
}

void Cmd_ChatMessage(CmdLineIntf* caller, const std::string& msg, int type = TXT_NOTICE) {
	cServer->SendGlobalText(msg, type);
}

void Cmd_PrivateMessage(CmdLineIntf* caller, const std::string& params, int type = TXT_NOTICE) {
	int id = -1;
	id = atoi(params);
	CWorm *w = CheckWorm(caller, id, "PrivateMessage");
	if( !w || !w->getClient() || !w->getClient()->getNetEngine() )
		return;

	std::string msg;
	if( params.find(" ") != std::string::npos )
		msg = params.substr( params.find(" ")+1 );

	w->getClient()->getNetEngine()->SendText(msg, type);
}

void Cmd_GetWormList(CmdLineIntf* caller, const std::string& params)
{
	CWorm *w = cServer->getWorms();
	for(int i=0; i < MAX_WORMS; i++, w++)
	{
		if(!w->isUsed())
			continue;

		caller->pushReturnArg(itoa(w->getID()));
	}
}

void Cmd_GetComputerWormList(CmdLineIntf* caller) {
	CWorm *w = cServer->getWorms();
	for(int i = 0; i < MAX_WORMS; i++, w++) {
		if(w->isUsed() && w->getType() == PRF_COMPUTER)
			caller->pushReturnArg(itoa(w->getID()));
	}
}

void Cmd_GetWormName(CmdLineIntf* caller, const std::string& params) {
	int id = atoi(params);
	CWorm* w = CheckWorm(caller, id, "GetWormName");
	if(!w) return;
	caller->pushReturnArg(w->getName());
}

void Cmd_GetWormTeam(CmdLineIntf* caller, const std::string& params) {
	int id = atoi(params);
	CWorm* w = CheckWorm(caller, id, "GetWormTeam");
	if(!w) return;
	caller->pushReturnArg(itoa(w->getTeam()));
}

void Cmd_GetWormIp(CmdLineIntf* caller, const std::string& params) {
	int id = -1;
	id = atoi(params);
	CWorm* w = CheckWorm(caller, id, "GetWormIp");

	// TODO: Perhaps we can cut out the second argument for the signal- but that would lead to the signal being much larger. Is it worth it?
	std::string str_addr;
	if(w && w->getClient() && w->getClient()->getChannel())
		NetAddrToString(w->getClient()->getChannel()->getAddress(), str_addr);
	if (str_addr != "")
		caller->pushReturnArg(str_addr);
	else
		caller->writeMsg("GetWormIp: str_addr == \"\"");
}

void Cmd_GetWormLocationInfo(CmdLineIntf* caller, const std::string& params) {
	int id = -1;
	id = atoi(params);
	CWorm* w = CheckWorm(caller, id,"GetWormCountryInfo");
	if (!w)
	{
		return;
	}

	std::string str_addr;
	IpInfo info;

	NetAddrToString(w->getClient()->getChannel()->getAddress(), str_addr);
	if (str_addr != "")
	{
		info = tIpToCountryDB->GetInfoAboutIP(str_addr);
		caller->pushReturnArg(info.Continent);
		caller->pushReturnArg(info.Country);
		caller->pushReturnArg(info.CountryShortcut);
	}
	else
	{
		caller->writeMsg("GetWormCountryInfo: str_addr == \"\"");
	}
}

void Cmd_GetWormPing(CmdLineIntf* caller, const std::string& params) {
	int id = -1;
	id = atoi(params);
	CWorm* w = CheckWorm(caller, id, "GetWormPing");
	if(!w || !w->getClient() || !w->getClient()->getChannel())
		return;

	caller->pushReturnArg(itoa(w->getClient()->getChannel()->getPing()));
}

void Cmd_GetWormSkin(CmdLineIntf* caller, const std::string& params) {
	int id = -1;
	id = atoi(params);
	CWorm* w = CheckWorm(caller, id, "GetWormSkin");
	if (!w)
	{
		caller->pushReturnArg(0);
		caller->pushReturnArg("Default.png");
		return;
	}

	caller->pushReturnArg(itoa(w->getSkin().getDefaultColor().get()));
	caller->pushReturnArg(w->getSkin().getFileName());
}

void Cmd_Connect(CmdLineIntf* caller, const std::string& params) {
	JoinServer(params, params, "");
}

void Cmd_DumpGameState(CmdLineIntf* caller, const std::string& params) {
	cServer->DumpGameState();
}

void Cmd_DumpSysState(CmdLineIntf* caller, const std::string& params) {
	hints << "System state:" << endl;
	cServer->DumpGameState();
	// TODO: client game state
	hints << "Free system memory: " << (GetFreeSysMemory() / 1024) << " KB" << endl;
	hints << "Cache size: " << (cCache.GetCacheSize() / 1024) << " KB" << endl;
}

void Cmd_SaveConfig(CmdLineIntf* caller) {
	tLXOptions->SaveToDisc();
}

static void HandleCommand(const CmdLineIntf::Command& command) {
	std::string cmd = command.cmd; TrimSpaces(cmd);
	std::string params;
	size_t f = cmd.find(' ');
	if(f != std::string::npos) {
		params = cmd.substr(f + 1);
		TrimSpaces(params);
		cmd = cmd.substr(0, f);
	}
	stringlwr(cmd);
	if(cmd == "") return;
	
	// TODO: merge these commands with ingame console commands (Commands.cpp)
	if(cmd == "nextsignal") {
		if(DedicatedControl::Get()) {
			if(!DedicatedControl::Get()->GetNextSignal(command.sender)) return;
		}
		else
			command.sender->writeMsg("nextsignal is only available in dedicated mode");
	}
	else if(cmd == "quit")
		Cmd_Quit(command.sender);
	else if(cmd == "saveconfig")
		Cmd_SaveConfig(command.sender);
	else if(cmd == "setvar")
		Cmd_SetVar(command.sender, params);
	else if(cmd == "getvar")
		Cmd_GetVar(command.sender, params);
	else if(cmd == "script")
		Cmd_Script(command.sender, params);
	else if(cmd == "msg")
		Cmd_Message(command.sender, params);
	else if(cmd == "chatmsg")
		Cmd_ChatMessage(command.sender, params);
	else if(cmd == "privatemsg")
		Cmd_PrivateMessage(command.sender, params);
	else if(cmd == "startlobby")
		Cmd_StartLobby(command.sender, params);
	else if(cmd == "startgame")
		Cmd_StartGame(command.sender);
	else if(cmd == "gotolobby")
		Cmd_GotoLobby(command.sender);
	else if(cmd == "map")
		Cmd_Map(command.sender, params);

	else if(cmd == "addbot")
		Cmd_AddBot(command.sender, params);
	else if(cmd == "kickbot")
		Cmd_KickBot(command.sender, params);
	else if(cmd == "killbots")
		Cmd_KillBots(command.sender, params);

	else if(cmd == "kickworm")
		Cmd_KickWorm(command.sender, params);
	else if(cmd == "banworm")
		Cmd_BanWorm(command.sender, params);
	else if(cmd == "muteworm")
		Cmd_MuteWorm(command.sender, params);

	else if(cmd == "setwormteam")
		Cmd_SetWormTeam(command.sender, params);
	else if(cmd == "setwormspeedfactor")
		Cmd_SetWormSpeedFactor(command.sender, params);
	else if(cmd == "setwormdamagefactor")
		Cmd_SetWormDamageFactor(command.sender, params);
	else if(cmd == "setwormcanuseninja")
		Cmd_SetWormCanUseNinja(command.sender, params);
	else if(cmd == "setwormcanairjump")
		Cmd_SetWormCanAirJump(command.sender, params);

	else if(cmd == "authorizeworm")
		Cmd_AuthorizeWorm(command.sender, params);

	else if(cmd =="getwormlist")
		Cmd_GetWormList(command.sender, params);
	else if(cmd == "getcomputerwormlist")
		Cmd_GetComputerWormList(command.sender);
	else if(cmd == "getwormname")
		Cmd_GetWormName(command.sender, params);
	else if(cmd == "getwormteam")
		Cmd_GetWormTeam(command.sender, params);
	else if(cmd == "getwormip")
		Cmd_GetWormIp(command.sender, params);
	else if(cmd == "getwormlocationinfo")
		Cmd_GetWormLocationInfo(command.sender, params);
	else if(cmd == "getwormping")
		Cmd_GetWormPing(command.sender, params);
	else if(cmd == "getwormskin")
		Cmd_GetWormSkin(command.sender, params);
	else if(cmd == "getfullfilename")
		Cmd_GetFullFileName(command.sender, params);
	else if(cmd == "getwritefullfilename")
		Cmd_GetWriteFullFileName(command.sender, params);

	else if(cmd == "connect")
		Cmd_Connect(command.sender, params);

	else if(cmd == "dumpgamestate")
		Cmd_DumpGameState(command.sender, params);		
	else if(cmd == "dumpsysstate")
		Cmd_DumpSysState(command.sender, params);		
	
	else if(Cmd_ParseLine(cmd + " " + params)) {}
	else {
		command.sender->writeMsg("unknown command: " + cmd + " " + params);
	}
	
	command.sender->finalizeReturn();
}


struct CmdQueue {
	
	SDL_mutex* pendingCommandsMutex;
	std::list<CmdLineIntf::Command> pendingCommands;
	
	CmdQueue() {
		pendingCommandsMutex = SDL_CreateMutex();
	}
	
	~CmdQueue() {
		SDL_DestroyMutex(pendingCommandsMutex);
		pendingCommandsMutex = NULL;
	}

};

static CmdQueue cmdQueue;


void Execute(const CmdLineIntf::Command& command) {
	ScopedLock lock(cmdQueue.pendingCommandsMutex);
	cmdQueue.pendingCommands.push_back(command);
}

void HandlePendingCommands() {
	std::list<CmdLineIntf::Command> cmds;
	{
		ScopedLock lock(cmdQueue.pendingCommandsMutex);
		cmds.swap(cmdQueue.pendingCommands);
	}
	
	while( cmds.size() > 0 ) {
		CmdLineIntf::Command command = cmds.front();
		cmds.pop_front();

		HandleCommand(command);
		command.sender->finishedCommand(command.cmd);
	}
}
