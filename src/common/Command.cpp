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

#include <iostream>

#include "LieroX.h"

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


using namespace std;

command_t	*Commands = NULL;

std::string	Arguments[MAX_ARGS];
int		NumArgs;


///////////////////
// Add an argument to the list
void Cmd_AddArg(const std::string& text)
{
	if(NumArgs >= MAX_ARGS)
		cout << "WARNING: too much arguments, ignoring: " << text << endl;
	else
		Arguments[NumArgs++] = text;
}


///////////////////
// Get the number of arguments
int Cmd_GetNumArgs(void)
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


///////////////////
// Parse a line of text
bool Cmd_ParseLine(const std::string& text)
{
	bool quote = false;
	std::string	token;

	// Clear the arguments
	NumArgs = 0;

	std::string::const_iterator i = text.begin();
	for(; i != text.end(); i++) {

		// Check delimeters
		if(*i == ' ' || *i == ',') {
			if(!token.empty())
				Cmd_AddArg(token);
			token = "";

			continue;
		}

		// Check comments
		std::string::const_iterator i2 = i; i2++;
		if (i2 != text.end())  {
			if(*i == '/' && *i2 == '/') {
				if(!token.empty())
					Cmd_AddArg(token);
				token = "";

				// Just end here
				break;
			}
		}

		// Check quotes
		if(*i == '"') {
			quote = true;

			// Read until another quote
			for(i++; i != text.end(); i++) {

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
		Cmd_AddArg(token);
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
	Con_Printf(CNC_NOTIFY, "Unknown command '" + tmp + "'");
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
		Con_Printf(CNC_WARNING,"%s already used as a variable",strName);
		return false;
	}*/


	// Make sure the command isn't already used
	if(Cmd_GetCommand(strName)) {
		Con_Printf(CNC_WARNING, strName + " already defined as a command");
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
void Cmd_Free(void)
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
void Cmd_Kick(void)
{
	if(tLX->iGameType == GME_JOIN)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "kick <worm_name>");
        return;
    }

    if(cServer)
        cServer->kickWorm(Cmd_GetArg(1));
}

///////////////////
// Tell the server to kick and ban someone
void Cmd_Ban(void)
{
	if(tLX->iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "ban <worm_name>");
        return;
    }

    if(cServer)
       cServer->banWorm(Cmd_GetArg(1));
}

///////////////////
// Tell the server to mute someone
void Cmd_Mute(void)
{
	if(tLX->iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "mute <worm_name>");
        return;
    }

    if(cServer)
       cServer->muteWorm(Cmd_GetArg(1));
}

///////////////////
// Tell the server to unmute someone
void Cmd_Unmute(void)
{
	if(tLX->iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "unmute <worm_name>");
        return;
    }

    if(cServer)
       cServer->unmuteWorm(Cmd_GetArg(1));
}

///////////////////
// Tell the server to kick someone by ID
void Cmd_KickId(void)
{
	if(tLX->iGameType == GME_JOIN)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "kickid <worm_id>");
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
void Cmd_BanId(void)
{
	if(tLX->iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "banid <worm_id>");
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
void Cmd_MuteId(void)
{
	if(tLX->iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "muteid <worm_id>");
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
void Cmd_UnmuteId(void)
{
	if(tLX->iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "unmuteid <worm_id>");
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
void Cmd_Crash(void)
{
	Con_Printf(CNC_NORMAL,"In a previous version, the game would crash now!");
	// HINT: please don't add any code, which could make the game unstable
	//		(I myself just tested this command without knowing and BANG,
	//		I got an access violation. Perhaps the hoster of an important
	//		clan war does it...)
#ifdef DEBUG
	Con_Printf(CNC_WARNING, "This version will crash too, though.");
	// IMPORTANT TODO: remove this before Beta6 release! (it's for testing the new CrashHandler)
	(*(int*)0x13) = 42;
#endif
}

///////////////////
// Suicide
void Cmd_Suicide(void)
{
	if (cClient)  {
		if(bDedicated) {
			Con_Printf(CNC_NORMAL, "Cannot suicide in dedicated mode!");
			return;
		}

		if(cClient->getStatus() != NET_PLAYING)  {
			Con_Printf(CNC_NORMAL, "Cannot suicide when not playing!");
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
void Cmd_Unstuck(void)
{

	if (cClient)  {
		if(bDedicated) {
			Con_Printf(CNC_NORMAL, "Cannot unstuck in dedicated mode!");
			return;
		}

		// Not playing
		if(cClient->getStatus() != NET_PLAYING)  {
			Con_Printf(CNC_NORMAL, "Cannot unstuck when not playing!");
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
void Cmd_WantsJoin(void)
{
	// Check arguments
	if (Cmd_GetNumArgs() == 1)  {
		Con_Printf(CNC_NORMAL, "Usage: wantsjoin <on/off>");
		return;
	}

	std::string arg = Cmd_GetArg(1);

	if (!stringcasecmp(arg,"on") || !stringcasecmp(arg,"true") || !stringcasecmp(arg,"1") || !stringcasecmp(arg,"yes"))  {
		tLXOptions->bAllowWantsJoinMsg = true;
		Con_Printf(CNC_NORMAL, "\"Wants to join\" messages have been enabled");
	}
	else  {
		tLXOptions->bAllowWantsJoinMsg = false;
		Con_Printf(CNC_NORMAL, "\"Wants to join\" messages have been disabled");
	}
}

void Cmd_RenameServer(void)
{
	// Check arguments
	if (Cmd_GetNumArgs() == 1)  {
		Con_Printf(CNC_NORMAL, "Usage: servername <new name>");
		return;
	}

	// Check if hosting
	if (tLX->iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host");
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
	Con_Printf(CNC_NORMAL, "Available commands:");
	static std::string cmd_help_buf;
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
				Con_Printf(CNC_NORMAL, "  " + cmd_help_buf);
				cmd_help_buf = "";
			}
		}
	}
	if(count && cmd_help_buf != "") {
		Con_Printf(CNC_NORMAL, "  " + cmd_help_buf);
	}
}

void Cmd_About() {
	Con_Printf(CNC_NOTIFY, GetFullGameName());
}

void Cmd_BadWord() {
	Con_Printf(CNC_NOTIFY, sex(50));
}

void Cmd_Quit() {
	SetQuitEngineFlag("Console Cmd_Quit");
}

///////////////////
// Set sound volume
void Cmd_Volume()  {
	if (Cmd_GetNumArgs() == 1)  {
		Con_Printf(CNC_NORMAL, "Usage: volume <0-100>");
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
		Con_Printf(CNC_NORMAL, "Usage: sound <on/off>");
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
		Con_Printf(CNC_NORMAL, "Usage: ssh <on/off>");
	} else {
		std::string arg = Cmd_GetArg(1);

		// Set the ssh
		tLXOptions->bServerSideHealth =  !stringcasecmp(arg,"on") || !stringcasecmp(arg,"true") || !stringcasecmp(arg,"1") || !stringcasecmp(arg,"yes");
	}

	Con_Printf(CNC_NORMAL, std::string("Server-side health is now ") + (tLXOptions->bServerSideHealth ? std::string("enabled.") : std::string("disabled.")));
*/

	Con_Printf(CNC_NORMAL, "Sorry, server side health has been removed for non-dedicated servers");
}

//////////////////
// Send message to IRC chat from inside game
void Cmd_SendIrcMessage()  {
	// Check arguments
	if (Cmd_GetNumArgs() == 1)  {
		Con_Printf(CNC_NORMAL, "Usage: irc your message");
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
	Cmd_AddCommand("ssh", Cmd_ServerSideHealth);
	Cmd_AddCommand("irc", Cmd_SendIrcMessage);
	Cmd_AddCommand("chat", Cmd_SendIrcMessage);

}
