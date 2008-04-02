/*
 *  DedicatedControl.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 11.01.08.
 *  code under LGPL
 *
 */

// define HAVE_BOOST if you want to compile dedicated server for Win32 and have Boost headers installed.

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <SDL_thread.h>
#include <fcntl.h>


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
#include "CScriptableVars.h"

#ifdef WIN32
#undef S_NORMAL // TODO: This is a reserved constant under WIN32, rename the constant(s) in this file
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

using namespace std;

struct pstream_pipe_t; // popen-streamed-library independent wrapper (kinda)

#if ( ! defined(HAVE_BOOST) && defined(WIN32) ) || ( defined(_MSC_VER) && (_MSC_VER <= 1200) )

struct pstream_pipe_t	// Stub
{
	int dummy;
	pstream_pipe_t(): dummy(0) {};
	std::ostream & in(){ return cout; };
	std::istream & out(){ return cin; };
	void close_in() {  };
	void close() {  }
	bool open( const std::string & cmd, std::vector< std::string > params = std::vector< std::string > () )
	{
		cout << "ERROR: Dedicated server is not compiled into this version of OpenLieroX" << endl;
		MessageBox( NULL, "ERROR: Dedicated server is not compiled into this version of OpenLieroX", "OpenLieroX", MB_OK );
		return false;
	};
};

#else

#ifdef WIN32
// Install Boost headers for your compiler and #define HAVE_BOOST to compile dedicated server for Win32
// You don't need to link to any lib to compile it, just headers.
#include <boost/process.hpp> // This one header pulls turdy shitload of Boost headers
struct pstream_pipe_t
{
	boost::process::child *p;
	pstream_pipe_t(): p(NULL) {};
	std::ostream & in(){ return p->get_stdin(); };
	std::istream & out(){ return p->get_stdout(); };
	void close_in(){ p->get_stdin().close(); };
	void close() { close_in(); }
	bool open( const std::string & cmd, std::vector< std::string > params = std::vector< std::string > () )
	{
		if(p) 
			delete p;
		if( params.size() == 0 )
			params.push_back(cmd);
		boost::process::context ctx;
		ctx.m_stdin_behavior = boost::process::capture_stream(); // Pipe for win32
		ctx.m_stdout_behavior = boost::process::capture_stream();
		ctx.m_stderr_behavior = boost::process::close_stream(); // we don't grap the stderr, it should directly be forwarded to console
		try
		{
			p = new boost::process::child(boost::process::launch(cmd, params, ctx)); // Throws exception on error
		}
		catch( const std::exception & e )
		{
			return false;
		};
		return true;
	};
	~pstream_pipe_t(){ if(p) delete p; };
};

#else
#include <pstream.h>
struct pstream_pipe_t
{
	redi::pstream p;
	std::ostream & in(){ return p; };
	std::istream & out(){ return p.out(); };
	void close_in(){ p << redi::peof; };
	void close() {
		close_in();
		// TODO: this should close the process that pipeThread will stop
	}
	bool open( const std::string & cmd, std::vector< std::string > params = std::vector< std::string > () )
	{
		if( params.size() == 0 )
			params.push_back(cmd);
		p.open( cmd, params, redi::pstreams::pstdin | redi::pstreams::pstdout ); // we don't grap the stderr, it should directly be forwarded to console
		return p.rdbuf()->error() == 0;
	};
};
#endif
#endif

static DedicatedControl* dedicatedControlInstance = NULL;

DedicatedControl* DedicatedControl::Get() { return dedicatedControlInstance; }

bool DedicatedControl::Init() {
	dedicatedControlInstance = new DedicatedControl();
	return dedicatedControlInstance->Init_priv();
}

void DedicatedControl::Uninit() {
	delete dedicatedControlInstance;
}

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
	SDL_Thread* stdinThread;
	pstream_pipe_t pipe;
	SDL_mutex* pipeOutputMutex;
	stringstream pipeOutput;
	bool quitSignal;
	
	static DedIntern* Get() { return (DedIntern*)dedicatedControlInstance->internData; }
	DedIntern() : quitSignal(false), state(S_NORMAL) {}
	~DedIntern() {
		Sig_Quit();
		quitSignal = true;
		
		printf("waiting for stdinThread ...\n");
		SDL_WaitThread(stdinThread, NULL);
		
		printf("waiting for pipeThread ...\n");
		pipe.close();
		SDL_WaitThread(pipeThread, NULL);
				
		SDL_DestroyMutex(pipeOutputMutex);
		printf("DedicatedControl destroyed\n");
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
	
	// reading lines from stdin and put them to pipeOutput
	static int stdinThreadFunc(void*) {
		DedIntern* data = Get();
		
#ifndef WIN32
		// TODO: there's no fcntl for Windows!
		if(fcntl(0, F_SETFL, O_NONBLOCK) == -1)
#endif
			cout << "ERROR setting standard input into non-blocking mode" << endl;
		
		while(true) {			
			string buf;
			while(true) {
				SDL_Delay(10); // TODO: maxfps here
				if(data->quitSignal) return 0;
				
				char c;

				if(read(0, &c, 1) >= 0) {
					if(c == '\n') break;
					buf += c;
				}
			}
			
			SDL_mutexP(data->pipeOutputMutex);
			data->pipeOutput << buf << endl;
	 		SDL_mutexV(data->pipeOutputMutex);
		}
		return 0;
	}
	
	// -------------------------------
	// ------- state -----------------

	
	enum State {
		S_NORMAL, // server was not started
		S_LOBBY, // in lobby
		S_PREPARING, // in game: just started, will go to S_WEAPONS
		S_WEAPONS, // in game: in weapon selection
		S_PLAYING // in game: playing
		};
	State state;
	
	// --------------------------------
	// ---- commands ------------------
	
	void Cmd_Quit() {
		*bGame = false; // this means if we were in menu => quit
		tMenu->bMenuRunning = false; // if we were in menu, quit menu
		
		tLX->bQuitGame = true; // quit main-main-loop
		tLX->bQuitEngine = true; // quit main-game-loop
	}
	
	void Cmd_Message(const std::string& msg) {
		cout << "DedicatedControl: message: " << msg << endl;
	}
	
	// adds a worm to the game
	void Cmd_AddBot(const std::string & params) {
		// TODO ...
		// also ensure here that we don't add human players
	}

	void Cmd_KillBots(const std::string & params) {
		for( int f=0; f<cClient->getNumWorms(); f++ )
			if( cClient->getWorm(f)->getType() == PRF_COMPUTER )
			{
				cServer->getWorms()[cClient->getWorm(f)->getID()].setLives(0);
				cClient->SendDeath(cClient->getWorm(f)->getID(), cClient->getWorm(f)->getID());
			}
	}
	
	// Kick and ban will both function using ID
	// It's up to the control-program to supply the ID 
	// - if it sends a string atoi will fail at converting it to something sensible
	void Cmd_KickWorm(const std::string & params) 
	{
		std::string reason = "";
		int id = 0;
		std::vector<std::string> sSplit = explode(params," ");

		if (sSplit.size() == 1)
			id = atoi(params);
		else if (sSplit.size() >= 2)
		{
			id = atoi(sSplit[0]);
			for(std::vector<std::string>::iterator it = sSplit.begin();it != sSplit.end(); it++)
			{
				reason += *it;
				if (it+1 != sSplit.end())
					reason += " ";
			}
		}
		else
		{
			std::cout << "DedicatedControl: KickWorm: Wrong syntax" << std::endl;
			return;
		}

		if(id <0 || id >= MAX_WORMS)
		{
			std::cout << "DedicatedControl: KickWorm: Faulty ID" << std::endl;
			return;
		}
		CWorm *w = cServer->getWorms() + id;
		if(!w->isUsed())
		{
			std::cout << "DedicatedControl: KickWorm: ID not in use" << std::endl;
			return;
		}

		cServer->kickWorm(id,reason);	
	}

	void Cmd_BanWorm(const std::string & params)
	{
		std::string reason = "";
		int id = 0;
		std::vector<std::string> sSplit = explode(params," ");

		if (sSplit.size() == 1)
			id = atoi(params);
		else if (sSplit.size() >= 2)
		{
			id = atoi(sSplit[0]);
			for(std::vector<std::string>::iterator it = sSplit.begin();it != sSplit.end(); it++)
			{
				reason += *it;
				if (it+1 != sSplit.end())
					reason += " ";
			}
		}
		else
		{
			std::cout << "DedicatedControl: BanWorm: Wrong syntax" << std::endl;
			return;
		}

		if(id <0 || id >= MAX_WORMS)
		{
			std::cout << "DedicatedControl: BanWorm: Faulty ID" << std::endl;
			return;
		}
		CWorm *w = cServer->getWorms() + id;
		if(!w->isUsed())
		{
			std::cout << "DedicatedControl: BanWorm: ID not in use" << std::endl;
			return;
		}

		cServer->banWorm(id,reason);

	}

	// TODO: Add name muting, if wanted.
	void Cmd_MuteWorm(const std::string & params)
	{
		int id = 0;
		id = atoi(params);

		if(id <0 || id >= MAX_WORMS)
		{
			std::cout << "DedicatedControl: MuteWorm: Faulty ID" << std::endl;
			return;
		}
		CWorm *w = cServer->getWorms() + id;
		if(!w->isUsed())
		{
			std::cout << "DedicatedControl: MuteWorm: ID not in use" << std::endl;
			return;
		}

		cServer->muteWorm(id);
	}

	void Cmd_SetWormTeam(const std::string & params)
	{
		int id = 0;
		id = atoi(params);
		int team = 0;
		if( params.find(" ") != std::string::npos )
			team = atoi( params.substr( params.find(" ")+1 ) );

		if(id <0 || id >= MAX_WORMS)
		{
			std::cout << "DedicatedControl: SetWormTeam: Faulty ID" << std::endl;
			return;
		}
		CWorm *w = cServer->getWorms() + id;
		if(!w->isUsed())
		{
			std::cout << "DedicatedControl: SetWormTeam: ID not in use" << std::endl;
			return;
		}
		if( team < 0 || team > 3 )
		{
			std::cout << "DedicatedControl: SetWormTeam: invalid team number" << std::endl;
			return;
		}
		
		w->getLobby()->iTeam = team; 
		w->setTeam(team);
		cServer->UpdateWorms();
		cServer->SendWormLobbyUpdate();
	}

	// This command just fits here perfectly
	void Cmd_SetVar(const std::string& params) {
		if( params.find(" ") == std::string::npos )
		{
			cout << "DedicatedControl: SetVar: wrong params: " << params << endl;
			return;
		};
		std::string var = params.substr( 0, params.find(" ") );
		std::string value = params.substr( params.find(" ")+1 );
		TrimSpaces( var );
		TrimSpaces( value );
		// Strip quotes if they are
		if( value.size() > 1 )
			if( value[0] == '"' && value[value.size()-1] == '"' )
				value = value.substr( 1, value.size()-2 );
		CScriptableVars::ScriptVarPtr_t varptr = CScriptableVars::GetVar(var);
		if( varptr.b == NULL )
		{
			cout << "DedicatedControl: SetVar: no var with name " << var << endl;
			cout << "Available vars:\n" << CScriptableVars::DumpVars() << endl;
			return;
		};
		CScriptableVars::SetVarByString(varptr, value);
		
		cout << "DedicatedControl: SetVar " << var << " = " << value << endl;
	}
	
	void Cmd_SendLobbyUpdate() {
		// TODO: Temporary hack, we should move game_t and game_lobby_t into GameOptions.
		cServer->getLobby()->nGameMode = tGameInfo.iGameMode;
		cServer->getLobby()->nLives = tGameInfo.iLives;
		cServer->getLobby()->nMaxWorms = tLXOptions->tGameinfo.iMaxPlayers;
		cServer->getLobby()->nMaxKills = tGameInfo.iKillLimit;
		cServer->getLobby()->nLoadingTime = tGameInfo.iLoadingTimes;
		cServer->getLobby()->bBonuses = tGameInfo.bBonusesOn;
		cServer->getLobby()->szMapName = tGameInfo.sMapFile;
		cServer->getLobby()->szDecodedMapName = tGameInfo.sMapName;
		cServer->getLobby()->szModName = tGameInfo.sModName;
		cServer->getLobby()->szModDir = tGameInfo.sModDir;

		cServer->UpdateGameLobby();
	};
	
	void Cmd_StartLobby() {
		tGameInfo.iNumPlayers = 1; // for now...
		tGameInfo.cPlayers[0] = FindProfile("[CPU] Kamikazee!"); // TODO: this is just temporary (adds the first CPU-worm), make this later with Cmd_AddWorm
		
		tGameInfo.sServername = "dedicated server";
		tGameInfo.sWelcomeMessage = "hello";
	
		tLXOptions->tGameinfo.iMaxPlayers = 8;
		tLXOptions->tGameinfo.iMaxPlayers = MAX(tLXOptions->tGameinfo.iMaxPlayers,2);
		tLXOptions->tGameinfo.iMaxPlayers = MIN(tLXOptions->tGameinfo.iMaxPlayers,MAX_PLAYERS);
		//tLXOptions->tGameinfo.bRegServer = false; 
		tLXOptions->tGameinfo.bAllowWantsJoinMsg = true;
		tLXOptions->tGameinfo.bWantsJoinBanned = false;
		tLXOptions->tGameinfo.bAllowRemoteBots = true;
		tLXOptions->tGameinfo.bAllowNickChange = false;
		tLXOptions->bServerSideHealth = false;
	
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
		
		Sig_LobbyStarted();
	}

	void Cmd_StartGame() {
		if(cServer->getNumPlayers() <= 1) {
			cout << "DedControl: cannot start game, too less players" << endl;
			Sig_ErrorStartGame();
			return;
		}
		
		// Start the game
		cClient->setSpectate(false); // don't spectate; if we have added some players like bots, use them
		cServer->StartGame();	// start in dedicated mode

		// Leave the frontend
		*bGame = true;
		tMenu->bMenuRunning = false;
		tGameInfo.iGameType = GME_HOST;
	}

	void Cmd_ChatMessage(const std::string& msg, int type = TXT_NOTICE) {
		cServer->SendGlobalText(OldLxCompatibleString(msg), type);	
	}

	// TODO: make it send more info.
	void Cmd_GetWormList(const std::string& params)
	{
		int id = -1;
		if (!params.empty()) // ID specified
			id = atoi(params);

		CWorm *w = cServer->getWorms();
		for(int i=0; i < MAX_WORMS; i++, w++) 
		{
			if(!w->isUsed())
				continue;

			if (id == -1)
				Sig_WormList(w);
			else if (w->getID() == id)
				Sig_WormList(w);
		}
		Sig_EndList();
	}

	void Cmd_GetComputerWormList() {
		profile_t *p = GetProfiles();
		for(;p;p=p->tNext) {
			if(p->iType == PRF_COMPUTER)
				Sig_ComputerWormList(p);
		}
		Sig_EndList();
	}


	void HandleCommand(const std::string& cmd_, const std::string& params) {
		std::string cmd = cmd_; stringlwr(cmd); TrimSpaces(cmd);
		if(cmd == "") return;
		
#ifdef DEBUG
		cout << "DedicatedControl: exec: " << cmd << " " << params << endl;		
#endif
		if(cmd == "quit")
			Cmd_Quit();
		else if(cmd == "setvar")
			Cmd_SetVar(params);
		else if(cmd == "msg")
			Cmd_Message(params);
		else if(cmd == "chatmsg")
			Cmd_ChatMessage(params);
		else if(cmd == "sendlobbyupdate")
			Cmd_SendLobbyUpdate();
		else if(cmd == "startlobby")
			Cmd_StartLobby();
		else if(cmd == "startgame")
			Cmd_StartGame();

		else if(cmd == "addbot")
			Cmd_AddBot(params);

		else if(cmd == "killbots")
			Cmd_KillBots(params);

		else if(cmd == "kickworm")
			Cmd_KickWorm(params);
		else if(cmd == "banworm")
			Cmd_BanWorm(params);
		else if(cmd == "muteworm")
			Cmd_MuteWorm(params);

		else if(cmd == "setwormteam")
			Cmd_SetWormTeam(params);

		else if(cmd =="getwormlist")
			Cmd_GetWormList(params);
		else if(cmd == "getcomputerwormlist")
			Cmd_GetComputerWormList();
		else
			cout << "DedicatedControl: unknown command: " << cmd << " " << params << endl;
	}

	// ----------------------------------
	// ----------- signals --------------
	
	// Keep up with how THIS recieves signals - "params" are split by space, use same when sending.
	void Sig_LobbyStarted() { pipe.in() << "lobbystarted" << endl; state = S_LOBBY; }
	void Sig_GameLoopStart() { pipe.in() << "gameloopstart" << endl; state = S_PREPARING; }
	void Sig_GameLoopEnd() {
		pipe.in() << "gameloopend" << endl;
		if(state != S_LOBBY) // this is because of the current game logic, it will end the game
								 // loop and then return to the lobby but only in the case if we got a
								 // BackToLobby-signal before; if we didn't get such a signal and
								 // the gameloop was ended, that means that the game was stopped
								 // completely
			state = S_NORMAL;
	}
	void Sig_WeaponSelections() { pipe.in() << "weaponselections" << endl; state = S_WEAPONS; }
	void Sig_GameStarted() { pipe.in() << "gamestarted" << endl; state = S_PLAYING; }
	void Sig_BackToLobby() { pipe.in() << "backtolobby" << endl; state = S_LOBBY; }
	void Sig_ErrorStartLobby() { pipe.in() << "errorstartlobby" << endl; state = S_NORMAL; }
	void Sig_ErrorStartGame() { pipe.in() << "errorstartgame" << endl; }
	void Sig_Quit() { pipe.in() << "quit" << endl; pipe.close_in(); state = S_NORMAL; }

	void Sig_NewWorm(CWorm* w) { pipe.in() << "newworm " << w->getID() << " " << w->getName() << endl; }
	void Sig_WormLeft(CWorm* w) { pipe.in() << "wormleft " << w->getID() << " " << w->getName() << endl; }
	void Sig_WormList(CWorm* w) { pipe.in() << "wormlistinfo " << w->getID() << " " << w->getName() << endl; }
	void Sig_ComputerWormList(profile_t * w) { pipe.in() << "computerwormlistinfo " << w->iID << " " << w->sName << endl; }
	void Sig_EndList() { pipe.in() << "endlist" << endl; }
	
	// TODO: Make other commands for requesting more infos to a worm. Don't spam wormlist.
	// Suggesting IP, country (if turned on?) and more non-game/lobby specific.


	
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
		while(pipeOutput.str().size() > (size_t)pipeOutput.tellg()) {
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

	std::string scriptfn = GetFullFileName(scriptfn_rel);
	if(!IsFileAvailable(scriptfn, true)) {
		printf("ERROR: %s not found\n", scriptfn_rel.c_str());
		return false;		
	}
	std::string command = scriptfn;
	std::vector<std::string> commandArgs( 1, command );
	
	#ifdef WIN32
	// Determine what interpreter to run for this script
	FILE * ff = OpenGameFile(scriptfn, "r");
	char t[128];
	fgets( t, sizeof(t), ff );
	fclose(ff);
	if( std::string(t).find("bash") != std::string::npos )
	{
		command = "msys/bin/bash.exe";
		commandArgs.clear();
		commandArgs.push_back(command);
		commandArgs.push_back("-l");
		commandArgs.push_back("-c");
		commandArgs.push_back(scriptfn);
	}
	else if( std::string(t).find("python") != std::string::npos )
	{
		command = "python/python.exe";
		commandArgs.clear();
		commandArgs.push_back(command);
		commandArgs.push_back("-u");
		commandArgs.push_back(scriptfn);
	}
	#endif
		
	DedIntern* dedIntern = new DedIntern;
	internData = dedIntern;
	if( ! dedIntern->pipe.open(command, commandArgs) )
	{
		cout << "ERROR: cannot start dedicated server - cannot run: " << scriptfn << endl;
		// TODO: print error msg (the reason) here
		return false;
	};
	dedIntern->pipe.in() << "hello world\n" << flush; // just a test
	dedIntern->pipeOutputMutex = SDL_CreateMutex();
	dedIntern->pipeThread = SDL_CreateThread(&DedIntern::pipeThreadFunc, NULL);
	dedIntern->stdinThread = SDL_CreateThread(&DedIntern::stdinThreadFunc, NULL);

	return true;
}

// This is the main game loop, the one that do all the simulation etc.
void DedicatedControl::GameLoopStart_Signal() { DedIntern::Get()->Sig_GameLoopStart(); }
void DedicatedControl::GameLoopEnd_Signal() { DedIntern::Get()->Sig_GameLoopEnd(); }
void DedicatedControl::BackToLobby_Signal() { DedIntern::Get()->Sig_BackToLobby(); }
void DedicatedControl::WeaponSelections_Signal() { DedIntern::Get()->Sig_WeaponSelections(); }
void DedicatedControl::GameStarted_Signal() { DedIntern::Get()->Sig_GameStarted(); }

void DedicatedControl::Menu_Frame() { DedIntern::Get()->Frame_Basic(); }
void DedicatedControl::GameLoop_Frame() { DedIntern::Get()->Frame_Basic(); }
void DedicatedControl::NewWorm_Signal(CWorm* w) { DedIntern::Get()->Sig_NewWorm(w); }
void DedicatedControl::WormLeft_Signal(CWorm* w) { DedIntern::Get()->Sig_WormLeft(w); }

// TODO: these are probably intended to be used for the scripts. though there are not used there atm
// should be fixed before release or we will have no forward-compatibility
static bool register_gameinfo_vars = CScriptableVars::RegisterVars("GameServer.GameInfo")
	( tGameInfo.iGameMode, "iGameMode" )
	( tGameInfo.sModName,  "sModName" )
	( tGameInfo.sMapFile, "sMapFile" )
	( tGameInfo.sMapName, "sMapName" )
	( tGameInfo.sModDir, "sModDir" )
	( tGameInfo.iLoadingTimes, "iLoadingTimes" )
	( tGameInfo.sServername, "sServername" )
	( tGameInfo.sWelcomeMessage, "sWelcomeMessage" )
	( tGameInfo.iLives, "iLives" )
	( tGameInfo.iKillLimit, "iKillLimit" )
	( tGameInfo.fTimeLimit, "fTimeLimit" )
	( tGameInfo.iTagLimit, "iTagLimit" )
	( tGameInfo.bBonusesOn, "bBonusesOn" )
	( tGameInfo.bShowBonusName, "bShowBonusName" )
	( tGameInfo.iNumPlayers, "iNumPlayers" )
	;

