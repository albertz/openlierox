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
class CWorm;

class DedicatedControl {
private:
	DedicatedControl(); ~DedicatedControl();
	bool Init_priv();
public:
	void* internData;
	
	static bool Init(); static void Uninit();
	static DedicatedControl* Get();
	
	void BackToLobby_Signal();
	void GameLoopStart_Signal();
	void GameLoopEnd_Signal();
	void NewWorm_Signal(CWorm* w);
	void WormLeft_Signal(CWorm* w);
	void WeaponSelections_Signal();
	void GameStarted_Signal();
	void ChatMessage_Signal(CWorm* w, const std::string& message);
	void PrivateMessage_Signal(CWorm* w, CWorm* to, const std::string& message);
	void WormDied_Signal(CWorm* died, CWorm* killer);
	
	void Menu_Frame();
	void GameLoop_Frame();
};

#endif
