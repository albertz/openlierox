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
#include "LieroX.h" // for setCurThreadName
#include "StringUtils.h"
#include "CClient.h"
#include "CServer.h"
#include "DedicatedControl.h"
#include "StringUtils.h"
#include "Debug.h"
#include "ConversationLogger.h"
#include "FindFile.h"

#include "../breakpad/BreakPad.h"



class DummyCrashHandler : public CrashHandler {
public:
	DummyCrashHandler() {
		notes << "Dummy CrashHandler implementation" << endl;
	}
};


#ifdef NBREAKPAD

//
// WIN 32
//

#if defined(_MSC_VER)

#define itoa _itoa

#include <DbgHelp.h>
#include <ShlObj.h>
#include "FindFile.h" // for IsFileAvailable and mkdir
#include "Cache.h"  // For freeing the cache
#include "CGameMode.h"

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

		notes << "Win32 Exception Filter installed" << endl;
	}

};

extern void OlxWriteCoreDump_Win32(const char* fileName, PEXCEPTION_POINTERS pExInfo);

///////////////////
// This callback function is called whenever an unhandled exception occurs
LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
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

	OlxWriteCoreDump_Win32(checkname, pExInfo);

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

	// End conversation logging (to make the XML valid)
	__try  {
		if (convoLogger)
			delete convoLogger;
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

#if defined(__linux__) || defined(__APPLE__)
// TODO: why is execinfo needed here? at least on MacOSX, it's not needed here
//#include <execinfo.h>
/* get REG_EIP / REG_RIP from ucontext.h */
#define _XOPEN_SOURCE
#include <ucontext.h>
#endif

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

static int handlerSignalList[] = {
SIGSEGV, SIGTRAP, SIGABRT, SIGHUP, SIGBUS, SIGILL, SIGFPE, SIGSYS, SIGUSR1, SIGUSR2
};

typedef const char * cchar;

class CrashHandlerImpl : public CrashHandler {
public:
	CrashHandlerImpl() {
		// Install exception handler only if we don't have Google breakpad
		// All this old code can be removed completly later on.
#ifdef NBREAKPAD
		setSignalHandlers();
		DumpCallstack(NullOut()); // dummy call to force loading dynamic lib at this point (with sane heap) for backtrace and friends

		notes << "registered simple signal handler" << endl;
#endif
	}
	

	static void setSignalHandlers() {
		struct sigaction sa;
		
		sa.sa_sigaction = SimpleSignalHandler;
		sigemptyset (&sa.sa_mask);
		sa.sa_flags = SA_RESTART | SA_SIGINFO;

		for (unsigned int i = 0; i < sizeof(handlerSignalList) / sizeof(int); i++)
			sigaction(handlerSignalList[i], &sa, NULL);
	}
	
	static void unsetSignalHandlers() {
		for (unsigned int i = 0; i < sizeof(handlerSignalList) / sizeof(int); i++)
			signal(handlerSignalList[i], SIG_DFL);
	}
	
	void enable() { if(tLXOptions->bRecoverAfterCrash) setSignalHandlers(); }
	void disable() { unsetSignalHandlers(); }
	
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
#if defined(__APPLE__)
#	if defined(__x86_64__)
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext->__ss.__rip ;
#	elif defined(__hppa__)
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext.sc_iaoq[0] & ~0x3UL ;
#	elif (defined (__ppc__)) || (defined (__powerpc__))
		ucontext_t* uc = (ucontext_t*) secret;
#		if __DARWIN_UNIX03
		pnt = (void*) uc->uc_mcontext->__ss.__srr0 ;
#		else
		pnt = (void*) uc->uc_mcontext->ss.srr0 ;
#		endif
#	elif defined(__sparc__)
		struct sigcontext* sc = (struct sigcontext*) secret;
#		if __WORDSIZE == 64
		pnt = (void*) scp->sigc_regs.tpc ;
#		else
		pnt = (void*) scp->si_regs.pc ;
#		endif
#	elif defined(__i386__)
		ucontext_t* uc = (ucontext_t*) secret;
#		if __DARWIN_UNIX03
		pnt = (void*) uc->uc_mcontext->__ss.__eip ;
#		else
		pnt = (void*) uc->uc_mcontext->ss.eip ;
#		endif
#	else
#		warning mcontext is not defined for this arch, thus a dumped backtrace could be crippled
#	endif
#elif defined(__linux__)
#	if defined(__x86_64__)
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext.gregs[REG_RIP] ;
#	elif defined(__hppa__)
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext.sc_iaoq[0] & ~0x3UL ;
#	elif (defined (__ppc__)) || (defined (__powerpc__))
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext.regs->nip ;
#	elif defined(__sparc__)
		struct sigcontext* sc = (struct sigcontext*) secret;
#		if __WORDSIZE == 64
		pnt = (void*) scp->sigc_regs.tpc ;
#		else
		pnt = (void*) scp->si_regs.pc ;
#		endif
#	elif defined(__i386__)
		ucontext_t* uc = (ucontext_t*) secret;
		pnt = (void*) uc->uc_mcontext.gregs[REG_EIP] ;
#	else
#		warning mcontext is not defined for this arch, thus a dumped backtrace could be crippled
#	endif
#else
#	warning mcontest is not defined for this system, thus a dumped backtraced could be crippled
#endif
		
		/* potentially correct for other archs:
		 * alpha: ucp->m_context.sc_pc
		 * arm: ucp->m_context.ctx.arm_pc
		 * ia64: ucp->m_context.sc_ip & ~0x3UL
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
		// commented out for now because it still doesn't work that good
		//OlxWriteCoreDump(d ? d->name : NULL);
#endif
		
        if(!recoverAfterCrash)
        {
			printf("no recovering, aborting now\n");
			fflush(stdout);
			abort();
			return;
        }
		setSignalHandlers(); // reset handler
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
			char SigName[16], PidName[16], Version[32];

			// TODO: sprintf allocates memory on the heap internally, is it safe to do it here?
			sprintf(SigName, "%i", Sig);
			sprintf(PidName, "%i", MyPid);
			strcpy( Version, GetFullGameName() );

			Arg[Args++] = "drkonqi";
			//Arg[Args++] = "--display";
			//Arg[Args++] = XDisplayString(o.XDisplay());
			Arg[Args++] = "--appname";
			Arg[Args++] = GetFullGameName();
			Arg[Args++] = "--programname";
			Arg[Args++] = GetFullGameName();
			Arg[Args++] = "--appversion";
			Arg[Args++] = Version;
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
			char SigName[16], PidName[16], Version[40];

			sprintf(SigName, "%i", Sig);
			sprintf(PidName, "%i", MyPid);
			strcpy( Version, GetFullGameName() );

			Arg[Args++] = "bug-buddy";
			//Arg[Args++] = "--display";
			//Arg[Args++] = XDisplayString(o.XDisplay());
			Arg[Args++] = "--appname";
			Arg[Args++] = Version;
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

typedef DummyCrashHandler CrashHandlerImpl;

#endif

#else // NBREAKPAD

// We have BreakPad support, so use it

struct CrashHandlerImpl : CrashHandler {
	BreakPad breakpad;
	
	CrashHandlerImpl() : breakpad(GetReportsDestPath()) {
		notes << "installed Breakpad handler" << endl;	
	}

	static std::string GetReportsDestPath() {
		std::string crashreportsdestpath = GetWriteFullFileName("crashreports", true);
		CreateRecDir(crashreportsdestpath, true);
		return crashreportsdestpath;
	}
};

#endif


CrashHandler* crashHandlerInstance = NULL;

void CrashHandler::init() {
	if(crashHandlerInstance) {
		warnings << "CrashHandler tried to init twice" << endl;
		return;
	}

	if(AmIBeingDebugged()) {
		notes << "Debugging .. ";
		crashHandlerInstance = new DummyCrashHandler();
	}
	else {
		notes << "Installing CrashHandler .. ";	
		crashHandlerInstance = new CrashHandlerImpl();		
	}
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

bool CrashHandler::recoverAfterCrash = false;
bool CrashHandler::restartAfterCrash = false;
