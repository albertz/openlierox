#include "events.h"

#include "base_worm.h"
#include "base_object.h"
#include "base_action.h"
//#include "game_actions.h"
#include "util/macros.h"
#include "game.h"

#include <vector>
#include <string>
#include <map>
#include <stdexcept>

using namespace std;

Event::Event()
{
}

Event::Event(std::vector<BaseAction*>& actions_)
{
	actions.swap(actions_);
}

Event::~Event()
{
	foreach(i, actions)
	{
		delete *i;
	}
}

// This will be oobsol33t
bool Event::addAction( const string& name, const vector<string>& params )
{
	map<string, BaseAction*(*)( const std::vector< std::string > &) >::iterator tempAction = game.actionList.find(name);
	if ( tempAction != game.actionList.end() )
	{
		BaseAction* action = tempAction->second(params);

		actions.push_back( action );
		return true;
	}
	else
	{
		cerr << "Action with name \"" << name << "\" does not exist" << endl;
		return false;
	}
}

void Event::run( BaseObject *object, BaseObject *object2, BaseWorm *worm, Weapon *weapon )
{
	ActionParams params(object, object2, worm, weapon);
	for ( vector<BaseAction*>::iterator action = actions.begin(); action != actions.end(); action++)
	{
		(*action)->run( params );
	}
}
