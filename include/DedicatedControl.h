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

class DedicatedControl {
private:
	DedicatedControl();
	bool Init_priv();
public:
	static bool Init(); static void Uninit();
	static DedicatedControl* Get();
	
	void GameLoopStart_Signal();
	void GameLoopEnd_Signal();
	void Menu_Frame();
	void GameLoop_Frame();
};

#endif
