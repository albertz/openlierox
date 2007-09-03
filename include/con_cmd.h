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


// Command/variable parsing header
// Created 9/4/02
// Jason Boettcher


#ifndef __CON_COMMAND_H__
#define __CON_COMMAND_H__

#include <string>


#define		MAX_ARGS		32
#define		MAX_ARGLENGTH	128


// Command structure
class command_t { public:
	std::string		strName;
	void			(*func) ( void );
	bool			bHidden;

	command_t	*Next;
};


// Arguments
int		Cmd_GetNumArgs(void);
void	Cmd_AddArg(const std::string& text);
std::string Cmd_GetArg(int a);



// Command routines
command_t	*Cmd_GetCommand(const std::string& strName);
void	Cmd_ParseLine(const std::string& text);
int		Cmd_AutoComplete(std::string& strVar);
int		Cmd_AddCommand(const std::string& strName, void (*func) ( void ), bool hide = false);
void	Cmd_FreeCommands(void);
void	Cmd_Free(void);


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
void	Cmd_SetName();
void	Cmd_SetSkin();
void	Cmd_SetColour();
void	Cmd_ServerSideHealth();



#endif  //  __CON_COMMAND_H__
