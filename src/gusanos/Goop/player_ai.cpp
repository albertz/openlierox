#include "player_ai.h"
#include "player_options.h"
#include "base_player.h"
#include "worm.h"
#include "game.h"
#include "weapon.h"
#include "util/angle.h"
#include "util/vec.h"
#include <vector>
#include <list>
#include <cmath>

const Angle PlayerAI::maxInaccuracy(10.0);
const Angle PlayerAI::maxAimErrorOffset(20.0);
const Angle PlayerAI::aimSpeed(1.0);

// Code stolen from allegro line to check if the straight line towards target is all clear.
// returns true if the line was blocked somewhere
bool check_materials( int x1, int y1, int x2, int y2 ) 
{
	int dx = x2-x1;
	int dy = y2-y1;
	int i1, i2;
	int x, y;
	int dd;

	/* worker macro */
#define DO_COL(pri_sign, pri_c, pri_cond, sec_sign, sec_c, sec_cond)		\
{										\
	if (d##pri_c == 0) {							\
		return game.level.getMaterial(x1, y1).particle_pass;		\
	}									\
										\
	i1 = 2 * d##sec_c;							\
	dd = i1 - (sec_sign (pri_sign d##pri_c));				\
	i2 = dd - (sec_sign (pri_sign d##pri_c));				\
										\
	x = x1;									\
	y = y1;									\
										\
	while (pri_c pri_cond pri_c##2) {					\
		if ( !game.level.getMaterial(x, y).particle_pass )		\
			return true;						\
										\
		if (dd sec_cond 0) {						\
			sec_c sec_sign##= 1;					\
			dd += i2;						\
		}								\
		else								\
			dd += i1;						\
										\
		pri_c pri_sign##= 1;						\
	}									\
	return false;								\
}

   if (dx >= 0) {
	if (dy >= 0) {
		if (dx >= dy) {
			DO_COL(+, x, <=, +, y, >=);
		}
		else {
			DO_COL(+, y, <=, +, x, >=);
		}
	}
	else {
		if (dx >= -dy) {
			DO_COL(+, x, <=, -, y, <=);
		}
		else {
			DO_COL(-, y, >=, +, x, >=);
		}
	}
   }
   else {
	   if (dy >= 0) {
		   if (-dx >= dy) {
			   DO_COL(-, x, >=, +, y, >=);
		   }
		   else {
			   DO_COL(+, y, <=, -, x, <=);
		   }
	   }
	   else {
		   if (-dx >= -dy) {
			   DO_COL(-, x, >=, -, y, <=);
		   }
		   else {
			   DO_COL(-, y, >=, -, x, <=);
		   }
	   }
   }
}

PlayerAI::PlayerAI(int team_, BaseWorm* worm)
: BasePlayer(shared_ptr<PlayerOptions>(new PlayerOptions("bot")), worm)
, m_pathSteps(100), m_thinkTime(0)
, m_target(0)
, m_movingRight(false)
, m_movingLeft(false)
, m_shooting(false)
{
	colour = universalColor(rndInt(256), rndInt(256), rndInt(256));
	team = team_;
}

PlayerAI::~PlayerAI()
{
	
}

bool PlayerAI::checkMaterialsTo( const Vec& pos )
{
	return check_materials( m_worm->pos.x, m_worm->pos.y, pos.x, pos.y );
}

// getTarget assumes that a m_worm is a valid pointer, please dont call this if m_worm is null or sth
void PlayerAI::getTarget()
{
	m_target = NULL;
	m_targetBlocked = true;
	float tmpDist = -1;
#ifdef USE_GRID
	for ( Grid::iterator worm = game.objects.beginColLayer(Grid::WormColLayer); worm; ++worm)
	{
		if ( worm->getOwner() != this )
		if ( !game.options.teamPlay || (worm->getOwner()->team != team || team == -1) )
		if ( BaseWorm * tmpWorm = dynamic_cast<BaseWorm*>(&*worm) )
		if ( tmpWorm->isActive() )
		{
			bool blocked = checkMaterialsTo( worm->pos );
			float dist = ( m_worm->pos - worm->pos ).length();
			bool distIsShorter = dist < tmpDist;
			if ( ( !blocked && ( m_targetBlocked || distIsShorter ) ) || ( blocked && m_targetBlocked && distIsShorter ) || tmpDist < 0 )
			{
				m_targetBlocked = blocked;
				m_target = &*worm;
				tmpDist = dist;
			}
		}
	}
#else
	ObjectsList::ColLayerIterator worm;
	for ( worm = game.objects.colLayerBegin(Game::WORMS_COLLISION_LAYER); worm; ++worm)
	{
		BaseWorm *tmpWorm;
		if ( (*worm)->getOwner() != this )
		if ( ( tmpWorm = dynamic_cast<BaseWorm*>(*worm) ) && tmpWorm->isActive() )
		{
			bool blocked = checkMaterialsTo( (*worm)->pos );
			bool distIsShorter = ( m_worm->pos - (*worm)->pos ).length() < tmpDist;
			if ( ( !blocked && ( m_targetBlocked || distIsShorter ) ) || ( blocked && m_targetBlocked && distIsShorter ) || tmpDist < 0 )
			{
				m_targetBlocked = blocked;
				m_target = *worm;
				tmpDist = ( m_worm->pos - (*worm)->pos ).length();
			}
		}
	}
#endif
}

void PlayerAI::getPath()
{
	Vec pos = m_worm->pos;		//AI position
	Vec target = m_worm->pos;		//Target position
	
	//create "nodes" array
	for (int y = 0; y < 128; y++)
	{
		for (int x = 0; x < 128; x++)
		{
			m_nodes[y][x] = 0;	//0 - unwalkable
			m_nodes[y][x] = 1;	//1 - walkable (uncalculated)
		}
	}
}

void PlayerAI::subThink()
{
	if ( m_thinkTime > 0) --m_thinkTime;
	else if ( m_worm )
	{
		m_thinkTime = thinkDelay;
		
		if ( !m_worm->isActive() )
			baseActionStart(RESPAWN);
		
		getTarget();
		if (!m_target)
			return;
		
		Vec pos = m_worm->pos;		//AI position
		Vec target = m_target->pos;	//Target position
		
		if ( m_worm->isActive() )
		if ( pos.x < target.x )
		{
			if ( m_movingLeft ) baseActionStop( LEFT );
			if ( !m_movingRight ) baseActionStart( RIGHT );
			m_movingLeft = false;
			m_movingRight = true;
		}else
		{
			if ( m_movingRight ) baseActionStop( RIGHT );
			if ( !m_movingLeft ) baseActionStart( LEFT );
			m_movingRight = false;
			m_movingLeft = true;
		}
		
		randomError = maxAimErrorOffset * midrnd();
		
		Vec tmpVec = ( target - pos );
		if ( tmpVec.x < 0 ) tmpVec.x = -tmpVec.x;
		Angle angle2Target = tmpVec.getAngle();
		Angle wormAimAngle = m_worm->aimAngle + randomError;
		
		AngleDiff targetAngleDiff = wormAimAngle.relative(angle2Target);
		
		if ( !m_targetBlocked )
		{
			//if ( wormAimAngle - maxInaccuracy < angle2Target && wormAimAngle + maxInaccuracy > angle2Target )
			if( abs(targetAngleDiff) < maxInaccuracy )
			{
				baseActionStart(FIRE);
				m_shooting = true;
			} else
			{
				baseActionStop(FIRE);
				m_shooting = false;
			}
		}else
		{
			if ( m_shooting ) 
			{
				baseActionStop(FIRE);
				m_shooting = false;
			}
		}
	
	
		if ( ( m_worm->getCurrentWeapon()->reloading && ( rand() % 8 == 0 ) ) || rand() % 15 == 0)
		{
			m_worm->changeWeaponTo(m_worm->getWeaponIndexOffset( rand() % 50 ) );
		}
	
		// TODO: Make decent behaviour
		//jump
		if (rand() % 6 == 0)
			baseActionStart(JUMP);
	
		//rope
		if (rand() % 10 == 0)
			baseActionStart(NINJAROPE);
		else if (rand() % 96 == 0)
			baseActionStop(NINJAROPE);

	}
	if ( m_worm )
	{
		if ( m_target )
		{
			Vec pos = m_worm->pos;		//AI position
			Vec target = m_target->pos;	//Target position
			
			Vec tmpVec = ( target - pos );
			if ( tmpVec.x < 0 ) tmpVec.x = -tmpVec.x;
			Angle angle2Target = tmpVec.getAngle();
			Angle wormAimAngle = m_worm->aimAngle + randomError;
			
			AngleDiff targetAngleDiff = wormAimAngle.relative(angle2Target);
			
			if ( targetAngleDiff > 0 ) m_worm->aimSpeed = aimSpeed;
			if ( targetAngleDiff < 0 ) m_worm->aimSpeed = -aimSpeed;
		}
		else
		{
			m_worm->aimSpeed = 0;
		}
	}
}

