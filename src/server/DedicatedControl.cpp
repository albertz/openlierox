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
#include <SDL_thread.h>

#include "DedicatedControl.h"
#include "LieroX.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "Menu.h"
#include "ProfileSystem.h"
#include "CClient.h"
#include "CServer.h"
#include "CWorm.h"
#include "CGameScript.h"
#include "Unicode.h"
#include "Protocol.h"

static DedicatedControl* dedicatedControlInstance = NULL;

DedicatedControl* DedicatedControl::Get() { return dedicatedControlInstance; }

bool DedicatedControl::Init() {
	dedicatedControlInstance = new DedicatedControl();
	return dedicatedControlInstance->Init_priv();
}

void DedicatedControl::Uninit() {
	delete dedicatedControlInstance;
}

#ifdef WIN32

// TODO: find alternative to PStreams for Windows

// Stubs - no dedicated server for Windows
DedicatedControl::DedicatedControl() : internData(NULL) {}
DedicatedControl::~DedicatedControl() {	}

bool DedicatedControl::Init_priv() {
	printf("ERROR: DedicatedControl cannot work as we haven't found an alternative to PStreams for Windows yet\n");
	return false;
}

void DedicatedControl::BackToLobby_Signal() {}
void DedicatedControl::GameLoopStart_Signal() {}
void DedicatedControl::GameLoopEnd_Signal() {}
void DedicatedControl::Menu_Frame() { }
void DedicatedControl::GameLoop_Frame() {}
void DedicatedControl::NewWorm_Signal(CWorm* w) {}

#else

#include <pstream.h>

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

struct DedIntern {	
	SDL_Thread* pipeThread;
	pstream pipe;
	SDL_mutex* pipeOutputMutex;
	stringstream pipeOutput;
	
	static DedIntern* Get() { return (DedIntern*)dedicatedControlInstance->internData; }
	DedIntern() : state(S_NORMAL) {}
	~DedIntern() {
		Sig_Quit();
		SDL_WaitThread(pipeThread, NULL);
		SDL_DestroyMutex(pipeOutputMutex);
	}
	
	// reading lines from pipe-out and put them to pipeOutput
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
	
	// -------------------------------
	// ------- state -----------------
	
	enum State { S_NORMAL, S_LOBBY, S_PLAYING };
	State state;
	
	// --------------------------------
	// ---- commands ------------------
	
	void Cmd_Message(const std::string& msg) {
		cout << "DedicatedControl: message: " << msg << endl;
	}
	
	void Cmd_GetComputerWormList() {
		profile_t *p = GetProfiles();
		for(;p;p=p->tNext) {
			if(p->iType == PRF_COMPUTER)
				pipe << "worm " << p->iID << ", " << p->sName << endl;
		}
		pipe << "endwormlist" << endl;	
	}

	void Cmd_AddWorm() {
	
	}
	
	void Cmd_KickWorm() {
	
	}
	
	void Cmd_StartLobby() {
		tGameInfo.iNumPlayers = 0; // for now...
		//tGameInfo.cPlayers[count++] = FindProfile(wormid);
		
		tGameInfo.sServername = "dedicated server";
		tGameInfo.sWelcomeMessage = "hello";
	
		tLXOptions->tGameinfo.iMaxPlayers = 8;
		tLXOptions->tGameinfo.iMaxPlayers = MAX(tLXOptions->tGameinfo.iMaxPlayers,2);
		tLXOptions->tGameinfo.iMaxPlayers = MIN(tLXOptions->tGameinfo.iMaxPlayers,MAX_PLAYERS);
		tLXOptions->tGameinfo.bRegServer = true;
		tLXOptions->tGameinfo.bAllowWantsJoinMsg = true;
		tLXOptions->tGameinfo.bWantsJoinBanned = false;
		tLXOptions->tGameinfo.bAllowRemoteBots = true;
		tLXOptions->tGameinfo.bAllowNickChange = false;
		tLXOptions->bServerSideHealth = false;
	
		tLXOptions->tGameinfo.bAllowConnectDuringGame = false;
		tLXOptions->tGameinfo.iAllowConnectDuringGameLives = 10;
		tLXOptions->tGameinfo.iAllowConnectDuringGameLivesMin = 10;
	
		tGameInfo.iGameType = GME_HOST;
		
		// Fill in some game details
		tGameInfo.iLoadingTimes = tLXOptions->tGameinfo.iLoadingTime;
		tGameInfo.iLives = tLXOptions->tGameinfo.iLives;
		tGameInfo.iKillLimit = tLXOptions->tGameinfo.iKillLimit;
		tGameInfo.bBonusesOn = tLXOptions->tGameinfo.bBonusesOn;
		tGameInfo.bShowBonusName = tLXOptions->tGameinfo.bShowBonusName;
		tGameInfo.iGameMode = tLXOptions->tGameinfo.nGameType;
		tGameInfo.fTimeLimit = tLXOptions->tGameinfo.fTimeLimit = 10;
		
		cClient->Shutdown();
		cClient->Clear();
		
		// Start the server
		if(!cServer->StartServer( tGameInfo.sServername, tLXOptions->iNetworkPort, tLXOptions->tGameinfo.iMaxPlayers, tLXOptions->tGameinfo.bRegServer )) {
			// Crappy
			printf("ERROR: Server wouldn't start\n");
			Sig_ErrorStartLobby();
			return;
		}
	
		// Lets connect me to the server
		if(!cClient->Initialize()) {
			// Crappy
			printf("ERROR: Client wouldn't initialize\n");
			Sig_ErrorStartLobby();
			return;
		}
	
		cClient->Connect("127.0.0.1");
	
		// Set up the server's lobby details
		game_lobby_t *gl = cServer->getLobby();
		gl->bSet = true;
		gl->nGameMode = tLXOptions->tGameinfo.nGameType;
		gl->nLives = tLXOptions->tGameinfo.iLives;
		gl->nMaxKills = tLXOptions->tGameinfo.iKillLimit;
		gl->nLoadingTime = tLXOptions->tGameinfo.iLoadingTime;
		gl->bBonuses = tLXOptions->tGameinfo.bBonusesOn;
		
		Sig_LobbyStarted();
	}

	void Cmd_StartGame() {
		tGameInfo.sModDir = "MW 1.0";
		if(!CGameScript::CheckFile(tGameInfo.sModDir, tGameInfo.sModName)) {
			cout << "ERROR: no mod for dedicated" << endl;
			// TODO..
		}
		tLXOptions->tGameinfo.szModName = tGameInfo.sModDir;

		// Get the game type
		tGameInfo.iGameMode = GMT_DEATHMATCH;
		tLXOptions->tGameinfo.nGameType = tGameInfo.iGameMode;

		tLXOptions->tGameinfo.sMapFilename = "CastleStrike.lxl";
		tGameInfo.sMapFile = tLXOptions->tGameinfo.sMapFilename;
		tGameInfo.sMapName = Menu_GetLevelName(tGameInfo.sMapFile);

		// Start the game
		cServer->StartGame( true );	// start in dedicated mode

		// Leave the frontend
		*bGame = true;
		tMenu->bMenuRunning = false;
		tGameInfo.iGameType = GME_HOST;
	}

	void Cmd_ChatMessage(const std::string& msg, int type = TXT_NOTICE) {
		cServer->SendGlobalText(OldLxCompatibleString(msg), type);	
	}

	void HandleCommand(const std::string& cmd_, const std::string& params) {
		std::string cmd = cmd_; stringlwr(cmd); TrimSpaces(cmd);
		if(cmd == "") return;
		
#ifdef DEBUG
		cout << "DedicatedControl: exec: " << cmd << " " << params << endl;		
#endif
		if(cmd == "msg")
			Cmd_Message(params);
		else if(cmd == "chatmsg")
			Cmd_ChatMessage(params);
		else if(cmd == "startlobby")
			Cmd_StartLobby();
		else if(cmd == "addworm")
			Cmd_AddWorm();
		else if(cmd == "kickworm")
			Cmd_KickWorm();
		else if(cmd == "startgame")
			Cmd_StartGame();
		else if(cmd == "getcomputerwormlist")
			Cmd_GetComputerWormList();
		else
			cout << "DedicatedControl: unknown command: " << cmd << " " << params << endl;
	}

	// ----------------------------------
	// ----------- signals --------------
	
	void Sig_GameLoopStart() { pipe << "gameloopstart" << endl; state = S_PLAYING; }
	void Sig_BackToLobby() { pipe << "backtolobby" << endl; state = S_LOBBY; }
	void Sig_GameLoopEnd() { pipe << "gameloopend" << endl; state = S_NORMAL; }
	void Sig_ErrorStartLobby() { pipe << "errorstartlobby" << endl; state = S_NORMAL; }
	void Sig_LobbyStarted() { pipe << "lobbystarted" << endl; state = S_LOBBY; }	
	void Sig_NewWorm(CWorm* w) { pipe << "newworm" << endl; }	
	void Sig_Quit() { pipe << "quit" << peof; }
	
	// ----------------------------------
	// ---------- frame handlers --------
	
	void Frame_Lobby() {
		// Process the server & client frames
		cServer->Frame();
		cClient->Frame();
	}
	
	void Frame_Playing() {
		// we don't have to process server/client frames here as it is done already by the main loop
		
	}
	
	void Frame_Basic() {
		SDL_mutexP(pipeOutputMutex);
		while(pipeOutput.rdbuf()->in_avail() > 0) {
			string cmd, rest;
			Ded_ParseCommand(pipeOutput, cmd, rest);
			SDL_mutexV(pipeOutputMutex);
			
			HandleCommand(cmd, rest);
			SDL_mutexP(pipeOutputMutex);
		}
		SDL_mutexV(pipeOutputMutex);
		
		switch(state) {
		case S_LOBBY: Frame_Lobby(); break;
		case S_PLAYING: Frame_Playing(); break;
		default: break;
		}
	}
};

DedicatedControl::DedicatedControl() : internData(NULL) {}
DedicatedControl::~DedicatedControl() {	if(internData) delete (DedIntern*)internData; internData = NULL; }

bool DedicatedControl::Init_priv() {
	const std::string scriptfn_rel = "scripts/dedicated_control";

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

void DedicatedControl::BackToLobby_Signal() { DedIntern::Get()->Sig_BackToLobby(); }
void DedicatedControl::GameLoopStart_Signal() { DedIntern::Get()->Sig_GameLoopStart(); }
void DedicatedControl::GameLoopEnd_Signal() { DedIntern::Get()->Sig_GameLoopEnd(); }
void DedicatedControl::Menu_Frame() { DedIntern::Get()->Frame_Basic(); }
void DedicatedControl::GameLoop_Frame() { DedIntern::Get()->Frame_Basic(); }
void DedicatedControl::NewWorm_Signal(CWorm* w) { DedIntern::Get()->Sig_NewWorm(w); }

#endif
