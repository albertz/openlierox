#include "parser.h"

#include <vector>
#include <string>

using namespace std;

namespace Parser
{
	
	const vector<string> tokenize(const string &text)
	{
		unsigned int left = 0;
		unsigned int right = 0;
	
		string lastChar = " ";
		
		vector<string> stringList;
		
		while (right != string::npos)
		{
			left = text.find_first_not_of(lastChar, right);
			
			if (left != string::npos)
			{
				right = text.find_first_of(", =()",left);
				if (right != string::npos)
				{
					lastChar = text[right];
					
					if (right > left)
					stringList.push_back( text.substr(left, right - left) );
					
					if ( lastChar != " " )
					{
						stringList.push_back( lastChar );
					}
				}else
				stringList.push_back( text.substr(left) );
			}
			else
				right = string::npos;
		}
		
		return stringList;
	}
	
	// May the god of inderdaad forgive me for this sin :(
	int identifyLine( const vector<string> & tokens )
	{
		int id = INVALID;
		
		if ( tokens.size() > 1 )
		{
			vector<string>::const_iterator token = tokens.begin();
			
			if( (*token)[0] == '#' ) // Is it a comment?
				return INVALID;
			
			if ( *token == "on" ) //First token is 'on'? Then its the start of an event
			{
				id = EVENT_START;
			} else
			{
				token++;
				
				if ( *token == "=" ) // Second token is '='? Then its a property assignment
				{
					token++;
					if ( token != tokens.end() )
					{
						id = PROP_ASSIGMENT;
					}
				}
				
				if ( *token == "(" ) // Second token is '('? Then its an action
				{
					// I check for the closing brackets of the action
					for(; token != tokens.end() ; token++) 
					{
						if ( *token == ")" )
							id = ACTION;
					}
				}
				
			}
		}
		return id;
	}
	
	vector<string> getActionParams( const vector<string> & tokens )
	{
		vector<string>::const_iterator token = tokens.begin();
		
		vector<string> params;
		
		if( tokens.size() > 3 )
		{
			token++;
			token++;
			while( token != tokens.end())
			{
				if ( *token == ")" ) break;
				if ( *token != "," )
				{
					params.push_back(*token);
				}
				token++;
			}
		}
		
		return params;
	}
}












