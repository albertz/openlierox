/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Chat command handling
// Created 24/12/07
// Karel Petranek

#ifndef __CHATCOMMAND_H__
#define	__CHATCOMMAND_H__

#include <string>
#include <vector>

// Command structure
class ChatCommand { public:
	std::string sName;
	std::string sAlias;

	size_t		iMinParamCount;
	size_t		iMaxParamCount;  // -1 for unlimited
	size_t		iParamsToRepeat; // Number of parameters that should be put in front of every part of a split message
								 // -1 if the message cannot be split
	typedef		std::string	( * tProcFunc_t ) (const std::vector<std::string>& params, int sender_id);
	tProcFunc_t	tProcFunc;
};

// Processing functions
// These functions return error string (or blank string if no error)
std::string ProcessAdminMsg(const std::vector<std::string>& params, int sender_id);
std::string ProcessAuthorise(const std::vector<std::string>& params, int sender_id);
std::string ProcessKick(const std::vector<std::string>& params, int sender_id);
std::string ProcessBan(const std::vector<std::string>& params, int sender_id);
std::string ProcessMute(const std::vector<std::string>& params, int sender_id);
std::string ProcessUnmute(const std::vector<std::string>& params, int sender_id);
std::string ProcessPrivate(const std::vector<std::string>& params, int sender_id);
std::string ProcessTeamChat(const std::vector<std::string>& params, int sender_id);
std::string ProcessMe(const std::vector<std::string>& params, int sender_id); // "/me" command for old clients
std::string ProcessSetMyName(const std::vector<std::string>& params, int sender_id);
std::string ProcessSetName(const std::vector<std::string>& params, int sender_id);
std::string ProcessSetMySkin(const std::vector<std::string>& params, int sender_id);
std::string ProcessSetSkin(const std::vector<std::string>& params, int sender_id);
std::string ProcessSetMyColour(const std::vector<std::string>& params, int sender_id);
std::string ProcessSetColour(const std::vector<std::string>& params, int sender_id);
std::string ProcessSuicide(const std::vector<std::string>& params, int sender_id);
std::string ProcessSpectate(const std::vector<std::string>& params, int sender_id);
std::string ProcessLogin(const std::vector<std::string>& params, int sender_id);
std::string ProcessStart(const std::vector<std::string>& params, int sender_id);
std::string ProcessLobby(const std::vector<std::string>& params, int sender_id);
std::string ProcessMod(const std::vector<std::string>& params, int sender_id);
std::string ProcessLevel(const std::vector<std::string>& params, int sender_id);
std::string ProcessLt(const std::vector<std::string>& params, int sender_id);
std::string ProcessDedicated(const std::vector<std::string>& params, int sender_id);
std::string ProcessScript(const std::vector<std::string>& params, int sender_id);
std::string ProcessSetVar(const std::vector<std::string>& params, int sender_id);
std::string ProcessWeapons(const std::vector<std::string>& params, int sender_id);


// List of known commands (filled in in ChatCommand.cpp)
extern ChatCommand tKnownCommands[];

// Command finding
ChatCommand *GetCommand(const std::string& name);

// Main parsing function
std::vector<std::string> ParseCommandMessage(const std::string& msg, bool ignore_blank_params);

#endif // __CHATCOMMAND_H__
