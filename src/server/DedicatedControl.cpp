/*
 *  DedicatedControl.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 11.01.08.
 *  code under LGPL
 *
 */

#include <pstream.h>
#include "DedicatedControl.h"

static DedicatedControl* dedicatedControlInstance = NULL;

DedicatedControl* DedicatedControl::Get() { return dedicatedControlInstance; }

DedicatedControl::DedicatedControl() {}

bool DedicatedControl::Init_priv() {
	return true;
}

bool DedicatedControl::Init() {
	dedicatedControlInstance = new DedicatedControl();
	return dedicatedControlInstance->Init_priv();
}

void DedicatedControl::Uninit() {
	delete dedicatedControlInstance;
}

void DedicatedControl::GameLoopStart_Signal() {}
void DedicatedControl::GameLoopEnd_Signal() {}
void DedicatedControl::Menu_Frame() {}
void DedicatedControl::GameLoop_Frame() {}
