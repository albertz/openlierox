/*
 *  DedicatedControl.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 11.01.08.
 *  code under LGPL
 *
 */

#include <iostream>
#include <string>
#include <pstream.h>

#include "DedicatedControl.h"
#include "LieroX.h"
#include "FindFile.h"

using namespace std;
using namespace redi;

static DedicatedControl* dedicatedControlInstance = NULL;

DedicatedControl* DedicatedControl::Get() { return dedicatedControlInstance; }

DedicatedControl::DedicatedControl() {}

bool DedicatedControl::Init_priv() {
	const std::string scriptfn_rel = "scripts/dedicated_control.sh";

	printf("DedicatedControl initing...\n");
	std::string scriptfn = GetFullFileName(scriptfn_rel);
	if(!IsFileAvailable(scriptfn)) {
		printf("ERROR: %s not found\n", scriptfn_rel.c_str());
		return false;		
	}
	
	pstream ps(scriptfn, pstreams::pstdin|pstreams::pstdout|pstreams::pstderr);
	ps << "hello world\n" ; // << peof;
	ps.flush();
	
	        string buf;
        while (getline(ps.out(), buf))
            cout << "STDOUT: " << buf << endl;
        ps.clear();
        while (getline(ps.err(), buf))
            cout << "STDERR: " << buf << endl;
            
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
