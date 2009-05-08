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
#include <vector>


std::vector<std::string> ParseParams(const std::string& params);



void	Cmd_Initialize();
void	Cmd_Free();

bool	Cmd_ParseLine(const std::string& text);
int		Cmd_AutoComplete(std::string& strVar);





/*
	The intended way to use:
	
	After pushing a CLI::Command to the command queue, you have to wait
	and you will get multiple pushReturnArg calls and at the end a
	finalizeReturn call. You can do any further handling in the finalizeReturn
	call.
	
	At the very end, you also get a finishedCommand call. This one is ignored
	in most CLI implementations. The chat CLI uses is to destroy itself
	because it has an own instance for each executed command.
*/
struct CmdLineIntf {
	virtual void pushReturnArg(const std::string& str) = 0;
	virtual void finalizeReturn() = 0;
	virtual void writeMsg(const std::string& msg) = 0;
	virtual void finishedCommand(const std::string& cmd) {} // gets called after a cmd was executed from this CLI
	virtual ~CmdLineIntf() {}
	
	struct Command {
		CmdLineIntf* sender;
		std::string cmd;
		Command(CmdLineIntf* s, const std::string& c) : sender(s), cmd(c) {}
	};
};


// pushs a command into the command queue
void Execute(const CmdLineIntf::Command& cmd);
inline void Execute(CmdLineIntf* sender, const std::string& cmd) { Execute(CmdLineIntf::Command(sender, cmd)); }

void HandlePendingCommands();


#endif  //  __CON_COMMAND_H__
