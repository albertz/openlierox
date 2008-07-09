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
#include <iostream>

using namespace std;

//
// WIN 32
//

#if defined(_MSC_VER)

#include <DbgHelp.h>
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

	// Get the file name
	static std::string checkname;

	FILE *f = NULL;
	for (int i=1;1;i++)  {
		checkname = "./bug_reports/report" + itoa(i) + ".dmp";
		if (!IsFileAvailable(checkname, true))
			break;
	}

	mkdir("bug_reports", 0);

	// Open the file
	HANDLE hFile = CreateFile((LPCSTR)checkname.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);


	// Write the minidump
	if (hFile)
		MiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),hFile,MiniDumpScanMemory,&eInfo,NULL,&cbMiniDump);

	// Close the file
	CloseHandle(hFile);

	// Quit SDL
	SDL_Quit();

	// Close all opened files
	fcloseall();

	// Notify the user
	char buf[1024];  // Static not needed here
	//sprintf(buf,"An error occured in OpenLieroX\n\nThe development team asks you for sending the crash report file.\nThis will help fixing this bug.\n\nPlease send the crash report file to karel.petranek@tiscali.cz.\n\nThe file is located in:\n %s",checkname);
	//MessageBox(0,buf,"An Error Has Occured",MB_OK);


	snprintf(buf,sizeof(buf),"\"%s\"",checkname.c_str()); fix_markend(buf);
	//MessageBox(0,GetFullFileName("BugReport.exe"),"Debug",MB_OK);

	//std::string ffn = "BugReport.exe";
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
		// TODO: check if drkonqi exists
		// TODO: check if other crash handlers are present; check at least also for Appart and bug-buddy
		// (bug-buddy usage is very similar to drkonqi)
		signal(SIGSEGV, &DrKonqiSignalHandler);
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
