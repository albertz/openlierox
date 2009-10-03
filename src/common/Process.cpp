/*
 *  Process.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 10.02.09.
 *  code under LGPL
 *
 */

#include <cassert>
#include "Process.h"
#include "Debug.h"
#include "FindFile.h"

#if ( ! defined(HAVE_BOOST) && defined(WIN32) ) || ( defined(_MSC_VER) && (_MSC_VER <= 1200) )

#include <windows.h>

struct ProcessIntern	// Stub
{
	int dummy;
	ProcessIntern(): dummy(0) {};
	std::ostream & in(){ return std::cout; };
	std::istream & out(){ return std::cin; };
	void close() {  }
	bool open( const std::string & cmd, std::vector< std::string > params, const std::string& working_dir )
	{
		errors << "Dedicated server is not compiled into this version of OpenLieroX" << endl;
		MessageBox( NULL, "ERROR: Dedicated server is not compiled into this version of OpenLieroX", "OpenLieroX", MB_OK );
		return false;
	}
};

#elif WIN32

// Install Boost headers for your compiler and #define HAVE_BOOST to compile dedicated server for Win32
// You don't need to link to any lib to compile it, just headers.
#ifdef new  // Boost is incompatible with leak detection in MSVC, just disable it for the boost headers
#undef new
#include <boost/process.hpp> // This one header pulls turdy shitload of Boost headers
#define new DEBUG_NEW // Re-enable
#else
#include <boost/process.hpp>
#endif

struct ProcessIntern
{
	boost::process::child *p;
	ProcessIntern(): p(NULL) {};
	std::ostream & in() { assert(p != NULL); return p->get_stdin(); };
	std::istream & out() { assert(p != NULL); return p->get_stdout(); };
	void close() { if (p) { p->get_stdin().close(); } }
	bool open( const std::string & cmd, std::vector< std::string > params, const std::string& working_dir )
	{
		if(p)
			delete p;
		
		for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); it++)
			*it = Utf8ToSystemNative(*it);
		
		boost::process::context ctx;
		ctx.m_stdin_behavior = boost::process::capture_stream(); // Pipe for win32
		ctx.m_stdout_behavior = boost::process::capture_stream();
		ctx.m_stderr_behavior = boost::process::close_stream(); // we don't grap the stderr, it is not outputted anywhere, sadly
		ctx.m_work_directory = Utf8ToSystemNative(working_dir);
		if( ctx.m_work_directory == "" )
			ctx.m_work_directory = ".";
		try
		{	
			p = new boost::process::child(boost::process::launch(Utf8ToSystemNative(cmd), params, ctx)); // Throws exception on error
		}
		catch( const std::exception & e )
		{
			errors << "Error running command " << cmd << " : " << e.what() << endl;
			return false;
		}
		return true;
	}
	~ProcessIntern(){ close(); if(p) delete p; };
};

#else
#include <pstream.h>
struct ProcessIntern
{
	ProcessIntern() : p(NULL) {}
	~ProcessIntern() { close(); reset(); }
	redi::pstream* p;
	std::ostream & in() { return *p; };
	std::istream & out() { return p->out(); };
	void reset() { if(p) delete p; p = NULL; }
	void close() {
		// p << redi::peof;
		if(!p)
			return;
		if(p->rdbuf()) p->rdbuf()->kill();
		if(p->rdbuf()) p->rdbuf()->kill(SIGKILL);
	}
	bool open( const std::string & cmd, std::vector< std::string > params, const std::string& working_dir )
	{
		reset(); p = new redi::pstream();
		p->open( cmd, params, redi::pstreams::pstdin | redi::pstreams::pstdout, working_dir ); // we don't grap the stderr, it should directly be forwarded to console
		return p->rdbuf()->error() == 0;
	}
};
#endif


Process::Process() {
	data = new ProcessIntern();
}

Process::~Process() {
	assert(data != NULL);
	delete data;
	data = NULL;
}


std::ostream& Process::in() { return data->in(); }
std::istream& Process::out() { return data->out(); }
void Process::close() { data->close(); }

#ifdef WIN32
struct SysCommand {
	std::string exec;
	std::vector<std::string> params;
};

static SysCommand GetExecForScriptInterpreter(const std::string& interpreter) {
	SysCommand ret;
	std::string& command = ret.exec;
	std::vector<std::string>& commandArgs = ret.params;
	
	std::string cmdPathRegKey = "";
	std::string cmdPathRegValue = "";
	// TODO: move that out to an own function!
	if( interpreter == "python" )
	{
		// TODO: move that out to an own function!
		command = "python.exe";			
		commandArgs.clear();
		commandArgs.push_back(command);
		commandArgs.push_back("-u");
		cmdPathRegKey = "SOFTWARE\\Python\\PythonCore\\2.5\\InstallPath";
	}
	else if( interpreter == "bash" )
	{
		// TODO: move that out to an own function!
		command = "bash.exe";
		commandArgs.clear();
		commandArgs.push_back(command);
		//commandArgs.push_back("-l");	// Not needed for Cygwin
		commandArgs.push_back("-c");
		cmdPathRegKey = "SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\/usr/bin";
		cmdPathRegValue = "native";
	}
	else if( interpreter == "php" )
	{
		// TODO: move that out to an own function!
		command = "php.exe";
		commandArgs.clear();
		commandArgs.push_back(command);
		commandArgs.push_back("-f");
		cmdPathRegKey = "SOFTWARE\\PHP";
		cmdPathRegValue = "InstallDir";
	}
	else
	{
		command = interpreter + ".exe";
	}
	
	// TODO: move that out to an own function!
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
			errors << "registry key " << cmdPathRegKey << "\\" << cmdPathRegValue << " not found - make sure interpreter is installed" << endl;
			lszCmdPath[0] = '\0'; // Perhaps it is installed in PATH
		}
		returnStatus = RegQueryValueEx(hKey, cmdPathRegValue.c_str(), NULL, &dwType,(LPBYTE)lszCmdPath, &dwSize);
		RegCloseKey(hKey);
		if (returnStatus != ERROR_SUCCESS)
		{
			errors << "registry key " << cmdPathRegKey << "\\" << cmdPathRegValue << " could not be read - make sure interpreter is installed" << endl;
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
	
	return ret;
}
#endif

bool Process::open( const std::string & cmd, std::vector< std::string > params, const std::string& working_dir ) {
	if(params.size() == 0)
		params.push_back(cmd);
	
	std::string realcmd = cmd;
#ifdef WIN32
	std::string interpreter = GetScriptInterpreterCommandForFile(cmd);
	if(interpreter != "") {
		size_t f = interpreter.find(" ");
		if(f != std::string::npos) interpreter.erase(f);
		interpreter = GetBaseFilename(interpreter);
		SysCommand newcmd = GetExecForScriptInterpreter(interpreter);
		realcmd = newcmd.exec;
		params.swap( newcmd.params );
		params.reserve( params.size() + newcmd.params.size() );
		for(std::vector<std::string>::iterator i = newcmd.params.begin(); i != newcmd.params.end(); ++i)
			params.push_back(*i);

		notes << "running \"" << realcmd << "\" for script \"" << cmd << "\"" << endl;
	}
#endif
	
	return data->open( realcmd, params, working_dir );
}

