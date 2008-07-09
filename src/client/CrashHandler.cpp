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
#include <iostream>

using namespace std;

#if defined(_MSC_VER)

// Leak checking
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

class CrashHandlerImpl : public CrashHandler {
public:
	CrashHandlerImpl() {
#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

		InstallExceptionFilter();
		nameThread(-1,"Main game thread");

		cout << "Win32 Exception Filter installed" << endl;
	}
};


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
		printf("CrashHandler trigger MyPid=%i'n", MyPid);

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
