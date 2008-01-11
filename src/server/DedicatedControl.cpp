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
#include <sstream>
#include <pstream.h>
#include <SDL_thread.h>

#include "DedicatedControl.h"
#include "LieroX.h"
#include "FindFile.h"

using namespace std;
using namespace redi;

static void Ded_ParseCommand(stringstream& s, string& cmd, string& rest) {
	cmd = ""; rest = "";
	
	char c;
	while( true ) {
		c = s.get();
		if(c > 32) {
			cmd += c;
		} else {
			if(c == 13 || c == 10) return;
			break;
		}
	}

	while( true ) {
		c = s.get();
		if(c == 13 || c == 10) return;
		rest += c;
	}
}


static DedicatedControl* dedicatedControlInstance = NULL;

DedicatedControl* DedicatedControl::Get() { return dedicatedControlInstance; }

struct DedIntern {	
	SDL_Thread* pipeThread;
	pstream pipe;
	SDL_mutex* pipeOutputMutex;
	stringstream pipeOutput;
	
	static DedIntern* Get() { return (DedIntern*)dedicatedControlInstance->internData; }
	~DedIntern() {
		pipe << peof;
		SDL_WaitThread(pipeThread, NULL);
		SDL_DestroyMutex(pipeOutputMutex);
	}
	
	// reading lines from pipe and put them to pipeOutput
	static int pipeThreadFunc(void*) {
		DedIntern* data = Get();
		
		while(!data->pipe.out().eof()) {
			string buf;
			getline(data->pipe.out(), buf);
			SDL_mutexP(data->pipeOutputMutex);
			data->pipeOutput << buf << endl;
			SDL_mutexV(data->pipeOutputMutex);
		}
		return 0;
	}
	
	void BasicFrame() {
		SDL_mutexP(pipeOutputMutex);
		while(pipeOutput.rdbuf()->in_avail() > 0) {
			string cmd, rest;
			Ded_ParseCommand(pipeOutput, cmd, rest);
			cout << "Ded: " << cmd << " :: " << rest << endl;
		}
		SDL_mutexV(pipeOutputMutex);
	}
};

DedicatedControl::DedicatedControl() : internData(NULL) {}
DedicatedControl::~DedicatedControl() {	if(internData) delete (DedIntern*)internData; internData = NULL; }

bool DedicatedControl::Init_priv() {
	const std::string scriptfn_rel = "scripts/dedicated_control.sh";

	printf("DedicatedControl initing...\n");
	std::string scriptfn = GetFullFileName(scriptfn_rel);
	if(!IsFileAvailable(scriptfn)) {
		printf("ERROR: %s not found\n", scriptfn_rel.c_str());
		return false;		
	}
	
	DedIntern* dedIntern = new DedIntern;
	internData = dedIntern;
	
	dedIntern->pipe.open(scriptfn, pstreams::pstdin|pstreams::pstdout|pstreams::pstderr);
	dedIntern->pipe << "hello world\n" << flush;
	dedIntern->pipeOutputMutex = SDL_CreateMutex();
	dedIntern->pipeThread = SDL_CreateThread(&DedIntern::pipeThreadFunc, NULL);
            
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
void DedicatedControl::Menu_Frame() { DedIntern::Get()->BasicFrame(); }
void DedicatedControl::GameLoop_Frame() {}
