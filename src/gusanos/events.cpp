#include "events.h"

#include "CWorm.h"
#include "CGameObject.h"
#include "base_action.h"
//#include "game_actions.h"
#include "util/macros.h"
#include "gusgame.h"
#include "Debug.h"

#include <vector>
#include <string>
#include <map>
#include <stdexcept>

using namespace std;

GameEvent::GameEvent()
{
}

GameEvent::GameEvent(Actions& actions_)
{
	actions.swap(actions_);
}

GameEvent::~GameEvent()
{
	actions.clear();
}

// This will be oobsol33t
bool GameEvent::addAction( const string& name, const vector<string>& params )
{
	map<string, BaseAction*(*)( const std::vector< std::string > &) >::iterator tempAction = gusGame.actionList.find(name);
	if ( tempAction != gusGame.actionList.end() )
	{
		boost::shared_ptr<BaseAction> action ( tempAction->second(params) );

		actions.push_back( action );
		return true;
	}
	else
	{
		errors << "Action with name \"" << name << "\" does not exist" << endl;
		return false;
	}
}

void GameEvent::run( CGameObject *object, CGameObject *object2, CWorm *worm, Weapon *weapon )
{
	ActionParams params(object, object2, worm, weapon);
	for ( Actions::iterator action = actions.begin(); action != actions.end(); action++)
	{
		(*action)->run( params );
	}
}
