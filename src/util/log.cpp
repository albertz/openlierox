#include "log.h"

LogOptions logOptions;
LogStreams logStreams_;

LogOptions::LogOptions()
: debug(true), level(LOG_WARNINGS)
{
}

void Location::print(std::string const& msg) const
{
	if(file)
		std::cerr << *file << ':' << line << ": " << msg << '\n';
}

