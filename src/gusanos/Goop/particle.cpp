#include "particle.h"

#include "game.h"
#include "base_object.h"
#include "base_worm.h"
#include "base_player.h"
#include "part_type.h"
#ifndef DEDSERV
#include "gfx.h"
#include "sprite.h"
#include "sprite_set.h"
#include "base_animator.h"
#include "blitters/blitters.h"
#include "viewport.h"
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
#include <zoidcom.h>
#include "posspd_replicator.h"

#include <vector>
#include <iostream>
#define BOOST_NO_MT
#include <boost/pool/pool.hpp>

using namespace std;

namespace
{

	boost::pool<> particlePool(sizeof(Particle));
}

class ParticleInterceptor : public ZCom_NodeReplicationInterceptor
{
public:
	enum type
	{
		Position
	};
	
	ParticleInterceptor( Particle* parent_ )
	: parent(parent_)
	{
	}

	bool inPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator, zU32 _estimated_time_sent)
	{ return true; }

	// Not used virtual stuff
	void outPreReplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role)
	{
		ZCom_BitStream *type = new ZCom_BitStream;
		Encoding::encode(*type, parent->m_type->getIndex(), partTypeList.size());
		if(parent->m_owner)
			type->addInt(parent->m_owner->getNodeID(), 32);
		else
			type->addInt(0, 32);
		parent->m_node->setAnnounceData( type );
	}
	
	void outPreDereplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role)
	{}
	
	bool outPreUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role)
	{ return true; }
	
	bool outPreUpdateItem(ZCom_Node* node, ZCom_ConnID from, eZCom_NodeRole remote_role, ZCom_Replicator* replicator)
	{
		switch ( replicator->getSetup()->getInterceptID() )
		{
			case Position:
				if(!(parent->flags & Particle::RepPos))
				{
					return false;
				}
			break;
		}
		
		return true;
	}
	
	void outPostUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits)
	{}
	
	bool inPreUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role)
	{ return true; }
	
	void inPostUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits)
	{}

private:
	Particle* parent;
};

LuaReference Particle::metaTable;

ZCom_ClassID Particle::classID = ZCom_Invalid_ID;

void* Particle::operator new(size_t count)
{

	assert(count <= sizeof(Particle));
	return particlePool.malloc();
}


void Particle::operator delete(void* block)
{
	particlePool.free(block);
}

Particle::Particle(PartType *type, Vec pos_, Vec spd_, int dir, BasePlayer* owner, Angle angle)
: BaseObject(owner, pos_, spd_), /*m_dir(dir), */m_type(type)
, m_health(type->health), m_angle(angle), m_angleSpeed(0)
#ifndef DEDSERV
, m_alpha(m_type->alpha), m_fadeSpeed(0), m_animator(0)
, m_alphaDest(255), m_sprite(m_type->sprite)
#endif
, m_origin(pos_)
, m_node(0), interceptor(0)
{
	m_type->touch();
	
	m_angle.clamp();
	
	flags = (dir > 0 ? FaceRight : 0)
	        | RepPos;
	
#ifndef DEDSERV
	if ( m_sprite )
	{
		m_animator = m_type->allocateAnimator();
	}
#endif
	// If it's deleted, we must allocate animator so that we can render it,
	// but we don't need any timers.
	
	if ( m_type->creation )
	{
		m_type->creation->run(this);
		if(deleteMe)
			return;
	}
	
	//for ( vector< TimerEvent* >::iterator i = m_type->timer.begin(); i != m_type->timer.end(); i++)
	foreach(i, m_type->timer)
	{
		timer.push_back( (*i)->createState() );
	}
	
	
}

Particle::~Particle()
{
#ifndef DEDSERV
	delete m_animator;
#endif
	delete m_node;
	delete interceptor;
}

void Particle::assignNetworkRole( bool authority )
{
	m_node = new ZCom_Node();
	/* operator new never returns 0
	if (!m_node)
	{
		allegro_message("ERROR: Unable to create particle node.");
	}*/

	m_node->beginReplicationSetup(0);
	
		static ZCom_ReplicatorSetup posSetup( ZCOM_REPFLAG_MOSTRECENT | ZCOM_REPFLAG_INTERCEPT, ZCOM_REPRULE_AUTH_2_ALL, ParticleInterceptor::Position, -1, 1000);
		
		m_node->addReplicator(new PosSpdReplicator( &posSetup, &pos, &spd, game.level.vectorEncoding, game.level.diffVectorEncoding ), true);
		
	m_node->endReplicationSetup();
	
	interceptor = new ParticleInterceptor( this );
	m_node->setReplicationInterceptor(interceptor);

	//DLOG("Registering node, type " << m_type->getIndex() << " of " << partTypeList.size());
	if( authority )
	{
		
		//DLOG("Announce data set");
		m_node->setEventNotification(true, false); // Enables the eEvent_Init.
		if( !m_node->registerNodeDynamic(classID, network.getZControl() ) )
			allegro_message("ERROR: Unable to register particle authority node.");
	}else
	{
		m_node->setEventNotification(false, true); // Same but for the remove event.
		//DLOG("Event notification set");
		if( !m_node->registerRequestedNode( classID, network.getZControl() ) )
			allegro_message("ERROR: Unable to register particle requested node.");
	}
	
	//DLOG("Node registered");

	m_node->applyForZoidLevel(1);
	
	//DLOG("Applied for zoidlevel");
}

inline Vec getCorrection( const Vec& objPos, const Vec& pointPos, float radius )
{
	Vec diff = pointPos - objPos;
	if ( diff.lengthSqr() < radius*radius )
	{
		float lengthDiff = diff.length() - radius;
		return diff.normal() * lengthDiff;
	}else
	{
		return Vec();
	}
}

Vec getCorrectionBox( const Vec& objPos, const IVec& boxPos, float radius )
{
	IVec iObjPos = IVec( objPos );
	if ( iObjPos.x == boxPos.x )
	{
		if ( objPos.y < boxPos.y )
		{
			return getCorrection( objPos, Vec ( objPos.x, boxPos.y), radius );
		}else
		{
			return getCorrection( objPos, Vec ( objPos.x, boxPos.y+1), radius );
		}
	}else if ( iObjPos.y == boxPos.y )
	{
		if ( objPos.x < boxPos.x )
		{
			return getCorrection( objPos, Vec( boxPos.x, objPos.y), radius );
		}else
		{
			return getCorrection( objPos, Vec( boxPos.x+1, objPos.y), radius );
		}
	}else if ( objPos.y < boxPos.y )
	{
		if ( objPos.x < boxPos.x )
		{
			return getCorrection( objPos, Vec( boxPos.x, boxPos.y), radius );
		}else
		{
			return getCorrection( objPos, Vec( boxPos.x+1, boxPos.y), radius );
		}
	}else
	{
		if ( objPos.x < boxPos.x )
		{
			return getCorrection( objPos, Vec( boxPos.x, boxPos.y + 1 ), radius );
		}else
		{
			return getCorrection( objPos, Vec( boxPos.x+1, boxPos.y + 1), radius );
		}
	}
}

void Particle::think()
{

	if ( m_node )
	{
		while ( m_node->checkEventWaiting() )
		{
			eZCom_Event type;
			eZCom_NodeRole    remote_role;
			ZCom_ConnID       conn_id;
			
			ZCom_BitStream *data = m_node->getNextEvent(&type, &remote_role, &conn_id);
			switch ( type )
			{
				case eZCom_EventUser:
				if ( data )
				{
					//TODO: NetEvents event = (NetEvents)Encoding::decode(*data, EVENT_COUNT);
					
					// Only one event now, lua event
					int index = data->getInt(8);
					if(LuaEventDef* event = network.indexToLuaEvent(Network::LuaEventGroup::Particle, index))
					{
						event->call(getLuaReference(), data);
					}
				}
				break;
				
				case eZCom_EventRemoved:
				{
					deleteMe = true;
				}
				break;
				
				case eZCom_EventInit:
				{
					LuaReference r = m_type->networkInit.get();
					if(r)
						(lua.call(r), getLuaReference(), conn_id)();
				}
				break;
				
				default: break;
			}
		}
	}
	if ( deleteMe ) return;
	
	for ( int i = 0; i < m_type->repeat; ++i)
	{
		if ( m_health <= 0 && m_type->death )
		{
			m_type->death->run(this);
			m_health = m_type->health;
			if ( deleteMe ) break;
		}
	
		spd.y += m_type->gravity;

		if ( m_type->acceleration )
		{
			Vec dir(m_angle);
			if ( m_type->maxSpeed < 0 || spd.dotProduct(dir) < m_type->maxSpeed )
				spd += dir * m_type->acceleration;
		}
				
		spd *= m_type->damping;
		
		if ( m_type->radius > 0 ) // HAX
		{
			Vec averageCorrection;
			float radius = m_type->radius;
			float speedCorrection = m_type->bounceFactor;
			float friction = m_type->groundFriction;
			IVec iPos = IVec( pos );
			int n = 0;
			int iradius = static_cast<int>(radius);
			for ( int y = -iradius; y <= iradius; ++y )
			for ( int x = -iradius; x <= iradius; ++x )
			{
				if ( !game.level.getMaterial( iPos.x + x, iPos.y + y ).particle_pass )
				{
					averageCorrection += getCorrectionBox( pos , iPos + IVec( x, y ), radius );
					++n;
				}
			}
			if ( n > 0 )
			{
				if ( averageCorrection.length() > 0 )
				{
					averageCorrection /= n;
					Vec tmpNorm = averageCorrection.normal();
					spd -= ( tmpNorm.perp() * tmpNorm.perpDotProduct(spd) ) * ( 1 - friction );
					pos += averageCorrection;
					spd += averageCorrection* speedCorrection * 2;
				}
			}
		}

		bool collision = false;
		if ( !game.level.getMaterial( roundAny(pos.x + spd.x), roundAny(pos.y) ).particle_pass)
		{
			spd.x *= -m_type->bounceFactor; // TODO: Precompute the negative of this
			spd.y *= m_type->groundFriction;
			collision = true;
		}
		if ( !game.level.getMaterial( roundAny(pos.x), roundAny(pos.y + spd.y) ).particle_pass)
		{
			spd.y *= -m_type->bounceFactor; // TODO: Precompute the negative of this
			spd.x *= m_type->groundFriction;
			collision = true;
		}
		if( collision )
		{
			if ( m_type->groundCollision )
			{
				m_type->groundCollision->run(this);
				if ( deleteMe ) break;
			}
#ifndef DEDSERV
			if ( !m_type->animOnGround && m_animator )
				m_animator->freeze(5); //I GOT DEFEATED!
#endif
		}

		//for ( vector< DetectEvent* >::iterator t = m_type->detectRanges.begin(); t != m_type->detectRanges.end(); ++t )
		foreach(t, m_type->detectRanges)
		{
			(*t)->check(this);
		}
		if ( deleteMe ) break;
		
		//for ( vector< TimerEvent::State* >::iterator t = timer.begin(); t != timer.end(); t++)
		foreach(t, timer)
		{
			if ( t->tick() )
			{
				t->event->run(this);
			}
			if ( deleteMe ) break;
		}
		
		if(m_type->angularFriction)
		{
			if ( abs(m_angleSpeed) < m_type->angularFriction ) m_angleSpeed = 0;
			else if ( m_angleSpeed < 0 ) m_angleSpeed += m_type->angularFriction;
			else m_angleSpeed -= m_type->angularFriction;
		}
		
		m_angle += m_angleSpeed;
		m_angle.clamp();
		
		//Position update
		pos += spd;
		
#ifndef DEDSERV
		// Animation
		if ( m_animator ) m_animator->tick();
		
		// Alpha Fade
		if ( ( m_type->blender || m_type->lightHax ) && m_fadeSpeed )
		{
			if ( fabs(m_alphaDest - m_alpha) < fabs(m_fadeSpeed) )
			{
				m_fadeSpeed = 0;
				m_alpha = m_alphaDest;
			}
			else
				m_alpha += m_fadeSpeed;
		}
#endif
	}
}

Angle Particle::getAngle()
{
	return m_angle;
}

void Particle::addAngleSpeed( AngleDiff speed )
{
	m_angleSpeed += speed;
}

#ifndef DEDSERV
void Particle::setAlphaFade(int frames, int dest)
{
	m_fadeSpeed = ( dest - m_alpha ) / frames;
	m_alphaDest = dest;
}
#endif

void Particle::customEvent( size_t index )
{
	if ( index < m_type->customEvents.size() && m_type->customEvents[index] )
	{
		m_type->customEvents[index]->run(this);
	}
}

void Particle::damage( float amount, BasePlayer* damager )
{
	m_health -= amount;
}

void Particle::remove()
{
	if ( !m_node || m_node->getRole() == eZCom_RoleAuthority )
	{
		deleteMe = true;
	}
}

#ifndef DEDSERV

void Particle::drawLine2Origin( Viewport* viewport, BlitterContext const& blitter)
{
	if(m_type->wupixels)
	{
		Vec rPos = viewport->convertCoordsPrec( pos );
		Vec rOPos = viewport->convertCoordsPrec( m_origin );
		blitter.linewu(viewport->dest, rOPos.x, rOPos.y, rPos.x, rPos.y, m_type->colour);
	}
	else
	{
		IVec rPos = viewport->convertCoords( IVec(pos) );
		IVec rOPos = viewport->convertCoords( IVec(m_origin) );
		blitter.line(viewport->dest, rOPos.x, rOPos.y, rPos.x, rPos.y, m_type->colour);
		//mooo.createPath( 7, 7);
		//mooo.render(where, x,y,x2,y2, m_type->colour);
		//mooo.createPath( 5, 10);
		//mooo.render(where, x,y,x2,y2, m_type->colour);
	}
}

void Particle::draw(Viewport* viewport)
{

	IVec rPos = viewport->convertCoords( IVec(pos) );
	Vec rPosPrec = viewport->convertCoordsPrec( pos );
	BITMAP* where = viewport->dest;
	int x = rPos.x;
	int y = rPos.y;
	
	BlitterContext blitter(m_type->blender, (int)m_alpha);
	
	if (!m_sprite)
	{
		if(!m_type->invisible)
		{
			if(m_type->wupixels)
				blitter.putpixelwu(where, rPosPrec.x, rPosPrec.y, m_type->colour);
			else
				blitter.putpixel(where, x, y, m_type->colour);
		}
				
		if ( m_type->line2Origin )
			drawLine2Origin( viewport, blitter );
	}
	else
	{
		if ( !m_type->culled )
		{
			m_sprite->getSprite(m_animator->getFrame(), m_angle)->draw(where, x, y, blitter);
			
			//Blitters::drawSpriteRotate_solid_32(where, m_sprite->getSprite(m_animator->getFrame())->m_bitmap, x, y, -m_angle.toRad()); 
		}
		else
		{
			Sprite* renderSprite = m_sprite->getSprite(m_animator->getFrame(), m_angle);
			game.level.culledDrawSprite(renderSprite, viewport, IVec(pos), (int)m_alpha );
		}
	}

	if (m_type->distortion)
	{
		m_type->distortion->apply( where, x, y, m_type->distortMagnitude );
	}
	if ( game.level.config()->darkMode && m_type->lightHax )
	{
		game.level.culledDrawLight( m_type->lightHax, viewport, IVec(pos), (int)m_alpha );
	}
}
#endif

void Particle::sendLuaEvent(LuaEventDef* event, eZCom_SendMode mode, zU8 rules, ZCom_BitStream* userdata, ZCom_ConnID connID)
{
	if(!m_node) return;
	ZCom_BitStream* data = new ZCom_BitStream;
	//addEvent(data, LuaEvent);
	data->addInt(event->idx, 8);
	if(userdata)
	{
		data->addBitStream(userdata);
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
	delete m_node; m_node = 0;
	m_type = 0; // This pointer may become invalid at any moment
}
