/*
 *  DedicatedControl.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 11.01.08.
 *  code under LGPL
 *
 */

#ifndef __DEDICATEDCONTROL_H__
#define __DEDICATEDCONTROL_H__

#include <string>

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


struct DedIntern;
class CWorm;

class DedicatedControl {
private:
	DedicatedControl(); ~DedicatedControl();
	bool Init_priv();
public:
	DedIntern* internData;
	
	static bool Init(); static void Uninit();
	static DedicatedControl* Get();
	
	void BackToServerLobby_Signal();
	void BackToClientLobby_Signal();
	void GameLoopStart_Signal();
	void GameLoopEnd_Signal();
	void NewWorm_Signal(CWorm* w);
	void WormLeft_Signal(CWorm* w);
	void WeaponSelections_Signal();
	void GameStarted_Signal();
	void ChatMessage_Signal(CWorm* w, const std::string& message);
	void PrivateMessage_Signal(CWorm* w, CWorm* to, const std::string& message);
	void WormDied_Signal(CWorm* died, CWorm* killer);
	void WormSpawned_Signal(CWorm* worm);
	void WormGotAdmin_Signal(CWorm* worm);
	void WormAuthorized_Signal(CWorm* worm);
	
	void Execute(CmdLineIntf::Command cmd);
	
	void Menu_Frame();
	void GameLoop_Frame();
};



#endif
