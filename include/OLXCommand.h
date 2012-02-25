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
#include <map>
#include "CodeAttributes.h"

typedef std::map<size_t,size_t> ParamSeps;

ParamSeps ParseParams_Seps(const std::string& params);
std::vector<std::string> ParseParams(const std::string& params);

struct CmdLineIntf;
class AutocompletionInfo;

CmdLineIntf& stdoutCLI();



// Colours
enum CmdLineMsgType {
	CNC_NORMAL = 0,
	CNC_NOTIFY = 1, //Color(200,200,200)
	CNC_ERROR = 2, //Color(255,0,0)
	CNC_WARNING = 3, //Color(200,128,128)
	CNC_DEV = 4, //Color(100,100,255)
	CNC_CHAT = 5, //Color(100,255,100)
};

INLINE std::string CmdLineMsgTypeAsString(CmdLineMsgType type) {
	switch (type) {
		case CNC_NORMAL: return "";
		case CNC_NOTIFY: return "NOTIFY";
		case CNC_ERROR: return "ERROR";
		case CNC_WARNING: return "WARNING";
		case CNC_DEV: return "DEV";
		case CNC_CHAT: return "CHAT";
	}
	return "INVALIDMSGTYPE";
}


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
	virtual void writeMsg(const std::string& msg, CmdLineMsgType type = CNC_NORMAL) = 0;
	virtual void finishedCommand(const std::string& cmd) {} // gets called after a cmd was executed from this CLI
	virtual ~CmdLineIntf() {}
	
	struct Command {
		CmdLineIntf* sender;
		std::string cmd;
		Command(CmdLineIntf* s = NULL, const std::string& c = "") : sender(s), cmd(c) {}
	};


};


// Pushs a command into the command queue. This will not do any parsing nor executing.
// All that is done when you call HandlePendingCommands().
void Execute(const CmdLineIntf::Command& cmd);
INLINE void Execute(CmdLineIntf* sender, const std::string& cmd) { Execute(CmdLineIntf::Command(sender, cmd)); }

bool havePendingCommands();

// Executes all commands in the queue. This is called from the gameloopthread.
void HandlePendingCommands();

// Executes in current thread, right now. Returns all pushed return values.
// WARNING: Only call this if you know this is safe. If you don't really need that, use Execute above.
std::vector<std::string> Execute_Here(const std::string& cmd);


#endif  //  __CON_COMMAND_H__
