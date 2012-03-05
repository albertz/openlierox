#include "game/CWorm.h"

#include "CVec.h"
#include "util/angle.h"
#include "util/log.h"
#include "gusgame.h"
#include "game/CGameObject.h"
#include "game/WormInputHandler.h"
#include "weapon_type.h"
#include "particle.h"
#include "CWormHuman.h"
#ifndef DEDICATED_ONLY
#include "base_animator.h"
#include "animators.h"
#include "sprite_set.h"
#include "sprite.h"
#include "gfx.h"
#include "CViewport.h"
#include "font.h"
#include "blitters/blitters.h"
#endif
#include "weapon.h"
#include "game/CMap.h"
#include "CGameScript.h"
#include "CGameMode.h"

#include "glua.h"
#include "luaapi/context.h"
#include "lua/bindings-objects.h"
#include "game/Game.h"
#include "gusanos/network.h"
#include "gusanos/net_worm.h"
#include "CServer.h"

#include <math.h>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;


LuaReference CWorm::metaTable;

Weapon* CWorm::getCurrentWeaponRef()
{
	if(currentWeapon >= m_weapons.size())
		return NULL;
	return m_weapons[currentWeapon];
}

void CWorm::base_setWeapon( size_t index, WeaponType* type )
{
	if(index >= m_weapons.size())
		return;
	
	luaDelete(m_weapons[index]);
	m_weapons[index] = 0;
	
	if ( type )
		m_weapons[index] = new Weapon( type, this );	
}

void CWorm::setWeapon( size_t index, WeaponType* type )
{
	if( !m_node || !network.isClient() ) {
		base_setWeapon(index, type);
		
		// NetWorm code
		if( m_node && !network.isClient() ) {
			BitStream *data = new BitStream;
			addEvent(data, SetWeapon);
			Encoding::encode(*data, index, gusGame.options.maxWeapons);
			if ( type )
			{
				data->addBool(true);
				Encoding::encode(*data, type->getIndex(), gusGame.weaponList.size());
			}else
				data->addBool(false);
			m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_AUTH_2_ALL, data);			
		}
	}
}

void CWorm::setWeapons( std::vector<WeaponType*> const& weaps )
{
	if(weaps.size() > m_weapons.size())
		return;

	// Here is where the interception of the server can be done on the weapon selection
	clearWeapons();
	for ( size_t i = 0; i < weaps.size(); ++i ) {
		setWeapon( i, weaps[i] );
	}
}

void CWorm::base_clearWeapons()
{
	for ( size_t i = 0; i < m_weapons.size(); ++i) {
		luaDelete(m_weapons[i]);
		m_weapons[i] = 0;
	}	
}

void CWorm::clearWeapons()
{
	if( !m_node || !network.isClient() ) {
		base_clearWeapons();
		
		// NetWorm code
		if( m_node && !network.isClient() ) {
			BitStream *data = new BitStream;
			addEvent(data, ClearWeapons);
			m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_AUTH_2_ALL, data);			
		}
	}
}

void CWorm::calculateReactionForce(VectorD2<long> origin, Direction d)
{
	VectorD2<long> step;
	long len = 0;

	int bottom = gusGame.options.worm_weaponHeight;
	int top = bottom - gusGame.options.worm_height + 1;
	int left = (-gusGame.options.worm_width) / 2;
	int right = (gusGame.options.worm_width) / 2;

	switch(d) {
			case Down: {
				origin += VectorD2<long>(left, top);
				step = VectorD2<long>(1, 0);
				len = gusGame.options.worm_width;
			}
			break;

			case Left: {
				origin += VectorD2<long>(right, top + 1);
				step = VectorD2<long>(0, 1);
				len = gusGame.options.worm_height - 2;
			}
			break;

			case Up: {
				origin += VectorD2<long>(left, bottom);
				step = VectorD2<long>(1, 0);
				len = gusGame.options.worm_width;
			}
			break;

			case Right: {
				origin += VectorD2<long>(left, top + 1);
				step = VectorD2<long>(0, 1);
				len = gusGame.options.worm_height - 2;
			}
			break;

			default:
			return;
	}



	for(reacts[d] = 0; len > 0; --len) {
		Material const& g = game.gameMap()->getMaterial(origin.x, origin.y);

		if(!g.worm_pass) {
			++reacts[d];
		}

		origin += step;
	}


}

void CWorm::calculateAllReactionForces(VectorD2<float>& nextPos, VectorD2<long>& inextPos)
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
	else if(inextPos.x > (long)game.gameMap()->GetWidth() - 5)
		reacts[Left] += 5;

	if(inextPos.y < 5)
		reacts[Down] += 5;
	else if(inextPos.y > (long)game.gameMap()->GetHeight() - 5)
		reacts[Up] += 5;

	if(reacts[Down] < 2 && reacts[Up] > 0
	        && (reacts[Left] > 0 || reacts[Right] > 0)) {
		pos().write().y -= correctionSpeed;
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
	        && (reacts[Left] > 0 || reacts[Right] > 0)) {
		// Move one pixel per second
		pos().write().y += correctionSpeed;
		// Update next position as well
		nextPos.y += correctionSpeed;
		inextPos.y = static_cast<long>(nextPos.y);

		// Recalculate horizontal reaction forces.
		// Liero does not recalculate vertical ones,
		// so that is not a bug.

		calculateReactionForce(inextPos, Left);
		calculateReactionForce(inextPos, Right);
	}

	if(gusGame.options.worm_disableWallHugging) {
		if(reacts[Up] == 1 && reacts[Down] == 1
		        && (reacts[Left] > 0 || reacts[Right] > 0)) {
			reacts[Up] = 0;
			reacts[Down] = 0;
		}
	}
}

void CWorm::processPhysics()
{
	if(reacts[Up] > 0) {
		// Friction
		velocity().write().x *= gusGame.options.worm_friction;
	}

	velocity() *= gusGame.options.worm_airFriction;

	if(velocity().get().x > 0.f) {
		if(reacts[Left] > 0) {
			if(velocity().get().x > gusGame.options.worm_bounceLimit) {
				// TODO: Play bump sound
				velocity().write().x *= -gusGame.options.worm_bounceQuotient;
			} else
				velocity().write().x = 0.f;
		}
	} else if(velocity().get().x < 0.f) {
		if(reacts[Right] > 0) {
			if(velocity().get().x < -gusGame.options.worm_bounceLimit) {
				// TODO: Play bump sound
				velocity().write().x *= -gusGame.options.worm_bounceQuotient;
			} else
				velocity().write().x = 0.f;
		}
	}

	if(velocity().get().y > 0.f) {
		if(reacts[Up] > 0) {
			if(velocity().get().y > gusGame.options.worm_bounceLimit) {
				// TODO: Play bump sound
				velocity().write().y *= -gusGame.options.worm_bounceQuotient;
			} else
				velocity().write().y = 0.f;
		}
	} else if(velocity().get().y < 0.f) {
		if(reacts[Down] > 0) {
			if(velocity().get().y < -gusGame.options.worm_bounceLimit) {
				// TODO: Play bump sound
				velocity().write().y *= -gusGame.options.worm_bounceQuotient;
			} else
				velocity().write().y = 0.f;
		}
	}

	if(reacts[Up] == 0) {
		velocity().write().y += gusGame.options.worm_gravity;
	}

	if(velocity().get().x >= 0.f) {
		if(reacts[Left] < 2)
			pos().write().x += velocity().get().x;
	} else {
		if(reacts[Right] < 2)
			pos().write().x += velocity().get().x;
	}

	if(velocity().get().y >= 0.f) {
		if(reacts[Up] < 2)
			pos().write().y += velocity().get().y;
	} else {
		if(reacts[Down] < 2)
			pos().write().y += velocity().get().y;
	}
}


void CWorm::processJumpingAndNinjaropeControls()
{
	if(jumping && reacts[Up]) {
		//Jump

		velocity().write().y -= gusGame.options.worm_jumpForce;
		jumping = false;
	}
}



void CWorm::processMoveAndDig(void)
{
	float acc = gusGame.options.worm_acceleration;

	if(reacts[Up] <= 0)
		acc *= gusGame.options.worm_airAccelerationFactor;
	if(movingLeft && !movingRight) {
		//TODO: Air acceleration
		if(velocity().get().x > -gusGame.options.worm_maxSpeed) {
			velocity().write().x -= acc;
		}

		if(m_dir > 0) {
			aimSpeed = 0;
			m_dir = -1;
		}

		animate = true;
	} else if(movingRight && !movingLeft) {
		//TODO: Air acceleration
		if(velocity().get().x < gusGame.options.worm_maxSpeed) {
			velocity().write().x += acc;
		}

		if(m_dir < 0) {
			aimSpeed = 0;
			m_dir = 1;
		}

		animate = true;
	} else if(movingRight && movingLeft) {
		// TODO: Digging
		animate = false;
	} else {
		animate = false;
	}
}

void CWorm::think()
{	
	if(!game.gameScript()->gusEngineUsed()) {
		// we do that in any case, it may be that some map object was trying to kill us
		if(getAlive()) {
			if ( health <= 0 )
				die();
		}		

		// NOTE: This was from Worm::think() which isn't used right now
		renderPos = pos();
		return;
	}
	
	if(getAlive()) {
		if ( health <= 0 )
			die();

		VectorD2<float> next = pos() + velocity();

		VectorD2<long> inext(static_cast<long>(next.x), static_cast<long>(next.y));

		calculateAllReactionForces(next, inext);

		processJumpingAndNinjaropeControls();
		processPhysics();
		processMoveAndDig();

		aimAngle += aimSpeed;
		if(aimAngle < Angle(0.0)) {
			aimAngle = Angle(0.0);
			aimSpeed = 0;
		}
		if(aimAngle > Angle::almost(180.0)) {
			aimAngle = Angle::almost(180.0);
			aimSpeed = 0;
		}

		for ( size_t i = 0; i < m_weapons.size(); ++i ) {
			if ( m_weapons[i] )
				m_weapons[i]->think( i == currentWeapon, i );
		}

#ifndef DEDICATED_ONLY
		if(animate)
			m_animator->tick();
		else
			m_animator->reset();


		if ( m_currentFirecone ) {
			if ( m_fireconeTime == 0 )
				m_currentFirecone = NULL;
			--m_fireconeTime;

			m_fireconeAnimator->tick();
		}
#endif

	} else {
		if ( m_timeSinceDeath > gusGame.options.maxRespawnTime && gusGame.options.maxRespawnTime >= 0 ) {
			respawn();
		}
		++m_timeSinceDeath;
	}

	// NOTE: This was from Worm::think() which isn't used right now
	renderPos = pos();

	NetWorm_think();
}

Vec CWorm::getWeaponPos()
{
	return pos();
}

#ifndef DEDICATED_ONLY
Vec CWorm::getRenderPos()
{
	return renderPos;// - Vec(0,0.5);
}
#endif

Angle CWorm::getPointingAngle()
{
	return m_dir > 0 ? aimAngle : Angle(360.0) - aimAngle ;
}

int CWorm::getWeaponIndexOffset( int offset )
{
	if ( m_weaponCount > 0 ) {
		if(offset < 0)
			offset = -1;
		else if(offset > 0)
			offset = 1;
		else
			return currentWeapon;

		unsigned int i = currentWeapon;
		do
			i = (i + offset + m_weaponCount) % m_weaponCount;
		while(!m_weapons[i] && i != currentWeapon);

		return i;
	} else {
		return currentWeapon;
	}
}

void CWorm::setDir(int d)
{
	m_dir = d;
}

bool CWorm::isCollidingWith( Vec const& point, float radius )
{
	if ( !getAlive() )
		return false;

	float top = pos().get().y - gusGame.options.worm_boxTop;
	if(point.y < top) {
		float left = pos().get().x - gusGame.options.worm_boxRadius;
		if(point.x < left)
			return (point - Vec(left, top)).lengthSqr() < radius*radius;

		float right = pos().get().x + gusGame.options.worm_boxRadius;
		if(point.x > right)
			return (point - Vec(right, top)).lengthSqr() < radius*radius;

		return top - point.y < radius;
	}

	float bottom = pos().get().y + gusGame.options.worm_boxBottom;
	if(point.y > bottom) {
		float left = pos().get().x - gusGame.options.worm_boxRadius;
		if(point.x < left)
			return (point - Vec(left, bottom)).lengthSqr() < radius*radius;

		float right = pos().get().x + gusGame.options.worm_boxRadius;
		if(point.x > right)
			return (point - Vec(right, bottom)).lengthSqr() < radius*radius;

		return point.y - bottom < radius;
	}

	float left = pos().get().x - gusGame.options.worm_boxRadius;
	if(point.x < left)
		return left - point.x < radius;

	float right = pos().get().x + gusGame.options.worm_boxRadius;
	if(point.x > right)
		return point.x - right < radius;

	return true;
}

bool CWorm::isActive()
{
	return getAlive();
}

void CWorm::removeRefsToPlayer(CWormInputHandler* player)
{
	if ( m_lastHurt == player )
		m_lastHurt = NULL;
	CGameObject::removeRefsToPlayer(player);
}

//#define DEBUG_WORM_REACTS

#ifndef DEDICATED_ONLY
#include "AuxLib.h"

void CWorm::draw(CViewport* viewport)
{
	if(!game.gameScript() || !game.gameScript()->gusEngineUsed())
		// OLX will draw this worm
		return;
	
	if (getAlive() && isVisible(viewport) && gusSkinVisble) {
		/*
		bool flipped = false;
		if ( m_dir < 0 ) flipped = true;*/

		ALLEGRO_BITMAP* where = viewport->dest;
		IVec rPos = viewport->convertCoords( IVec(renderPos) );

		{
			int x = rPos.x;
			int y = rPos.y;

			int renderX = x;
			int renderY = y;

			if (cNinjaRope.active) {
				IVec nrPos = viewport->convertCoords( IVec(Vec(cNinjaRope.pos())) );
				line(where, x, y, nrPos.x, nrPos.y, cNinjaRope.getColour());
			}

			if ( m_weapons[currentWeapon] )
				m_weapons[currentWeapon]->drawBottom(where, renderX, renderY);

			Angle angle = getPointingAngle();
			bool flipped = false;
			if(angle > Angle(180.0)) {
				angle = Angle(360.0) - angle;
				flipped = true;
			}
			
			// Find the right pic
			int f = (m_animator->getFrame() % 3) * 7;
			int ang = MIN((int)( (angle.toDeg())/151 * 7 ), 6);  // clamp the value because LX skins don't have the very bottom aim
			f += ang;
			
			//int colour = universalToLocalColor(this->getSkin().getColor().get());
			//skin->getColoredSprite(m_animator->getFrame(), skinMask, colour, getPointingAngle())->draw(where, renderX, renderY);			
			cSkin.get().DrawHalf(where->surf.get(), where->sub_x + renderX - cSkin.get().getSkinWidth() / 4, where->sub_y + renderY - cSkin.get().getSkinHeight() / 4, f, false, flipped);

			if ( m_weapons[currentWeapon] )
				m_weapons[currentWeapon]->drawTop(where, renderX, renderY);

			if ( m_currentFirecone ) {
				Vec distance = Vec(aimAngle, (double)m_fireconeDistance);
				m_currentFirecone->getSprite(m_fireconeAnimator->getFrame(), getPointingAngle())->
				draw(where, renderX+static_cast<int>(distance.x)*m_dir, renderY+static_cast<int>(distance.y));
			}
		}

#ifdef DEBUG_WORM_REACTS
		{
			int x = rPos.x;
			int y = rPos.y;
			gusGame.infoFont->draw(where, lexical_cast<std::string>(reacts[Up]), x, y + 15, 0);
			gusGame.infoFont->draw(where, lexical_cast<std::string>(reacts[Down]), x, y - 15, 0);
			gusGame.infoFont->draw(where, lexical_cast<std::string>(reacts[Left]), x + 15, y, 0);
			gusGame.infoFont->draw(where, lexical_cast<std::string>(reacts[Right]), x - 15, y, 0);
		}
#endif

	}


}
#endif //DEDICATED_ONLY

void CWorm::respawn()
{
	if(m_isAuthority || !m_node) {
		// Check if its already allowed to respawn
		if ( m_timeSinceDeath > gusGame.options.minRespawnTime ) {
			Vec pos = game.gameMap()->getSpawnLocation( m_owner );
			
			if(game.gameMode()->Spawn(this, CVec(pos)))
				respawn( pos );
		}
	}
}

void CWorm::respawn( const Vec& newPos)
{
	health = 100;
	bAlive = true;
	aimAngle = Angle(90.0);
	velocity() = CVec ( 0, 0 );
	pos() = newPos;
	m_dir = 1;
#ifndef DEDICATED_ONLY

	renderPos = pos();
#endif

	m_lastHurt = NULL;
	for ( size_t i = 0; i < m_weapons.size(); ++i ) {
		if ( m_weapons[i] )
			m_weapons[i]->reset();
	}

	// NetWorm code
	if ( m_isAuthority && m_node && getAlive() )
	{
		BitStream *data = new BitStream;
		addEvent(data, Respawn);
		/*
		 data->addFloat(pos.x,32);
		 data->addFloat(pos.y,32);*/
		game.gameMap()->vectorEncoding.encode<Vec>(*data, pos());
		m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_AUTH_2_ALL, data);
	}	
}

void CWorm::dig()
{
	if( m_isAuthority || !m_node ) {
		if ( getAlive() ) {
			dig( pos(), getPointingAngle() );
		
			// NetWorm code
			if( m_isAuthority && m_node ) {
				BitStream *data = new BitStream;
				addEvent(data, Dig);
				game.gameMap()->vectorEncoding.encode<Vec>(*data, pos());
				data->addInt(int(getPointingAngle()), Angle::prec);
				m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_AUTH_2_ALL, data);
			}
		}
	}
}

void CWorm::dig( const Vec& digPos, Angle angle )
{
	if( gusGame.digObject )
		gusGame.digObject->newParticle( gusGame.digObject, digPos, Vec(angle), m_dir, m_owner, angle );
}

void CWorm::base_die() {
	if(game.gameScript()->gusEngineUsed()) {
		// NetWorm code
		if( m_isAuthority && m_node ) {
			BitStream *data = new BitStream;
			addEvent(data, Die);
			if ( m_lastHurt )
			{
				data->addInt( static_cast<int>( m_lastHurt->getNodeID() ), 32 );
			}
			else
			{
				data->addInt( INVALID_NODE_ID, 32 );
			}
			m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_AUTH_2_ALL, data);			
		}
		
		EACH_CALLBACK(i, wormDeath) {
			(lua.call(*i), getLuaReference())();
		}
	}
	
	bAlive = false;
	fTimeofDeath = tLX->currentTime;

	if (m_owner) {
		gusGame.displayKillMsg(m_owner, m_lastHurt); //TODO: Record what weapon it was?
	}
	
	if(m_isAuthority) {
		cServer->killWorm(getID(), m_lastHurt ? m_lastHurt->getWorm()->getID() : -1, 0);
	}
	
	cNinjaRope.remove();
	m_timeSinceDeath = 0;
	if ( gusGame.deathObject ) {
		gusGame.deathObject->newParticle( gusGame.deathObject, pos(), velocity(), m_dir, m_owner, Vec(velocity()).getAngle() );
	}	
}

void CWorm::die()
{
	if( m_isAuthority || !m_node )
		base_die();
}

void CWorm::changeWeaponTo( unsigned int weapIndex )
{
	if( m_node ) {
		BitStream *data = new BitStream;
		addEvent(data, ChangeWeapon);
		Encoding::encode(*data, weapIndex, m_weapons.size());
		m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_OWNER_2_AUTH | Net_REPRULE_AUTH_2_PROXY, data);		
	}
	
	if ( m_weapons[currentWeapon] ) {
		m_weapons[currentWeapon]->actionStop( Weapon::PRIMARY_TRIGGER );
		m_weapons[currentWeapon]->actionStop( Weapon::SECONDARY_TRIGGER );
	}
	if ( weapIndex < m_weapons.size() && m_weapons[weapIndex] )
		currentWeapon = weapIndex;
}

void CWorm::damage( float amount, CWormInputHandler* damager )
{
	if( m_isAuthority || !m_node ) {
		// TODO: maybe we could implement an armor system? ;O
		m_lastHurt = damager;
		health -= amount;
		if ( health < 0 )
			health = 0;
	}
}

void CWorm::addAimSpeed( AngleDiff speed )
{
	if ( m_owner )
		aimSpeed += speed;
}

void CWorm::addRopeLength( float distance )
{
	cNinjaRope.addLength(distance);
}

#ifndef DEDICATED_ONLY
void CWorm::showFirecone( SpriteSet* sprite, int frames, float distance )
{
	if(sprite) {
		m_fireconeTime = frames;
		m_currentFirecone = sprite;
		delete m_fireconeAnimator;
		m_fireconeAnimator = new AnimLoopRight( sprite, frames );
		m_fireconeDistance = distance;
	}
}
#endif

void CWorm::actionStart( Actions action )
{
	switch ( action ) {
			case MOVELEFT:
			movingLeft = true;
			break;

			case MOVERIGHT:
			movingRight = true;
			break;

			case FIRE:
			if ( getAlive() && m_weapons[currentWeapon] )
				m_weapons[currentWeapon]->actionStart( Weapon::PRIMARY_TRIGGER );
			break;

			case JUMP:
			jumping = true;
			break;

			case NINJAROPE:
			if ( getAlive() ) {
				Vec v(getPointingAngle(), (double)gusGame.options.ninja_rope_shootSpeed);
				if(cClient->getGameLobby()[FT_RopeAddParentSpeed])
					v += velocity();
				cNinjaRope.shoot(getWeaponPos(), v);
			}
			break;

			case CHANGEWEAPON:
			changing = true;
			break;

			case RESPAWN:
			respawn();
			break;

			default:
			break;
	}
}

void CWorm::actionStop( Actions action )
{
	switch ( action ) {
			case MOVELEFT:
			movingLeft = false;
			break;

			case MOVERIGHT:
			movingRight = false;
			break;

			case FIRE:
			if ( getAlive() && m_weapons[currentWeapon] )
				m_weapons[currentWeapon]->actionStop( Weapon::PRIMARY_TRIGGER );
			break;

			case JUMP:
			jumping = false;
			break;

			case NINJAROPE:
			cNinjaRope.remove();
			break;

			case CHANGEWEAPON:
			changing = false;
			break;

			default:
			break;
	}
}

void CWorm::makeReference()
{
	lua.pushFullReference(*this, metaTable);
}

void CWorm::finalize()
{
	EACH_CALLBACK(i, wormRemoved) {
		(lua.call(*i), getLuaReference())();
	}

	for ( size_t i = 0; i < m_weapons.size(); ++i) {
		luaDelete(m_weapons[i]);
		m_weapons[i] = 0;
	}
	
	game.onRemoveWorm(this);
	thisRef.obj.overwriteShared(NULL);

	if(m_node) delete m_node; m_node = 0;
	if(m_interceptor) delete m_interceptor; m_interceptor = 0;
}
