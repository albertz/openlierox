#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

namespace Parser
{
	enum
	{
		INVALID = -1,
		PROP_ASSIGMENT,
		EVENT_START,
		ACTION
	};
	
	const std::vector<std::string> tokenize ( const std::string & text );
	
	int identifyLine( const std::vector<std::string> & tokens );
	
	std::vector<std::string> getActionParams( const std::vector<std::string> & tokens );
}

#endif  // _GAME_ACTIONS_H_
