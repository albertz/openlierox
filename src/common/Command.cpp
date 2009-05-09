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


#include <limits.h>
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
			res.push_back(token);
			token = "";
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


class Command {
public:
	virtual ~Command() {}
	
	std::string name;
	std::string desc;
	std::string usage;
	unsigned int minParams, maxParams;

	std::string minMaxStr() const {
		std::string s = itoa(minParams) + "-";
		if(maxParams == UINT_MAX) s += "*";
		else s += itoa(maxParams);
		return s;
	}
	
	std::string usageStr() const {
		if(usage != "") return name + " " + usage;
		else return name;
	}
	
	void printUsage(CmdLineIntf* caller) {
		caller->writeMsg("usage: " + usageStr());
	}
	
	void exec(CmdLineIntf* caller, const std::string& params) {
		std::vector<std::string> ps = ParseParams(params);
		if(ps.size() < minParams || ps.size() > maxParams) {
			caller->writeMsg(minMaxStr() + " params needed, usage: " + usageStr());
			//caller->writeMsg("bad cmd: " + name + " " + params);
			return;
		}
		
		exec(caller, ps);
	}
	
protected:
	virtual void exec(CmdLineIntf* caller, const std::vector<std::string>& params) = 0;
	
};

typedef std::map<std::string, Command*, stringcaseless> CommandMap;
static CommandMap commands;

#ifndef TOSTRING
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#endif

#define COMMAND(_name, _desc, _us, _minp, _maxp) \
	class Cmd_##_name : public Command { \
	public: \
		Cmd_##_name() { \
			name = TOSTRING(_name); desc = _desc; usage = _us; minParams = _minp; maxParams = _maxp; \
			commands.insert( CommandMap::value_type(name, this) ); \
		} \
	protected: \
		void exec(CmdLineIntf* caller, const std::vector<std::string>& params); \
	} \
	__cmd_##_name;

COMMAND(quit, "quit game", "", 0, 0);
void Cmd_quit::exec(CmdLineIntf* caller, const std::vector<std::string>&) {
	*DeprecatedGUI::bGame = false; // this means if we were in menu => quit
	DeprecatedGUI::tMenu->bMenuRunning = false; // if we were in menu, quit menu

	tLX->bQuitGame = true; // quit main-main-loop
	SetQuitEngineFlag("DedicatedControl::Cmd_Quit()"); // quit main-game-loop
}

COMMAND(msg, "print message on stdout", "text", 1, 1);
void Cmd_msg::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	hints << "DedicatedControl: message: " << params[0] << endl;
}

COMMAND(script, "load extern dedicated script", "[script]", 0, 1);
void Cmd_script::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	std::string script = (params.size() > 0) ? params[0] : "";
	
	if(!DedicatedControl::Get()) {
		caller->writeMsg("Error: can only load extern script in dedicated mode");
		return;
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

COMMAND(addBot, "add bot to game", "[botprofile]", 0, 1);
// adds a worm to the game (By string - id is way to complicated)
void Cmd_addBot::exec(CmdLineIntf* caller, const std::vector<std::string>& params)
{
	if( cClient->getNumWorms() + 1 >= MAX_WORMS ) {
		caller->writeMsg("Too many worms!");
		return;
	}
	
	// try to find the requested worm
	if(params.size() > 0) {
		std::string localWorm = params[0];
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

COMMAND(kickBot, "kick bot from game", "[reason]", 0, 1);
void Cmd_kickBot::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	std::string reason = (params.size() > 0) ? params[0] : "Dedicated command";
	int worm = cServer->getLastBot();
	if(worm < 0) {
		caller->writeMsg("there is no bot on the server");
		return;
	}
	cServer->kickWorm(worm, reason);
}

COMMAND(killBots, "kill all bots out of game", "", 0, 0);
void Cmd_killBots::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	for( int f=0; f<cClient->getNumWorms(); f++ )
		if( cClient->getWorm(f)->getType() == PRF_COMPUTER )
	{
		cServer->getWorms()[cClient->getWorm(f)->getID()].setLives(0);
		cClient->getNetEngine()->SendDeath(cClient->getWorm(f)->getID(), cClient->getWorm(f)->getID());
	}
}

COMMAND(kickWorm, "kick worm", "id [reason]", 1, 2);
// Kick and ban will both function using ID
// It's up to the control-program to supply the ID
// - if it sends a string atoi will fail at converting it to something sensible
void Cmd_kickWorm::exec(CmdLineIntf* caller, const std::vector<std::string>& params)
{
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	std::string reason = "";
	if (params.size() >= 2)
		reason = params[1];

	if(!CheckWorm(caller, id, "kickWorm"))
		return;

	cServer->kickWorm(id,reason);
}

COMMAND(banWorm, "ban worm", "id [reason]", 1, 2);
void Cmd_banWorm::exec(CmdLineIntf* caller, const std::vector<std::string>& params)
{
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	std::string reason = "";
	if (params.size() >= 2)
		reason = params[1];
	
	if(!CheckWorm(caller, id, "banWorm"))
		return;

	cServer->banWorm(id,reason);
}

COMMAND(muteWorm, "mute worm", "id", 1, 1);
// TODO: Add name muting, if wanted.
void Cmd_muteWorm::exec(CmdLineIntf* caller, const std::vector<std::string>& params)
{
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	if(!CheckWorm(caller, id, "muteWorm"))
		return;

	cServer->muteWorm(id);
}

COMMAND(setWormTeam, "set worm team", "id team", 2, 2);
void Cmd_setWormTeam::exec(CmdLineIntf* caller, const std::vector<std::string>& params)
{
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	CWorm *w = CheckWorm(caller, id,"setWormTeam");
	if (!w) return;
	
	int team = from_string<int>(params[1], fail);
	if(fail) {
		printUsage(caller);
		return;
	}

	if( team < 0 || team > 3 ) {
		caller->writeMsg("setWormTeam: invalid team number");
		return;
	}

	w->getLobby()->iTeam = team;
	w->setTeam(team);
	cServer->UpdateWorms();
	cServer->SendWormLobbyUpdate();
	cServer->RecheckGame();
}

COMMAND(setWormSpeedFactor, "set worm speedfactor", "id factor", 2, 2);
void Cmd_setWormSpeedFactor::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	if(!CheckWorm(caller, id, "setWormSpeedFactor")) return;
	
	float factor = from_string<float>(params[1], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	cServer->SetWormSpeedFactor(id, factor);
}

COMMAND(setWormDamageFactor, "set worm damagefactor", "id factor", 2, 2);
void Cmd_setWormDamageFactor::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	if(!CheckWorm(caller, id, "setWormDamageFactor")) return;
	
	float factor = from_string<float>(params[1], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	cServer->SetWormDamageFactor(id, factor);
}

COMMAND(setWormCanUseNinja, "(dis)allow worm to use ninja", "id true/false", 2, 2);
void Cmd_setWormCanUseNinja::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	if(!CheckWorm(caller, id, "setWormCanUseNinja")) return;
	
	bool canUse = from_string<bool>(params[1], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	cServer->SetWormCanUseNinja(id, canUse);
}

COMMAND(setWormCanAirJump, "enable/disable air jump for worm", "id true/false", 2, 2);
void Cmd_setWormCanAirJump::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	if(!CheckWorm(caller, id, name)) return;
	
	bool canUse = from_string<bool>(params[1], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	cServer->SetWormCanAirJump(id, canUse);
}

COMMAND(authorizeWorm, "authorize worm", "id", 1, 1);
void Cmd_authorizeWorm::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	if(!CheckWorm(caller, id, name)) return;

	cServer->authorizeWorm(id);
}

COMMAND(setVar, "set variable", "variable value", 2, 2);
void Cmd_setVar::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	std::string var = params[0];
	std::string value = params[1];

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

COMMAND(getVar, "read variable", "variable", 1, 1);
void Cmd_getVar::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	std::string var = params[0];
	
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

COMMAND(getFullFileName, "get full filename", "relativefilename", 1, 1);
void Cmd_getFullFileName::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	caller->pushReturnArg(GetAbsolutePath(GetFullFileName(params[0], NULL)));
}

COMMAND(getWriteFullFileName, "get writeable full filename", "relativefilename", 1, 1);
void Cmd_getWriteFullFileName::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	caller->pushReturnArg(GetAbsolutePath(GetWriteFullFileName(params[0])));
}

COMMAND(startLobby, "start server lobby", "[serverport]", 0, 1);
void Cmd_startLobby::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	if(!DeprecatedGUI::tMenu->bMenuRunning) {
		caller->writeMsg("we cannot start the lobby in current state");
		caller->writeMsg("stop lobby/game if you want to restart it");
		return; // just ignore it and stay in current state
	}
	
	if( params.size() > 0 ) {
		int port = atoi(params[0]);
		if( port ) tLXOptions->iNetworkPort = port;
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

COMMAND(startGame, "start game", "", 0, 0);
void Cmd_startGame::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	if(tLX->iGameType == GME_JOIN) {
		caller->writeMsg("cannot start game as client");
		return;
	}
	
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
}

COMMAND(map, "set map", "filename", 1, 1);
void Cmd_map::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	if(tLX->iGameType == GME_JOIN) {
		caller->writeMsg("cannot set map as client");
		return;
	}
	
	std::string filename = params[0];
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

COMMAND(gotoLobby, "go to lobby", "", 0, 0);
void Cmd_gotoLobby::exec(CmdLineIntf* caller, const std::vector<std::string>&) {
	if(tLX->iGameType == GME_JOIN) {
		caller->writeMsg("cannot goto lobby as client");
		return;
	}
	
	cServer->gotoLobby();
	*DeprecatedGUI::bGame = false;
	DeprecatedGUI::tMenu->bMenuRunning = true;
}

COMMAND(chatMsg, "give a global chat message", "text", 1, 1);
void Cmd_chatMsg::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	if(tLX->iGameType == GME_JOIN) { caller->writeMsg(name + " works only as server"); return; }
	
	std::string msg = params[0];
	int type = TXT_NOTICE; // TODO: make variable
	cServer->SendGlobalText(msg, type);
}

COMMAND(privateMessage, "give a private message to a worm", "id text", 2, 2);
void Cmd_privateMessage::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	if(tLX->iGameType == GME_JOIN) { caller->writeMsg(name + " works only as server"); return; }
	
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	
	CWorm *w = CheckWorm(caller, id, name); if(!w) return;
	if( !w->getClient() || !w->getClient()->getNetEngine() ) {
		caller->writeMsg("worm " + itoa(id) + " is somehow crippled");
		return;
	}

	int type = TXT_NOTICE; // TODO: make variable
	w->getClient()->getNetEngine()->SendText(params[1], type);
}

COMMAND(getWormList, "get worm list", "", 0, 0);
void Cmd_getWormList::exec(CmdLineIntf* caller, const std::vector<std::string>& params)
{
	if(tLX->iGameType == GME_JOIN) { caller->writeMsg(name + " works only as server"); return; }
	
	CWorm *w = cServer->getWorms();
	for(int i=0; i < MAX_WORMS; i++, w++)
	{
		if(!w->isUsed())
			continue;

		caller->pushReturnArg(itoa(w->getID()));
	}
}

COMMAND(getComputerWormList, "get computer worm list", "", 0, 0);
void Cmd_getComputerWormList::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	if(tLX->iGameType == GME_JOIN) { caller->writeMsg(name + " works only as server"); return; }
	
	CWorm *w = cServer->getWorms();
	for(int i = 0; i < MAX_WORMS; i++, w++) {
		if(w->isUsed() && w->getType() == PRF_COMPUTER)
			caller->pushReturnArg(itoa(w->getID()));
	}
}

COMMAND(getWormName, "get worm name", "id", 1, 1);
void Cmd_getWormName::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	if(tLX->iGameType == GME_JOIN) { caller->writeMsg(name + " works only as server"); return; }
	
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	CWorm* w = CheckWorm(caller, id, name); if(!w) return;
	
	caller->pushReturnArg(w->getName());
}

COMMAND(getWormTeam, "get worm team", "id", 1, 1);
void Cmd_getWormTeam::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	if(tLX->iGameType == GME_JOIN) { caller->writeMsg(name + " works only as server"); return; }
	
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	CWorm* w = CheckWorm(caller, id, name); if(!w) return;
	
	caller->pushReturnArg(itoa(w->getTeam()));
}

COMMAND(getWormIp, "get worm IP", "id", 1, 1);
void Cmd_getWormIp::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	if(tLX->iGameType == GME_JOIN) { caller->writeMsg(name + " works only as server"); return; }

	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	CWorm* w = CheckWorm(caller, id, name); if(!w) return;
	
	// TODO: Perhaps we can cut out the second argument for the signal- but that would lead to the signal being much larger. Is it worth it?
	std::string str_addr;
	if(w->getClient() && w->getClient()->getChannel())
		NetAddrToString(w->getClient()->getChannel()->getAddress(), str_addr);
	if (str_addr != "")
		caller->pushReturnArg(str_addr);
	else
		caller->writeMsg("GetWormIp: str_addr == \"\"");
}

COMMAND(getWormLocationInfo, "get worm location info", "id", 1, 1);
void Cmd_getWormLocationInfo::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	CWorm* w = CheckWorm(caller, id, name); if(!w) return;

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


COMMAND(getWormPing, "get worm ping", "id", 1, 1);
void Cmd_getWormPing::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	CWorm* w = CheckWorm(caller, id, name); if(!w) return;
	
	if(!w->getClient() || !w->getClient()->getChannel()) {
		caller->writeMsg("worm " + itoa(id) + " has a crippled connection");
		return;
	}

	caller->pushReturnArg(itoa(w->getClient()->getChannel()->getPing()));
}

COMMAND(getWormSkin, "get worm skin", "id", 1, 1);
void Cmd_getWormSkin::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	bool fail = true;
	int id = from_string<int>(params[0], fail);
	if(fail) {
		printUsage(caller);
		return;
	}
	CWorm* w = CheckWorm(caller, id, name); if(!w) return;

	caller->pushReturnArg(itoa(w->getSkin().getDefaultColor().get()));
	caller->pushReturnArg(w->getSkin().getFileName());
}

COMMAND(connect, "connect to server", "serveraddress", 1, 1);
void Cmd_connect::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	JoinServer(params[0], params[1], "");
}

COMMAND(dumpGameState, "dump game state", "", 0, 0);
void Cmd_dumpGameState::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	cServer->DumpGameState();
}

COMMAND(dumpSysState, "dump system state", "", 0, 0);
void Cmd_dumpSysState::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	hints << "System state:" << endl;
	cServer->DumpGameState();
	// TODO: client game state
	hints << "Free system memory: " << (GetFreeSysMemory() / 1024) << " KB" << endl;
	hints << "Cache size: " << (cCache.GetCacheSize() / 1024) << " KB" << endl;
}

COMMAND(saveConfig, "save current config", "", 0, 0);
void Cmd_saveConfig::exec(CmdLineIntf* caller, const std::vector<std::string>& params) {
	tLXOptions->SaveToDisc();
}

static void HandleCommand(const CmdLineIntf::Command& command) {
	std::string cmdstr = command.cmd; TrimSpaces(cmdstr);
	std::string params;
	size_t f = cmdstr.find(' ');
	if(f != std::string::npos) {
		params = cmdstr.substr(f + 1);
		TrimSpaces(params);
		cmdstr = cmdstr.substr(0, f);
	}
	if(cmdstr == "") return;
	
	CommandMap::iterator cmd = commands.find(cmdstr);
	
	if(cmd != commands.end()) {
		cmd->second->exec(command.sender, params);
	}
	// we must handle this command seperate
	else if( stringcaseequal(cmdstr, "nextsignal") ) {
		if(DedicatedControl::Get()) {
			if(!DedicatedControl::Get()->GetNextSignal(command.sender)) return;
		}
		else
			command.sender->writeMsg("nextsignal is only available in dedicated mode");
	}
	else if(Cmd_ParseLine(cmdstr + " " + params)) {}
	else {
		command.sender->writeMsg("unknown command: " + cmdstr + " " + params);
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
