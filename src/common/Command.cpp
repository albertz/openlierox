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
#include "CServer.h"
#include "CClient.h"
#include "console.h"
#include "StringUtils.h"
#include "sex.h"
#include "CWorm.h"
#include "AuxLib.h"


command_t	*Commands = NULL;

std::string	Arguments[MAX_ARGS];
int		NumArgs;


///////////////////
// Add an argument to the list
void Cmd_AddArg(const std::string& text)
{
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
void Cmd_ParseLine(const std::string& text)
{
	bool quote = false;
	std::string	token;

	// Clear the arguments
	NumArgs = 0;

	std::string::const_iterator i = text.begin();
	std::string::const_iterator i2 = i;
	for(; i != text.end(); i++) {

		if (i2 != text.end())
			i2++;

		// Check delimeters
		if(*i == ' ' || *i == ',') {
			if(!token.empty())
				Cmd_AddArg(token);
			token = "";

			continue;
		}

		// Check comments
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
		return;


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
		return;
	}

	std::string tmp = Cmd_GetArg(0);
	Con_Printf(CNC_NOTIFY, "Unknown command '" + tmp + "'");
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
	int len = strVar.size();
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
	for(cmd=Commands ; cmd ; cmd=cmd->Next)
		if(!stringcasecmp(strVar, cmd->strName.substr(0,len))) {
			strVar = cmd->strName + " ";
			return true;
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
	if(tGameInfo.iGameType != GME_HOST)  {
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
	if(tGameInfo.iGameType != GME_HOST)  {
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
	if(tGameInfo.iGameType != GME_HOST)  {
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
	if(tGameInfo.iGameType != GME_HOST)  {
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
	if(tGameInfo.iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "kickid <worm_id>");
        return;
    }

	std::string arg = Cmd_GetArg(1);
	char cID[6];
	int ID;
	size_t j = 0;
	for (size_t i=0; i<arg.size() && j<6; i++) // TODO: iterators!
		if (arg[i] >= 48 && arg[i] <= 57)  {
			cID[j] = arg[i];
			j++;
		}

	cID[j] = '\0';
	arg = cID;
	ID = atoi(arg);

    if(cServer)
       cServer->kickWorm(ID);
}

///////////////////
// Tell the server to kick and ban someone by ID
void Cmd_BanId(void)
{
	if(tGameInfo.iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "banid <worm_id>");
        return;
    }

	char cID[6];
	int ID;
	std::string arg = Cmd_GetArg(1);
	size_t j = 0;
	for (size_t i=0; i<arg.size() && j<6; i++) // TODO: iterators!
		if (arg[i] >= 48 && arg[i] <= 57)  {
			cID[j] = arg[i];
			j++;
		}

	cID[j] = '\0';
	arg = cID;
	ID = atoi(arg);

    if(cServer)
       cServer->banWorm(ID);
}

///////////////////
// Tell the server to mute someone by ID
void Cmd_MuteId(void)
{
	if(tGameInfo.iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "muteid <worm_id>");
        return;
    }

	char cID[6];
	int ID;
	std::string arg = Cmd_GetArg(1);
	size_t j = 0;
	for (size_t i=0; i< arg.size() && j<6; i++) // TODO: iterators!
		if (arg[i] >= 48 && arg[i] <= 57)  {
			cID[j] = arg[i];
			j++;
		}

	cID[j] = '\0';
	arg = cID;
	ID = atoi(arg);

    if(cServer)
       cServer->muteWorm(ID);
}

///////////////////
// Tell the server to unmute someone by ID
void Cmd_UnmuteId(void)
{
	if(tGameInfo.iGameType != GME_HOST)  {
		Con_Printf(CNC_NORMAL, "This command is available only for host.");
		return;
	}

    if(Cmd_GetNumArgs() == 1) {
        Con_Printf(CNC_NORMAL, "Usage:");
        Con_Printf(CNC_NORMAL, "unmuteid <worm_id>");
        return;
    }

	char cID[6];
	int ID;
	std::string arg = Cmd_GetArg(1);
	size_t j = 0;
	for (size_t i=0; i<arg.size() && j<6; i++) // TODO: iterators!
		if (arg[i] >= 48 && arg[i] <= 57)  {
			cID[j] = arg[i];
			j++;
		}

	cID[j] = '\0';
	arg = cID;
	ID = atoi(arg);

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
}

///////////////////
// Suicide
void Cmd_Suicide(void)
{
	if (cClient)  {
		if(cClient->getStatus() != NET_PLAYING)  {
			Con_Printf(CNC_NORMAL, "Cannot suicide when not playing!");
			return;
		}

		CWorm *w = cClient->getWorm(0);
		// Without arguments, just commit one suicide
		if (Cmd_GetNumArgs() == 1)  {
			if(w->isUsed())
				cClient->InjureWorm(w, w->getHealth() + 1, w->getID());
		}
		// A number has been entered, suicide the specified number
		else  {
			// Get the number
			char cNumber[6];
			int number;
			std::string arg = Cmd_GetArg(1);
			size_t j = 0;
			size_t i;
			for (i=0; i< arg.size() && j<6; i++) // TODO: iterators!
				if (arg[i] >= 48 && arg[i] <= 57)  {
					cNumber[j] = arg[i];
					j++;
				}

			cNumber[j] = '\0';
			arg = cNumber;
			number = atoi(arg);
			if (number > tGameInfo.iLives+1)  // Safety, not needed really (should be covered in next condition)
				number = tGameInfo.iLives+1;
			if (number > w->getLives()+1)
				number = w->getLives()+1;
			if (number < 1)
				number = 1;

			// Suicide
			if (w->isUsed() && w->getAlive())
				for (i = 0; i<(uint)number; i++)
					cClient->SendDeath(w->getID(), w->getID());
		}
	}
}

//////////////////
// Unstuck a stucked worm
void Cmd_Unstuck(void)
{

	if (cClient)  {
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
		tLXOptions->tGameinfo.bAllowWantsJoinMsg = true;
		Con_Printf(CNC_NORMAL, "\"Wants to join\" messages have been enabled");
	}
	else  {
		tLXOptions->tGameinfo.bAllowWantsJoinMsg = false;
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
	if (tGameInfo.iGameType != GME_HOST)  {
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
	std::string name = GetGameName();
	Con_Printf(CNC_NOTIFY, name + " v" + LX_VERSION);
}

void Cmd_BadWord() {
	Con_Printf(CNC_NOTIFY, sex(50));
}

void Cmd_Quit() {
	tLX->iQuitEngine = true;
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
		tLXOptions->iSoundOn = true;
	}
	else  {
		StopSoundSystem();
		tLXOptions->iSoundOn = false;
	}
}

//////////////////
// Change name (only if host in on beta 3)
void Cmd_SetName() {
	// Check arguments
	if(Cmd_GetNumArgs() == 1) {
		Con_Printf(CNC_NORMAL, "Usage: setname \"<name>\"");
		return;
	}

	// Check if the host is on beta 3
	if(!cClient->getHostBeta3()) {
		Con_Printf(CNC_ERROR, "This command is only available on OpenLX beta 3+ servers");
		return;
	}

	std::string name = Cmd_GetArg(1);
	cClient->SendText("/setname \"" + name + "\"", cClient->getWorm(0)->getName());
}


//////////////////
// Change skin (only if host in on beta 3)
void Cmd_SetSkin() {
	// Check arguments
	if(Cmd_GetNumArgs() == 1) {
		Con_Printf(CNC_NORMAL, "Usage: setskin \"<skin_filename>\"");
		return;
	}

	// Check if the host is on beta 3
	if(!cClient->getHostBeta3()) {
		Con_Printf(CNC_ERROR, "This command is only available on OpenLX beta 3+ servers");
		return;
	}

	std::string skin = Cmd_GetArg(1);
	cClient->SendText("/setskin \"" + skin + "\"", cClient->getWorm(0)->getName());
}


//////////////////
// Change colour (only if host in on beta 3)
void Cmd_SetColour() {
	// Check arguments
	if(Cmd_GetNumArgs() < 4) {
		Con_Printf(CNC_NORMAL, "Usage: setcolour <red> <green> <blue>");
		return;
	}

	// Check if the host is on beta 3
	if(!cClient->getHostBeta3()) {
		Con_Printf(CNC_ERROR, "This command is only available on OpenLX beta 3+ servers");
		return;
	}

	cClient->SendText("/setcolour " + Cmd_GetArg(1) + " " + Cmd_GetArg(2) + " " + Cmd_GetArg(3), cClient->getWorm(0)->getName());
}

/////////////////
// Turn on/off server-side health
void Cmd_ServerSideHealth()  {
	// Check arguments
	if (Cmd_GetNumArgs() == 1)  {
		Con_Printf(CNC_NORMAL, "Usage: sound <on/off>");
	}

	std::string arg = Cmd_GetArg(1);

	// Set the ssh
	tLXOptions->bServerSideHealth =  !stringcasecmp(arg,"on") || !stringcasecmp(arg,"true") || !stringcasecmp(arg,"1") || !stringcasecmp(arg,"yes");

	Con_Printf(CNC_NORMAL, std::string("Server-side health is now ") + (tLXOptions->bServerSideHealth ? std::string("enabled.") : std::string("disabled.")));
}
