/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Command/variable parsing
// Created 9/4/02
// Jason Boettcher


#include "defs.h"
#include "lierox.h"
#include "console.h"

command_t	*Commands = NULL;

char	Arguments[MAX_ARGS][MAX_ARGLENGTH];
int		NumArgs;


///////////////////
// Add an argument to the list
void Cmd_AddArg(char *text)
{
	strcpy(Arguments[NumArgs++],text);
}


///////////////////
// Get the number of arguments
int Cmd_GetNumArgs(void)
{
	return NumArgs;
}


///////////////////
// Get an argument by index
char *Cmd_GetArg(int a)
{
	if(a>=0 && a<NumArgs)
		return Arguments[a];

	return "";
}


///////////////////
// Parse a line of text
void Cmd_ParseLine(char *text)
{
	int		i,ti;
	int		quote = false;
	char	token[128];

	// Clear the arguments
	NumArgs = 0;

	ti = 0;
	for(i=0;i<(int) strlen(text);i++) {

		// Check delimeters
		if(text[i] == ' ' || text[i] == ',') {
			token[ti] = '\0';
			if(ti)
				Cmd_AddArg(token);
			ti=0;
			continue;
		}

		// Check comments
		if(text[i] == '/' && text[i+1] == '/') {
			token[ti] = '\0';
			if(ti)
				Cmd_AddArg(token);
			ti=0;
			// Just end here
			break;
		}

		// Check quotes
		if(text[i] == '"') {
			quote = true;

			i++;
			for(;i<(int)strlen(text);i++) {

				if(text[i] == '"') {
					quote = false;
					break;
				}

				token[ti++] = text[i];
			}
			continue;
		}

		// Normal text
		token[ti++] = text[i];
	}	
	
	// Add the last token, only if it's not in unfinished quotes
	if(ti && !quote) {
		token[ti] = '\0';
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

	Con_Printf(CNC_NOTIFY,"Unknown command '%s'",Cmd_GetArg(0));
}


///////////////////
// Find a command with the same name
command_t *Cmd_GetCommand(char *strName)
{
	command_t *cmd;

	for(cmd=Commands ; cmd ; cmd=cmd->Next)
		if(!strcmp(strName, cmd->strName))
			return cmd;

	return NULL;
}


///////////////////
// Auto complete a command
int Cmd_AutoComplete(char *strVar, int *iLength)
{
	int len = strlen(strVar);
	command_t *cmd;

	if(!len)
		return false;

	// See if it's an exact match
	cmd = Cmd_GetCommand(strVar);
	if(cmd) {
		sprintf(strVar,"%s ",cmd->strName);
		*iLength = strlen(strVar);
		return true;
	}

	// See if it's a partial match
	for(cmd=Commands ; cmd ; cmd=cmd->Next)
		if(!strncmp(strVar, cmd->strName,len)) {
			sprintf(strVar,"%s ",cmd->strName);
			*iLength = strlen(strVar);
			return true;
		}


	return false;
}


///////////////////
// Add a command to the list
int Cmd_AddCommand(char *strName, void (*func) ( void ))
{
	// Make sure the command isn't a variable
	/*if(CV_Find(strName)) {
		Con_Printf(CNC_WARNING,"%s already used as a variable",strName);
		return false;
	}*/


	// Make sure the command isn't already used
	if(Cmd_GetCommand(strName)) {
		Con_Printf(CNC_WARNING,"%s already defined as a command",strName);
		return false;
	}


	// Allocate room for the new var
	command_t *cmd;

	cmd = (command_t *)malloc(sizeof(command_t));
	cmd->strName = (char *)malloc(strlen(strName) + 1);
	strcpy(cmd->strName,strName);	
	cmd->func = func;
	
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
		
		if(cmd->strName)
			free(cmd->strName);
		
		if(cmd)
			free(cmd);
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

	char *arg = "";
	char cID[6];
	int ID;
	arg = Cmd_GetArg(1);
	int j = 0;
	for (int i=0; i<(int)strlen(arg) && j<6; i++)
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

	char *arg = "";
	char cID[6];
	int ID;
	arg = Cmd_GetArg(1);
	int j = 0;
	for (int i=0; i<(int) strlen(arg) && j<6; i++)
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

	char *arg = "";
	char cID[6];
	int ID;
	arg = Cmd_GetArg(1);
	int j = 0;
	for (int i=0; i<(int)strlen(arg) && j<6; i++)
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

	char *arg = "";
	char cID[6];
	int ID;
	arg = Cmd_GetArg(1);
	int j = 0;
	for (int i=0; i<(int)strlen(arg) && j<6; i++)
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
	// Let's make an access violation
	char *crashbuf = NULL;
	*(crashbuf+789456) = '\n';
}

///////////////////
// Suicide
void Cmd_Suicide(void)
{
	if (cClient)  {
		if(cClient->getStatus() != NET_PLAYING)  {
			Con_Printf(CNC_NORMAL,"%s","Cannot suicide when not playing!");
			return;
		}

		CWorm *w = cClient->getWorm(0);
		// Without arguments, just commit one suicide
		if (Cmd_GetNumArgs() == 1)  {
			if(w->isUsed() && w->getAlive())
				cClient->InjureWorm(w,101,w->getID());
		}
		// A number has been entered, suicide the specified number
		else  {
			// Get the number
			char *arg = "";
			char cNumber[6];
			int number;
			arg = Cmd_GetArg(1);
			int j = 0;
			for (int i=0; i<(int)strlen(arg) && j<6; i++)
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
				for (i = 0; i<number; i++)
					cClient->InjureWorm(w,101,w->getID());
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
			Con_Printf(CNC_NORMAL,"%s","Cannot unstuck when not playing!");
			return;
		}

		// Unstuck
		CWorm *w = cClient->getWorm(0);
		if (w->isUsed() && w->getAlive())
			w->setPos(cClient->FindNearestSpot(w));
	}
}
