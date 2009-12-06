#ifndef EVENTS_H
#define EVENTS_H

//#include "base_action.h"

#include <string>
#include <vector>

class Weapon;
class BaseWorm;
class BaseObject;
class BaseAction;

struct GameEvent
{
public:
	GameEvent();
	GameEvent(std::vector<BaseAction*>&);
	virtual ~GameEvent();

	bool addAction( const std::string& name, const std::vector<std::string>& params );
	//void swapActionList(std::vector<BaseAction*>& b); //TODO
	void run( BaseObject *object, BaseObject *object2 = NULL, BaseWorm *worm = NULL, Weapon *weapon = NULL );
	
	//private:
	
	std::vector<BaseAction*> actions;
};

#endif  // _PART_EVENTS_H_
