#include "net_worm.h"

#include "util/vec.h"
#include "util/angle.h"
#include "util/log.h"
#include "gusgame.h"
#include "weapon.h"
#include "weapon_type.h"
#include "CWorm.h"
#include "CGameObject.h"
#include "game/WormInputHandler.h"
#include "player_options.h"
#include "ninjarope.h"
#include "network.h"
#include "vector_replicator.h"
#include "posspd_replicator.h"
#include "encoding.h"
#include "gconsole.h"
#include "CMap.h"
#include "game/Game.h"

#include <math.h>
#include <vector>
#include "netstream.h"
#include "CWorm.h"

using namespace std;

Net_ClassID CWorm::classID = Net_Invalid_ID;

const float CWorm::MAX_ERROR_RADIUS = 10.0f;

void CWorm::NetWorm_Init(bool isAuthority)
{
	timeSinceLastUpdate = 1;
	
	m_playerID = INVALID_NODE_ID;
	m_node = new Net_Node();
	if (!m_node)
	{
		allegro_message("ERROR: Unable to create worm node.");
	}
	
	m_node->beginReplicationSetup();
	
		static Net_ReplicatorSetup posSetup( Net_REPFLAG_MOSTRECENT | Net_REPFLAG_INTERCEPT, Net_REPRULE_AUTH_2_PROXY | Net_REPRULE_OWNER_2_AUTH , Position, -1, 1000);
		
		//m_node->setInterceptID( static_cast<Net_InterceptID>(Position) );
		
		m_node->addReplicator(new PosSpdReplicator( &posSetup, &pos(), &velocity(), gusGame.level().vectorEncoding, gusGame.level().diffVectorEncoding ), true);
		
		static Net_ReplicatorSetup nrSetup( Net_REPFLAG_MOSTRECENT, Net_REPRULE_AUTH_2_PROXY | Net_REPRULE_OWNER_2_AUTH );
		
		m_node->addReplicator(new VectorReplicator( &nrSetup, &m_ninjaRope->getPosReference(), gusGame.level().vectorEncoding ), true);
		
		m_node->addReplicationFloat ((Net_Float*)&m_ninjaRope->getLengthReference(), 16, Net_REPFLAG_MOSTRECENT, Net_REPRULE_AUTH_2_PROXY | Net_REPRULE_OWNER_2_AUTH);
		
		static Net_ReplicatorSetup angleSetup( Net_REPFLAG_MOSTRECENT, Net_REPRULE_AUTH_2_PROXY | Net_REPRULE_OWNER_2_AUTH );
				
		m_node->addReplicationFloat ((Net_Float*)&health, 16, Net_REPFLAG_MOSTRECENT, Net_REPRULE_AUTH_2_ALL);
		
		m_node->addReplicator(new AngleReplicator( &angleSetup, &aimAngle), true );
		
		// Intercepted stuff
		m_node->setInterceptID( PlayerID );
		
		m_node->addReplicationInt( (Net_S32*)&m_playerID, 32, false, Net_REPFLAG_MOSTRECENT | Net_REPFLAG_INTERCEPT, Net_REPRULE_AUTH_2_ALL , INVALID_NODE_ID);
		
	m_node->endReplicationSetup();

	m_interceptor = new NetWormInterceptor( this );
	m_node->setReplicationInterceptor(m_interceptor);

	m_isAuthority = isAuthority;
	if( isAuthority)
	{
		m_node->setEventNotification(true, false); // Enables the eEvent_Init.
		if( !m_node->registerNodeDynamic(classID, network.getNetControl() ) )
			allegro_message("ERROR: Unable to register worm authority node.");
	}else
	{
		if( !m_node->registerRequestedNode( classID, network.getNetControl() ) )
		allegro_message("ERROR: Unable to register worm requested node.");
	}
	
	m_node->applyForNetLevel(1);
}

void CWorm::NetWorm_Shutdown()
{
	delete m_node;
	delete m_interceptor;
}

void CWorm::addEvent(Net_BitStream* data, CWorm::NetEvents event)
{
#ifdef COMPACT_EVENTS
	//data->addInt(event, Encoding::bitsOf(NetWorm::EVENT_COUNT - 1));
	Encoding::encode(*data, event, CWorm::EVENT_COUNT);
#else
	data->addInt(static_cast<int>(event),8 );
#endif
}

void CWorm::NetWorm_think()
{
	CWorm::think();
#ifndef DEDICATED_ONLY
	//renderPos += (pos - renderPos)*0.2;
	double fact = 1.0 / (1.0 + Vec(renderPos, pos()).length() / 4.0);
	renderPos = renderPos * (1.0f - (float)fact) + Vec(pos()) * (float)fact;
#endif

	++timeSinceLastUpdate;
	
	if ( !m_node )
		return;
	
	while ( m_node->checkEventWaiting() )
	{
		eNet_Event type;
		eNet_NodeRole    remote_role;
		Net_ConnID       conn_id;
		
		Net_BitStream *data = m_node->getNextEvent(&type, &remote_role, &conn_id);
		switch(type)
		{
		case eNet_EventUser:
			if ( data )
			{
#ifdef COMPACT_EVENTS
				//NetEvents event = (NetEvents)data->getInt(Encoding::bitsOf(EVENT_COUNT - 1));
				NetEvents event = (NetEvents)Encoding::decode(*data, EVENT_COUNT);
#else
				NetEvents event = (NetEvents)data->getInt(8);
#endif
				switch ( event )
				{
					case PosCorrection:
					{
						/*
						pos.x = data->getFloat(32);
						pos.y = data->getFloat(32);
						spd.x = data->getFloat(32);
						spd.y = data->getFloat(32);*/
						pos() = CVec(gusGame.level().vectorEncoding.decode<Vec>(*data));
						velocity() = CVec(gusGame.level().vectorEncoding.decode<Vec>(*data));
					}
					break;
					case Respawn:
					{
						Vec newpos = gusGame.level().vectorEncoding.decode<Vec>(*data);
						//newpos.x = data->getFloat(32);
						//newpos.y = data->getFloat(32);
						CWorm::respawn( newpos );
					}
					break;
					case Dig:
					{
						Vec digPos = gusGame.level().vectorEncoding.decode<Vec>(*data);
						Angle digAngle = Angle((int)data->getInt(Angle::prec));
						CWorm::dig(digPos, digAngle);
					}
					break;
					case Die:
					{
						m_lastHurt = gusGame.findPlayerWithID( data->getInt(32) );
						CWorm::die();
					}
					break;
					case ChangeWeapon:
					{
						//size_t weapIndex = data->getInt(Encoding::bitsOf(gusGame.weaponList.size() - 1));
						size_t weapIndex = Encoding::decode(*data, m_weapons.size());
						changeWeaponTo( weapIndex );
					}
					break;
					case WeaponMessage:
					{
						//size_t weapIndex = data->getInt(Encoding::bitsOf(gusGame.weaponList.size() - 1));
						size_t weapIndex = Encoding::decode(*data, m_weapons.size());
						if ( weapIndex < m_weapons.size() && m_weapons[weapIndex] )
							m_weapons[weapIndex]->recieveMessage( data );
					}
					break;
					case SetWeapon:
					{
						size_t index = Encoding::decode(*data, gusGame.options.maxWeapons);
						if ( data->getBool() )
						{
							size_t weaponIndex = Encoding::decode(*data, gusGame.weaponList.size());
							if(weaponIndex < gusGame.weaponList.size())
								CWorm::setWeapon( index, gusGame.weaponList[weaponIndex] );
							else
								CWorm::setWeapon( index, 0 );
						}
						else
						{
							CWorm::setWeapon( index, 0 );
						}
					}
					break;
					case ClearWeapons:
					{
						DLOG("Clearing weapons");
						CWorm::clearWeapons();
					}
					break;
					case SYNC:
					{
						bAlive = data->getBool();
						m_ninjaRope->active = data->getBool();
						//currentWeapon = data->getInt(Encoding::bitsOf(gusGame.weaponList.size() - 1));
						currentWeapon = Encoding::decode(*data, m_weapons.size());
						CWorm::clearWeapons();
						while ( data->getBool() )
						{
							size_t index = Encoding::decode(*data, m_weapons.size());
							size_t weapTypeIndex = Encoding::decode(*data, gusGame.weaponList.size());
							if(weapTypeIndex < gusGame.weaponList.size() && index < m_weapons.size())
							{
								luaDelete(m_weapons[index]); m_weapons[index] = 0; 
								m_weapons[index] = new Weapon(gusGame.weaponList[weapTypeIndex], this);
							}
						}
					}
					break;
					
					case LuaEvent:
					{
						int index = data->getInt(8);
						if(LuaEventDef* event = network.indexToLuaEvent(Network::LuaEventGroup::Worm, index))
						{
							event->call(getLuaReference(), data);
						}
					}
					break;
					
					case EVENT_COUNT: break;
				}
			}
			break;
			
			case eNet_EventInit:
			{
				sendSyncMessage( conn_id );
			}
			break;
			
			default: break; // Annoying warnings >:O
		}
	}
}

void CWorm::NetWorm_sendLuaEvent(LuaEventDef* event, eNet_SendMode mode, Net_U8 rules, Net_BitStream* userdata, Net_ConnID connID)
{
	if(!m_node) return;
	Net_BitStream* data = new Net_BitStream;
	addEvent(data, LuaEvent);
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

void CWorm::correctOwnerPosition()
{
	Net_BitStream *data = new Net_BitStream;
	addEvent(data, PosCorrection);
	/*
	data->addFloat(pos.x,32); // Maybe this packet is too heavy...
	data->addFloat(pos.y,32);
	data->addFloat(spd.x,32);
	data->addFloat(spd.y,32);*/
	gusGame.level().vectorEncoding.encode<Vec>(*data, pos()); // ...nah ;o
	gusGame.level().vectorEncoding.encode<Vec>(*data, velocity());
	m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_AUTH_2_OWNER, data);
}

void CWorm::NetWorm_assignOwner( CWormInputHandler* owner)
{
	CWorm::assignOwner(owner);
	m_playerID = m_owner->getNodeID();
}

void CWorm::setOwnerId( Net_ConnID _id )
{
	m_node->setOwner(_id,true);
}


void CWorm::sendSyncMessage( Net_ConnID id )
{
	Net_BitStream *data = new Net_BitStream;
	addEvent(data, SYNC);
	data->addBool(getAlive());
	data->addBool(m_ninjaRope->active);
	//data->addInt(currentWeapon, Encoding::bitsOf(gusGame.weaponList.size() - 1));
	Encoding::encode(*data, currentWeapon, m_weapons.size());
	
	for( size_t i = 0; i < m_weapons.size(); ++i )
	{
		if ( m_weapons[i] )
		{
			data->addBool(true);
			Encoding::encode(*data, i, m_weapons.size());
			Encoding::encode(*data, m_weapons[i]->getType()->getIndex(), gusGame.weaponList.size());
		}
	}
	data->addBool(false);
	
	m_node->sendEventDirect(eNet_ReliableOrdered, data, id);
}

void CWorm::NetWorm_sendWeaponMessage( int index, Net_BitStream* weaponData, Net_U8 repRules )
{
	Net_BitStream *data = new Net_BitStream;
	addEvent(data, WeaponMessage);
	//data->addInt(index, Encoding::bitsOf(gusGame.weaponList.size() - 1));
	Encoding::encode(*data, index, m_weapons.size());
	data->addBitStream( weaponData );
	m_node->sendEvent(eNet_ReliableOrdered, repRules, data);
}

Net_NodeID CWorm::getNodeID()
{
	if ( m_node )
		return m_node->getNetworkID();
	else
		return INVALID_NODE_ID;
}

void CWorm::NetWorm_respawn()
{
	if ( m_isAuthority && m_node )
	{
		CWorm::respawn();
		if ( getAlive() )
		{
			Net_BitStream *data = new Net_BitStream;
			addEvent(data, Respawn);
			/*
			data->addFloat(pos.x,32);
			data->addFloat(pos.y,32);*/
			gusGame.level().vectorEncoding.encode<Vec>(*data, pos());
			m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_AUTH_2_ALL, data);
		}
	}
}

void CWorm::NetWorm_dig()
{
	if ( m_isAuthority && m_node )
	{
		CWorm::dig();
		if ( getAlive() )
		{
			Net_BitStream *data = new Net_BitStream;
			addEvent(data, Dig);
			gusGame.level().vectorEncoding.encode<Vec>(*data, pos());
			data->addInt(int(getPointingAngle()), Angle::prec);
			m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_AUTH_2_ALL, data);
		}
	}
}

void CWorm::NetWorm_die()
{
	if ( m_isAuthority && m_node )
	{
		Net_BitStream *data = new Net_BitStream;
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
		CWorm::die();
	}
}

void CWorm::NetWorm_changeWeaponTo( unsigned int weapIndex )
{
	if ( m_node )
	{
		Net_BitStream *data = new Net_BitStream;
		addEvent(data, ChangeWeapon);
		Encoding::encode(*data, weapIndex, m_weapons.size());
		m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_OWNER_2_AUTH | Net_REPRULE_AUTH_2_PROXY, data);
		CWorm::changeWeaponTo( weapIndex );
	}
}

void CWorm::NetWorm_setWeapon( size_t index, WeaponType* type )
{
	if ( !network.isClient() )
	{
		CWorm::setWeapon( index, type );
		if ( m_node )
		{
			Net_BitStream *data = new Net_BitStream;
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

void CWorm::NetWorm_clearWeapons()
{
	if ( !network.isClient() )
	{
		CWorm::clearWeapons();
		if ( m_node )
		{
			Net_BitStream *data = new Net_BitStream;
			addEvent(data, ClearWeapons);
			m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_AUTH_2_ALL, data);
		}
	}
}

void CWorm::NetWorm_damage( float amount, CWormInputHandler* damager )
{
	if ( m_isAuthority )
	{
		CWorm::damage( amount, damager );
	}
}

void CWorm::NetWorm_finalize()
{
	CWorm::finalize();
	delete m_node; m_node = 0;
	delete m_interceptor; m_interceptor = 0;
}

NetWormInterceptor::NetWormInterceptor( CWorm* parent )
{
	m_parent = parent;
}

bool NetWormInterceptor::inPreUpdateItem (Net_Node *_node, Net_ConnID _from, eNet_NodeRole _remote_role, Net_Replicator *_replicator)
{
	switch ( _replicator->getSetup()->getInterceptID() )
	{
		case CWorm::PlayerID:
		{
			Net_NodeID recievedID = *static_cast<Net_U32*>(_replicator->peekData());
			list<CWormInputHandler*>::iterator playerIter;
			for ( playerIter = game.players.begin(); playerIter != game.players.end(); playerIter++)
			{
				if ( (*playerIter)->getNodeID() == recievedID )
				{
					(*playerIter)->assignWorm(m_parent);
				}
			}
		}
		break;
		/*case NetWorm::Position:
		{
			Vec recievedPos = *static_cast<Vec*>(_replicator->peekData());
			Vec speedPrediction = (recievedPos - m_parent->lastPosUpdate) / m_parent->timeSinceLastUpdate;
			m_parent->lastPosUpdate = recievedPos;
			m_parent->timeSinceLastUpdate = 0;
			m_parent->spd = m_parent->spd*0.2 + speedPrediction*0.8;
			return true;
		} break;*/
	}
	return true;
}

bool NetWormInterceptor::outPreUpdateItem (Net_Node* node, Net_ConnID from, eNet_NodeRole remote_role, Net_Replicator* replicator)
{

	switch ( replicator->getSetup()->getInterceptID() )
	{
		case CWorm::Position:
			if(!m_parent->getAlive()) // Prevent non-active worms from replicating position
				return false;
		break;
	}

	return true;
}
