#ifndef EVENTS_H
#define EVENTS_H

//#include "base_action.h"

#include <string>
#include <vector>

class Weapon;
class CWorm;
class CGameObject;
class BaseAction;

struct GameEvent
{
public:
	GameEvent();
	GameEvent(std::vector<BaseAction*>&);
	virtual ~GameEvent();

	bool addAction( const std::string& name, const std::vector<std::string>& params );
	//void swapActionList(std::vector<BaseAction*>& b); //TODO
	void run( CGameObject *object, CGameObject *object2 = NULL, CWorm *worm = NULL, Weapon *weapon = NULL );
	
	//private:
	
	std::vector<BaseAction*> actions;
};

#endif  // _PART_EVENTS_H_
