#ifndef UTILITY_LOG_H
#define UTILITY_LOG_H

#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <map>
#include "util/macros.h"
#include <boost/preprocessor/cat.hpp>

struct Location
{
	Location()
	: file(0)
	{
	}
	
	Location(Location const& l)
	: file(l.file), line(l.line)
	{
	}
	
	Location(std::string const& file_, int line_)
	: file(&file_), line(line_)
	{
	}
	
	// Compiler generated op=
		
	void print(std::string const& msg) const;
	
private:
	std::string const* file;
	int line;
};

struct LogOptions
{
	LogOptions();
	
	bool debug;
	int level;
};

inline bool cstrComp(char const* a, char const* b)
{
	return strcmp(a, b) < 0;
}

struct LogStreams
{
	LogStreams()
	: streams(cstrComp)
	{
	}
	
	~LogStreams()
	{
		foreach(i, streams)
		{
			delete i->second;
		}
	}
	
	std::ostream& operator()(char const* name, char const* path)
	{
		let_(i, streams.find(name));
		if(i == streams.end())
		{
			std::ostream* str = new std::ofstream(path);
			streams[name] = str;
			return *str;
		}
		else
			return *i->second;
	}
	
	std::map<char const*, std::ostream*, bool(*)(char const* a, char const* b)> streams;
};

extern LogOptions logOptions;
extern LogStreams logStreams_;

#define LOG_ERRORS 0
#define LOG_INFO 1
#define LOG_WARNINGS 2
#define LOG_TRACE 3

#define LOG(x_) (std::cout << x_ << std::endl)

#define FLOG(f_, x_) (logStreams_(#f_, #f_ ".log") << x_ << std::endl)

#define WLOG_ONCE(x_) do { static bool warned = false; if(!warned) { WLOG(x_); warned = true; } } while(0)

#define LUA_WLOG_ONCE(x_) if(context.logOnce(std::cout)) std::cout << x_ << std::endl

#define LUA_ELOG(x_) context.log(std::cerr); std::cerr << x_ << '\n'

#ifdef LOG_RUNTIME
#	define DLOG(x_)	if(logOptions.debug) { (std::cout << __FILE__ ":" << __LINE__ << ": " << x_ << std::endl); } else (void)0
#	define DLOGL(l_, x_) if(logOptions.debug) { l_.print(x_); } else (void)0
#	define TLOG(x_) if(logOptions.level >= LOG_TRACE) { (std::cout << __FILE__ ":" << __LINE__ << ": " << x_ << std::endl); } else (void)0
#	define WLOG(x_) if(logOptions.level >= LOG_WARNINGS) { (std::cerr << __FILE__ ":" << __LINE__ << ": " << x_ << '\n'); } else (void)0
#	define ILOG(x_) if(logOptions.level >= LOG_INFO) { (std::cerr << x_ << '\n'); } else (void)0
#	define ELOG(x_) if(logOptions.level >= LOG_ERRORS) { (std::cerr << __FILE__ ":" << __LINE__ << ": " << x_ << '\n'); } else (void)0
#else
#	ifndef LOG_LEVEL
#		define LOG_LEVEL LOG_WARNINGS
#	endif
#	ifdef LOG_DEBUG
#		define DLOG(x_) (std::cout << __FILE__ ":" << __LINE__ << ": " << x_ << std::endl)
#		define DLOGL(l_, x_) l_.print(x_)
#	else
#		define DLOG(x_) (void)0
#		define DLOGL(l_, x_) (void)0
#	endif
#	if LOG_LEVEL >= LOG_TRACE
#		define TLOG(x_) (std::cout << __FILE__ ":" << __LINE__ << ": " << x_ << std::endl)
#	else
#		define TLOG(x_) (void)0
#	endif
#	if LOG_LEVEL >= LOG_INFO
#		define ILOG(x_) (std::cout << x_ << std::endl)
#	else
#		define ILOG(x_) (void)0
#	endif
#	if LOG_LEVEL >= LOG_WARNINGS
#		define WLOG(x_) (std::cerr << __FILE__ ":" << __LINE__ << ": " << x_ << '\n')
#	else
#		define WLOG(x_) (void)0
#	endif
#	if LOG_LEVEL >= LOG_ERRORS
#		define ELOG(x_) (std::cerr << __FILE__ ":" << __LINE__ << ": " << x_ << '\n')
#	else
#		define ELOG(x_) (void)0
#	endif
#endif

#endif //UTILITY_LOG_H
