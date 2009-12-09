#include "events.h"

#include "CWorm.h"
#include "CGameObject.h"
#include "base_action.h"
//#include "game_actions.h"
#include "util/macros.h"
#include "gusgame.h"

#include <vector>
#include <string>
#include <map>
#include <stdexcept>

using namespace std;

GameEvent::GameEvent()
{
}

GameEvent::GameEvent(std::vector<BaseAction*>& actions_)
{
	actions.swap(actions_);
}

GameEvent::~GameEvent()
{
	foreach(i, actions)
	{
		delete *i;
	}
}

// This will be oobsol33t
bool GameEvent::addAction( const string& name, const vector<string>& params )
{
	map<string, BaseAction*(*)( const std::vector< std::string > &) >::iterator tempAction = gusGame.actionList.find(name);
	if ( tempAction != gusGame.actionList.end() )
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

void GameEvent::run( CGameObject *object, CGameObject *object2, CWorm *worm, Weapon *weapon )
{
	ActionParams params(object, object2, worm, weapon);
	for ( vector<BaseAction*>::iterator action = actions.begin(); action != actions.end(); action++)
	{
		(*action)->run( params );
	}
}
