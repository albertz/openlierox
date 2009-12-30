/*
 *  TeeStdoutHandler.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 01.12.09.
 *  code under LGPL
 *
 */

#include <string>
#include <cstddef>
#include "Debug.h"

#ifndef WIN32

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

static char* teeOlxOutputFile = NULL;
static const size_t MAXFILENAMESIZE = 2048;

const char* GetLogFilename() { if(teeOlxOutputFile) return teeOlxOutputFile; return ""; }

#include <unistd.h>
#include <signal.h>

static void teeOlxOutputToFileHandler(int c) {
	static std::string buffer;
	static std::string currentOlxOutputFilename = "";
	static FILE* out = NULL;
	
	if(c == EOF) {
		if(out) fclose(out); out = NULL;
		return;
	}
	
	char ch = c;
	if(out)
		fwrite(&ch, 1, 1, out);
	else
		buffer += c;
	
	if(ch == '\n' && currentOlxOutputFilename != teeOlxOutputFile) {
		if(out) fclose(out); out = NULL;
		currentOlxOutputFilename = teeOlxOutputFile;
		if(currentOlxOutputFilename != "") {
			out = fopen(currentOlxOutputFilename.c_str(), "a");
			if(out)
				// We want to have everything written immediatly, to have the last output when it crashes
				setvbuf(out, NULL, _IONBF, 0);				
			if(out && !buffer.empty()) {
				fwrite(buffer.c_str(), buffer.size(), 1, out);
				buffer = "";
			}
		}
	}
}

static void teeOlxOutputHandler(int in, int out) {
	unsigned long c = 0;
	const static bool printlinenum = false; // just for debugging
	bool newline = true;
	
	// The main process will quit this fork by closing the pipe.
	// There will be problems if this fork terminates earlier.
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	while( true ) {
		char ch = 0;
		if(read(in, &ch, 1) <= 0) break;
		if(printlinenum && newline) {
			char tmp[10];
			sprintf(tmp, "%06lu: ", c);
			write(out, &tmp, 8);
			c++;
			newline = false;
		}
		write( out, &ch, 1 );
		if(ch == '\n')
			newline = true;
		teeOlxOutputToFileHandler((unsigned char)ch);
	}
	
	teeOlxOutputToFileHandler(EOF);
}

struct TeeStdoutInfo {
	pid_t proc;
	int pipeend;
	int oldstdout;
	int oldstderr;
};

static TeeStdoutInfo teeStdoutInfo = {0,0,0,0};

#include <sys/errno.h>
#include <cstdio>
#include <cstring>

void teeStdoutInit() {
	int pipe_to_handler[2];
	
#ifdef __APPLE__
	teeOlxOutputFile = (char*) mmap(0, MAXFILENAMESIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, 0, 0);
#else	
	int fd = open("/dev/zero", O_RDWR);
	if(fd < 0) {
		errors << "teeStdout: cannot open dummy file" << endl;
		return;
	}
	
	teeOlxOutputFile = (char*) mmap(0, MAXFILENAMESIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
#endif
	
	if(!teeOlxOutputFile || teeOlxOutputFile == (char*)-1) {
		teeOlxOutputFile = NULL;
		errors << "teeStdout: cannot mmap: " << strerror(errno) << endl;
		return;
	}
	
	if(pipe(pipe_to_handler) != 0) { // error creating pipe
		errors << "teeStdout: cannot create pipe: " << strerror(errno) << endl;		
		return;
	}
	
	pid_t p = fork();
	if(p < 0) { // error forking
		errors << "teeStdout: cannot fork: " << strerror(errno) << endl;		
		return;
	}
	else if(p == 0) { // fork		
		close(pipe_to_handler[1]);
		teeOlxOutputHandler(pipe_to_handler[0], STDOUT_FILENO);
		_exit(0);
	}
	else { // parent
		close(pipe_to_handler[0]);
		teeStdoutInfo.proc = p;
		teeStdoutInfo.pipeend = pipe_to_handler[1];
		teeStdoutInfo.oldstdout = dup(STDOUT_FILENO);
		teeStdoutInfo.oldstderr = dup(STDERR_FILENO);
		dup2(pipe_to_handler[1], STDOUT_FILENO);
		dup2(pipe_to_handler[1], STDERR_FILENO);
		setvbuf(stdout, NULL, _IONBF, 0);
		setvbuf(stderr, NULL, _IONBF, 0);
	}
}

void teeStdoutFile(const std::string& file) {
	if(!teeOlxOutputFile) return;
	
	if(file.size() >= MAXFILENAMESIZE - 1) {
		errors << "teeStdoutFile: filename " << file << " too big" << endl;
		strcpy(teeOlxOutputFile, "");
		return;
	}
	
	strcpy(teeOlxOutputFile, file.c_str());
}

#include <sys/wait.h>

// NOTE: We are calling this also when we crashed, so be sure that we only do save operations here!
void teeStdoutQuit(bool wait) {
	if(teeStdoutInfo.proc) {
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		close(teeStdoutInfo.pipeend);
		// The forked process should quit itself now.
		if(wait) waitpid(teeStdoutInfo.proc, NULL, 0);
		
		// Recover stdout/err
		dup2(teeStdoutInfo.oldstdout, STDOUT_FILENO);
		dup2(teeStdoutInfo.oldstderr, STDERR_FILENO);
		close(teeStdoutInfo.oldstdout);
		close(teeStdoutInfo.oldstderr);
		printf("Standard Out/Err recovered\n");
	}
	
	if(teeOlxOutputFile) {
		munmap(teeOlxOutputFile, MAXFILENAMESIZE);
		teeOlxOutputFile = NULL;
	}
}

#else

#include "Unicode.h"
#include "Version.h"
#include "FindFile.h"
#include "AuxLib.h"

static char teeLogfile[2048] = "stdout.txt";

void teeStdoutInit() {}
void teeStdoutFile(const std::string& f) {
#ifndef _DEBUG
	if(f.size() < sizeof(teeLogfile) - 1)
		strcpy(teeLogfile, f.c_str());
	else
		errors << "teeStdoutFile: filename " << f << " is too long" << endl;
	
	// If you would just see the old output, it would look kind of strange
	// that it suddenly stops, so print where we continue.
	notes << "Logfile: " << f << endl;
	
	if(freopen(Utf8ToSystemNative(f).c_str(), "a", stdout) == NULL) {
		freopen("stdout.txt", "a", stdout); // reopen fallback
		errors << "Could not open logfile" << endl;
		return;
	}
	// no caching for stdout/logfile, it should be written immediatly to
	// have the important information in case of a crash
	setvbuf(stdout, NULL, _IONBF, 0);
	
	// print some messages again because they are missing in the logfile
	hints << GetFullGameName() << " is starting ..." << endl;
#ifdef DEBUG
	hints << "This is a DEBUG build." << endl;
#endif
#ifdef DEDICATED_ONLY
	hints << "This is a DEDICATED_ONLY build." << endl;
#endif
	notes << "Free memory: " << (GetFreeSysMemory() / 1024 / 1024) << " MB" << endl;
	notes << "Current time: " << GetDateTimeText() << endl;
	
	// print the searchpaths, this may be very usefull for the user
	notes << "Searchpaths (in this order):\n";
	for(searchpathlist::const_iterator p2 = tSearchPaths.begin(); p2 != tSearchPaths.end(); p2++) {
		std::string path = *p2;
		ReplaceFileVariables(path);
		notes << "  " << path << "\n";
	}
	notes << "Searchpaths finished." << endl;
	
	// we still miss some more output but let's hope that this is enough
#endif
}
void teeStdoutQuit(bool wait) {}
const char* GetLogFilename() { return teeLogfile; }

#endif



