/*
 *  DedicatedControl.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 11.01.08.
 *  code under LGPL
 *
 */

// define HAVE_BOOST if you want to compile dedicated server for Win32 and have Boost headers installed.

// A workaround for a bug in boost - doesn't compile with DEBUG_NEW in MSVC 2005
#if defined(_MSC_VER) && _MSC_VER == 1400
#undef new
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <SDL_thread.h>
#include <fcntl.h>


#include "LieroX.h"
#include "IpToCountryDB.h"
#include "DedicatedControl.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "DeprecatedGUI/Menu.h"
#include "ProfileSystem.h"
#include "CClient.h"
#include "CServer.h"
#include "CWorm.h"
#include "CGameScript.h"
#include "Unicode.h"
#include "Protocol.h"
#include "CScriptableVars.h"
#include "CClientNetEngine.h"
#include "CChannel.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"


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
	std::ostream & in() { if (p) return p->get_stdin(); static ostringstream os; return os; };
	std::istream & out() { if (p) return p->get_stdout(); static istringstream is; return is; };
	void close_in() { if (p) { p->get_stdin().close(); } };
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
		ctx.m_stderr_behavior = boost::process::close_stream(); // we don't grap the stderr, it is not outputted anywhere, sadly
		try
		{
			p = new boost::process::child(boost::process::launch(cmd, params, ctx)); // Throws exception on error
		}
		catch( const std::exception & e )
		{
			printf("Error running command %s: %s\n", cmd.c_str(), e.what());
			return false;
		};
		return true;
	};
	~pstream_pipe_t(){ close(); if(p) delete p; };
};

#else
#include <pstream.h>
struct pstream_pipe_t
{
	redi::pstream p;
	std::ostream & in() { return p; };
	std::istream & out() { return p.out(); };
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

	DedIntern() : pipeThread(NULL), stdinThread(NULL),
		pipeOutputMutex(NULL), quitSignal(false), state(S_INACTIVE) { }
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
		S_INACTIVE, // server was not started
		S_LOBBY, // in lobby
		S_PREPARING, // in game: just started, will go to S_WEAPONS
		S_WEAPONS, // in game: in weapon selection
		S_PLAYING // in game: playing
		};
	State state;

	// TODO: Move this?
	CWorm* CheckWorm(int id, string caller)
	{
		if(id <0 || id >= MAX_WORMS)
		{
			std::cout << "DedicatedControl: " << caller << " : Faulty ID (got " << id << ")" << std::endl;
			return NULL;
		}
		CWorm *w = cServer->getWorms() + id;
		if(!w->isUsed())
		{
			std::cout << "DedicatedControl: " << caller << " : ID not in use" << std::endl;
			return NULL;
		}
		return w;
	}
	
	// --------------------------------
	// ---- commands ------------------

	void Cmd_Quit() {
		*DeprecatedGUI::bGame = false; // this means if we were in menu => quit
		DeprecatedGUI::tMenu->bMenuRunning = false; // if we were in menu, quit menu

		tLX->bQuitGame = true; // quit main-main-loop
		SetQuitEngineFlag("DedicatedControl::Cmd_Quit()"); // quit main-game-loop
	}

	void Cmd_Message(const std::string& msg) {
		cout << "DedicatedControl: message: " << msg << endl;
	}

	// adds a worm to the game (By string - id is way to complicated)
	void Cmd_AddBot(const std::string & params)
	{
		// Default botname
		// New variable so that we won't break const when we trim spaces.
		std::string localWorm = "[CPU] Kamikazee!";
		if (params != "")
		{
			localWorm = params;
			TrimSpaces(localWorm);
		}
		
		// try to find the requested worm or find any other worm
		profile_t *p = FindProfile(localWorm);
		if(!p) p = GetProfiles();
		for(;p;p=p->tNext)
		{
			if(p->iType == PRF_COMPUTER)
			{
				// we found a bot, so add it
				if( cClient->getNumWorms() >= MAX_WORMS )
				{
					cout << "ERROR: Too many worms!" << endl;
					return;
				}
				cClient->getLocalWormProfiles()[cClient->getNumWorms()] = p;
				cClient->setNumWorms(cClient->getNumWorms()+1);
				
				// TODO: this is really hacky, but currently there is no better way to do so
				// TODO: we need some function in the client + net protocol to allow adding/removing a worm to a client on-the-fly
				//cClient->ReinitLocalWorms();
				cClient->Connect("127.0.0.1");
				
				return;
			}
		}
		
		cout << "ERROR: Can't find ANY bot!" << endl;
		return;
	}

	void Cmd_KillBots(const std::string & params) {
		for( int f=0; f<cClient->getNumWorms(); f++ )
			if( cClient->getWorm(f)->getType() == PRF_COMPUTER )
			{
				cServer->getWorms()[cClient->getWorm(f)->getID()].setLives(0);
				cClient->getNetEngine()->SendDeath(cClient->getWorm(f)->getID(), cClient->getWorm(f)->getID());
			}
	}

	// Kick and ban will both function using ID
	// It's up to the control-program to supply the ID
	// - if it sends a string atoi will fail at converting it to something sensible
	void Cmd_KickWorm(const std::string & params)
	{
		std::string reason = "";
		int id = -1;
		std::vector<std::string> sSplit = explode(params," ");

		if (sSplit.size() == 1)
			id = atoi(params);
		else if (sSplit.size() >= 2)
		{
			id = atoi(sSplit[0]);
			for(std::vector<std::string>::iterator it = sSplit.begin();it != sSplit.end(); it++)
			{
				if(it == sSplit.begin())
					continue;
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

		if(!CheckWorm(id, "KickWorm"))
			return;

		cServer->kickWorm(id,reason);
	}

	void Cmd_BanWorm(const std::string & params)
	{
		std::string reason = "";
		int id = -1;
		std::vector<std::string> sSplit = explode(params," ");

		if (sSplit.size() == 1)
			id = atoi(params);
		else if (sSplit.size() >= 2)
		{
			id = atoi(sSplit[0]);
			for(std::vector<std::string>::iterator it = sSplit.begin();it != sSplit.end(); it++)
			{
				if(it == sSplit.begin())
					continue;
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
		if(!CheckWorm(id, "BanWorm"))
			return;

		cServer->banWorm(id,reason);

	}

	// TODO: Add name muting, if wanted.
	void Cmd_MuteWorm(const std::string & params)
	{
		int id = -1;
		id = atoi(params);
		if(!CheckWorm(id, "MuteWorm"))
			return;

		cServer->muteWorm(id);
	}

	void Cmd_SetWormTeam(const std::string & params)
	{
		//TODO: Is this correct? Does atoi only catch the first number sequence?
		int id = -1;
		id = atoi(params);
		int team = -1;
		if( params.find(" ") != std::string::npos )
			team = atoi( params.substr( params.find(" ")+1 ) );


		CWorm *w = CheckWorm(id,"SetWormTeam");
		if (!w)
			return;

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

	void Cmd_AuthorizeWorm(const std::string & params)
	{
		int id = -1;
		id = atoi(params);
		if(!CheckWorm(id, "AuthorizeWorm"))
			return;

		cServer->authorizeWorm(id);
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
			cout << "\nFor Python ded control script:\n" << endl;
			for( CScriptableVars::iterator it = CScriptableVars::begin(); it != CScriptableVars::end(); it++ )
			{
				cout << "setvar( \"" << it->first << "\", ";
				if( it->second.type == CScriptableVars::SVT_BOOL )
					cout << (int) * it->second.b;
				else if( it->second.type == CScriptableVars::SVT_INT )
					cout << * it->second.i;
				else if( it->second.type == CScriptableVars::SVT_FLOAT )
					cout << * it->second.f;
				else if( it->second.type == CScriptableVars::SVT_STRING )
					cout << "\"" << * it->second.s << "\"";
				else if( it->second.type == CScriptableVars::SVT_COLOR )
					cout << "0x" << hex << * it->second.cl << dec;
				else
					cout << "\"\"";
				cout << ")" << endl;
			}
			return;
		};
		CScriptableVars::SetVarByString(varptr, value);
		
		cout << "DedicatedControl: SetVar " << var << " = " << value << endl;

		cServer->UpdateGameLobby();
	}

	void Cmd_StartLobby(std::string param) {
	
		if( param != "" )
		{
			int port = atoi(param);
			if( port )
				tLXOptions->iNetworkPort = port;
		};

		tLXOptions->tGameInfo.iMaxPlayers = MAX(tLXOptions->tGameInfo.iMaxPlayers,2);
		tLXOptions->tGameInfo.iMaxPlayers = MIN(tLXOptions->tGameInfo.iMaxPlayers,MAX_PLAYERS);

		tLX->iGameType = GME_HOST;

		cClient->Shutdown();
		cClient->Clear();

		// Start the server
		if(!cServer->StartServer()) {
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

		tLXOptions->tGameInfo.sModDir = "MW 1.0";
		if(!CGameScript::CheckFile(tLXOptions->tGameInfo.sModDir, tLXOptions->tGameInfo.sModName)) {
			cout << "ERROR: no mod for dedicated" << endl;
			// TODO..
		}

		// Get the game type
		tLXOptions->tGameInfo.iGameMode = GMT_DEATHMATCH;

		tLXOptions->tGameInfo.sMapFile = "CastleStrike.lxl";
		tLXOptions->tGameInfo.sMapName = DeprecatedGUI::Menu_GetLevelName(tLXOptions->tGameInfo.sMapFile);

		Sig_LobbyStarted();
	}

	void Cmd_StartGame() {
		if(cServer->getNumPlayers() <= 1) {
			cout << "DedControl: cannot start game, too few players" << endl;
			Sig_ErrorStartGame();
			return;
		}

		// Start the game
		cClient->setSpectate(false); // don't spectate; if we have added some players like bots, use them
		cServer->StartGame();	// start in dedicated mode

		// Leave the frontend
		*DeprecatedGUI::bGame = true;
		DeprecatedGUI::tMenu->bMenuRunning = false;
		tLX->iGameType = GME_HOST;
	}

	void Cmd_GotoLobby()
	{
		cServer->gotoLobby();
		*DeprecatedGUI::bGame = false;
		DeprecatedGUI::tMenu->bMenuRunning = true;
	};

	void Cmd_ChatMessage(const std::string& msg, int type = TXT_NOTICE) {
		cServer->SendGlobalText(msg, type);
	}

	void Cmd_PrivateMessage(const std::string& params, int type = TXT_NOTICE) {
		int id = -1;
		id = atoi(params);
		CWorm *w = CheckWorm(id, "PrivateMessage");
		if( !w || ! w->getClient() )
			return;

		std::string msg;
		if( params.find(" ") != std::string::npos )
			msg = params.substr( params.find(" ")+1 );
		
		w->getClient()->getNetEngine()->SendText(msg, type);
	}

	// TODO: make it send more info. No.
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

	void Cmd_GetWormIp(const std::string& params)
	{
		int id = -1;
		id = atoi(params);
		CWorm* w = CheckWorm(id, "GetWormIp");
		if (!w)
			return;

		// TODO: Perhaps we can cut out the second argument for the signal- but that would lead to the signal being much larger. Is it worth it?
		std::string str_addr;
		NetAddrToString(w->getClient()->getChannel()->getAddress(), str_addr);
		if (str_addr != "")
			Sig_WormIp(w,str_addr);
		else
			std::cout << "DedicatedControl: GetWormIp: str_addr == \"\"" << std::endl;
	}

	void Cmd_GetWormLocationInfo(const std::string& params)
	{
		int id = -1;
		id = atoi(params);
		CWorm* w = CheckWorm(id,"GetWormCountryInfo");
		if (!w)
			return;

		std::string str_addr;
		IpInfo info;

		NetAddrToString(w->getClient()->getChannel()->getAddress(), str_addr);
		if (str_addr != "")
		{
			info = tIpToCountryDB->GetInfoAboutIP(str_addr);
			Sig_WormLocationInfo(w,info.Continent,info.Country,info.CountryShortcut);
		}
		else
			std::cout << "DedicatedControl: GetWormCountryInfo: str_addr == \"\"" << std::endl;

	}

	void Cmd_GetWormPing(const std::string& params)
	{
		int id = -1;
		id = atoi(params);
		CWorm* w = CheckWorm(id, "GetWormPing");
		if (!w)
			return;

		Sig_WormPing(w,w->getClient()->getChannel()->getPing());
	}

	void Cmd_GetWormSkin(const std::string& params)
	{
		int id = -1;
		id = atoi(params);
		CWorm* w = CheckWorm(id, "GetWormSkin");
		if (!w)
			return;

		Sig_WormSkin(w);
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
		else if(cmd == "privatemsg")
			Cmd_PrivateMessage(params);
		else if(cmd == "startlobby")
			Cmd_StartLobby(params);
		else if(cmd == "startgame")
			Cmd_StartGame();
		else if(cmd == "gotolobby")
			Cmd_GotoLobby();

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
			
		else if(cmd == "authorizeworm")
			Cmd_AuthorizeWorm(params);

		else if(cmd =="getwormlist")
			Cmd_GetWormList(params);
		else if(cmd == "getcomputerwormlist")
			Cmd_GetComputerWormList();
		else if(cmd == "getwormip")
			Cmd_GetWormIp(params);
		else if(cmd == "getwormlocationinfo")
			Cmd_GetWormLocationInfo(params);
		else if(cmd == "getwormping")
			Cmd_GetWormPing(params);
		else if(cmd == "getwormskin")
			Cmd_GetWormSkin(params);
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
			state = S_INACTIVE;
	}
	void Sig_WeaponSelections() { pipe.in() << "weaponselections" << endl; state = S_WEAPONS; }
	void Sig_GameStarted() { pipe.in() << "gamestarted" << endl; state = S_PLAYING; }
	void Sig_BackToLobby() { pipe.in() << "backtolobby" << endl; state = S_LOBBY; }
	void Sig_ErrorStartLobby() { pipe.in() << "errorstartlobby" << endl; state = S_INACTIVE; }
	void Sig_ErrorStartGame() { pipe.in() << "errorstartgame" << endl; }
	void Sig_Quit() { pipe.in() << "quit" << endl; pipe.close_in(); state = S_INACTIVE; }

	void Sig_NewWorm(CWorm* w) { pipe.in() << "newworm " << w->getID() << " " << w->getName() << endl; }
	void Sig_WormLeft(CWorm* w) { pipe.in() << "wormleft " << w->getID() << " " << w->getName() << endl; }
	void Sig_WormList(CWorm* w) { pipe.in() << "wormlistinfo " << w->getID() << " " << w->getName() << endl; }
	void Sig_ComputerWormList(profile_t * w) { pipe.in() << "computerwormlistinfo " << w->iID << " " << w->sName << endl; }
	void Sig_EndList() { pipe.in() << "endlist" << endl; }
	void Sig_ChatMessage(CWorm* w, string message) { pipe.in() << "chatmessage " << w->getID() << " " << message << endl; }
	void Sig_PrivateMessage(CWorm* w, CWorm* to, string message) { pipe.in() << "privatemessage " << w->getID() << " " << to->getID() << " " << message << endl; }
	void Sig_WormDied(CWorm* died, CWorm* killer) { pipe.in() << "wormdied " << died->getID() << " " << killer->getID() << endl; }
	void Sig_WormSpawned(CWorm* worm) { pipe.in() << "wormspawned " << worm->getID() << endl; }
	void Sig_WormIp(CWorm* w, string ip) { pipe.in() << "wormip " << w->getID() << " " << ip << endl; }
	// Continents don't have spaces in em.
	// TODO: Bad forward compability. We might get new continents.
	// CountryShortcuts don't have spaces in em.
	// Countries CAN have spacies in em. (United Arab Emirates for example, pro country)
	void Sig_WormLocationInfo(CWorm* w,string continent, string country, string countryShortcut) {
		pipe.in() << "wormlocationinfo " << w->getID() << " " << continent << " " << countryShortcut << " " << country  << endl; }

	void Sig_WormPing(CWorm* w, int ping) {	pipe.in() << "wormping " << w->getID() << " " << ping << endl; }
	void Sig_WormSkin(CWorm* w) { pipe.in() << "wormskin " << w->getID() << " " << w->getSkin().getDefaultColor() << " " << w->getSkin().getFileName() << endl; }

	// TODO: Make other commands for requesting more infos from a worm. Don't spam wormlist.
	// Like some more non-game/lobby specific things (I don't know what i mean by this, perhaps you do?)
	// TODO: Send all kills/deaths/teamkills after each game? Could get ugly real fast thou.



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
	// Interpreter should be with full path specified, or it won't run correctly on Windows
	// so we'll read it's path from Windows registry - executing Windows shell failed last time
	FILE * ff = OpenGameFile(scriptfn, "r");
	char t[128];
	fgets( t, sizeof(t), ff );
	fclose(ff);
	std::string cmdPathRegKey = "";
	std::string cmdPathRegValue = "";
	if( std::string(t).find("python") != std::string::npos )
	{
		command = "python.exe";
		commandArgs.clear();
		commandArgs.push_back(command);
		commandArgs.push_back("-u");
		commandArgs.push_back(scriptfn);
		cmdPathRegKey = "SOFTWARE\\Python\\PythonCore\\2.5\\InstallPath";
	}
	else if( std::string(t).find("bash") != std::string::npos )
	{
		command = "bash.exe";
		commandArgs.clear();
		commandArgs.push_back(command);
		//commandArgs.push_back("-l");	// Not needed for Cygwin
		commandArgs.push_back("-c");
		commandArgs.push_back(scriptfn);
		cmdPathRegKey = "SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\/usr/bin";
		cmdPathRegValue = "native";
	}
	else if( std::string(t).find("php") != std::string::npos )
	{
		command = "php.exe";
		commandArgs.clear();
		commandArgs.push_back(command);
		commandArgs.push_back("-f");
		commandArgs.push_back(scriptfn);
		cmdPathRegKey = "SOFTWARE\\PHP";
		cmdPathRegValue = "InstallDir";
	}
	else
	{
		printf("ERROR: scripts/dedicated_control file should be Python or Bash or PHP script, ask devs to add other interpreters for Windows build\n");
		return false;
	}

	if( cmdPathRegKey != "" )
	{
		HKEY hKey;
		LONG returnStatus;
		DWORD dwType=REG_SZ;
		char lszCmdPath[256]="";
		DWORD dwSize=255;
		returnStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cmdPathRegKey.c_str(), 0L,  KEY_READ, &hKey);
		if (returnStatus != ERROR_SUCCESS)
		{
			printf("ERROR: registry key %s not found - make sure interpreter is installed\n", ( cmdPathRegKey + "\\" + cmdPathRegValue ).c_str());
			lszCmdPath[0] = '\0'; // Perhaps it is installed in PATH
		}
		returnStatus = RegQueryValueEx(hKey, cmdPathRegValue.c_str(), NULL, &dwType,(LPBYTE)lszCmdPath, &dwSize);
		RegCloseKey(hKey);
		if (returnStatus != ERROR_SUCCESS)
		{
			printf( "Error: registry key %s not found - make sure interpreter is installed\n", ( cmdPathRegKey + "\\" + cmdPathRegValue ).c_str());
			lszCmdPath[0] = '\0'; // Perhaps it is installed in PATH
		}

		// Add trailing slash if needed
		std::string path(lszCmdPath);
		if (path.size())  {
			if (*path.rbegin() != '\\' && *path.rbegin() != '/')
				path += '\\';
		}
		command = std::string(lszCmdPath) + command;
		commandArgs[0] = command;
	}
	#endif

	DedIntern* dedIntern = new DedIntern;
	internData = dedIntern;
	printf("Dedicated server: running command \"%s\"\n", command.c_str());
	if( ! dedIntern->pipe.open(command, commandArgs) )
	{
		printf( "ERROR: cannot start dedicated server - cannot run script %s\n", scriptfn.c_str() );
		return false;
	};
	dedIntern->pipe.in() << "init" << endl;
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
void DedicatedControl::ChatMessage_Signal(CWorm* w,const string& message) { DedIntern::Get()->Sig_ChatMessage(w,message); }
void DedicatedControl::PrivateMessage_Signal(CWorm* w, CWorm* to, const string& message) { DedIntern::Get()->Sig_PrivateMessage(w,to,message); }
void DedicatedControl::WormDied_Signal(CWorm* worm, CWorm* killer) { DedIntern::Get()->Sig_WormDied(worm,killer); }
void DedicatedControl::WormSpawned_Signal(CWorm* worm){ DedIntern::Get()->Sig_WormSpawned(worm); };

