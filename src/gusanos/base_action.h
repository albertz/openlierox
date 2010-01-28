#ifndef BASE_ACTION_H
#define BASE_ACTION_H

class CGameObject;
class CWorm;
class Weapon;

struct ActionParams
{
	ActionParams( CGameObject *object_, CGameObject *object2_, CWorm *worm_, Weapon *weapon_ ) :
			object(object_), object2(object2_), worm(worm_), weapon(weapon_)
	{
	}
	CGameObject* object;
	CGameObject *object2;
	CWorm *worm;
	Weapon *weapon;
};

class BaseAction
{
public:
	BaseAction();
	virtual ~BaseAction(); // <GLIP> Virtual dtor always needed for classes with virtual functions

	virtual void run( ActionParams const& params ) = 0;
};

#endif  // _BASE_ACTION_H_
