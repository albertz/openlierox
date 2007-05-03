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


#define		MAX_ARGS		32
#define		MAX_ARGLENGTH	128


// Command structure
class command_t { public:
	UCString		strName;
	void			(*func) ( void );

	command_t	*Next;
};


// Arguments
int		Cmd_GetNumArgs(void);
void	Cmd_AddArg(const UCString& text);
UCString Cmd_GetArg(int a);



// Command routines
command_t	*Cmd_GetCommand(const UCString& strName);
void	Cmd_ParseLine(const UCString& text);
int		Cmd_AutoComplete(UCString& strVar);
int		Cmd_AddCommand(const UCString& strName, void (*func) ( void ));
void	Cmd_FreeCommands(void);
void	Cmd_Free(void);


// User commands
void    Cmd_Kick(void);
void	Cmd_Ban(void);
void	Cmd_KickId(void);
void	Cmd_BanId(void);
void    Cmd_Mute(void);
void	Cmd_MuteId(void);
void	Cmd_Unmute(void);
void	Cmd_UnmuteId(void);
void	Cmd_Crash(void);
void	Cmd_Suicide(void);
void	Cmd_Unstuck(void);
void	Cmd_WantsJoin(void);
void	Cmd_RenameServer(void);
void	Cmd_Help();
void	Cmd_About();
void	Cmd_BadWord();
void	Cmd_Quit();
void	Cmd_Volume();
void	Cmd_Sound();



#endif  //  __CON_COMMAND_H__
