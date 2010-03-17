/*
 *  Debug.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 01.01.09.
 *  code under LGPL
 *
 */

#include "Debug.h"
#include "StringUtils.h"
#include "CrashHandler.h"
#include "OLXCommand.h"


#include <time.h>

#ifdef WIN32

#include <windows.h>

// TODO implement
bool AmIBeingDebugged() { return false; }

void RaiseDebugger() {
#ifdef DEBUG
#ifndef __MINGW32_VERSION
	// HINT: ignored when not in debugger
	// If it just does nothing then, remove the surrounding #ifdef DEBUG
	// I read about a Win32's IsDebuggerPresent() function, perhaps you should use that one here.
	__asm  { int 3 };
#endif
#endif
}

#else

#if defined(__APPLE__)

#include <cassert>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysctl.h>

// Based on Apple's recommended method as described in
// http://developer.apple.com/qa/qa2004/qa1361.html
bool AmIBeingDebugged()
    // Returns true if the current process is being debugged (either
    // running under the debugger or has a debugger attached post facto).
{
	// Initialize mib, which tells sysctl what info we want.  In this case,
	// we're looking for information about a specific process ID.
	int mib[] =
	{
		CTL_KERN,
		KERN_PROC,
		KERN_PROC_PID,
		getpid()
	};
	
	// Caution: struct kinfo_proc is marked __APPLE_API_UNSTABLE.  The source and
	// binary interfaces may change.
	struct kinfo_proc info;
	size_t info_size = sizeof ( info );
	
	int sysctl_result = sysctl ( mib, sizeof(mib) / sizeof(*mib), &info, &info_size, NULL, 0 );
	if ( sysctl_result != 0 )
		return false;
	
	// This process is being debugged if the P_TRACED flag is set.
	return ( info.kp_proc.p_flag & P_TRACED ) != 0;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>


bool AmIBeingDebugged() {
	// We can look in /proc/self/status for TracerPid.  We are likely used in crash
	// handling, so we are careful not to use the heap or have side effects.
	int status_fd = open("/proc/self/status", O_RDONLY);
	if (status_fd == -1)
		return false;

	// We assume our line will be in the first 1024 characters and that we can
	// read this much all at once.  In practice this will generally be true.
	// This simplifies and speeds up things considerably.
	char buf[1024];

	ssize_t num_read = read(status_fd, buf, sizeof(buf));
	fix_markend(buf);
	close(status_fd);
	if (num_read <= 0) return false;

	const char* searchStr = "TracerPid:\t";
	const char* f = strstr(buf, searchStr);
	if(f == NULL) return false;
	
	// Our pid is 0 without a debugger, assume this for any pid starting with 0.
	f += strlen(searchStr);
	return f < &buf[num_read] && *f != '0';
}

#endif // OSX/LINUX

#include <signal.h>

void RaiseDebugger() {
	if(AmIBeingDebugged()) {
		printf("I am being debugged, raising debugger ...\n");
		if(CrashHandler::get()) CrashHandler::get()->disable();
		// TODO: We need a way to set another ucontext here. (And that should be specified via a parameter
		// to RaiseDebugger().) E.g. when we use this function in the debugger thread, we want to set the
		// ucontext of the main loop thread.
		raise(SIGABRT);
		if(CrashHandler::get()) CrashHandler::get()->enable();
	} else
		printf("I am not being debugged, ignoring debugger raise.\n");
}

#endif

// not for MingW as it seems it has some different dbghelp version (or none at all)
#if defined(WIN32) && !defined(__MINGW32_VERSION)

#include "AuxLib.h" // for Windows.h

#include <dbghelp.h>
#include <shlobj.h>

#include "LieroX.h"
#include "CClient.h"
#include "CServer.h"
#include "DedicatedControl.h"
#include "StringUtils.h"
#include "ConversationLogger.h"
#include "CGameMode.h"
#include "game/Mod.h"
#include "game/Level.h"

#define itoa _itoa

void *ReadGameStateForReport(char *buffer, size_t bufsize)
{
	memset(buffer, 0, bufsize);
	__try {
		if (cClient)  {
			strncat(buffer, "Game state:\n", bufsize);
			if (cClient->getStatus() == NET_CONNECTED)  {
				if (cClient->getGameReady())
					strncat(buffer, "In game, selecting weapons.", bufsize);
				else
					strncat(buffer, "In lobby.", bufsize);
			} else if (cClient->getStatus() == NET_PLAYING)  {
				strncat(buffer, "In game, playing.", bufsize);
			} else if (cClient->getStatus() == NET_CONNECTING)  {
				strncat(buffer, "Connecting to a server.", bufsize);
			} else if (cClient->getStatus() == NET_DISCONNECTED)  {
				strncat(buffer, "Disconnected.\n", bufsize);
			} else {
				strncat(buffer, "Unknown state.\n", bufsize);
			}
		}
		buffer[bufsize - 1] = '\0';
	} __except (EXCEPTION_EXECUTE_HANDLER)
	{ return buffer; }

	return buffer;
}

void *ReadGameInfoForReport(char *buffer, size_t bufsize)
{
	memset(buffer, 0, bufsize);
	if (!tLXOptions || !tLX)
		return buffer;
	char tmp[32];
	__try  {
		// Game type
		strncat(buffer, "iGameType = ", bufsize);
		switch (tLX->iGameType)  {
		case GME_LOCAL:
			strncat(buffer, "GME_LOCAL", bufsize);
			break;
		case GME_HOST:
			strncat(buffer, "GME_HOST", bufsize);
			break;
		case GME_JOIN:
			strncat(buffer, "GME_JOIN", bufsize);
			break;
		default:
			itoa(tLX->iGameType, tmp, 10);
			fix_markend(tmp);
			strncat(buffer, "UNKNOWN ", bufsize); strncat(buffer, tmp, bufsize);
		}
		strncat(buffer, "\n", bufsize);

		// Game mode
		strncat(buffer, "GameMode = ", bufsize);
		//char tmp[16];
		itoa(gameSettings[FT_GameMode].as<GameModeInfo>()->mode->GeneralGameType(), tmp, 10);
		fix_markend(tmp);
		strncat(buffer, tmp, bufsize);
		strncat(buffer, "\n", bufsize);

		// Mod name
		strncat(buffer, "sModName = ", bufsize);
		if (gameSettings[FT_Mod].as<ModInfo>()->name.size())
			strncat(buffer, gameSettings[FT_Mod].as<ModInfo>()->name.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Map file
		strncat(buffer, "sMapFile = ", bufsize);
		if (gameSettings[FT_Map].as<LevelInfo>()->path.size())
			strncat(buffer, gameSettings[FT_Map].as<LevelInfo>()->path.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Map name
		strncat(buffer, "sMapName = ", bufsize);
		if (gameSettings[FT_Map].as<LevelInfo>()->name.size())
			strncat(buffer, gameSettings[FT_Map].as<LevelInfo>()->name.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Mod dir
		strncat(buffer, "sModDir = ", bufsize);
		if (gameSettings[FT_Mod].as<ModInfo>()->path.size())
			strncat(buffer, gameSettings[FT_Mod].as<ModInfo>()->path.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Loading time
		itoa(gameSettings[FT_LoadingTime], tmp, 10);
		fix_markend(tmp);
		strncat(buffer, "iLoadingTimes = ", bufsize);
		strncat(buffer, tmp, bufsize);
		strncat(buffer, "\n", bufsize);

		// Server name
		strncat(buffer, "sServerName = ", bufsize);
		if (tLXOptions->sServerName.size())
			strncat(buffer, tLXOptions->sServerName.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Welcome message
		strncat(buffer, "sWelcomeMessage = ", bufsize);
		if (tLXOptions->sWelcomeMessage.size())
			strncat(buffer, tLXOptions->sWelcomeMessage.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Lives
		itoa(gameSettings[FT_Lives], tmp, 10);
		fix_markend(tmp);
		strncat(buffer, "iLives = ", bufsize);
		strncat(buffer, tmp, bufsize);
		strncat(buffer, "\n", bufsize);

		// Max kills
		itoa(gameSettings[FT_KillLimit], tmp, 10);
		fix_markend(tmp);
		strncat(buffer, "iKillLimit = ", bufsize);
		strncat(buffer, tmp, bufsize);
		strncat(buffer, "\n", bufsize);

		// Time limit
		itoa((int)((float)gameSettings[FT_TimeLimit] * 10), tmp, 10);
		fix_markend(tmp);
		strncat(buffer, "fTimeLimit = ", bufsize);
		strncat(buffer, tmp, bufsize);
		strncat(buffer, "\n", bufsize);

		// Bonuses on
		strncat(buffer, "bBonusesOn = ", bufsize);
		strncat(buffer, gameSettings[FT_Bonuses] ? "true" : "false", bufsize);
		strncat(buffer, "\n", bufsize);

		// Bonus names
		strncat(buffer, "bShowBonusName = ", bufsize);
		strncat(buffer, gameSettings[FT_ShowBonusName] ? "true" : "false", bufsize);
		strncat(buffer, "\n", bufsize);

		// Number of players
		if (cServer)  {
			itoa(cServer->getNumPlayers(), tmp, 10);
			fix_markend(tmp);
			strncat(buffer, "iNumPlayers = ", bufsize);
			strncat(buffer, tmp, bufsize);
			strncat(buffer, "\n", bufsize);
		}

		buffer[bufsize - 1] = '\0';
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return buffer;
	}
	return buffer;
}

// This function also used in CrashHandler.cpp
void OlxWriteCoreDump_Win32(const char* fileName, PEXCEPTION_POINTERS pExInfo )
{
	// MSVC-compatible core dump, GDB cannot read it :(
	// Set the exception info for the minidump
	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = pExInfo;
	eInfo.ClientPointers = FALSE;

	// Set the minidump info
	MINIDUMP_CALLBACK_INFORMATION cbMiniDump;
	cbMiniDump.CallbackRoutine = NULL;
	cbMiniDump.CallbackParam = 0;

	// Additional data
	MINIDUMP_USER_STREAM pExtraInfo[3];

	// Version info
	char version[64];
	strcpy(version, GetFullGameName());
	pExtraInfo[0].Type = LastReservedStream + 1;
	pExtraInfo[0].BufferSize = sizeof(version);
	pExtraInfo[0].Buffer = (void *)&version[0];

	// Current game info
	char game_info[1024];
	pExtraInfo[1].Type = LastReservedStream + 2;
	pExtraInfo[1].BufferSize = sizeof(game_info);
	pExtraInfo[1].Buffer = ReadGameInfoForReport(game_info, sizeof(game_info));

	// Current game state
	char game_state[1024];
	pExtraInfo[2].Type = LastReservedStream + 3;
	pExtraInfo[2].BufferSize = sizeof(game_state);
	pExtraInfo[2].Buffer = ReadGameStateForReport(game_state, sizeof(game_state));

	MINIDUMP_USER_STREAM_INFORMATION iStreams;
	iStreams.UserStreamCount = sizeof(pExtraInfo)/sizeof(MINIDUMP_USER_STREAM);
	iStreams.UserStreamArray = pExtraInfo;

	// Open the file
	HANDLE hFile = CreateFile((LPCSTR)fileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	// Write the minidump
	if (hFile)  {
		MINIDUMP_TYPE type = (MINIDUMP_TYPE)(MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory);
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, type, &eInfo,&iStreams,&cbMiniDump);
	}

	// Close the file
	CloseHandle(hFile);
}

void OlxWriteCoreDump(const char* fileName) 
{
	OlxWriteCoreDump_Win32(fileName, NULL);
}


// all rest cases, except mingw
#elif !defined(__MINGW32_VERSION)

#ifdef GCOREDUMPER
#include <google/coredumper.h>
#endif
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdio>

#if !defined(GCOREDUMPER)
static void GdbWriteCoreDump(const char* fname) {
	// WARNING: this is terribly slow like this
	char gdbparam[1000];
	sprintf(gdbparam,
			"attach %i \n"
			"gcore %s \n"
			"detach \n"
			"quit \n",
			getpid(), fname);
	FILE* p = popen("gdb -q", "w");
	if(p) {
		fprintf(p, "%s", gdbparam);
		fflush(p);
		int status = 0; wait(&status);
		pclose(p);
	}
}
#endif

void OlxWriteCoreDump(const char* file_postfix) {
	char corefile[PATH_MAX + 100];
	if(getcwd(corefile, PATH_MAX) == NULL) strcpy(corefile, "");
	strcat(corefile, "/core.OpenLieroX");
	if(file_postfix) { strcat(corefile, "."); strcat(corefile, file_postfix); }
	printf("writing coredump to %s\n", corefile);
	
	printf("dumping core ... "); fflush(0);
#if defined(GCOREDUMPER)
	WriteCoreDump(corefile);
#else
	GdbWriteCoreDump(corefile);
#endif
	printf("ready\n");
}


// MingW case for coredump, maybe later also others
#else

void OlxWriteCoreDump(const char*) {
	errors << "Coredumping not supported in this version" << endl;
}

#endif

// ----------------- core dump code end -------------


// --------------- backtrace stuff start --------------

#ifndef HAVE_EXECINFO
#	if defined(__linux__)
#		define HAVE_EXECINFO 1
#	elif defined(__DARWIN_VERS_1050)
#		define HAVE_EXECINFO 1
#	else
#		define HAVE_EXECINFO 0
#	endif
#endif

#if HAVE_EXECINFO

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void DumpCallstackPrintf(void* callpnt) {
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	printf("backtrace() returned %d addresses\n", framesC);
	if(callpnt != NULL && framesC > 3) callstack[3] = callpnt; // expected to be called from signal handler
	char** strs = backtrace_symbols(callstack, framesC);
	for(int i = 0; i < framesC; ++i) {
		if(strs[i])
			printf("%s\n", strs[i]);
		else
			break;
	}
	free(strs);
}

void DumpCallstack(const PrintOutFct& printer) {
	void *callstack[128];
	int framesC = backtrace(callstack, sizeof(callstack));
	printer.print("DumpCallstack: " + itoa(framesC) + " addresses:");
	char** strs = backtrace_symbols(callstack, framesC);
	for(int i = 0; i < framesC; ++i) {
		if(strs[i])
			printer.print(std::string(" ") + strs[i] + "\n");
		else
			break;
	}
	free(strs);
}

#elif defined(WIN32)

#include "StackWalker.h"  // Call Luke Stackwalker for help

typedef void (*DbgPrintOutFct) (const std::string&);

// Override the default stackwalker with our own print functions
class PrintStackWalker : public StackWalker  {
private:
	DbgPrintOutFct m_print;
	PrintOutFct *m_print2;

public:
	PrintStackWalker(DbgPrintOutFct fct = NULL) : StackWalker(RetrieveVerbose) { m_print = fct; m_print2 = NULL; }
	PrintStackWalker(PrintOutFct *fct) : StackWalker(RetrieveVerbose) { m_print2 = fct; m_print = NULL; }
	void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion)
	{

	}

	void OnOutput(LPCSTR szText)  
	{
		if (m_print)
			m_print(std::string(szText));
		else if (m_print2)
			m_print2->print(std::string(szText));
		else
			printf(szText);
			
		StackWalker::OnOutput(szText);
	}
};

void DumpCallstackPrintf(void* callpnt) 
{
	PrintStackWalker sw;
	sw.ShowCallstack();
	
}
void DumpCallstack(void (*LineOutFct) (const std::string&)) 
{  
	PrintStackWalker sw(LineOutFct);
	sw.ShowCallstack();
}

void DumpCallstack(const PrintOutFct& printer) {
	PrintStackWalker sw(const_cast<PrintOutFct *>(&printer));
	sw.ShowCallstack();
}

#else

#warning No DumpCallstack implementation for this arch/sys

void DumpCallstackPrintf(void* callpnt) {
	printf("DumpCallstackPrintf not implemented\n");
}

void DumpCallstack(const PrintOutFct& printer) {
	printf("DumpCallstack not implemented\n");
}

#endif

std::string GetLogTimeStamp()
{
	// TODO: please recode this, don't use C-strings!
	char buf[64];
	const time_t unif_time = time(NULL);
	struct tm *t = localtime(&unif_time);
	if (t == NULL)
		return "";
	
	strftime(buf, sizeof(buf), "[%H:%M:%S] ", t);
	fix_markend(buf);
	return std::string(buf);
}

Logger notes(0,2,1000, "n: ");
Logger hints(0,1,100, "H: ");
Logger warnings(0,0,10, "W: ");
Logger errors(-1,-1,1, "E: ");

#include <iostream>
#include <sstream>
#include "ThreadPool.h"
#include "Options.h"
#include "OLXConsole.h"
#include "StringUtils.h"

static SDL_mutex* globalCoutMutex = NULL;

Logger::Logger(int o, int ingame, int callst, const std::string& p)
: minCoutVerb(o), minIngameConVerb(ingame), minCallstackVerb(callst), prefix(p), lastWasNewline(true), mutex(NULL) {
	mutex = SDL_CreateMutex();
	if(!globalCoutMutex)
		globalCoutMutex = SDL_CreateMutex();
}

Logger::~Logger() {
	SDL_DestroyMutex(mutex); mutex = NULL;
	if(globalCoutMutex) {
		SDL_DestroyMutex(globalCoutMutex);
		globalCoutMutex = NULL;
	}
}

void Logger::lock() {
	SDL_mutexP(mutex);
}

void Logger::unlock() {
	SDL_mutexV(mutex);
}

struct CoutPrint : PrintOutFct {
	void print(const std::string& str) const {
		// TODO: We have used std::cout here before but it doesn't seem to work after a while for some reason.
		printf("%s", str.c_str());
	}
};

template<int col>
struct ConPrint : PrintOutFct {
	void print(const std::string& str) const {
		// TODO: Con_AddText adds a line but we only want to add str
		std::string buf = str;
		if(buf.size() > 0 && buf[buf.size()-1] == '\n') buf.erase(buf.size()-1);
		Con_AddText(col, buf, false);
	}
};

// true if last was newline
static bool logger_output(Logger& log, const std::string& buf) {
	bool ret = true;

	std::string prefix = log.prefix;
	if (tLXOptions && tLXOptions->bLogTimestamps)
		prefix = GetLogTimeStamp() + prefix;

	if(!tLXOptions || tLXOptions->iVerbosity >= log.minCoutVerb) {
		SDL_mutexP(globalCoutMutex);
		ret = PrettyPrint(prefix, buf, CoutPrint(), log.lastWasNewline);
		//std::cout.flush();
		SDL_mutexV(globalCoutMutex);
	}
	if(tLXOptions && tLXOptions->iVerbosity >= log.minCallstackVerb) {
		DumpCallstackPrintf();
	}
	if(tLXOptions && Con_IsInited() && tLXOptions->iVerbosity >= log.minIngameConVerb) {
		// the check is a bit hacky (see Con_AddText) but I really dont want to overcomplicate this
		if(!strStartsWith(buf, "Ingame console: ")) {
			// we are not safing explicitly a color in the Logger, thus we try to assume a good color from the verbosity level
			if(log.minIngameConVerb < 0)
				ret = PrettyPrint(prefix, buf, ConPrint<CNC_ERROR>(), log.lastWasNewline);
			else if(log.minIngameConVerb == 0)
				ret = PrettyPrint(prefix, buf, ConPrint<CNC_WARNING>(), log.lastWasNewline);
			else if(log.minIngameConVerb == 1)
				ret = PrettyPrint(prefix, buf, ConPrint<CNC_NOTIFY>(), log.lastWasNewline);
			else if(log.minIngameConVerb < 5)
				ret = PrettyPrint(prefix, buf, ConPrint<CNC_NORMAL>(), log.lastWasNewline);
			else // >=5
				ret = PrettyPrint(prefix, buf, ConPrint<CNC_DEV>(), log.lastWasNewline);
		}
		if(tLXOptions->iVerbosity >= log.minCallstackVerb) {
			DumpCallstack(ConPrint<CNC_DEV>());
		}
	}
	return ret;
}

Logger& Logger::flush() {
	lock();
	lastWasNewline = logger_output(*this, buffer);
	buffer = "";
	unlock();
	return *this;
}
