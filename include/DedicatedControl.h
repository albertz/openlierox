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
	void WeaponSelections_Signal() {}
	void GameStarted_Signal() {}
	
	void Menu_Frame();
	void GameLoop_Frame();
};

#endif
