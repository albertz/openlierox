#include "particle.h"

#include "gusgame.h"
#include "game/CGameObject.h"
#include "game/CWorm.h"
#include "game/WormInputHandler.h"
#include "part_type.h"
#ifndef DEDICATED_ONLY
#include "gfx.h"
#include "sprite.h"
#include "sprite_set.h"
#include "base_animator.h"
#include "blitters/blitters.h"
#include "CViewport.h"
#endif
#include "glua.h"
#include "luaapi/context.h"
#include "lua/bindings-objects.h"
#include "detect_event.h"
#include "noise_line.h"
#include "util/macros.h"
#include "util/vec.h"
#include "util/angle.h"
#include "util/log.h"
#include "util/math_func.h"
#include "network.h"
#include "netstream.h"
#include "posspd_replicator.h"
#include "game/CMap.h"

#include <vector>
#include <iostream>
#define BOOST_NO_MT
#include <boost/pool/pool.hpp>
#include "CodeAttributes.h"

using namespace std;

namespace
{

	boost::pool<> particlePool(sizeof(Particle));
}

class ParticleInterceptor : public Net_NodeReplicationInterceptor
{
	public:
		enum type
		{
		    Position
	};

		ParticleInterceptor( Particle* parent_ )
				: parent(parent_)
		{}

		bool inPreUpdateItem(Net_Node *_node, Net_ConnID _from, eNet_NodeRole _remote_role, Net_Replicator *_replicator)
		{
			return true;
		}

		void outPreReplicateNode(Net_Node *_node, eNet_NodeRole _remote_role)
		{
			if(partTypeList.size() == 0) {
				errors << "ParticleInterceptor:outPreReplicateNode for " << _node->debugName() << ": partTypeList is empty" << endl;
				return;
			}
			BitStream *type = new BitStream;
			Encoding::encode(*type, parent->m_type->getIndex(), partTypeList.size());
			if(parent->m_owner)
				type->addInt(parent->m_owner->getNodeID(), 32);
			else
				type->addInt(0, 32);
			parent->m_node->setAnnounceData( type );
		}

		bool outPreUpdateItem(Net_Node* node, eNet_NodeRole remote_role, Net_Replicator* replicator)
		{
			switch ( replicator->getSetup()->getInterceptID() ) {
					case Position:
					if(!(parent->flags & Particle::RepPos)) {
						return false;
					}
					break;
			}

			return true;
		}

		bool outPreUpdate (Net_Node* node, eNet_NodeRole remote_role) { return true; }

	private:
		Particle* parent;
};

LuaReference Particle::metaTable;

Net_ClassID Particle::classID = INVALID_CLASS_ID;

void* Particle::operator new(size_t count)
{

	assert(count <= sizeof(Particle));
	return particlePool.malloc();
}


void Particle::operator delete(void* block)
{
	particlePool.free(block);
}

Particle::Particle(PartType *type, Vec pos_, Vec spd_, int dir, CWormInputHandler* owner, Angle angle)
		: CGameObject(owner, pos_, spd_), 
		/*m_dir(dir),*/
		m_type(type),
		m_health(type->health),
		m_angle(angle),
		m_angleSpeed(0),
#ifndef DEDICATED_ONLY
		m_fadeSpeed(0),
		m_alpha((float)m_type->alpha),
		m_alphaDest(255),
		m_sprite(m_type->sprite),
		m_animator(0),
#endif
		m_origin(pos_),
		m_node(0),
		interceptor(0)
{
	m_type->touch();

	m_angle.clamp();

	flags = (dir > 0 ? FaceRight : 0)
	        | RepPos;

#ifndef DEDICATED_ONLY

	if ( m_sprite ) {
		m_animator = m_type->allocateAnimator();
	}
#endif
	// If it's deleted, we must allocate animator so that we can render it,
	// but we don't need any timers.

	if ( m_type->creation ) {
		m_type->creation->run(this);
		if(deleteMe)
			return;
	}

	//for ( vector< TimerEvent* >::iterator i = m_type->timer.begin(); i != m_type->timer.end(); i++)
	foreach(i, m_type->timer) {
		timer.push_back( (*i)->createState() );
	}


}

Particle::~Particle()
{
#ifndef DEDICATED_ONLY
	delete m_animator;
#endif

	delete m_node;
	delete interceptor;
}

void Particle::assignNetworkRole( bool authority )
{
	m_node = new Net_Node();
	/* operator new never returns 0
	if (!m_node)
{
		allegro_message("ERROR: Unable to create particle node.");
}*/

	m_node->beginReplicationSetup();

	static Net_ReplicatorSetup posSetup( Net_REPFLAG_MOSTRECENT | Net_REPFLAG_INTERCEPT, Net_REPRULE_AUTH_2_ALL, ParticleInterceptor::Position, -1, 1000);

	m_node->addReplicator(new PosSpdReplicator( &posSetup, &pos(), &velocity() ), true);

	m_node->endReplicationSetup();

	interceptor = new ParticleInterceptor( this );
	m_node->setReplicationInterceptor(interceptor);

	//DLOG("Registering node, type " << m_type->getIndex() << " of " << partTypeList.size());
	if( authority ) {

		//DLOG("Announce data set");
		m_node->setEventNotification(true, false); // Enables the eEvent_Init.
		if( !m_node->registerNodeDynamic(classID, network.getNetControl() ) )
			allegro_message("ERROR: Unable to register particle authority node.");
	} else {
		m_node->setEventNotification(false, true); // Same but for the remove event.
		//DLOG("GameEvent notification set");
		if( !m_node->registerRequestedNode( classID, network.getNetControl() ) )
			allegro_message("ERROR: Unable to register particle requested node.");
	}

	//DLOG("Node registered");

	m_node->applyForNetLevel(1);

	//DLOG("Applied for zoidlevel");
}

INLINE Vec getCorrection( const Vec& objPos, const Vec& pointPos, float radius )
{
	Vec diff = pointPos - objPos;
	if ( diff.lengthSqr() < radius*radius ) {
		float lengthDiff = (float)diff.length() - radius;
		return diff.normal() * lengthDiff;
	} else {
		return Vec();
	}
}

Vec getCorrectionBox( const Vec& objPos, const IVec& boxPos, float radius )
{
	IVec iObjPos = IVec( objPos );
	if ( iObjPos.x == boxPos.x ) {
		if ( objPos.y < boxPos.y ) {
			return getCorrection( objPos, Vec ( objPos.x, (float)boxPos.y), radius );
		} else {
			return getCorrection( objPos, Vec ( objPos.x, (float)(boxPos.y+1)), radius );
		}
	} else if ( iObjPos.y == boxPos.y ) {
		if ( objPos.x < boxPos.x ) {
			return getCorrection( objPos, Vec( (float)boxPos.x, objPos.y), radius );
		} else {
			return getCorrection( objPos, Vec( (float)(boxPos.x+1), objPos.y), radius );
		}
	} else if ( objPos.y < boxPos.y ) {
		if ( objPos.x < boxPos.x ) {
			return getCorrection( objPos, Vec( (float)boxPos.x, (float)boxPos.y), radius );
		} else {
			return getCorrection( objPos, Vec( (float)(boxPos.x+1), (float)boxPos.y), radius );
		}
	} else {
		if ( objPos.x < boxPos.x ) {
			return getCorrection( objPos, Vec( (float)boxPos.x, (float)(boxPos.y + 1) ), radius );
		} else {
			return getCorrection( objPos, Vec( (float)(boxPos.x+1), (float)(boxPos.y + 1) ), radius );
		}
	}
}

void Particle::think()
{

	if ( m_node ) {
		while ( m_node->checkEventWaiting() ) {
			eNet_Event type;
			eNet_NodeRole    remote_role;
			Net_ConnID       conn_id;

			BitStream *data = m_node->getNextEvent(&type, &remote_role, &conn_id);
			switch ( type ) {
					case eNet_EventUser:
					if ( data ) {
						//TODO: NetEvents event = (NetEvents)Encoding::decode(*data, EVENT_COUNT);

						// Only one event now, lua event
						int index = data->getInt(8);
						if(LuaEventDef* event = network.indexToLuaEvent(Network::LuaEventGroup::Particle, index)) {
							event->call(getLuaReference(), data);
						}
					}
					break;

					case eNet_EventRemoved: {
						deleteMe = true;
					}
					break;

					case eNet_EventInit: {
						LuaReference r = m_type->networkInit.get();
						if(r)
							(lua.call(r), getLuaReference(), conn_id)();
					}
					break;

					default:
					break;
			}
		}
	}
	if ( deleteMe )
		return;

	for ( int i = 0; i < m_type->repeat; ++i) {
		if ( m_health <= 0 && m_type->death ) {
			m_type->death->run(this);
			m_health = m_type->health;
			if ( deleteMe )
				break;
		}

		velocity().y += m_type->gravity;

		if ( m_type->acceleration ) {
			Vec dir(m_angle);
			if ( m_type->maxSpeed < 0 || Vec(velocity()).dotProduct(dir) < m_type->maxSpeed )
				velocity() += CVec(dir * m_type->acceleration);
		}

		velocity() *= m_type->damping;

		if ( m_type->radius > 0 ) // HAX
		{
			Vec averageCorrection;
			float radius = m_type->radius;
			float speedCorrection = m_type->bounceFactor;
			float friction = m_type->groundFriction;
			IVec iPos = IVec(Vec(pos()));
			int n = 0;
			int iradius = static_cast<int>(radius);
			for ( int y = -iradius; y <= iradius; ++y )
				for ( int x = -iradius; x <= iradius; ++x )
				{
					if ( !game.gameMap()->getMaterial( iPos.x + x, iPos.y + y ).particle_pass ) {
						averageCorrection += getCorrectionBox( pos() , iPos + IVec( x, y ), radius );
						++n;
					}
				}
			if ( n > 0 )
			{
				if ( averageCorrection.length() > 0 ) {
					averageCorrection /= (float)n;
					Vec tmpNorm = averageCorrection.normal();
					velocity() -= CVec( tmpNorm.perp() * tmpNorm.perpDotProduct(velocity()) ) * ( 1 - friction );
					pos() += CVec(averageCorrection);
					velocity() += CVec(averageCorrection) * speedCorrection * 2;
				}
			}
		}

		bool collision = false;
		if ( !game.gameMap()->getMaterial( roundAny(pos().x + velocity().x), roundAny(pos().y) ).particle_pass) {
			velocity().x *= -m_type->bounceFactor; // TODO: Precompute the negative of this
			velocity().y *= m_type->groundFriction;
			collision = true;
		}
		if ( !game.gameMap()->getMaterial( roundAny(pos().x), roundAny(pos().y + velocity().y) ).particle_pass) {
			velocity().y *= -m_type->bounceFactor; // TODO: Precompute the negative of this
			velocity().x *= m_type->groundFriction;
			collision = true;
		}
		if( collision ) {
			if ( m_type->groundCollision ) {
				m_type->groundCollision->run(this);
				if ( deleteMe )
					break;
			}
#ifndef DEDICATED_ONLY
			if ( !m_type->animOnGround && m_animator )
				m_animator->freeze(5); //I GOT DEFEATED!
#endif

		}

		//for ( vector< DetectEvent* >::iterator t = m_type->detectRanges.begin(); t != m_type->detectRanges.end(); ++t )
		foreach(t, m_type->detectRanges) {
			(*t)->check(this);
		}
		if ( deleteMe )
			break;

		//for ( vector< TimerEvent::State* >::iterator t = timer.begin(); t != timer.end(); t++)
		foreach(t, timer) {
			if ( t->tick() ) {
				t->event->run(this);
			}
			if ( deleteMe )
				break;
		}

		if(m_type->angularFriction) {
			if ( abs(m_angleSpeed) < m_type->angularFriction )
				m_angleSpeed = 0;
			else if ( m_angleSpeed < 0 )
				m_angleSpeed += m_type->angularFriction;
			else
				m_angleSpeed -= m_type->angularFriction;
		}

		m_angle += m_angleSpeed;
		m_angle.clamp();

		//Position update
		pos() += velocity();

#ifndef DEDICATED_ONLY
		// Animation
		if ( m_animator )
			m_animator->tick();

		// Alpha Fade
		if ( ( m_type->blender || m_type->lightHax ) && m_fadeSpeed ) {
			if ( fabs(m_alphaDest - m_alpha) < fabs(m_fadeSpeed) ) {
				m_fadeSpeed = 0;
				m_alpha = (float)m_alphaDest;
			} else
				m_alpha += m_fadeSpeed;
		}
#endif

	}
}

Angle Particle::getPointingAngle()
{
	return m_angle;
}

void Particle::addAngleSpeed( AngleDiff speed )
{
	m_angleSpeed += speed;
}

#ifndef DEDICATED_ONLY
void Particle::setAlphaFade(int frames, int dest)
{
	m_fadeSpeed = ( dest - m_alpha ) / frames;
	m_alphaDest = dest;
}
#endif

void Particle::customEvent( size_t index )
{
	if ( index < m_type->customEvents.size() && m_type->customEvents[index] ) {
		m_type->customEvents[index]->run(this);
	}
}

void Particle::damage( float amount, CWormInputHandler* damager )
{
	m_health -= amount;
}

void Particle::remove()
{
	if ( !m_node || m_node->getRole() == eNet_RoleAuthority ) {
		deleteMe = true;
	}
}

#ifndef DEDICATED_ONLY

void Particle::drawLine2Origin( CViewport* viewport, BlitterContext const& blitter)
{
	if(m_type->wupixels) {
		Vec rPos = viewport->convertCoordsPrec( pos() );
		Vec rOPos = viewport->convertCoordsPrec( m_origin );
		blitter.linewu(viewport->dest, rOPos.x, rOPos.y, rPos.x, rPos.y, m_type->colour);
	} else {
		IVec rPos = viewport->convertCoords( IVec(Vec(pos())) );
		IVec rOPos = viewport->convertCoords( IVec(m_origin) );
		blitter.line(viewport->dest, rOPos.x, rOPos.y, rPos.x, rPos.y, m_type->colour);
		//mooo.createPath( 7, 7);
		//mooo.render(where, x,y,x2,y2, m_type->colour);
		//mooo.createPath( 5, 10);
		//mooo.render(where, x,y,x2,y2, m_type->colour);
	}
}

void Particle::draw(CViewport* viewport)
{

	IVec rPos = viewport->convertCoords( IVec(Vec(pos())) );
	Vec rPosPrec = viewport->convertCoordsPrec( pos() );
	ALLEGRO_BITMAP* where = viewport->dest;
	int x = rPos.x;
	int y = rPos.y;

	BlitterContext blitter(m_type->blender, (int)m_alpha);

	if (!m_sprite) {
		if(!m_type->invisible) {
			if(m_type->wupixels)
				blitter.putpixelwu(where, rPosPrec.x, rPosPrec.y, m_type->colour);
			else
				blitter.putpixel(where, x, y, m_type->colour);
		}

		if ( m_type->line2Origin )
			drawLine2Origin( viewport, blitter );
	} else {
		if ( !m_type->culled ) {
			m_sprite->getSprite(m_animator->getFrame(), m_angle)->draw(where, x, y, blitter);

			//Blitters::drawSpriteRotate_solid_32(where, m_sprite->getSprite(m_animator->getFrame())->m_bitmap, x, y, -m_angle.toRad());
		} else {
			Sprite* renderSprite = m_sprite->getSprite(m_animator->getFrame(), m_angle);
			game.gameMap()->culledDrawSprite(renderSprite, viewport, IVec(Vec(pos())), (int)m_alpha );
		}
	}

	if (m_type->distortion) {
		m_type->distortion->apply( where, x, y, m_type->distortMagnitude );
	}
	if ( game.gameMap()->config()->darkMode && m_type->lightHax ) {
		game.gameMap()->culledDrawLight( m_type->lightHax, viewport, IVec(Vec(pos())), (int)m_alpha );
	}
}
#endif

void Particle::sendLuaEvent(LuaEventDef* event, eNet_SendMode mode, Net_U8 rules, BitStream* userdata, Net_ConnID connID)
{
	if(!m_node)
		return;
	BitStream* data = new BitStream;
	//addEvent(data, LuaEvent);
	data->addInt(event->idx, 8);
	if(userdata) {
		data->addBitStream(*userdata);
	}
	if(!connID)
		m_node->sendEvent(mode, rules, data);
	else
		m_node->sendEventDirect(mode, data, connID);
}

void Particle::makeReference()
{
	lua.pushFullReference(*this, metaTable);
}

/*
LuaReference Particle::getLuaReference()
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

void Particle::finalize()
{
	delete m_node;
	m_node = 0;
	m_type = 0; // This pointer may become invalid at any moment
}
