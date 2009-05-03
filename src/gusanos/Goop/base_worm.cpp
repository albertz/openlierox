#include "base_worm.h"

#include "util/vec.h"
#include "util/angle.h"
#include "util/log.h"
#include "game.h"
#include "base_object.h"
#include "base_player.h"
#include "weapon_type.h"
#include "particle.h"
#include "player_options.h"
#include "player.h"
#ifndef DEDSERV
#include "base_animator.h"
#include "animators.h"
#include "sprite_set.h"
#include "sprite.h"
#include "gfx.h"
#include "viewport.h"
#include "font.h"
#include "blitters/blitters.h"
#endif
#include "weapon.h"
#include "ninjarope.h"

#include "glua.h"
#include "luaapi/context.h"
#include "lua/bindings-objects.h"

#include <math.h>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

/*
void* BaseWorm::operator new(size_t count)
{
	//BaseWorm* p = (BaseWorm *)lua_newuserdata (lua, count);
	BaseWorm* p = (BaseWorm *)lua.pushObject(LuaBindings::wormMetaTable, count);
	p->luaReference = lua.createReference();
	return (void *)p;
}*/

LuaReference BaseWorm::metaTable;

BaseWorm::BaseWorm()
: BaseObject(), aimSpeed(0.0), aimAngle(90.0)
#ifndef DEDSERV
, m_animator(0)
#endif
, animate(false), movable(false), changing(false), m_dir(1)
, m_lastHurt(0)
{
#ifndef DEDSERV
	skin = spriteList.load("skin");
	skinMask = spriteList.load("skin-mask");
	m_animator = new AnimLoopRight(skin,35);

	m_fireconeTime = 0;
	m_currentFirecone = NULL;
	m_fireconeAnimator = NULL;
	m_fireconeDistance = 0;
#endif
	
	m_timeSinceDeath = 0;
	
	m_isActive = false;

	health = 0;
	aimRecoilSpeed = 0;
	
	currentWeapon = 0;
	
	m_weapons.assign(game.options.maxWeapons, 0 );
	m_weaponCount = 0;
	
	for ( size_t i = 0; i < m_weapons.size(); ++i )
	{
		m_weapons[i] = new Weapon(game.weaponList[rndInt(game.weaponList.size())], this);
		m_weaponCount++;
	}
	
	m_ninjaRope = new NinjaRope(game.NRPartType, this);
	movingLeft = false;
	movingRight = false;
	jumping = false;
}

BaseWorm::~BaseWorm()
{
#ifndef DEDSERV
	delete m_animator; m_animator = 0;
	delete m_fireconeAnimator; m_fireconeAnimator = 0;
#endif

	//m_ninjaRope->deleteMe = true;
	for ( size_t i = 0; i < m_weapons.size(); ++i)
	{
		luaDelete(m_weapons[i]); m_weapons[i] = 0;
	}
}

void BaseWorm::assignOwner( BasePlayer* owner)
{
	m_owner = owner;
}

NinjaRope* BaseWorm::getNinjaRopeObj()
{
	return m_ninjaRope;
}

Weapon* BaseWorm::getCurrentWeapon()
{
	return m_weapons[currentWeapon];
}

void BaseWorm::setWeapon( size_t index, WeaponType* type )
{
	if(index >= m_weapons.size())
		return;
		
	luaDelete(m_weapons[index]);
	m_weapons[index] = 0;

	if ( type )
		m_weapons[index] = new Weapon( type, this );
}

void BaseWorm::setWeapons( std::vector<WeaponType*> const& weaps )
{
	if(weaps.size() > m_weapons.size())
		return;
	
	// Here is where the interception of the server can be done on the weapon selection
	clearWeapons();
	for ( size_t i = 0; i < weaps.size(); ++i )
	{
		setWeapon( i, weaps[i] );
	}
}

void BaseWorm::clearWeapons()
{
	for ( size_t i = 0; i < m_weapons.size(); ++i)
	{
		luaDelete(m_weapons[i]); m_weapons[i] = 0;
	}
}

void BaseWorm::calculateReactionForce(BaseVec<long> origin, Direction d)
{
	BaseVec<long> step;
	long len = 0;
	
	int bottom = game.options.worm_weaponHeight;
	int top = bottom - game.options.worm_height + 1;
	int left = (-game.options.worm_width) / 2;
	int right = (game.options.worm_width) / 2;
		
	switch(d)
	{
		case Down:
		{
			origin += BaseVec<long>(left, top);
			step = BaseVec<long>(1, 0);
			len = game.options.worm_width;
		}
		break;
		
		case Left:
		{
			origin += BaseVec<long>(right, top + 1);
			step = BaseVec<long>(0, 1);
			len = game.options.worm_height - 2;
		}
		break;
		
		case Up:
		{
			origin += BaseVec<long>(left, bottom);
			step = BaseVec<long>(1, 0);
			len = game.options.worm_width;
		}
		break;
		
		case Right:
		{
			origin += BaseVec<long>(left, top + 1);
			step = BaseVec<long>(0, 1);
			len = game.options.worm_height - 2;
		}
		break;
		
		default: return;
	}
	
	
	
	for(reacts[d] = 0; len > 0; --len)
	{
		Material const& g = game.level.getMaterial(origin.x, origin.y);
		
		if(!g.worm_pass)
		{
			++reacts[d];
		}
	
		origin += step;
	}
	
	
}

void BaseWorm::calculateAllReactionForces(BaseVec<float>& nextPos, BaseVec<long>& inextPos)
{
	//static const float correctionSpeed = 70.0f / 100.0f;
	static const float correctionSpeed = 1.0f;

	// Calculate all reaction forces
	calculateReactionForce(inextPos, Down);
	calculateReactionForce(inextPos, Left);
	calculateReactionForce(inextPos, Up);
	calculateReactionForce(inextPos, Right);
	
	// Add more if the worm is outside the screen
	if(inextPos.x < 5)
		reacts[Right] += 5;
	else if(inextPos.x > game.level.width() - 5)
		reacts[Left] += 5;
		
	if(inextPos.y < 5)
		reacts[Down] += 5;
	else if(inextPos.y > game.level.height() - 5)
		reacts[Up] += 5;
	
	if(reacts[Down] < 2 && reacts[Up] > 0
	&& (reacts[Left] > 0 || reacts[Right] > 0))
	{
		pos.y -= correctionSpeed;
		// Update next position as well
		nextPos.y -= correctionSpeed;
		inextPos.y = static_cast<long>(nextPos.y);
		
		// Recalculate horizontal reaction forces.
		// Liero does not recalculate vertical ones,
		// so that is not a bug.

		calculateReactionForce(inextPos, Left);
		calculateReactionForce(inextPos, Right);
	}
	
	if(reacts[Up] < 2 && reacts[Down] > 0
	&& (reacts[Left] > 0 || reacts[Right] > 0))
	{
		// Move one pixel per second
		pos.y += correctionSpeed;
		// Update next position as well
		nextPos.y += correctionSpeed;
		inextPos.y = static_cast<long>(nextPos.y);
		
		// Recalculate horizontal reaction forces.
		// Liero does not recalculate vertical ones,
		// so that is not a bug.

		calculateReactionForce(inextPos, Left);
		calculateReactionForce(inextPos, Right);
	}
	
	if(game.options.worm_disableWallHugging)
	{
		if(reacts[Up] == 1 && reacts[Down] == 1
		&& (reacts[Left] > 0 || reacts[Right] > 0))
		{
			reacts[Up] = 0;
			reacts[Down] = 0;
		}
	}
}

void BaseWorm::processPhysics()
{
	if(reacts[Up] > 0)
	{
		// Friction
		spd.x *= game.options.worm_friction;
	}
	
	spd *= game.options.worm_airFriction;
	
	if(spd.x > 0.f)
	{
		if(reacts[Left] > 0)
		{
			if(spd.x > game.options.worm_bounceLimit)
			{
				// TODO: Play bump sound
				spd.x *= -game.options.worm_bounceQuotient;
			}
			else
				spd.x = 0.f;
		}
	}
	else if(spd.x < 0.f)
	{
		if(reacts[Right] > 0)
		{
			if(spd.x < -game.options.worm_bounceLimit)
			{
				// TODO: Play bump sound
				spd.x *= -game.options.worm_bounceQuotient;
			}
			else
				spd.x = 0.f;
		}
	}
	
	if(spd.y > 0.f)
	{
		if(reacts[Up] > 0)
		{
			if(spd.y > game.options.worm_bounceLimit)
			{
				// TODO: Play bump sound
				spd.y *= -game.options.worm_bounceQuotient;
			}
			else
				spd.y = 0.f;
		}
	}
	else if(spd.y < 0.f)
	{
		if(reacts[Down] > 0)
		{
			if(spd.y < -game.options.worm_bounceLimit)
			{
				// TODO: Play bump sound
				spd.y *= -game.options.worm_bounceQuotient;
			}
			else
				spd.y = 0.f;
		}
	}
	
	if(reacts[Up] == 0)
	{
		spd.y += game.options.worm_gravity;
	}
	
	if(spd.x >= 0.f)
	{
		if(reacts[Left] < 2)
			pos.x += spd.x;
	}
	else
	{
		if(reacts[Right] < 2)
			pos.x += spd.x;
	}
	
	if(spd.y >= 0.f)
	{
		if(reacts[Up] < 2)
			pos.y += spd.y;
	}
	else
	{
		if(reacts[Down] < 2)
			pos.y += spd.y;
	}
}


void BaseWorm::processJumpingAndNinjaropeControls()
{
	if(jumping && reacts[Up])
	{
		//Jump
		
		spd.y -= game.options.worm_jumpForce;
		jumping = false;
	}
}



void BaseWorm::processMoveAndDig(void)
{
	// ????????????? wtf is this for?
	//if(!movable && !movingLeft && !movingRight) 
	//	movable = true;
	
	//if(movable)
	if( true )
	{
		float acc = game.options.worm_acceleration;
		
		if(reacts[Up] <= 0)
			acc *= game.options.worm_airAccelerationFactor;
		if(movingLeft && !movingRight)
		{
			//TODO: Air acceleration
			if(spd.x > -game.options.worm_maxSpeed)
			{
				spd.x -= acc;
			}
			
			if(m_dir > 0)
			{
				aimSpeed = 0;
				m_dir = -1;
			}
			
			animate = true;
		}
		else if(movingRight && !movingLeft)
		{
			//TODO: Air acceleration
			if(spd.x < game.options.worm_maxSpeed)
			{
				spd.x += acc;
			}
			
			if(m_dir < 0)
			{
				aimSpeed = 0;
				m_dir = 1;
			}
			
			animate = true;
		}
		else if(movingRight && movingLeft)
		{
			// TODO: Digging
			animate = false;
		}
		else
		{
			animate = false;
		}
	}
}

void BaseWorm::think()
{
	if(m_isActive)
	{
		if ( health <= 0 ) die();
		
		BaseVec<float> next = pos + spd;
		
		BaseVec<long> inext(static_cast<long>(next.x), static_cast<long>(next.y));

		calculateAllReactionForces(next, inext);

		processJumpingAndNinjaropeControls();
		processPhysics();
		processMoveAndDig();

		aimAngle += aimSpeed;
		if(aimAngle < Angle(0.0))
		{
			aimAngle = Angle(0.0);
			aimSpeed = 0;
		}
		if(aimAngle > Angle::almost(180.0))
		{
			aimAngle = Angle::almost(180.0);
			aimSpeed = 0;
		}

		for ( size_t i = 0; i < m_weapons.size(); ++i )
		{
			if ( m_weapons[i] )
				m_weapons[i]->think( i == currentWeapon, i );
		}

#ifndef DEDSERV
		if(animate)
			m_animator->tick();
		else
			m_animator->reset();

		
		if ( m_currentFirecone )
		{
			if ( m_fireconeTime == 0 ) m_currentFirecone = NULL;
			--m_fireconeTime;
			/*
			if(m_fireconeAnimator)
				m_fireconeAnimator->tick();*/
			m_fireconeAnimator->tick();
		}
#endif
	}
	else
	{
		if ( m_timeSinceDeath > game.options.maxRespawnTime && game.options.maxRespawnTime >= 0 )
		{
			respawn();
		}
		++m_timeSinceDeath;
	}
	/* TODO
	}
	else
	{
		//Respawn
	}
	*/
/* OLD CODE
	spd.y+=game.options.worm_gravity;
	
	if ( m_ninjaRope->attached && (m_ninjaRope->pos - pos).length() > currentRopeLength)
	{
		spd += (m_ninjaRope->pos - pos).normal() * game.options.ninja_rope_pullForce;
	}

	if ( movingRight ) 
	{
		if ( spd.x < game.options.worm_maxSpeed )
		{
			if (game.level.getMaterial( (int)pos.x, (int)(pos.y + spd.y + 1) ).particle_pass)
				spd.x += game.options.worm_acceleration * game.options.worm_airAccelerationFactor;
			else
				spd.x += game.options.worm_acceleration;
		}
		dir = 1;
	}
	
	if ( movingLeft ) 
	{
		if ( -spd.x < game.options.worm_maxSpeed )
		{
			if (game.level.getMaterial( (int)pos.x, (int)(pos.y + spd.y + 1) ).particle_pass)
				spd.x -= game.options.worm_acceleration * game.options.worm_airAccelerationFactor;
			else
				spd.x -= game.options.worm_acceleration;
		}
		dir = -1;
	}
	
	
	// Bottom collision
	Material g = game.level.getMaterial( (int)pos.x, (int)(pos.y + spd.y) );
	if (!g.particle_pass && spd.y > 0)
	{
		// Floor friction;
		if ( fabs(spd.x) < game.options.worm_friction ) spd.x=0;
		if ( spd.x < 0 ) spd.x += game.options.worm_friction;
		if ( spd.x > 0 ) spd.x += -game.options.worm_friction;
			
		if ( spd.y < game.options.worm_bounceLimit ) spd.y=0;
		else
		{
			spd.y*=-game.options.worm_bounceQuotient;
		}
	}
	
	// Top collision
	g = game.level.getMaterial( (int)pos.x, (int)(pos.y + spd.y - game.options.worm_height) );
	if (!g.particle_pass && spd.y < 0)
	{
		// Roof friction;
		if ( fabs(spd.x) < game.options.worm_friction ) spd.x=0;
		if ( spd.x < 0 ) spd.x += game.options.worm_friction;
		if ( spd.x > 0 ) spd.x += -game.options.worm_friction;
			
		if ( -spd.y < game.options.worm_bounceLimit ) spd.y=0;
		else
		{
			spd.y*=-game.options.worm_bounceQuotient;
		}
	}
	
	//Side collisions and climbing
	//int o = 0;
	int upper = -1;
	int lower = -1;
	for ( int i = 0; i < game.options.worm_height; i++ )
	{
		if (!game.level.getMaterial( (int)(pos.x + spd.x), (int)(pos.y - i) ).particle_pass) lower = i;
	}
	for ( int i = 0; i <= game.options.worm_height; i++ )
	{
		if (!game.level.getMaterial( (int)(pos.x + spd.x), (int)(pos.y - game.options.worm_height + i) ).particle_pass) upper = i;
	}
	
	// Floor climb
	if (lower <= game.options.worm_maxClimb && lower != -1 && !spd.x == 0)
	{
		pos.y -= 1;
	}
	
	// Roof climb
	if (upper <= game.options.worm_maxClimb && upper != -1 && !spd.x == 0)
	{
		pos.y += 1;
	}
	
	if ( lower >= game.options.worm_maxClimb / 2 )
	{
		if ( fabs( spd.x ) > game.options.worm_bounceLimit )
		{
			spd.x *= -game.options.worm_bounceQuotient;
		}
		else spd.x = 0;
	}
	
	if ( upper == -1 && lower == -1 )
		pos.x += spd.x;
	pos.y += spd.y;
	
	if ( m_owner )
	{
		if ( fabs(aimSpeed) < m_owner->getOptions()->aimFriction ) aimSpeed = 0;
		else if ( aimSpeed > 0 ) aimSpeed -= m_owner->getOptions()->aimFriction;
		else if ( aimSpeed < 0 ) aimSpeed += m_owner->getOptions()->aimFriction;
	}
	aimAngle += aimSpeed;

	if( aimAngle < 0 )
	{
		aimAngle = 0;
		aimSpeed = 0;
	}
	if( aimAngle > 180 )
	{
		aimAngle = 180;
		aimSpeed = 0;
	}
		
	if ( movingLeft || movingRight ) m_animator->tick();
		
	// Make weapons think
	for ( size_t i = 0; i < m_weapons.size(); ++i )
	{
		m_weapons[i]->think();
	}
*/
}

Vec BaseWorm::getWeaponPos()
{
	return pos;
}

#ifndef DEDSERV
Vec BaseWorm::getRenderPos()
{
	return renderPos;// - Vec(0,0.5);
}
#endif
/*
Vec BaseWorm::getWeaponPos()
{
	return renderPos - Vec(0,game.options.worm_weaponHeight+0.5);
}*/

float BaseWorm::getHealth()
{
	return health;
}

Angle BaseWorm::getAngle()
{
	return m_dir > 0 ? aimAngle : Angle(360.0) - aimAngle ;
}

int BaseWorm::getWeaponIndexOffset( int offset )
{
	if ( m_weaponCount > 0 )
	{
		if(offset < 0)
			offset = -1;
		else if(offset > 0)
			offset = 1;
		else
			return currentWeapon;
		
		int i = currentWeapon;
		do
			i = (i + offset + m_weaponCount) % m_weaponCount;
		while(!m_weapons[i] && i != currentWeapon);
		
		return i;
	}
	else
	{
		return currentWeapon;
	}
}

void BaseWorm::setDir(int d)
{
	m_dir = d;
}

/*
bool BaseWorm::isCollidingWith( const Vec& point, float radius )
{
	if ( m_isActive )
	if ( pos.x+game.options.worm_boxRadius > point.x-radius && pos.x-game.options.worm_boxRadius < point.x+radius )
	if ( pos.y+game.options.worm_boxBottom > point.y-radius && pos.y-game.options.worm_boxTop < point.y+radius )
	{
		if ( point.x > pos.x+game.options.worm_boxRadius )
		{
			if ( point.y > pos.y+game.options.worm_boxBottom)
			{
				if ( (pos + Vec(game.options.worm_boxRadius,game.options.worm_boxBottom) - point).lengthSqr() < radius*radius )
					return true;
			}else if (point.y < pos.y-game.options.worm_boxTop)
			{
				if ( (pos + Vec(game.options.worm_boxRadius,-game.options.worm_boxTop) - point).lengthSqr() < radius*radius )
					return true;
			}else
				return true;
		}else if ( point.x < pos.x-game.options.worm_boxRadius )
		{
			if ( point.y > pos.y+game.options.worm_boxBottom)
			{
				if ( (pos + Vec(-game.options.worm_boxRadius,game.options.worm_boxBottom) - point).lengthSqr() < radius*radius )
					return true;
			}else if (point.y < pos.y-game.options.worm_boxTop)
			{
				if ( (pos + Vec(-game.options.worm_boxRadius,-game.options.worm_boxTop) - point).lengthSqr() < radius*radius )	
					return true;
			}else
				return true;
		}else
		{
			return true;
		}
	}
	return false;
}
*/

bool BaseWorm::isCollidingWith( Vec const& point, float radius )
{
	if ( !m_isActive )
		return false;
	
	float top = pos.y - game.options.worm_boxTop;
	if(point.y < top)
	{
		float left = pos.x - game.options.worm_boxRadius;
		if(point.x < left)
			return (point - Vec(left, top)).lengthSqr() < radius*radius;
		
		float right = pos.x + game.options.worm_boxRadius;
		if(point.x > right)
			return (point - Vec(right, top)).lengthSqr() < radius*radius;

		return top - point.y < radius;
	}
	
	float bottom = pos.y + game.options.worm_boxBottom;
	if(point.y > bottom)
	{
		float left = pos.x - game.options.worm_boxRadius;
		if(point.x < left)
			return (point - Vec(left, bottom)).lengthSqr() < radius*radius;
		
		float right = pos.x + game.options.worm_boxRadius;
		if(point.x > right)
			return (point - Vec(right, bottom)).lengthSqr() < radius*radius;
	
		return point.y - bottom < radius;
	}
	
	float left = pos.x - game.options.worm_boxRadius;
	if(point.x < left)
		return left - point.x < radius;
		
	float right = pos.x + game.options.worm_boxRadius;
	if(point.x > right)
		return point.x - right < radius;		

	return true;
}

bool BaseWorm::isActive()
{
	return m_isActive;
}

void BaseWorm::removeRefsToPlayer(BasePlayer* player)
{
	if ( m_lastHurt == player )
		m_lastHurt = NULL;
	BaseObject::removeRefsToPlayer(player);
}

//#define DEBUG_WORM_REACTS

#ifndef DEDSERV

void BaseWorm::draw(Viewport* viewport)
{
	if(!m_owner)
		return;
	
	if (m_isActive)
	{
		/*
		bool flipped = false;
		if ( m_dir < 0 ) flipped = true;*/
		
		BITMAP* where = viewport->dest;
		IVec rPos = viewport->convertCoords( IVec(renderPos) );
	
		{
			int x = rPos.x;
			int y = rPos.y;
			
			int renderX = x;
			int renderY = y;
			
			/*
			if ( m_weapons[currentWeapon] && m_weapons[currentWeapon]->reloading )
			{
				IVec crosshair = IVec(getAngle(), 25.0) + rPos;
				float radius = m_weapons[currentWeapon]->reloadTime / (float)m_weapons[currentWeapon]->m_type->reloadTime;
				circle(where, crosshair.x,crosshair.y,2,makecol(static_cast<int>(255*radius), static_cast<int>(255*(1-radius)),0));
			}
			else for(int i = 0; i < 10; i++)
			{
				Vec crosshair = Vec(getAngle(), rnd()*10.0+20.0);
				putpixel(where, x+static_cast<int>( crosshair.x ), y+static_cast<int>(crosshair.y), makecol(255,0,0));
			}*/
			
			
			if (m_ninjaRope->active)
			{
				IVec nrPos = viewport->convertCoords( IVec(m_ninjaRope->pos) );
				line(where, x, y, nrPos.x, nrPos.y, m_ninjaRope->getColour());
				/*linewu_solid(where
					, x
					, y
					, nrPos.x
					, nrPos.y
				, m_ninjaRope->getColour());*/
			}
			
			if ( m_weapons[currentWeapon] ) m_weapons[currentWeapon]->drawBottom(where, renderX, renderY);
			
			int colour = universalToLocalColor(m_owner->colour);
			skin->getColoredSprite(m_animator->getFrame(), skinMask, colour, getAngle())->draw(where, renderX, renderY);
			
			if ( m_weapons[currentWeapon] ) m_weapons[currentWeapon]->drawTop(where, renderX, renderY);
			
			if ( m_currentFirecone )
			{
				Vec distance = Vec(aimAngle, (double)m_fireconeDistance);
				m_currentFirecone->getSprite(m_fireconeAnimator->getFrame(), getAngle())->
						draw(where, renderX+static_cast<int>(distance.x)*m_dir, renderY+static_cast<int>(distance.y));
			}
				
			/*
			if(changing && m_weapons[currentWeapon])
			{
				std::string const& weaponName = m_weapons[currentWeapon]->m_type->name;
				std::pair<int, int> dim = game.infoFont->getDimensions(weaponName);
				int wx = x - dim.first / 2;
				int wy = y - dim.second / 2 - 10;
							
				game.infoFont->draw(where, weaponName, wx, wy);
			}*/
			
			/*
			if ( false && m_owner && !dynamic_cast<Player*>(m_owner) )
			{
				std::string const& playerName = m_owner->m_name;
				std::pair<int, int> dim = game.infoFont->getDimensions(playerName, 0, Font::Formatting);
				int wx = x - dim.first / 2;
				int wy = y - dim.second / 2 - 10;
								
				game.infoFont->draw(where, playerName, wx, wy, 0, 256, 255, 255, 255, Font::Formatting);
			}*/
		}
		
#ifdef DEBUG_WORM_REACTS
		{
			int x = rPos.x;
			int y = rPos.y;
			game.infoFont->draw(where, lexical_cast<std::string>(reacts[Up]), x, y + 15, 0);
			game.infoFont->draw(where, lexical_cast<std::string>(reacts[Down]), x, y - 15, 0);
			game.infoFont->draw(where, lexical_cast<std::string>(reacts[Left]), x + 15, y, 0);
			game.infoFont->draw(where, lexical_cast<std::string>(reacts[Right]), x - 15, y, 0);
		}
#endif
	}
	
	
}
#endif //DEDSERV

void BaseWorm::respawn()
{
	// Check if its already allowed to respawn
	if ( m_timeSinceDeath > game.options.minRespawnTime )
		respawn( game.level.getSpawnLocation( m_owner ) );
}

void BaseWorm::respawn( const Vec& newPos)
{
	m_isActive = true;
	health = 100;
	aimAngle = Angle(90.0);
	spd = Vec ( 0, 0 );
	pos = newPos;
	m_dir = 1;
#ifndef DEDSERV
	renderPos = pos;
#endif
	m_lastHurt = NULL;
	for ( size_t i = 0; i < m_weapons.size(); ++i )
	{
		if ( m_weapons[i] )
			m_weapons[i]->reset();
	}
}

void BaseWorm::dig()
{
	if ( m_isActive )
	dig( pos, getAngle() );
}

void BaseWorm::dig( const Vec& digPos, Angle angle )
{
	if( game.digObject )
		game.digObject->newParticle( game.digObject, digPos, Vec(angle), m_dir, m_owner, angle );
}

void BaseWorm::die()
{
	EACH_CALLBACK(i, wormDeath)
	{
		(lua.call(*i), getLuaReference())();
	}
	m_isActive = false;
	if (m_owner)
	{
		m_owner->stats->deaths++;
		game.displayKillMsg(m_owner, m_lastHurt); //TODO: Record what weapon it was?
	}
	if (m_lastHurt && m_lastHurt != m_owner) m_lastHurt->stats->kills++;
	
	m_ninjaRope->remove();
	m_timeSinceDeath = 0;
	if ( game.deathObject )
	{
		//game.insertParticle( new Particle( game.deathObject, pos, spd, m_dir, m_owner, spd.getAngle() ) );
		game.deathObject->newParticle( game.deathObject, pos, spd, m_dir, m_owner, spd.getAngle() );
	}
}

void BaseWorm::changeWeaponTo( unsigned int weapIndex )
{
	if ( m_weapons[currentWeapon] )
	{
		m_weapons[currentWeapon]->actionStop( Weapon::PRIMARY_TRIGGER );
		m_weapons[currentWeapon]->actionStop( Weapon::SECONDARY_TRIGGER );
	}
	if ( weapIndex < m_weapons.size() && m_weapons[weapIndex] )
		currentWeapon = weapIndex;
}

void BaseWorm::damage( float amount, BasePlayer* damager )
{
	// TODO: maybe we could implement an armor system? ;O
	m_lastHurt = damager;
	health -= amount;
	if ( health < 0 )
		health = 0;
}

void BaseWorm::addAimSpeed( AngleDiff speed )
{
	if ( m_owner )
		aimSpeed += speed;
}

void BaseWorm::addRopeLength( float distance )
{
	m_ninjaRope->addLength(distance);
}

#ifndef DEDSERV
void BaseWorm::showFirecone( SpriteSet* sprite, int frames, float distance )
{
	if(sprite)
	{
		m_fireconeTime = frames;
		m_currentFirecone = sprite;
		delete m_fireconeAnimator;
		m_fireconeAnimator = new AnimLoopRight( sprite, frames );
		m_fireconeDistance = distance;
	}
}
#endif

void BaseWorm::actionStart( Actions action )
{
	switch ( action )
	{
		case MOVELEFT:
			movingLeft = true;
		break;
		
		case MOVERIGHT:
			movingRight = true;
		break;
		
		case FIRE:
			if ( m_isActive && m_weapons[currentWeapon] )
			m_weapons[currentWeapon]->actionStart( Weapon::PRIMARY_TRIGGER );
		break;
		
		case JUMP:
			jumping = true;
		break;
			
		case NINJAROPE:
			if ( m_isActive )
			m_ninjaRope->shoot(getWeaponPos(), Vec(getAngle(), (double)game.options.ninja_rope_shootSpeed));
		break;
		
		case CHANGEWEAPON:
			changing = true;
		break;
		
		case RESPAWN:
			respawn();
		break;
		
		default: break;
	}
}

void BaseWorm::actionStop( Actions action )
{
	switch ( action )
	{
		case MOVELEFT:
			movingLeft = false;
		break;
		
		case MOVERIGHT:
			movingRight = false;
		break;
		
		case FIRE:
			if ( m_isActive && m_weapons[currentWeapon] )
				m_weapons[currentWeapon]->actionStop( Weapon::PRIMARY_TRIGGER );
		break;
		
		case JUMP:
			jumping = false;
		break;
		
		case NINJAROPE:
			m_ninjaRope->remove();
		break;
		
		case CHANGEWEAPON:
			changing = false;
		break;
		
		default: break;
	}
}

/*
void BaseWorm::pushLuaReference()
{
	lua.pushReference(luaReference);
}*/

void BaseWorm::makeReference()
{
	lua.pushFullReference(*this, metaTable);
}

/*
LuaReference BaseWorm::getLuaReference()
{
	if(luaReference)
		return luaReference;
	else
	{
		lua.pushFullReference(*this, metaTable);
		luaReference = lua.createReference();
		return luaReference;
	}
}*/

void BaseWorm::finalize()
{
	EACH_CALLBACK(i, wormRemoved)
	{
		(lua.call(*i), getLuaReference())();
	}
	
	for ( size_t i = 0; i < m_weapons.size(); ++i)
	{
		luaDelete(m_weapons[i]); m_weapons[i] = 0;
	}
}
