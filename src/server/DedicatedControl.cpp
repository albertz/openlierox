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


#ifdef PYTHON_DED_EMBEDDED
#include <Python.h>
#endif

#include <string>
#include <sstream>
#include <stdexcept>
#include "ThreadPool.h"
#include <fcntl.h>
#include <iostream>

#include "Debug.h"
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
#include "Command.h"
#include "Process.h"
#include "Event.h"
#include "CGameMode.h"
#include "Cache.h"
#include "AuxLib.h"

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif


using std::endl;



static DedicatedControl* dedicatedControlInstance = NULL;

DedicatedControl* DedicatedControl::Get() 
{
	return dedicatedControlInstance;
}

bool DedicatedControl::Init() {
	dedicatedControlInstance = new DedicatedControl();
	return dedicatedControlInstance->Init_priv();
}

void DedicatedControl::Uninit() {
	delete dedicatedControlInstance;
	dedicatedControlInstance = NULL;
}



struct ScriptDedInterface : DedInterface {
	Process pipe;
	ThreadPoolItem* thread;
	
#ifdef PYTHON_DED_EMBEDDED
	// The new way, embedded Python
	PyObject * scriptModule;
	PyObject * scriptMainLoop;
	SDL_sem * ScriptSignalHandlerRecursive;
	std::ostringstream inSignals;
	static PyMethodDef DedScriptEngineMethods[3]; // Array for registerging GetSignals() and SendCommand()
	bool usePython;
#endif


	ScriptDedInterface() : thread(NULL) {
#ifdef PYTHON_DED_EMBEDDED
		Py_SetProgramName("python"); // Where to look for Python DLL and standard modules
		Py_Initialize();
		Py_InitModule("OLX", DedScriptEngineMethods);
		//PyEval_InitThreads(); // Python-threading magic stuff, need so OLX won't crash
		scriptModule = NULL;
		scriptMainLoop = NULL;

		ScriptSignalHandlerRecursive = SDL_CreateSemaphore(1);
		usePython = false;
#endif
	}
	
	~ScriptDedInterface() {
		breakCurrentScript();

#ifdef PYTHON_DED_EMBEDDED
		Py_XDECREF(scriptMainLoop);
		Py_XDECREF(scriptModule);
		Py_Finalize();
		usePython = false;
		SDL_DestroySemaphore(ScriptSignalHandlerRecursive);
#endif
	}

	
	void pushReturnArg(const std::string& str) {
		pipeOut() << ":" << str << endl;
	}
	
	void finalizeReturn() {
		pipeOut() << "." << endl;
	}

	void writeMsg(const std::string& str) {
		hints << "Script Dedicated: " << str << endl;
	}

	
	std::ostream& pipeOut() {
#ifdef PYTHON_DED_EMBEDDED
		if( usePython )
			return inSignals;
#endif
		return pipe.in();
	}
	
	void closePipe() {
#ifdef PYTHON_DED_EMBEDDED
		if( usePython )
			return;
#endif
		pipe.close();
	}
	
	bool havePipe() {
#ifdef PYTHON_DED_EMBEDDED
		return usePython;
#endif		
		return thread != NULL;
	}

	
	
	bool breakCurrentScript() {
#ifdef PYTHON_DED_EMBEDDED
		if( usePython )
		{
			Py_XDECREF(scriptMainLoop);
			Py_XDECREF(scriptModule);
			scriptMainLoop = NULL;
			scriptModule = NULL;
			usePython = false;
			return true;
		}
#endif
		if(thread) {
			notes << "waiting for pipeThread ..." << endl;
			pipe.close();
			threadPool->wait(thread, NULL);
		}
		thread = NULL;
		
		return true;
	}
	
	bool loadScript(const std::string& script) 
	{
		breakCurrentScript();
#ifdef PYTHON_DED_EMBEDDED
		FILE * fp = OpenGameFile(script, "r");
		if( fp )
		{
			std::string fpContents = ReadUntil( fp, '\n' );
			fclose(fp);
			if( fpContents.find("python") != std::string::npos && tLXOptions->bDedicatedUseBuiltinPython )
				return loadScript_Python(script);
		}
#endif
		return loadScript_Pipe(script);
	}

	
	bool loadScript_Pipe(const std::string& script) {
		breakCurrentScript();
		
		std::string scriptfn = GetAbsolutePath(GetFullFileName(script));
		if(script != "/dev/null") {
			if(!IsFileAvailable(scriptfn, true)) {
				errors << "Dedicated: " << scriptfn << " not found" << endl;
				return false;
			}			

			notes << "Dedicated server: running script \"" << scriptfn << "\"" << endl;
			// HINT: If a script need this change in his own directory, it is a bug in the script.
			if(!pipe.open(scriptfn, std::vector<std::string>(), ExtractDirectory(scriptfn))) {
				errors << "cannot start dedicated server - cannot run script " << scriptfn << endl;
				return false;
			}

			thread = threadPool->start(&ScriptDedInterface::pipeThreadFunc, this, "Ded pipe watcher");
		}
		else
			notes << "Dedicated server: not running any script" << endl;
		
		return true;
	}

#ifdef PYTHON_DED_EMBEDDED
	// Embed Python into OLX

	static PyObject *
			GetSignal(PyObject *self, PyObject *args) // Get one signal from script
	{
		if(!PyArg_ParseTuple(args, ""))
			return Py_None;

		const std::string & sigs = ((DedIntern *)DedicatedControl::Get()->internData)->inSignals.str();
		std::string sig;
		if( sigs.find('\n') != std::string::npos )
		{
			sig = sigs.substr( 0, sigs.find('\n') );
			((DedIntern *)DedicatedControl::Get()->internData)->inSignals.str( sigs.substr( sigs.find('\n')+1 ) );
		}
		else if( sigs != "" )	// Error, some Sig_XXX() func didn't put '\n' in the end
		{
			warnings << "DedControlIntern::GetSignal(): extra data in inSignals: " << sigs << endl;
			((DedIntern *)DedicatedControl::Get()->internData)->inSignals.str( "" );
		}
			
		PyObject * ret = Py_BuildValue("s", sig.c_str());
		((DedIntern *)DedicatedControl::Get()->internData)->inSignals.str("");
		return ret;
	}

	static PyObject *
			SendCommand(PyObject *self, PyObject *args)
	{
		const char *command = NULL;
		if (!PyArg_ParseTuple(args, "s", &command))
			return Py_None;
			
		std::string cmd, rest;
		std::stringstream os;
		os << command << "\n";
		Ded_ParseCommand(os, cmd, rest);

		((DedIntern *)DedicatedControl::Get()->internData)->HandleCommand(cmd, rest);

		return Py_None;
	}

	// TODO: it should not be needed to always call this function after you put a signal
	void ScriptSignalHandler()
	{
		if(!usePython)
			return;
			
		if( SDL_SemTryWait(ScriptSignalHandlerRecursive) != 0 )
			return;
		
		//PyGILState_STATE gstate;
		//gstate = PyGILState_Ensure();	// Python-threading magic stuff, need so OLX won't crash
		
		while( inSignals.str() != "" ) // If there are signals available for ded script
		{
			PyObject * pArgs = PyTuple_New(0);
			PyObject * pRet = PyObject_CallObject(scriptMainLoop, pArgs);

			if( PyErr_Occurred() )
			{
				notes << "Python exception (in stderr)" << endl;
				PyErr_Print();
				PyErr_Clear();
			}

			Py_XDECREF(pArgs);
			Py_XDECREF(pRet);
		}

		//PyGILState_Release(gstate);	// Python-threading magic stuff, need so OLX won't crash
		
		SDL_SemPost(ScriptSignalHandlerRecursive);
	}
                     

	bool loadScript_Python(const std::string& script) 
	{
		std::string scriptfn = GetAbsolutePath(GetFullFileName(script));
		if(script != "/dev/null") {
			if(!IsFileAvailable(scriptfn, true)) {
				errors << "Dedicated: " << scriptfn << " not found" << endl;
				return false;
			}			

			notes << "Dedicated server: running script \"" << scriptfn << "\" using built-in Python" << endl;
			//PyGILState_STATE gstate;
			//gstate = PyGILState_Ensure();	// Python-threading magic stuff, need so OLX won't crash

			char tmp[1024];
			strcpy(tmp, scriptfn.c_str());
			char * tmp1 = tmp;
			PySys_SetArgv(1, & tmp1 );
			
			FILE * fp = OpenGameFile(scriptfn, "r");
			std::string fpContents = ReadUntil( fp, '\0' );
			fclose(fp);
			
			// This just compiles ded script, not executes it yet
			PyObject * codeObject = Py_CompileString( fpContents.c_str(), scriptfn.c_str(), Py_file_input);
			
			if( codeObject == NULL )
			{
				notes << "Dedicated server: compiling script \"" << scriptfn << "\" failed!" << endl;
				if( PyErr_Occurred() )
				{
					notes << "Python exception (in stderr)" << endl;
					PyErr_Print(); // TODO: prints to stderr, dunno how to fetch string from it
					PyErr_Clear();
				}
				return false;
				//PyGILState_Release(gstate);
			}
			
			// Execute and import the module (reloads it if called second time)
			usePython = true;
			SDL_SemWait( ScriptSignalHandlerRecursive );
			scriptModule = PyImport_ExecCodeModule("dedicated_control", codeObject);
			SDL_SemPost( ScriptSignalHandlerRecursive );
			
			Py_XDECREF(codeObject);
			
			if( scriptModule == NULL )
			{
				notes << "Dedicated server: importing script \"" << scriptfn << "\" failed!" << endl;
				if( PyErr_Occurred() )
				{
					notes << "Python exception (in stderr)" << endl;
					PyErr_Print(); // TODO: prints to stderr, dunno how to fetch string from it
					PyErr_Clear();
				}
				usePython = false;
				//PyGILState_Release(gstate);
				return false;
			}
			
			PyObject * pFunc = PyObject_GetAttrString(scriptModule, "MainLoop");
			if (pFunc && PyCallable_Check(pFunc))
				scriptMainLoop = pFunc;
			else
			{
				notes << "Dedicated server: importing script \"" << scriptfn << "\" failed - no MainLoop() function in module" << endl;
				if( PyErr_Occurred() )
				{
					notes << "Python exception (in stderr)" << endl;
					PyErr_Print(); // TODO: prints to stderr, dunno how to fetch string from it
					PyErr_Clear();
				}
				Py_XDECREF(pFunc);
				Py_XDECREF(scriptModule);
				pFunc = NULL;
				scriptModule = NULL;
				usePython = false;
				//PyGILState_Release(gstate);
				return false;
			}
			//PyGILState_Release(gstate);
		}
		else
			notes << "Dedicated server: not running any script" << endl;
		
		return true;
	}

#else
	void ScriptSignalHandler() {} // stub
#endif	
	
	
	
	
	// Pipe functions
	
	// reading lines from pipe-out and put them to pipeOutput
	static int pipeThreadFunc(void* o) {
		ScriptDedInterface* owner = (ScriptDedInterface*)o;

		while(!owner->pipe.out().eof()) {
			std::string buf;
			getline(owner->pipe.out(), buf);
			
			DedicatedControl::Get()->Execute( Command(owner, buf) );
		}
		return 0;
	}
	
};

struct StdinDedInterface : DedInterface {
	ThreadPoolItem* thread;
	
	StdinDedInterface() {
		thread = threadPool->start(&StdinDedInterface::stdinThreadFunc, this, "Ded stdin watcher");
	}
	
	~StdinDedInterface() {
		notes << "waiting for stdinThread ..." << endl;
		threadPool->wait(thread, NULL);
		thread = NULL;
	}
	
	void pushReturnArg(const std::string& str) {
		notes << "Dedicated return: " << str << endl;
	}
	
	void finalizeReturn() {
		notes << "Dedicated return." << endl;
	}

	void writeMsg(const std::string& str) {
		hints << "STDIN Dedicated: " << str << endl;
	}
	
	
	// reading lines from stdin and put them to pipeOutput
	static int stdinThreadFunc(void* o) {
		StdinDedInterface* owner = (StdinDedInterface*)o;

#ifndef WIN32
		// TODO: there's no fcntl for Windows!
		if(fcntl(0, F_SETFL, O_NONBLOCK) == -1)
#endif
			warnings << "ERROR setting standard input into non-blocking mode" << endl;

		while(true) {
			std::string buf;
			while(true) {
				SDL_Delay(10); // TODO: select() here
				if(tLX->bQuitGame) return 0;

				char c;

				if(read(0, &c, 1) >= 0) {
					if(c == '\n') break;
					// TODO: why is this needed? is that WIN32 only?
					if(c == -52) return 0;  // CTRL-C
					buf += c;
				}
			}

			DedicatedControl::Get()->Execute( Command(owner, buf) );
		}
		return 0;
	}	
};




struct DedIntern {

	static DedIntern* Get() { return (DedIntern*)dedicatedControlInstance->internData; }

	ScriptDedInterface* scriptInterface;
	StdinDedInterface* stdinInterface;
	
	SDL_mutex* pendingCommandsMutex;
	std::list<DedInterface::Command> pendingCommands;
	
	bool quitSignal;
	SDL_mutex* pendingSignalsMutex;
	bool waitingForNextSignal;
	struct Signal {
		std::string name;
		std::list<std::string> args;
		Signal(const std::string& n, const std::list<std::string>& a) : name(n), args(a) {}
	};
	std::list<Signal> pendingSignals;
	
	

	DedIntern() :
		scriptInterface(NULL), stdinInterface(NULL),
		pendingCommandsMutex(NULL), quitSignal(false),
		pendingSignalsMutex(NULL), waitingForNextSignal(false),
		state(S_INACTIVE)
	{
		dedicatedControlInstance->internData = this;
		pendingCommandsMutex = SDL_CreateMutex();
		pendingSignalsMutex = SDL_CreateMutex();

		scriptInterface = new ScriptDedInterface();
		stdinInterface = new StdinDedInterface();
	}
	
	~DedIntern() {
		Sig_Quit();
		quitSignal = true;

		delete stdinInterface; stdinInterface = NULL;
		delete scriptInterface; scriptInterface = NULL;
		
		SDL_DestroyMutex(pendingCommandsMutex);
		SDL_DestroyMutex(pendingSignalsMutex);

		notes << "DedicatedControl destroyed" << endl;
		dedicatedControlInstance->internData = NULL;
	}

	
	void pushSignal(const std::string& name, const std::list<std::string>& args = std::list<std::string>()) {
		if(!scriptInterface->havePipe()) return;
		ScopedLock lock(pendingSignalsMutex);
		if(waitingForNextSignal) {
			if(pendingSignals.size() > 0)
				errors << "Dedicated pending signals queue should be empty when the script is waiting for next signal" << endl;
			scriptInterface->pushReturnArg(name);
			for(std::list<std::string>::const_iterator i = args.begin(); i != args.end(); ++i)
				scriptInterface->pushReturnArg(*i);
			scriptInterface->finalizeReturn();
			waitingForNextSignal = false;
		}
		else {
			pendingSignals.push_back(Signal(name, args));
			if(pendingSignals.size() > 1000) {
				warnings << "Dedicated pending signals queue got too full!" << endl;
				pendingSignals.pop_front();
			}
		}
	}
	
	void pushSignal(const std::string& name, const std::string& arg1) {
		std::list<std::string> args; args.push_back(arg1);
		pushSignal(name, args);
	}
	
	void pushSignal(const std::string& name, const std::string& arg1, const std::string& arg2) {
		std::list<std::string> args; args.push_back(arg1); args.push_back(arg2);
		pushSignal(name, args);
	}
	
	void pushSignal(const std::string& name, const std::string& arg1, const std::string& arg2, const std::string& arg3) {
		std::list<std::string> args; args.push_back(arg1); args.push_back(arg2); args.push_back(arg3);
		pushSignal(name, args);
	}
	
	void Execute(const DedInterface::Command& command) {
		ScopedLock lock(pendingCommandsMutex);
		pendingCommands.push_back(command);
	}

	
	// -------------------------------
	// ------- state -----------------


	enum State {
		S_INACTIVE, // server was not started
		S_SVRLOBBY, // in lobby
		S_SVRPREPARING, // in game: just started, will go to S_SVRWEAPONS
		S_SVRWEAPONS, // in game: in weapon selection
		S_SVRPLAYING, // in game: playing
		S_CLICONNECTING, // client game: connecting right now
		S_CLILOBBY,
		S_CLIPLAYING
		};
	State state;

	// TODO: Move this
	static CWorm* CheckWorm(DedInterface* caller, int id, const std::string& request)
	{
		if(id <0 || id >= MAX_WORMS) {
			caller->writeMsg(request + " : Faulty ID " + itoa(id));
			return NULL;
		}
		CWorm *w = cServer->getWorms() + id;
		if(!w->isUsed()) {
			caller->writeMsg(request + " : ID " + itoa(id) + " not in use");
			return NULL;
		}
		return w;
	}

	// --------------------------------
	// ---- commands ------------------

	void Cmd_Quit(DedInterface* caller) {
		*DeprecatedGUI::bGame = false; // this means if we were in menu => quit
		DeprecatedGUI::tMenu->bMenuRunning = false; // if we were in menu, quit menu

		tLX->bQuitGame = true; // quit main-main-loop
		SetQuitEngineFlag("DedicatedControl::Cmd_Quit()"); // quit main-game-loop
	}

	void Cmd_Message(DedInterface* caller, const std::string& msg) {
		hints << "DedicatedControl: message: " << msg << endl;
	}
	
	void Cmd_Script(DedInterface* caller, const std::string& script) {
		if(IsAbsolutePath(script)) {
			caller->writeMsg("Error: absolute path names are not allowed for script command");
			return;
		}

		if(script.find("..") != std::string::npos) {
			caller->writeMsg("Error: invalid script filename: " + script);
			return;
		}
		
		{
			ScopedLock lock(pendingSignalsMutex);
			waitingForNextSignal = false;
			pendingSignals.clear();
		}

		if(script == "" || script == "/dev/null")
			scriptInterface->loadScript("/dev/null");
		else
			scriptInterface->loadScript("scripts/" + script);
	}

	// adds a worm to the game (By string - id is way to complicated)
	void Cmd_AddBot(DedInterface* caller, const std::string & params)
	{
		if( cClient->getNumWorms() + 1 >= MAX_WORMS ) {
			caller->writeMsg("Too many worms!");
			return;
		}
		
		// try to find the requested worm
		if(params != "") {
			std::string localWorm = params;
			TrimSpaces(localWorm);
			StripQuotes(localWorm);
			
			profile_t *p = FindProfile(localWorm);
			if(p) {
				cClient->AddWorm(p);
				return;
			}
			
			caller->writeMsg("cannot find worm profile " + localWorm + ", using random instead"); 
		}
		
		cClient->AddRandomBot();
	}

	void Cmd_KickBot(DedInterface* caller, const std::string& params) {
		std::string reason = params;
		if(reason == "") reason = "Dedicated command";
		int worm = cServer->getLastBot();
		if(worm < 0) {
			caller->writeMsg("there is no bot on the server");
			return;
		}
		cServer->kickWorm(worm, reason);
	}
	
	void Cmd_KillBots(DedInterface* caller, const std::string & params) {
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
	void Cmd_KickWorm(DedInterface* caller, const std::string & params)
	{
		std::string reason = "";
		int id = -1;
		std::vector<std::string> sSplit = explode(params," ");

		if (sSplit.size() == 1)
			id = atoi(params);
		else if (sSplit.size() >= 2) {
			id = atoi(sSplit[0]);
			for(std::vector<std::string>::iterator it = sSplit.begin();it != sSplit.end(); it++) {
				if(it == sSplit.begin())
					continue;
				reason += *it;
				reason += " ";
			}
			TrimSpaces(reason);
		}
		else {
			caller->writeMsg("KickWorm: Wrong syntax");
			return;
		}

		if(!CheckWorm(caller, id, "KickWorm"))
			return;

		cServer->kickWorm(id,reason);
	}

	void Cmd_BanWorm(DedInterface* caller, const std::string & params)
	{
		std::string reason = "";
		int id = -1;
		std::vector<std::string> sSplit = explode(params," ");

		if (sSplit.size() == 1)
			id = atoi(params);
		else if (sSplit.size() >= 2) {
			id = atoi(sSplit[0]);
			for(std::vector<std::string>::iterator it = sSplit.begin();it != sSplit.end(); it++) {
				if(it == sSplit.begin())
					continue;
				reason += *it;
				if (it+1 != sSplit.end())
					reason += " ";
			}
		}
		else {
			caller->writeMsg("BanWorm: Wrong syntax");
			return;
		}
		if(!CheckWorm(caller, id, "BanWorm"))
			return;

		cServer->banWorm(id,reason);
	}

	// TODO: Add name muting, if wanted.
	void Cmd_MuteWorm(DedInterface* caller, const std::string & params)
	{
		int id = -1;
		id = atoi(params);
		if(!CheckWorm(caller, id, "MuteWorm"))
			return;

		cServer->muteWorm(id);
	}

	void Cmd_SetWormTeam(DedInterface* caller, const std::string & params)
	{
		//TODO: Is this correct? Does atoi only catch the first number sequence?
		int id = -1;
		id = atoi(params);
		int team = -1;
		if( params.find(" ") != std::string::npos )
			team = atoi( params.substr( params.find(" ")+1 ) );


		CWorm *w = CheckWorm(caller, id,"SetWormTeam");
		if (!w)
			return;

		if( team < 0 || team > 3 )
		{
			caller->writeMsg("SetWormTeam: invalid team number");
			return;
		}

		w->getLobby()->iTeam = team;
		w->setTeam(team);
		cServer->UpdateWorms();
		cServer->SendWormLobbyUpdate();
		cServer->RecheckGame();
	}

	void Cmd_AuthorizeWorm(DedInterface* caller, const std::string & params)
	{
		int id = -1;
		id = atoi(params);
		if(!CheckWorm(caller, id, "AuthorizeWorm"))
			return;

		cServer->authorizeWorm(id);
	}

	void Cmd_SetVar(DedInterface* caller, const std::string& params) {
		if( params.find(" ") == std::string::npos ) {
			caller->writeMsg("SetVar: wrong params: " + params);
			return;
		}
		std::string var = params.substr( 0, params.find(" ") );
		std::string value = params.substr( params.find(" ")+1 );
		TrimSpaces( var );
		TrimSpaces( value );
		StripQuotes( value );

		ScriptVarPtr_t varptr = CScriptableVars::GetVar(var);
		if( varptr.b == NULL )
		{
			caller->writeMsg("SetVar: no var with name " + var);
			notes << "Available vars:\n" << CScriptableVars::DumpVars() << endl;
			notes << "\nFor Python ded control script:\n" << endl;
			for( CScriptableVars::const_iterator it = CScriptableVars::begin(); it != CScriptableVars::end(); it++ )
			{
				notes << "setvar( \"" << it->first << "\", ";
				if( it->second.type == SVT_BOOL )
					notes << (int) * it->second.b;
				else if( it->second.type == SVT_INT )
					notes << * it->second.i;
				else if( it->second.type == SVT_FLOAT )
					notes << * it->second.f;
				else if( it->second.type == SVT_STRING )
					notes << "\"" << * it->second.s << "\"";
				else if( it->second.type == SVT_COLOR )
					notes << "0x" << hex(*it->second.cl);
				else
					notes << "\"\"";
				notes << ")" << endl;
			}
			return;
		}

		if( varptr.type == SVT_CALLBACK ) {
			caller->writeMsg("SetVar: callbacks are not allowed");
			// If we want supoort for that, I would suggest a seperated command like "call ...".
			return;
		}
		
		CScriptableVars::SetVarByString(varptr, value);

		//notes << "DedicatedControl: SetVar " << var << " = " << value << endl;

		cServer->UpdateGameLobby();
	}

	void Cmd_GetVar(DedInterface* caller, const std::string& params) {
		if( params.find(" ") != std::string::npos ) {
			caller->writeMsg("GetVar: wrong params: " + params);
			return;
		}
		std::string var = params;
		TrimSpaces( var );
		ScriptVarPtr_t varptr = CScriptableVars::GetVar(var);
		if( varptr.b == NULL ) {
			caller->writeMsg("GetVar: no var with name " + var);
			return;
		}
		
		if( varptr.type == SVT_CALLBACK ) {
			caller->writeMsg("GetVar: callbacks are not allowed");
			// If we want supoort for that, I would suggest a seperated command like "call ...".
			return;
		}
		
		if( varptr.s == &tLXOptions->sServerPassword ) {
			caller->writeMsg("GetVar: this variable is restricted");
			// If you want to check if a worm is authorized, use another function for that.
			return;
		}
		
		caller->pushReturnArg(varptr.toString());
	}
	
	void Cmd_GetFullFileName(DedInterface* caller, std::string param) {
		std::string fn = param;
		TrimSpaces(fn);
		StripQuotes(fn);
		
		caller->pushReturnArg(GetAbsolutePath(GetFullFileName(fn, NULL)));
	}
	
	void Cmd_StartLobby(DedInterface* caller, std::string param) {
		if(state != S_INACTIVE) {
			caller->writeMsg("we cannot start the lobby in current state");
			caller->writeMsg("stop lobby/game if you want to restart it");
			return; // just ignore it and stay in current state
		}
		
		if( param != "" ) {
			int port = atoi(param);
			if( port )
				tLXOptions->iNetworkPort = port;
		}

		tLXOptions->tGameInfo.iMaxPlayers = CLAMP(tLXOptions->tGameInfo.iMaxPlayers, 2, (int)MAX_PLAYERS);

		tLX->iGameType = GME_HOST;

		cClient->Shutdown();
		cClient->Clear();

		// Start the server
		if(!cServer->StartServer()) {
			// Crappy
			caller->writeMsg("ERROR: Server wouldn't start");
			Sig_ErrorStartLobby();
			return;
		}

		// Lets connect me to the server
		if(!cClient->Initialize()) {
			// Crappy
			caller->writeMsg("ERROR: Client wouldn't initialize");
			Sig_ErrorStartLobby();
			return;
		}
		cClient->Connect("127.0.0.1:" + itoa(cServer->getPort()));

		if(tLXOptions->tGameInfo.sModDir == "")
			tLXOptions->tGameInfo.sModDir = "MW 1.0";
		if(!CGameScript::CheckFile(tLXOptions->tGameInfo.sModDir, tLXOptions->tGameInfo.sModName)) {
			caller->writeMsg("no mod for dedicated, " + tLXOptions->tGameInfo.sModDir + " not found");
			// TODO..
		}

		// Get the game type
		if(tLXOptions->tGameInfo.gameMode == NULL)
			tLXOptions->tGameInfo.gameMode = GameMode(GM_DEATHMATCH);

		if(tLXOptions->tGameInfo.sMapFile == "")
			tLXOptions->tGameInfo.sMapFile = "CastleStrike.lxl";
		tLXOptions->tGameInfo.sMapName = DeprecatedGUI::Menu_GetLevelName(tLXOptions->tGameInfo.sMapFile);

		Sig_LobbyStarted();
	}

	void Cmd_StartGame(DedInterface* caller) {
		if(cServer->getNumPlayers() <= 1 && !tLXOptions->tGameInfo.features[FT_AllowEmptyGames]) {
			caller->writeMsg("cannot start game, too few players");
			Sig_ErrorStartGame();
			return;
		}

		// Start the game
		cClient->setSpectate(false); // don't spectate; if we have added some players like bots, use them
		if(!cServer->StartGame()) {
			caller->writeMsg("cannot start game, got some error");
			Sig_ErrorStartGame();
			return;			
		}

		// Leave the frontend
		*DeprecatedGUI::bGame = true;
		DeprecatedGUI::tMenu->bMenuRunning = false;
		tLX->iGameType = GME_HOST;
	}

	void Cmd_Map(DedInterface* caller, const std::string& params) {
		std::string filename = params;
		TrimSpaces(filename);
		StripQuotes(filename);
		
		if(filename == "") {
			caller->writeMsg("specify map filename");
			return;
		}
		
		if(filename.find(".") == std::string::npos)
			filename += ".lxl";
		
		// TODO: search through all levels and match name if we don't have a proper filename
		// Don't do this explicitly though, we should cache the list (and also use it
		// everywhere else where we need it).
		
		tLXOptions->tGameInfo.sMapFile = filename;
		cServer->UpdateGameLobby();
	}
	
	void Cmd_GotoLobby(DedInterface* caller) {
		cServer->gotoLobby();
		*DeprecatedGUI::bGame = false;
		DeprecatedGUI::tMenu->bMenuRunning = true;
	}

	void Cmd_ChatMessage(DedInterface* caller, const std::string& msg, int type = TXT_NOTICE) {
		cServer->SendGlobalText(msg, type);
	}

	void Cmd_PrivateMessage(DedInterface* caller, const std::string& params, int type = TXT_NOTICE) {
		int id = -1;
		id = atoi(params);
		CWorm *w = CheckWorm(caller, id, "PrivateMessage");
		if( !w || !w->getClient() || !w->getClient()->getNetEngine() )
			return;

		std::string msg;
		if( params.find(" ") != std::string::npos )
			msg = params.substr( params.find(" ")+1 );

		w->getClient()->getNetEngine()->SendText(msg, type);
	}

	void Cmd_GetWormList(DedInterface* caller, const std::string& params)
	{
		CWorm *w = cServer->getWorms();
		for(int i=0; i < MAX_WORMS; i++, w++)
		{
			if(!w->isUsed())
				continue;

			caller->pushReturnArg(itoa(w->getID()));
		}
	}

	void Cmd_GetComputerWormList(DedInterface* caller) {
		CWorm *w = cServer->getWorms();
		for(int i = 0; i < MAX_WORMS; i++, w++) {
			if(w->isUsed() && w->getType() == PRF_COMPUTER)
				caller->pushReturnArg(itoa(w->getID()));
		}
	}
	
	void Cmd_GetWormName(DedInterface* caller, const std::string& params) {
		int id = atoi(params);
		CWorm* w = CheckWorm(caller, id, "GetWormName");
		if(!w) return;
		caller->pushReturnArg(w->getName());
	}

	void Cmd_GetWormTeam(DedInterface* caller, const std::string& params) {
		int id = atoi(params);
		CWorm* w = CheckWorm(caller, id, "GetWormTeam");
		if(!w) return;
		caller->pushReturnArg(itoa(w->getTeam()));
	}
	
	void Cmd_GetWormIp(DedInterface* caller, const std::string& params) {
		int id = -1;
		id = atoi(params);
		CWorm* w = CheckWorm(caller, id, "GetWormIp");

		// TODO: Perhaps we can cut out the second argument for the signal- but that would lead to the signal being much larger. Is it worth it?
		std::string str_addr;
		if(w && w->getClient() && w->getClient()->getChannel())
			NetAddrToString(w->getClient()->getChannel()->getAddress(), str_addr);
		if (str_addr != "")
			caller->pushReturnArg(str_addr);
		else
			caller->writeMsg("GetWormIp: str_addr == \"\"");
	}

	void Cmd_GetWormLocationInfo(DedInterface* caller, const std::string& params) {
		int id = -1;
		id = atoi(params);
		CWorm* w = CheckWorm(caller, id,"GetWormCountryInfo");
		if (!w)
		{
			return;
		}

		std::string str_addr;
		IpInfo info;

		NetAddrToString(w->getClient()->getChannel()->getAddress(), str_addr);
		if (str_addr != "")
		{
			info = tIpToCountryDB->GetInfoAboutIP(str_addr);
			caller->pushReturnArg(info.Continent);
			caller->pushReturnArg(info.Country);
			caller->pushReturnArg(info.CountryShortcut);
		}
		else
		{
			caller->writeMsg("GetWormCountryInfo: str_addr == \"\"");
		}
	}

	void Cmd_GetWormPing(DedInterface* caller, const std::string& params) {
		int id = -1;
		id = atoi(params);
		CWorm* w = CheckWorm(caller, id, "GetWormPing");
		if(!w || !w->getClient() || !w->getClient()->getChannel())
			return;

		caller->pushReturnArg(itoa(w->getClient()->getChannel()->getPing()));
	}

	void Cmd_GetWormSkin(DedInterface* caller, const std::string& params) {
		int id = -1;
		id = atoi(params);
		CWorm* w = CheckWorm(caller, id, "GetWormSkin");
		if (!w)
		{
			caller->pushReturnArg(0);
			caller->pushReturnArg("Default.png");
			return;
		}

		caller->pushReturnArg(itoa(w->getSkin().getDefaultColor()));
		caller->pushReturnArg(w->getSkin().getFileName());
	}

	void Cmd_Connect(DedInterface* caller, const std::string& params) {
		JoinServer(params, params, "");
		// TODO: move to JoinServer
		Sig_Connecting(params);
	}

	void Cmd_DumpGameState(DedInterface* caller, const std::string& params) {
		cServer->DumpGameState();
	}	

	void Cmd_DumpSysState(DedInterface* caller, const std::string& params) {
		hints << "System state:" << endl;
		cServer->DumpGameState();
		// TODO: client game state
		hints << "Free system memory: " << (GetFreeSysMemory() / 1024) << " KB" << endl;
		hints << "Cache size: " << (cCache.GetCacheSize() / 1024) << " KB" << endl;
	}	
	
	void Cmd_SaveConfig(DedInterface* caller) {
		tLXOptions->SaveToDisc();
	}

	void HandleCommand(const DedInterface::Command& command) {
		std::string cmd = command.cmd; TrimSpaces(cmd);
		std::string params;
		size_t f = cmd.find(' ');
		if(f != std::string::npos) {
			params = cmd.substr(f + 1);
			TrimSpaces(params);
			cmd = cmd.substr(0, f);
		}
		stringlwr(cmd);
		if(cmd == "") return;

		// TODO: merge these commands with ingame console commands (Commands.cpp)
		if(cmd == "nextsignal") {
			ScopedLock lock(pendingSignalsMutex);
			if(pendingSignals.size() > 0) {
				Signal sig = pendingSignals.front();
				pendingSignals.pop_front();
				command.sender->pushReturnArg(sig.name);
				for(std::list<std::string>::iterator i = sig.args.begin(); i != sig.args.end(); ++i)
					command.sender->pushReturnArg(*i);
			}
			else {
				// do nothing, we will send the next signal when it arrives
				waitingForNextSignal = true;
				return;
			}
		}
		else if(cmd == "quit")
			Cmd_Quit(command.sender);
		else if(cmd == "saveconfig")
			Cmd_SaveConfig(command.sender);
		else if(cmd == "setvar")
			Cmd_SetVar(command.sender, params);
		else if(cmd == "getvar")
			Cmd_GetVar(command.sender, params);			
		else if(cmd == "script")
			Cmd_Script(command.sender, params);
		else if(cmd == "msg")
			Cmd_Message(command.sender, params);
		else if(cmd == "chatmsg")
			Cmd_ChatMessage(command.sender, params);
		else if(cmd == "privatemsg")
			Cmd_PrivateMessage(command.sender, params);
		else if(cmd == "startlobby")
			Cmd_StartLobby(command.sender, params);
		else if(cmd == "startgame")
			Cmd_StartGame(command.sender);
		else if(cmd == "gotolobby")
			Cmd_GotoLobby(command.sender);
		else if(cmd == "map")
			Cmd_Map(command.sender, params);

		else if(cmd == "addbot")
			Cmd_AddBot(command.sender, params);
		else if(cmd == "kickbot")
			Cmd_KickBot(command.sender, params);
		else if(cmd == "killbots")
			Cmd_KillBots(command.sender, params);

		else if(cmd == "kickworm")
			Cmd_KickWorm(command.sender, params);
		else if(cmd == "banworm")
			Cmd_BanWorm(command.sender, params);
		else if(cmd == "muteworm")
			Cmd_MuteWorm(command.sender, params);

		else if(cmd == "setwormteam")
			Cmd_SetWormTeam(command.sender, params);

		else if(cmd == "authorizeworm")
			Cmd_AuthorizeWorm(command.sender, params);

		else if(cmd =="getwormlist")
			Cmd_GetWormList(command.sender, params);
		else if(cmd == "getcomputerwormlist")
			Cmd_GetComputerWormList(command.sender);
		else if(cmd == "getwormname")
			Cmd_GetWormName(command.sender, params);
		else if(cmd == "getwormteam")
			Cmd_GetWormTeam(command.sender, params);
		else if(cmd == "getwormip")
			Cmd_GetWormIp(command.sender, params);
		else if(cmd == "getwormlocationinfo")
			Cmd_GetWormLocationInfo(command.sender, params);
		else if(cmd == "getwormping")
			Cmd_GetWormPing(command.sender, params);
		else if(cmd == "getwormskin")
			Cmd_GetWormSkin(command.sender, params);
		else if(cmd == "getfullfilename")
			Cmd_GetFullFileName(command.sender, params);

		else if(cmd == "connect")
			Cmd_Connect(command.sender, params);

		else if(cmd == "dumpgamestate")
			Cmd_DumpGameState(command.sender, params);		
		else if(cmd == "dumpgsysstate")
			Cmd_DumpSysState(command.sender, params);		
		
		else if(Cmd_ParseLine(cmd + " " + params)) {}
		else {
			command.sender->writeMsg("unknown command: " + cmd + " " + params);
		}
		
		command.sender->finalizeReturn();
	}

	// ----------------------------------
	// ----------- signals --------------

	void Sig_LobbyStarted() { pushSignal("lobbystarted"); state = S_SVRLOBBY; }
	void Sig_GameLoopStart() { pushSignal("gameloopstart"); state = S_SVRPREPARING; }
	void Sig_GameLoopEnd() {
		pushSignal("gameloopend");
		if(state != S_SVRLOBBY && state != S_CLILOBBY)
			// This is because of the current game logic: It will end the game
			// loop and then return to the lobby but only in the case if we got a
			// BackToLobby-signal before; if we didn't get such a signal and
			// the gameloop was ended, that means that the game was stopped
			// completely.
			state = S_INACTIVE;
	}
	void Sig_WeaponSelections() { pushSignal("weaponselections"); state = S_SVRWEAPONS; }
	void Sig_GameStarted() { pushSignal("gamestarted"); state = S_SVRPLAYING; }
	void Sig_BackToLobby() { pushSignal("backtolobby"); state = S_SVRLOBBY; }
	void Sig_ErrorStartLobby() { pushSignal("errorstartlobby"); state = S_INACTIVE; }
	void Sig_ErrorStartGame() { pushSignal("errorstartgame"); }
	void Sig_Quit() { pushSignal("quit"); scriptInterface->closePipe(); state = S_INACTIVE; }

	void Sig_Connecting(const std::string& addr) { pushSignal("connecting", addr); state = S_CLICONNECTING; }
	void Sig_ConnectError(const std::string& err) { pushSignal("connecterror", err); state = S_INACTIVE; }
	void Sig_Connected() { pushSignal("connected"); state = S_CLILOBBY;  }
	void Sig_ClientError() { pushSignal("clienterror"); state = S_INACTIVE; }
	void Sig_ClientConnectionError(const std::string& err) { pushSignal("connectionerror", err); state = S_INACTIVE; }
	void Sig_ClientGameStarted() { pushSignal("clientgamestarted"); state = S_CLIPLAYING; }
	void Sig_ClientGotoLobby() { pushSignal("clientbacktolobby"); state = S_CLILOBBY; }

	void Sig_NewWorm(CWorm* w) { pushSignal("newworm", itoa(w->getID()), w->getName()); }
	void Sig_WormLeft(CWorm* w) { pushSignal("wormleft", itoa(w->getID()), w->getName()); }
	void Sig_ChatMessage(CWorm* w, const std::string& message) { pushSignal("chatmessage", itoa(w->getID()), message); }
	void Sig_PrivateMessage(CWorm* w, CWorm* to, const std::string& message) { pushSignal("privatemessage", itoa(w->getID()), itoa(to->getID()), message); }
	void Sig_WormDied(CWorm* died, CWorm* killer) { pushSignal("wormdied", itoa(died->getID()), itoa(killer->getID())); }
	void Sig_WormSpawned(CWorm* worm) { pushSignal("wormspawned", itoa(worm->getID())); }
	void Sig_WormGotAdmin(CWorm* w) { pushSignal("wormgotadmin", itoa(w->getID())); }
	void Sig_WormAuthorized(CWorm* w) { pushSignal("wormauthorized", itoa(w->getID())); }

	void Sig_Timer(const std::string& name) { pushSignal("timer", name); }


	// ----------------------------------
	// ---------- frame handlers --------

	void Frame_ServerLobby() {
		// Process the server & client frames
		cServer->Frame();
		cClient->Frame();
	}

	void Frame_Playing() {
		// we don't have to process server/client frames here as it is done already by the main loop
	}

	void Frame_ClientConnecting() {
		cClient->Frame();

		// are we connected?
		if(cClient->getStatus() == NET_CONNECTED) {
			Sig_Connected();
			return;
		}

		// error?
		if(cClient->getBadConnection()) {
			warnings << "Bad connection: " << cClient->getBadConnectionMsg() << endl;
			Sig_ConnectError(cClient->getBadConnectionMsg());
			cClient->Shutdown();
			return;
		}
	}

	void Frame_ClientLobby() {
		// Process the client
		cClient->Frame();

		// If there is a client error, leave
		if(cClient->getClientError()) {
			Sig_ClientError();
			return;
		}

		// If we have started, leave the frontend
		if(cClient->getGameReady()) {
			// Leave the frontend
			*DeprecatedGUI::bGame = true;
			DeprecatedGUI::tMenu->bMenuRunning = false;
			tLX->iGameType = GME_JOIN;
			Sig_ClientGameStarted();
			return;
		}


		// Check if the communication link between us & server is still ok
		if(cClient->getServerError()) {
			warnings << "Client connection error: " << cClient->getServerErrorMsg() << endl;
			Sig_ClientConnectionError(cClient->getServerErrorMsg());
			return;
		}
	}

	void Frame_Basic() {

		// TODO: make this clean!
		// TODO: no static var here
		// TODO: not a single timer, the script should register as much as it want
		// TODO: each timer should be configurable
		static AbsTime lastTimeHandlerCalled = tLX->currentTime;
		if( tLX->currentTime > lastTimeHandlerCalled + 1.0f ) // Call once per second, when no signals pending, TODO: configurable from ded script
		{
			Sig_Timer("second-ticker"); // TODO ...
			lastTimeHandlerCalled = tLX->currentTime;
		}

#ifdef PYTHON_DED_EMBEDDED
		if( ! usePython )
#endif
		{
			SDL_mutexP(pendingCommandsMutex);
			std::list<DedInterface::Command> cmds;
			cmds.swap(pendingCommands);
			SDL_mutexV(pendingCommandsMutex);
			
			while( cmds.size() > 0 ) {
				DedInterface::Command command = cmds.front();
				cmds.pop_front();

				HandleCommand(command);
				command.sender->finishedCommand(command.cmd);
			}
		}

		ProcessEvents();

		// Some debugging stuff
		// TODO: This really much spams my logs. There should be a better way to inform about this.
#if 0
//#if DEBUG
		int fps = GetFPS();
		static AbsTime lastFpsPrint = tLX->currentTime;
		if (tLX->currentTime - lastFpsPrint >= 20.0f)  {
			notes << "Current FPS: " << fps << endl;
			lastFpsPrint = tLX->currentTime;
		}

		static AbsTime lastBandwidthPrint = tLX->currentTime;
		if (tLX->currentTime - lastBandwidthPrint >= 20.0f)  {
			// Upload and download rates
			float up = 0;
			float down = 0;
			
			// Get the rates
			if( tLX->iGameType == GME_JOIN ) {
				if(cClient->getChannel()) {
					down = cClient->getChannel()->getIncomingRate() / 1024.0f;
					up = cClient->getChannel()->getOutgoingRate() / 1024.0f;
				}
			}
			else if( tLX->iGameType == GME_HOST ) {
				down = cServer->GetDownload() / 1024.0f;
				up = cServer->GetUpload() / 1024.0f;
			}

			notes << "Current upload rate: " << up << " kB/s" << endl;
			notes << "Current download rate: " << down << " kB/s" << endl;
			lastBandwidthPrint = tLX->currentTime;
		}
#endif


		switch(state) {
			case S_SVRLOBBY: Frame_ServerLobby(); break;
			case S_SVRPLAYING: Frame_Playing(); break;
			case S_CLICONNECTING: Frame_ClientConnecting(); break;
			case S_CLILOBBY: Frame_ClientLobby(); break;
			case S_CLIPLAYING: Frame_Playing(); break;
			default: break;
		}
	}
};


#ifdef PYTHON_DED_EMBEDDED

PyMethodDef DedIntern::DedScriptEngineMethods[3] = {
		{ "GetSignal",  GetSignal, METH_VARARGS, "Get next signal from OLX"},
		{ "SendCommand",  SendCommand, METH_VARARGS, "Send command to OLX, as string"},
		{NULL, NULL, 0, NULL}        /* Sentinel */
	};
#endif

DedicatedControl::DedicatedControl() : internData(NULL) {}
DedicatedControl::~DedicatedControl() {	if(internData) delete (DedIntern*)internData; internData = NULL; }

bool DedicatedControl::Init_priv() {
	DedIntern* dedIntern = new DedIntern;
	if(tLXOptions->sDedicatedScript != "" && tLXOptions->sDedicatedScript != "/dev/null") {
		if(IsAbsolutePath(tLXOptions->sDedicatedScript)) {
			dedIntern->scriptInterface->loadScript(tLXOptions->sDedicatedScript);
			return true;
		}

		if(strStartsWith(tLXOptions->sDedicatedScript, "scripts/")) { // old clients will use it like that
			return dedIntern->scriptInterface->loadScript(tLXOptions->sDedicatedScript);
			return true;
		}
		
		dedIntern->scriptInterface->loadScript("scripts/" + tLXOptions->sDedicatedScript);
	}
	else
		dedIntern->scriptInterface->loadScript("/dev/null");
	
	return true;
}


// This is the main game loop, the one that do all the simulation etc.
void DedicatedControl::GameLoopStart_Signal() { DedIntern::Get()->Sig_GameLoopStart(); }
void DedicatedControl::GameLoopEnd_Signal() { DedIntern::Get()->Sig_GameLoopEnd(); }
void DedicatedControl::BackToServerLobby_Signal() { DedIntern::Get()->Sig_BackToLobby(); }
void DedicatedControl::BackToClientLobby_Signal() { DedIntern::Get()->Sig_ClientGotoLobby(); }
void DedicatedControl::WeaponSelections_Signal() { DedIntern::Get()->Sig_WeaponSelections(); }
void DedicatedControl::GameStarted_Signal() { DedIntern::Get()->Sig_GameStarted(); }
void DedicatedControl::NewWorm_Signal(CWorm* w) { DedIntern::Get()->Sig_NewWorm(w); }
void DedicatedControl::WormLeft_Signal(CWorm* w) { DedIntern::Get()->Sig_WormLeft(w); }
void DedicatedControl::ChatMessage_Signal(CWorm* w, const std::string& message) { DedIntern::Get()->Sig_ChatMessage(w,message); }
void DedicatedControl::PrivateMessage_Signal(CWorm* w, CWorm* to, const std::string& message) { DedIntern::Get()->Sig_PrivateMessage(w,to,message); }
void DedicatedControl::WormDied_Signal(CWorm* worm, CWorm* killer) { DedIntern::Get()->Sig_WormDied(worm,killer); }
void DedicatedControl::WormSpawned_Signal(CWorm* worm){ DedIntern::Get()->Sig_WormSpawned(worm); };
void DedicatedControl::WormGotAdmin_Signal(CWorm* worm){ DedIntern::Get()->Sig_WormGotAdmin(worm); };
void DedicatedControl::WormAuthorized_Signal(CWorm* worm){ DedIntern::Get()->Sig_WormAuthorized(worm); };

void DedicatedControl::Menu_Frame() { DedIntern::Get()->Frame_Basic(); }
void DedicatedControl::GameLoop_Frame() { DedIntern::Get()->Frame_Basic(); }

void DedicatedControl::Execute(DedInterface::Command cmd) { DedIntern::Get()->Execute(cmd); }
