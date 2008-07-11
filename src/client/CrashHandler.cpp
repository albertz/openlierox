/*
	OpenLieroX CrashHandler

	registers a crash handler in the OS and handles the crashes

	code under LGPL
	created 09-07-2008 by Albert Zeyer
*/

#include "CrashHandler.h"
#include "StringUtils.h"
#include "Version.h"
#include "AuxLib.h"
#include "LieroX.h" // for nameThread
#include "StringUtils.h"
#include "CClient.h"
#include "CServer.h"
#include <iostream>

using namespace std;

//
// WIN 32
//

#if defined(_MSC_VER)

#include <DbgHelp.h>
#include <ShlObj.h>
#include "FindFile.h" // for IsFileAvailable and mkdir

// Leak checking
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);

// Crash handling
class CrashHandlerImpl : public CrashHandler {
public:
	CrashHandlerImpl() {
#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

		SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);
		nameThread(-1,"Main game thread");

		cout << "Win32 Exception Filter installed" << endl;
	}


};

void *ReadGameStateForReport(char *buffer, size_t bufsize)
{
	memset(buffer, 0, bufsize);
	try {
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
			} else if (cClient->getStatus() == NET_PLAYING_OLXMOD)  {
				strncat(buffer, "Playing an OLX MOD.\n", bufsize);
			} else {
				strncat(buffer, "Unknown state.\n", bufsize);
			}
		}
		buffer[bufsize - 1] = '\0';
	} catch (...)
	{ return buffer; }

	return buffer;
}

void *ReadGameInfoForReport(char *buffer, size_t bufsize)
{
	memset(buffer, 0, bufsize);
	try  {

		// Game type
		strncat(buffer, "iGameType = ", bufsize);
		switch (tGameInfo.iGameType)  {
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
			strncat(buffer, "UNKNOWN ", bufsize); strncat(buffer, itoa(tGameInfo.iGameType).c_str(), bufsize);
		}
		strncat(buffer, "\n", bufsize);

		// Game mode
		strncat(buffer, "iGameMode = ", bufsize);
		switch (tGameInfo.iGameMode)  {
		case GMT_CTF:
			strncat(buffer, "GMT_CTF", bufsize);
			break;
		case GMT_DEATHMATCH:
			strncat(buffer, "GMT_DEATHMATCH", bufsize);
			break;
		case GMT_DEMOLITION:
			strncat(buffer, "GMT_DEMOLITION", bufsize);
			break;
		case GMT_TAG:
			strncat(buffer, "GMT_TAG", bufsize);
			break;
		case GMT_TEAMCTF:
			strncat(buffer, "GMT_TEAMCTF", bufsize);
			break;
		case GMT_TEAMDEATH:
			strncat(buffer, "GMT_TEAMDEATH", bufsize);
			break;
		case GMT_VIP:
			strncat(buffer, "GMT_VIP", bufsize);
			break;
		default:
			strncat(buffer, "UNKNOWN ", bufsize); strncat(buffer, itoa(tGameInfo.iGameMode).c_str(), bufsize);
		}
		strncat(buffer, "\n", bufsize);

		// Mod name
		strncat(buffer, "sModName = ", bufsize);
		if (tGameInfo.sModName.size())
			strncat(buffer, tGameInfo.sModName.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Map file
		strncat(buffer, "sMapFile = ", bufsize);
		if (tGameInfo.sMapFile.size())
			strncat(buffer, tGameInfo.sMapFile.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Map name
		strncat(buffer, "sMapName = ", bufsize);
		if (tGameInfo.sMapName.size())
			strncat(buffer, tGameInfo.sMapName.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Mod dir
		strncat(buffer, "sModDir = ", bufsize);
		if (tGameInfo.sModDir.size())
			strncat(buffer, tGameInfo.sModDir.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Loading time
		strncat(buffer, "iLoadingTimes = ", bufsize);
		strncat(buffer, itoa(tGameInfo.iLoadingTimes).c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Server name
		strncat(buffer, "sServerName = ", bufsize);
		if (tGameInfo.sServername.size())
			strncat(buffer, tGameInfo.sServername.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Welcome message
		strncat(buffer, "sWelcomeMessage = ", bufsize);
		if (tGameInfo.sWelcomeMessage.size())
			strncat(buffer, tGameInfo.sWelcomeMessage.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Register server
		strncat(buffer, "bRegServer = ", bufsize);
		strncat(buffer, tGameInfo.bRegServer ? "true" : "false", bufsize);
		strncat(buffer, "\n", bufsize);

		// Lives
		strncat(buffer, "iLives = ", bufsize);
		strncat(buffer, itoa(tGameInfo.iLives).c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Max kills
		strncat(buffer, "iKillLimit = ", bufsize);
		strncat(buffer, itoa(tGameInfo.iKillLimit).c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Time limit
		strncat(buffer, "fTimeLimit = ", bufsize);
		strncat(buffer, ftoa(tGameInfo.fTimeLimit).c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Bonuses on
		strncat(buffer, "bBonusesOn = ", bufsize);
		strncat(buffer, tGameInfo.bBonusesOn ? "true" : "false", bufsize);
		strncat(buffer, "\n", bufsize);

		// Bonus names
		strncat(buffer, "bShowBonusName = ", bufsize);
		strncat(buffer, tGameInfo.bShowBonusName ? "true" : "false", bufsize);
		strncat(buffer, "\n", bufsize);

		// Number of players
		strncat(buffer, "iNumPlayers = ", bufsize);
		strncat(buffer, itoa(tGameInfo.iNumPlayers).c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		buffer[bufsize - 1] = '\0';
	} catch (...) {
		return buffer;
	}
	return buffer;
}

///////////////////
// This callback function is called whenever an unhandled exception occurs
LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
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
	strcpy(version, "OpenLieroX/");
	strncat(version, LX_VERSION, sizeof(version));
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

	// Get the path
	char buf[1024];
	if (!SHGetSpecialFolderPath(NULL, buf, CSIDL_PERSONAL, false))  {
		buf[0] = '\0';
		strcpy(buf, "bug_reports");
	} else  {
		size_t len = strnlen(buf, sizeof(buf));
		if (buf[len - 1] != '\\' && buf[len - 1] != '/')
			strncat(buf, "\\OpenLieroX\\bug_reports", sizeof(buf));
		else
			strncat(buf, "OpenLieroX\\bug_reports", sizeof(buf));
		fix_markend(buf);
	}
	CreateDirectory(buf, NULL);
	strncat(buf, "\\", sizeof(buf));

	// Get the file name
	std::string checkname;

	FILE *f = NULL;
	for (int i=1;1;i++)  {
		checkname = std::string(buf) + "report" + itoa(i) + ".dmp";
		f = fopen(checkname.c_str(), "rb");
		if (!f)
			break;
		fclose(f);
	}

	// Open the file
	HANDLE hFile = CreateFile((LPCSTR)checkname.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);


	// Write the minidump
	if (hFile)  {
		MINIDUMP_TYPE type = (MINIDUMP_TYPE)(MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory);
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, type, &eInfo,&iStreams,&cbMiniDump);
	}

	// Close the file
	CloseHandle(hFile);

	// Try to free the cache, it eats a lot of memory
	try {
		cCache.Clear();
	} catch(...) {}

	// Quit SDL
	try  {
		SDL_Quit();
	} catch(...) {}

	// Close all opened files
	fcloseall();


	strncpy(&buf[1], checkname.c_str(), sizeof(buf) - 1);
	buf[0] = '\"';
	strncat(buf, "\"", sizeof(buf));
	fix_markend(buf);

	ShellExecute(NULL,"open","BugReport.exe",buf,NULL,SW_SHOWNORMAL);

	return EXCEPTION_EXECUTE_HANDLER;
}

//
// Linux
//

#elif !defined(WIN32) && !defined(MACOSX)

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

class CrashHandlerImpl : public CrashHandler {
public:
	CrashHandlerImpl() {
		// TODO: check if other crash handlers are present
		// check at least for drkonqi, Appart and bug-buddy
		signal(SIGSEGV, &BugBuddySignalHandler);
		cout << "registered KCrash signal handler" << endl;
	}

	/////////////////////////////////////////////////////////////////////////////
	//
	// Attempts to cleanup and call drkonqi to process the crash
	// (code taken from Lgi (LGPL) and modified)
	//
	static void DrKonqiSignalHandler(int Sig) {
		// Don't get into an infinite loop
		signal(SIGSEGV, SIG_DFL);

		// Our pid
		int MyPid = getpid();
		printf("CrashHandler trigger MyPid=%i\n", MyPid);

		// Fork to run the crash handler
		pid_t Pid = fork();
		if (Pid <= 0)
		{
			// Pass our state down to the crash handler...
			int Args = 0;
			char *Arg[32];
			memset(Arg, 0, sizeof(Arg));
			char SigName[16], PidName[16];

			sprintf(SigName, "%i", Sig);
			sprintf(PidName, "%i", MyPid);

			Arg[Args++] = "drkonqi";
			//Arg[Args++] = "--display";
			//Arg[Args++] = XDisplayString(o.XDisplay());
			Arg[Args++] = "--appname";
			Arg[Args++] = GAMENAME;
			Arg[Args++] = "--programname";
			Arg[Args++] = GAMENAME;
			Arg[Args++] = "--appversion";
			Arg[Args++] = LX_VERSION;
			Arg[Args++] = "--apppath";
			Arg[Args++] = GetAppPath(); // should be save to call
			Arg[Args++] = "--signal";
			Arg[Args++] = SigName;
			Arg[Args++] = "--pid";
			Arg[Args++] = PidName;
			Arg[Args++] = "--bugaddress";
			Arg[Args++] = "openlierox@az2000.de";

			setgid(getgid());
			setuid(getuid());

			execvp("drkonqi", Arg);
		}
		else
		{
			// Wait for child to exit
			waitpid(Pid, NULL, 0);
			_exit(253);
		}
	}

	// Attempts to cleanup and call bug-buddy to process the crash
	static void BugBuddySignalHandler(int Sig) {
		// Don't get into an infinite loop
		signal(SIGSEGV, SIG_DFL);

		// Our pid
		int MyPid = getpid();
		printf("CrashHandler trigger MyPid=%i\n", MyPid);

		// Fork to run the crash handler
		pid_t Pid = fork();
		if (Pid <= 0)
		{
			// Pass our state down to the crash handler...
			int Args = 0;
			char *Arg[32];
			memset(Arg, 0, sizeof(Arg));
			char SigName[16], PidName[16];

			sprintf(SigName, "%i", Sig);
			sprintf(PidName, "%i", MyPid);

			Arg[Args++] = "bug-buddy";
			//Arg[Args++] = "--display";
			//Arg[Args++] = XDisplayString(o.XDisplay());
			Arg[Args++] = "--appname";
			Arg[Args++] = GAMENAME " " LX_VERSION;
			Arg[Args++] = "--pid";
			Arg[Args++] = PidName;
			Arg[Args++] = "--name";
			Arg[Args++] = "Albert Zeyer";
			Arg[Args++] = "--email";
			Arg[Args++] = "openlierox@az2000.de";

			setgid(getgid());
			setuid(getuid());

			execvp("bug-buddy", Arg);
		}
		else
		{
			// Wait for child to exit
			waitpid(Pid, NULL, 0);
			_exit(253);
		}
	}


};


#else
class CrashHandlerImpl : public CrashHandler {
public:
	CrashHandlerImpl() {
		cout << "Dummy CrashHandler implementation" << endl;
	}
};

#endif

CrashHandlerImpl* crashHandlerInstance = NULL;

void CrashHandler::init() {
	if(crashHandlerInstance) {
		cout << "WARNING: CrashHandler tried to init twice" << endl;
		return;
	}
	cout << "Installing CrashHandler .. " << flush;
	crashHandlerInstance = new CrashHandlerImpl();
}

void CrashHandler::uninit() {
	if(crashHandlerInstance) {
		delete crashHandlerInstance;
		crashHandlerInstance = NULL;
	}
}

CrashHandler* CrashHandler::get() {
	return crashHandlerInstance;
}
