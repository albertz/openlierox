#ifndef AI_H
#define AI_H

#include "base_player.h"
#include "worm.h"
#include "base_object.h"
#include "util/angle.h"
#include "util/vec.h"

//AI Worm Player
class PlayerAI : public BasePlayer
{
	public:
	PlayerAI(int team_, BaseWorm* worm);
	~PlayerAI();
	void getTarget();	//Find target
	void getPath();		//Create path for AI worm to follow (to get to target)
	void subThink();	//AI processing
	
	// Check if materials from my worm to the position are clear of particle_pass = false;
	bool checkMaterialsTo( const Vec& pos ); 
	
	private:
	unsigned int m_pathSteps;	//"steps" to take in A* pathfinding
	int m_nodes[128][128];	//A* nodes
	BaseObject* m_target;	//Target worm
	Angle randomError; // I temporally use this to make it aim worse or sth.
	
	int m_thinkTime; // A timer to limit the amount of events it spams ( it was bad for network )
	static const int thinkDelay = 30; // How often it will think the events part
	
	static const Angle maxInaccuracy;;
	static const Angle maxAimErrorOffset;
	static const Angle aimSpeed;
	
	bool m_targetBlocked;
	
	bool m_movingRight;
	bool m_movingLeft;
	bool m_shooting;
};

#endif
