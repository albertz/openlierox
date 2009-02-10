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
#include "DedicatedControl.h"
#include "StringUtils.h"
#include "Debug.h"





//
// WIN 32
//

#if defined(_MSC_VER)

#define itoa _itoa

#include <DbgHelp.h>
#include <ShlObj.h>
#include "FindFile.h" // for IsFileAvailable and mkdir
#include "Cache.h"  // For freeing the cache

LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);

// Crash handling
class CrashHandlerImpl : public CrashHandler {
public:
	CrashHandlerImpl() {
#ifdef _DEBUG
#ifdef USE_DEFAULT_MSC_DELEAKER
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif // _DEBUG

		SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);
		nameThread(-1,"Main game thread");

		notes << "Win32 Exception Filter installed" << endl;
	}


};

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
		strncat(buffer, "iGameMode = ", bufsize);
		switch (tLXOptions->tGameInfo.iGameMode)  {
		case GM_DEATHMATCH:
			strncat(buffer, "GM_DEATHMATCH", bufsize);
			break;
		case GM_TEAMDEATH:
			strncat(buffer, "GM_TEAMDEATH", bufsize);
			break;
		case GM_HIDEANDSEEK:
			strncat(buffer, "GM_HIDEANDSEEK", bufsize);
			break;
		default:
			itoa(tLXOptions->tGameInfo.iGameMode, tmp, 10);
			fix_markend(tmp);
			strncat(buffer, "UNKNOWN ", bufsize); strncat(buffer, tmp, bufsize);
		}
		strncat(buffer, "\n", bufsize);

		// Mod name
		strncat(buffer, "sModName = ", bufsize);
		if (tLXOptions->tGameInfo.sModName.size())
			strncat(buffer, tLXOptions->tGameInfo.sModName.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Map file
		strncat(buffer, "sMapFile = ", bufsize);
		if (tLXOptions->tGameInfo.sMapFile.size())
			strncat(buffer, tLXOptions->tGameInfo.sMapFile.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Map name
		strncat(buffer, "sMapName = ", bufsize);
		if (tLXOptions->tGameInfo.sMapName.size())
			strncat(buffer, tLXOptions->tGameInfo.sMapName.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Mod dir
		strncat(buffer, "sModDir = ", bufsize);
		if (tLXOptions->tGameInfo.sModDir.size())
			strncat(buffer, tLXOptions->tGameInfo.sModDir.c_str(), bufsize);
		strncat(buffer, "\n", bufsize);

		// Loading time
		itoa(tLXOptions->tGameInfo.iLoadingTime, tmp, 10);
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
		itoa(tLXOptions->tGameInfo.iLives, tmp, 10);
		fix_markend(tmp);
		strncat(buffer, "iLives = ", bufsize);
		strncat(buffer, tmp, bufsize);
		strncat(buffer, "\n", bufsize);

		// Max kills
		itoa(tLXOptions->tGameInfo.iKillLimit, tmp, 10);
		fix_markend(tmp);
		strncat(buffer, "iKillLimit = ", bufsize);
		strncat(buffer, tmp, bufsize);
		strncat(buffer, "\n", bufsize);

		// Time limit
		itoa((int)(tLXOptions->tGameInfo.fTimeLimit * 10), tmp, 10);
		fix_markend(tmp);
		strncat(buffer, "fTimeLimit = ", bufsize);
		strncat(buffer, tmp, bufsize);
		strncat(buffer, "\n", bufsize);

		// Bonuses on
		strncat(buffer, "bBonusesOn = ", bufsize);
		strncat(buffer, tLXOptions->tGameInfo.bBonusesOn ? "true" : "false", bufsize);
		strncat(buffer, "\n", bufsize);

		// Bonus names
		strncat(buffer, "bShowBonusName = ", bufsize);
		strncat(buffer, tLXOptions->tGameInfo.bShowBonusName ? "true" : "false", bufsize);
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
			strncat(buf, "\\OpenLieroX", sizeof(buf));
		else
			strncat(buf, "OpenLieroX", sizeof(buf));
		CreateDirectory(buf, NULL); // If the crash occurs at first startup, the OpenLieroX dir doesn't have to exist
		strncat(buf, "\\bug_reports", sizeof(buf));
		fix_markend(buf);
	}
	CreateDirectory(buf, NULL);
	strncat(buf, "\\", sizeof(buf));

	// Get the file name
	char checkname[1024];

	char tmp[32];
	FILE *f = NULL;
	for (int i=1;1;i++)  {
		itoa(i, tmp, 10);
		fix_markend(tmp);
		strncpy(checkname, buf, sizeof(checkname));
		strncat(checkname, "report", sizeof(checkname));
		strncat(checkname, tmp, sizeof(checkname));
		strncat(checkname, ".dmp", sizeof(checkname));
		f = fopen(checkname, "rb");
		if (!f)
			break;
		fclose(f);
	}

	// Open the file
	HANDLE hFile = CreateFile((LPCSTR)checkname,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);


	// Write the minidump
	if (hFile)  {
		MINIDUMP_TYPE type = (MINIDUMP_TYPE)(MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory);
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, type, &eInfo,&iStreams,&cbMiniDump);
	}

	// Close the file
	CloseHandle(hFile);

	// Try to free the cache, it eats a lot of memory
	__try {
		cCache.Clear();
	} 
	__except(EXCEPTION_EXECUTE_HANDLER) {}

	// Quit SDL
	__try  {
		SDL_Quit();
	} 
	__except(EXCEPTION_EXECUTE_HANDLER) {}

	// Close all opened files
	fcloseall();


	strncpy(&buf[1], checkname, sizeof(buf) - 1);
	buf[0] = '\"';
	strncat(buf, "\"", sizeof(buf));
	fix_markend(buf);

	// If ded server is running as service user won't see any dialog anyway
	if (!bDedicated)
		ShellExecute(NULL,"open","BugReport.exe",buf,NULL,SW_SHOWNORMAL);

	// If running as a dedicated server, restart the application (there usually isn't any person sitting
	// at the computer to fix this problem)
	// If ded server is running as service it's restarted automatically
#ifdef DEDICATED_ONLY
	//ShellExecute(NULL, "open", GetAppPath(), "-dedicated", NULL, SW_SHOWNORMAL);
#else
	if (bDedicated)  {
		//ShellExecute(NULL, "open", GetAppPath(), "-dedicated", NULL, SW_SHOWNORMAL);
	}
#endif

	return EXCEPTION_EXECUTE_HANDLER;
}



#elif !defined(WIN32) // MacOSX, Linux, Unix

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>
#include <execinfo.h>
#include <ucontext.h>


/* get REG_EIP / REG_RIP from ucontext.h */
//#define __USE_GNU
#include <ucontext.h>

#ifndef EIP
#define EIP     14
#endif

#if (defined (__x86_64__))
#ifndef REG_RIP
#define REG_RIP REG_INDEX(rip) /* seems to be 16 */
#endif
#endif


struct signal_def { char name[10]; int id; char description[40]; } ;

static signal_def signal_data[] =
{
{ "SIGHUP", SIGHUP, "Hangup (POSIX)" },
{ "SIGINT", SIGINT, "Interrupt (ANSI)" },
{ "SIGQUIT", SIGQUIT, "Quit (POSIX)" },
{ "SIGILL", SIGILL, "Illegal instruction (ANSI)" },
{ "SIGTRAP", SIGTRAP, "Trace trap (POSIX)" },
{ "SIGABRT", SIGABRT, "Abort (ANSI)" },
{ "SIGIOT", SIGIOT, "IOT trap (4.2 BSD)" },
{ "SIGBUS", SIGBUS, "BUS error (4.2 BSD)" },
{ "SIGFPE", SIGFPE, "Floating-point exception (ANSI)" },
{ "SIGKILL", SIGKILL, "Kill, unblockable (POSIX)" },
{ "SIGUSR1", SIGUSR1, "User-defined signal 1 (POSIX)" },
{ "SIGSEGV", SIGSEGV, "Segmentation violation (ANSI)" },
{ "SIGUSR2", SIGUSR2, "User-defined signal 2 (POSIX)" },
{ "SIGPIPE", SIGPIPE, "Broken pipe (POSIX)" },
{ "SIGALRM", SIGALRM, "Alarm clock (POSIX)" },
{ "SIGTERM", SIGTERM, "Termination (ANSI)" },
//{ "SIGSTKFLT", SIGSTKFLT, "Stack fault" },
{ "SIGCHLD", SIGCHLD, "Child status has changed (POSIX)" },
//{ "SIGCLD", SIGCLD, "Same as SIGCHLD (System V)" },
{ "SIGCONT", SIGCONT, "Continue (POSIX)" },
{ "SIGSTOP", SIGSTOP, "Stop, unblockable (POSIX)" },
{ "SIGTSTP", SIGTSTP, "Keyboard stop (POSIX)" },
{ "SIGTTIN", SIGTTIN, "Background read from tty (POSIX)" },
{ "SIGTTOU", SIGTTOU, "Background write to tty (POSIX)" },
{ "SIGURG", SIGURG, "Urgent condition on socket (4.2 BSD)" },
{ "SIGXCPU", SIGXCPU, "CPU limit exceeded (4.2 BSD)" },
{ "SIGXFSZ", SIGXFSZ, "File size limit exceeded (4.2 BSD)" },
{ "SIGVTALRM", SIGVTALRM, "Virtual alarm clock (4.2 BSD)" },
{ "SIGPROF", SIGPROF, "Profiling alarm clock (4.2 BSD)" },
{ "SIGWINCH", SIGWINCH, "Window size change (4.3 BSD, Sun)" },
{ "SIGIO", SIGIO, "I/O now possible (4.2 BSD)" },
//{ "SIGPOLL", SIGPOLL, "Pollable event occurred (System V)" },
//{ "SIGPWR", SIGPWR, "Power failure restart (System V)" },
{ "SIGSYS", SIGSYS, "Bad system call" },
};


typedef const char * cchar;

class CrashHandlerImpl : public CrashHandler {
public:
	CrashHandlerImpl() {
		setSignalHandlers();
		DumpCallstack(NullOut); // dummy call to force loading dynamic lib at this point (with sane heap) for backtrace and friends

		notes << "registered simple resuming signal handler" << endl;
	}
	
	static void setSignalHandlers() {
		struct sigaction sa;
		
		sa.sa_sigaction = SimpleSignalHandler;
		sigemptyset (&sa.sa_mask);
		sa.sa_flags = SA_RESTART | SA_SIGINFO;

		sigaction(SIGSEGV, &sa, NULL);
		sigaction(SIGTRAP, &sa, NULL);
		sigaction(SIGABRT, &sa, NULL);
		sigaction(SIGHUP, &sa, NULL);
		sigaction(SIGBUS, &sa, NULL);
		sigaction(SIGILL, &sa, NULL);
		sigaction(SIGFPE, &sa, NULL);		
		sigaction(SIGSYS, &sa, NULL);		
		sigaction(SIGUSR1, &sa, NULL);
		sigaction(SIGUSR2, &sa, NULL);
	}
		
	static void SimpleSignalHandler(int signr, siginfo_t *info, void *secret) {
		signal(signr, SIG_IGN); // discard all remaining signals

		signal_def *d = NULL;
		for (unsigned int i = 0; i < sizeof(signal_data) / sizeof(signal_def); i++)
			if (signr == signal_data[i].id)
			{ d = &signal_data[i]; break; }
		if (d)
			printf("Got signal 0x%02X (%s): %s\n", signr, d->name, d->description);
		else
			printf("Got signal 0x%02X\n", signr);
		
		/* 
		 see this article for further details: (thanks also for some code snippets)
		 http://www.linuxjournal.com/article/6391 */
		
		void *pnt = NULL;
#if defined(__x86_64__)
#	if defined(__APPLE__)
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext->__ss.__rip ;
#	else
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext.gregs[REG_RIP] ;
#	endif
#elif defined(__hppa__)
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext.sc_iaoq[0] & ~0◊3UL ;
#elif (defined (__ppc__)) || (defined (__powerpc__))
#	if defined(__APPLE__)
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext->__ss.__srr0 ;
#	else
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext.regs->nip ;
#	endif
#elif defined(__sparc__)
		struct sigcontext* sc = (struct sigcontext*) secret;
#	if __WORDSIZE == 64
		pnt = (void*) scp->sigc_regs.tpc ;
#	else
		pnt = (void*) scp->si_regs.pc ;
#	endif
#elif defined(__i386__)
#	if defined(__APPLE__)
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext->__ss.__eip ;
#	else
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext.gregs[REG_EIP] ;		
#	endif
#endif
		
		/* potentially correct for other archs:
		 * alpha: ucp->m_context.sc_pc
		 * arm: ucp->m_context.ctx.arm_pc
		 * ia64: ucp->m_context.sc_ip & ~0◊3UL
		 * mips: ucp->m_context.sc_pc
		 * s390: ucp->m_context.sregs->regs.psw.addr
		 */
		
		if (signr == SIGSEGV || signr == SIGBUS)
			printf("Faulty address is %p, called from %p\n", info->si_addr, pnt);

		/* The first two entries in the stack frame chain when you
		 * get into the signal handler contain, respectively, a
		 * return address inside your signal handler and one inside
		 * sigaction() in libc. The stack frame of the last function
		 * called before the signal (which, in case of fault signals,
		 * also is the one that supposedly caused the problem) is lost.
		 */
		
		/* the third parameter to the signal handler points to an
		 * ucontext_t structure that contains the values of the CPU
		 * registers when the signal was raised.
		 */
		
		// WARNING: dont use cout here in this function, it sometimes screws the cout up
		// look at signal(2) for a list of safe functions
		
		DumpCallstackPrintf(pnt);
		
#ifdef DEBUG
		// Generate coredump and continue
		// HINT: that does not work for me, it just forks and this forked process is just dead and consumes 100% CPU
		// Anyway, GDB still works without forking, it will still break the application.
		// So do we really need this? If it is just for generating the coredump, we can perhaps
		// include Google Breakpad here, generating a mini coredump is very easy there.
		// Also, we really should not fork here anyway. In the possible case that it crashes immediatly
		// after resuming again, we have a forkbomb.
		/*if(!fork()) 
		{
			// Crash the app in your favorite way here
			//raise(SIGQUIT);	// We don't catch SIGQUIT, and it generates coredump
			// TODO: why not just return here?
			exit(-1);
		}*/
#endif
        if((tLXOptions && ! tLXOptions->bRecoverAfterCrash) || (!tLX || tLX->bQuitGame))
        {
			fflush(stdout);
#ifdef DEBUG
        	raise(SIGQUIT);
#else
			exit(-1);
#endif
			return;
        }
		// I've found why we're making forkbomb - we should ignore all following signals, 
		// 'cause if that's segfault the problem will most probably repeat on each frame
		// signal(Sig, SimpleSignalHandler); // reset handler // Do not do that!
		printf("resuming ...\n");
		fflush(stdout);
		
		setSignalHandlers();
		siglongjmp(longJumpBuffer, 1); // jump back to main loop, maybe we'll be able to continue somehow
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
			cchar Arg[32];
			memset(Arg, 0, sizeof(Arg));
			char SigName[16], PidName[16];

			// TODO: sprintf allocates memory on the heap internally, is it safe to do it here?
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

			execvp("drkonqi", (char* const*)Arg);
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
			cchar Arg[32];
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

			execvp("bug-buddy", (char* const*)Arg);
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
		notes << "Dummy CrashHandler implementation" << endl;
	}
};

#endif

CrashHandlerImpl* crashHandlerInstance = NULL;

void CrashHandler::init() {
	if(crashHandlerInstance) {
		warnings << "CrashHandler tried to init twice" << endl;
		return;
	}
	notes << "Installing CrashHandler .. ";
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
