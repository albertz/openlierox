#include "base_player.h"
#include "worm.h"
#include "ninjarope.h"
#include "net_worm.h"
#include "player_options.h"
#include "objects_list.h"
#include "gconsole.h"
#include "encoding.h"
#include "weapon_type.h"
#include "util/log.h"

#include "glua.h"
#include "lua/bindings-game.h"
#include "game.h"

#include <zoidcom.h>
#include "network.h"
//#include <allegro.h>
#include <list>

using namespace std;

ZCom_ClassID BasePlayer::classID = ZCom_Invalid_ID;

BasePlayer::Stats::~Stats()
{
	if(luaData)
		lua.destroyReference(luaData);
}

/*
LuaReference BasePlayer::metaTable()
{
	return LuaBindings::playerMetaTable;
}
*/

BasePlayer::BasePlayer(shared_ptr<PlayerOptions> options, BaseWorm* worm)
: m_options(options), stats(new Stats), deleteMe(false)
, m_worm(0), m_id(0) // TODO: make a invalid_connection_id define thingy
, m_wormID(INVALID_NODE_ID)
, m_node(0), m_interceptor(0)
, m_isAuthority(false)
, colour(options->colour)
, team(options->team)
, luaData(0), local(false), deleted(false)
{
	localChangeName(m_options->name);
	m_options->clearChangeFlags();
	if(worm)
		assignWorm(worm);
}

LuaReference BasePlayer::getLuaReference()
{
	assert(!deleted);
	if(luaReference)
		return luaReference;
	else
	{
		lua.pushFullReference(*this, LuaBindings::BasePlayerMetaTable);
		luaReference = lua.createReference();
		return luaReference;
	}
}

void BasePlayer::pushLuaReference()
{
	lua.push(getLuaReference());
}

void BasePlayer::deleteThis()
{
	EACH_CALLBACK(i, playerRemoved)
	{
		(lua.call(*i), getLuaReference())();
	}
	
	deleted = true; // We set this after the callback loop otherwise getLuaReference() will think the player is already deleted
	
	for (Grid::iterator objIter = game.objects.beginAll(); objIter; ++objIter)
	{
		objIter->removeRefsToPlayer(this);
	}

	removeWorm();

	delete m_node; m_node = 0;
	delete m_interceptor; m_interceptor = 0;

	if(luaReference)
	{
		lua.destroyReference(luaReference);
		luaReference.reset();
	}
	else
	{
		delete this;
	}
}



BasePlayer::~BasePlayer()
{
	delete m_node; m_node = 0;
	delete m_interceptor; m_interceptor = 0;
	m_worm = 0;
}

void BasePlayer::removeWorm()
{
	if ( m_worm )
	{
		m_worm->deleteMe = true;
		if ( m_worm->getNinjaRopeObj() ) 
			m_worm->getNinjaRopeObj()->deleteMe = true;
		m_worm = 0;
	}
}

void BasePlayer::addEvent(ZCom_BitStream* data, BasePlayer::NetEvents event)
{
#ifdef COMPACT_EVENTS
//	data->addInt(event, Encoding::bitsOf(BasePlayer::EVENT_COUNT - 1));
	Encoding::encode(*data, event, BasePlayer::EVENT_COUNT);
#else
	data->addInt(static_cast<int>(event),8 );
#endif
}

void BasePlayer::think()
{
	subThink();
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
#ifdef COMPACT_EVENTS
					NetEvents event = (NetEvents)data->getInt(Encoding::bitsOf(EVENT_COUNT - 1));
#else
					NetEvents event = (NetEvents)data->getInt(8);
#endif
					switch ( event )
					{
						case ACTION_START: // ACTION TART LOL TBH
						{
#ifdef COMPACT_ACTIONS
							BaseActions action = (BaseActions)data->getInt(Encoding::bitsOf(ACTION_COUNT - 1));
#else
							BaseActions action = (BaseActions)data->getInt(8);
#endif
							if ( ( action == FIRE ) && m_worm)
							{
								m_worm->aimAngle = Angle((int)data->getInt(Angle::prec));
								if(m_worm->aimAngle > Angle(180.0))
								{
									m_worm->aimAngle = -m_worm->aimAngle;
									m_worm->setDir(-1);
								}
								else
								{
									m_worm->setDir(1);
								}
								m_worm->aimAngle.clamp();
							}
							
							baseActionStart(action);
						}
						break;
						case ACTION_STOP:
						{
#ifdef COMPACT_ACTIONS
							BaseActions action = (BaseActions)data->getInt(Encoding::bitsOf(ACTION_COUNT - 1));
#else
							BaseActions action = (BaseActions)data->getInt(8);
#endif
							baseActionStop(action);
						}
						break;
						case SYNC:
						{
							stats->kills = data->getInt(32);
							stats->deaths = data->getInt(32);
							m_name = data->getStringStatic();
							colour = data->getInt(24);
							team = data->getSignedInt(8);
							m_options->uniqueID = static_cast<unsigned int>(data->getInt(32));
						}
						break;
						
						case NAME_CHANGE:
						{
							changeName_( data->getStringStatic() );
						}
						break;

						case COLOR_CHANGE:
						{
							changeColor_(data->getInt(24));
						}
						break;
						
						case TEAM_CHANGE:
						{
							changeTeam_(data->getSignedInt(8));
						}
						break;
						
						case CHAT_MSG:
						{
							sendChatMsg( data->getStringStatic() );
						}
						break;
						
						case SELECT_WEAPONS:
						{
							size_t size = Encoding::decode(*data, game.options.maxWeapons+1);
							if(size > game.options.maxWeapons)
								break; // Avoid horrible crashes etc.
								
							vector<WeaponType*> weaps(size,0);
							for ( size_t i = 0; i < size; ++i )
							{
								weaps[i] = game.weaponList[Encoding::decode(*data, game.weaponList.size())];
							}
							selectWeapons(weaps);
						}
						break;
						
						case LuaEvent:
						{
							int index = data->getInt(8);
							if(LuaEventDef* event = network.indexToLuaEvent(Network::LuaEventGroup::Player, index))
							{
								event->call(getLuaReference(), data);
							}
						}
						break;
						
						case EVENT_COUNT: break; // Do nothing
					}
				}
				break;
				case eZCom_EventInit:
				{
					sendSyncMessage( conn_id );
					
					EACH_CALLBACK(i, playerNetworkInit)
					{
						(lua.call(*i), getLuaReference(), conn_id)();
					}
				}
				break;
				case eZCom_EventRemoved:
				{
					deleteMe = true;
				}
				break;
				
				default: break; // Got tired of spam that makes me miss important warnings
			}
		}
	}
	
	if ( m_options->nameChanged() )
	{
		if ( m_node )
		{
			if ( m_isAuthority )
			{
				changeName_( m_options->name );
			}
			else
			{
				nameChangePetition();
			}
		}
		else
		{
			changeName_(m_options->name);
		}
	}
	
	if( m_options->colorChanged() )
	{
		if ( m_node )
		{
			if(m_isAuthority)
				changeColor_(m_options->colour);
			else
				colorChangePetition_(m_options->colour);
		}
		else
			changeColor_(m_options->colour);
	}
	
	if( m_options->teamChanged() )
	{
		if ( m_node )
		{
			if(m_isAuthority)
				changeTeam_(m_options->team);
			else
				teamChangePetition_(m_options->team);
		}
		else
			changeTeam_(m_options->team);
	}
	
	EACH_CALLBACK(i, playerUpdate)
	{
		(lua.call(*i), getLuaReference())();
	}
}

void BasePlayer::sendLuaEvent(LuaEventDef* event, eZCom_SendMode mode, zU8 rules, ZCom_BitStream* userdata, ZCom_ConnID connID)
{
	if(!m_node) return;
	ZCom_BitStream* data = new ZCom_BitStream;
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

void BasePlayer::addActionStart(ZCom_BitStream* data, BasePlayer::BaseActions action)
{
	addEvent(data, BasePlayer::ACTION_START);
#ifdef COMPACT_ACTIONS
	data->addInt(action, Encoding::bitsOf(BasePlayer::ACTION_COUNT - 1));
#else
	data->addInt(static_cast<int>(action),8 );
#endif
}

void BasePlayer::addActionStop(ZCom_BitStream* data, BasePlayer::BaseActions action)
{
	addEvent(data, BasePlayer::ACTION_STOP);
#ifdef COMPACT_ACTIONS
	data->addInt(action, Encoding::bitsOf(BasePlayer::ACTION_COUNT - 1));
#else
	data->addInt(static_cast<int>(action),8 );
#endif
}

bool nameIsTaken( const std::string& name )
{
	list<BasePlayer*>::iterator playerIter;
	for ( playerIter = game.players.begin(); playerIter != game.players.end(); playerIter++ )
	{
		if ( (*playerIter)->m_name == name )
		{
			return true;
		}
	}
	return false;
}

void BasePlayer::changeName( std::string const& name)
{
	m_options->changeName(name);
}

void BasePlayer::localChangeName(std::string const& name, bool forceChange)
{
	if(m_name == name)
		return;
	
	string nameToUse = name;
	
	if(!forceChange)
	{
		int nameSuffix = 0;
		while ( nameIsTaken( nameToUse ) )
		{
			++nameSuffix;
			nameToUse = name + "(" + cast<string>(nameSuffix) + ")";
		}
	}
	
	if(!m_name.empty())
		console.addLogMsg( "* " + m_name + " CHANGED NAME TO " + nameToUse );

	m_name = nameToUse;
}

void BasePlayer::changeName_( const std::string& name )
{
	if(m_isAuthority)
	{
		localChangeName(name);
		if ( m_node )
		{
			ZCom_BitStream *data = new ZCom_BitStream;
			addEvent(data, NAME_CHANGE);
			data->addString( m_name.c_str() );
			m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_ALL, data);
		}
	}
	else
	{
		localChangeName(name, true);
	}
}

void BasePlayer::nameChangePetition()
{
	if ( m_node )
	{
		ZCom_BitStream *data = new ZCom_BitStream;
		addEvent(data, NAME_CHANGE);
		data->addString( m_options->name.c_str() );
		m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_OWNER_2_AUTH, data);
	}
}

void BasePlayer::changeColor_( int colour_ )
{
	if(m_isAuthority)
	{
		colour = colour_;
		if ( m_node )
		{
			ZCom_BitStream *data = new ZCom_BitStream;
			addEvent(data, COLOR_CHANGE);
			data->addInt( colour, 24 );
			m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_ALL, data);
		}
	}
	else
	{
		colour = colour_;
	}
}

void BasePlayer::colorChangePetition_( int colour_ )
{
	if ( m_node )
	{
		ZCom_BitStream *data = new ZCom_BitStream;
		addEvent(data, COLOR_CHANGE);
		data->addInt( colour_, 24 );
		m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_OWNER_2_AUTH, data);
	}
}

void BasePlayer::changeTeam_( int team_ )
{
	if(m_isAuthority)
	{
		team = team_;
		if ( m_node )
		{
			ZCom_BitStream *data = new ZCom_BitStream;
			addEvent(data, TEAM_CHANGE);
			data->addSignedInt( team, 8 );
			m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_ALL, data);
		}
	}
	else
	{
		team = team_;
	}
}

void BasePlayer::teamChangePetition_( int team_ )
{
	if ( m_node )
	{
		ZCom_BitStream *data = new ZCom_BitStream;
		addEvent(data, TEAM_CHANGE);
		data->addSignedInt( team_, 8 );
		m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_OWNER_2_AUTH, data);
	}
}

void BasePlayer::sendChatMsg( std::string const& message )
{
	game.displayChatMsg( m_name, message );
	if ( m_node )
	{
		ZCom_BitStream *data = new ZCom_BitStream;
		addEvent(data, CHAT_MSG);
		data->addString( message.c_str() );
		m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_OWNER_2_AUTH|ZCOM_REPRULE_AUTH_2_PROXY, data);
	}
}

void BasePlayer::selectWeapons( vector< WeaponType* > const& weaps )
{
	if ( !network.isClient() && m_worm )
	{
		m_worm->setWeapons( weaps );
	}
	if ( network.isClient() && m_node )
	{
		if(weaps.size() > game.options.maxWeapons)
		{
			cerr << "ERROR: Requested a too large weapon selection" << endl;
			return;
		}
			
		ZCom_BitStream *data = new ZCom_BitStream;
		addEvent(data, SELECT_WEAPONS );
		
		Encoding::encode(*data, weaps.size(), game.options.maxWeapons+1 );
		for( vector<WeaponType*>::const_iterator iter = weaps.begin(); iter != weaps.end(); ++iter )
		{
			Encoding::encode(*data, (*iter)->getIndex(), game.weaponList.size());
		}
		m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_OWNER_2_AUTH, data);
	}
}

void BasePlayer::assignWorm(BaseWorm* worm)
{
	m_worm = worm;
	worm->assignOwner( this );
	if ( NetWorm* netWorm = dynamic_cast<NetWorm*>(worm))
	{
		m_wormID = netWorm->getNodeID();
	}
}

shared_ptr<PlayerOptions> BasePlayer::getOptions()
{
	return m_options;
}

void BasePlayer::assignNetworkRole( bool authority )
{
	m_node = new ZCom_Node();
	if (!m_node)
	{
		allegro_message("ERROR: Unable to create player node.");
	}

	m_node->beginReplicationSetup(1);
		//m_node->addReplicationInt( (zS32*)&deaths, 32, false, ZCOM_REPFLAG_MOSTRECENT, ZCOM_REPRULE_AUTH_2_ALL , 0);
		m_node->setInterceptID( static_cast<ZCom_InterceptID>(WormID) );
		m_node->addReplicationInt( (zS32*)&m_wormID, 32, false, ZCOM_REPFLAG_MOSTRECENT | ZCOM_REPFLAG_INTERCEPT, ZCOM_REPRULE_AUTH_2_ALL , INVALID_NODE_ID);
	m_node->endReplicationSetup();

	m_interceptor = new BasePlayerInterceptor( this );
	m_node->setReplicationInterceptor(m_interceptor);

	m_isAuthority = authority;
	if( m_isAuthority)
	{
		m_node->setEventNotification(true, false); // Enables the eEvent_Init.
		if( !m_node->registerNodeDynamic(classID, network.getZControl() ) )
			allegro_message("ERROR: Unable to register player authority node.");
	}else
	{
		m_node->setEventNotification(false, true); // Same but for the remove event.
		if( !m_node->registerRequestedNode( classID, network.getZControl() ) )
		allegro_message("ERROR: Unable to register player requested node.");
	}

	m_node->applyForZoidLevel(1);
}

void BasePlayer::setOwnerId( ZCom_ConnID id )
{
	m_node->setOwner( id, true );
	m_id = id;
}



void BasePlayer::sendSyncMessage( ZCom_ConnID id )
{
	ZCom_BitStream *data = new ZCom_BitStream;
	addEvent(data, SYNC);
	data->addInt(stats->kills, 32);
	data->addInt(stats->deaths, 32);
	data->addString( m_name.c_str() );
	data->addInt(colour, 24);
	data->addSignedInt(static_cast<int>(team), 8);
	data->addInt(static_cast<int>(m_options->uniqueID), 32);
	m_node->sendEventDirect(eZCom_ReliableOrdered, data, id);
}

ZCom_NodeID BasePlayer::getNodeID()
{
	if (m_node)
		return m_node->getNetworkID();
	else
		return INVALID_NODE_ID;
}

ZCom_NodeID BasePlayer::getConnectionID()
{
	return m_id;
}

BasePlayerInterceptor::BasePlayerInterceptor( BasePlayer* parent )
{
	m_parent = parent;
}

bool BasePlayerInterceptor::inPreUpdateItem (ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator, zU32 _estimated_time_sent)
{
	switch ( (BasePlayer::ReplicationItems) _replicator->getSetup()->getInterceptID() )
	{
		case BasePlayer::WormID:
		{
			ZCom_NodeID recievedID = *static_cast<zU32*>(_replicator->peekData());
#ifdef USE_GRID
			for ( Grid::iterator iter = game.objects.beginAll(); iter; ++iter)
			{
				if ( NetWorm* worm = dynamic_cast<NetWorm*>(&*iter))
				{
					if ( worm->getNodeID() == recievedID )
					{
						m_parent->assignWorm(worm);
					}
				}
			}
#else
			ObjectsList::Iterator objIter;
			for ( objIter = game.objects.begin(); objIter; ++objIter)
			{
				if ( NetWorm* worm = dynamic_cast<NetWorm*>(*objIter) )
				{
					if ( worm->getNodeID() == recievedID )
					{
						m_parent->assignWorm(worm);
					}
				}
			}
#endif
			return true;
		} break;
		
		case BasePlayer::Other: break; // Do nothing?
	}
	return false;
}



void BasePlayer::baseActionStart ( BaseActions action )
{
	switch (action)
	{
		case LEFT:
		{
			if ( m_worm )
			{
				m_worm -> actionStart(Worm::MOVELEFT);
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStart(data, LEFT);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
			}
		}
		break;
		
		case RIGHT:
		{
			if ( m_worm )
			{
				m_worm -> actionStart(Worm::MOVERIGHT);
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStart(data, RIGHT);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
			}
		}
		break;
		
		case FIRE:
		{
			if ( m_worm )
			{
				m_worm -> actionStart(Worm::FIRE);
				if ( m_node )
				{
					ZCom_BitStream *data = new ZCom_BitStream;
					addActionStart(data, FIRE);
					data->addInt(int(m_worm->getAngle()), Angle::prec);
					m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
				}
			}
		}
		break;
		
		case JUMP:
		{
			if ( m_worm )
			{
				m_worm -> actionStart(Worm::JUMP);
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStart(data, JUMP);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
			}
		}
		break;
		
		case NINJAROPE:
		{
			if ( m_worm )
			{
				m_worm->actionStart(Worm::NINJAROPE);
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStart(data, NINJAROPE);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
			}
		}
		break;
		
		case DIG:
		{
			if ( m_worm )
			{
				m_worm->dig();
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStart(data, DIG);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
			}
		}
		break;
		
		case RESPAWN:
		{
			if ( m_worm && !m_worm->isActive() )
			{
				m_worm->respawn();
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStart(data, RESPAWN);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
				// I am sending the event to both the auth and the proxies, but I 
				//cant really think of any use for the proxies knowing this, maybe 
				//I should change it to only send to auth? :o
			}
		}
		break;
		
		case ACTION_COUNT: break; // Do nothing
	}
}



void BasePlayer::baseActionStop ( BaseActions action )
{
	switch (action)
	{
		case LEFT:
		{
			if ( m_worm )
			{
				m_worm -> actionStop(Worm::MOVELEFT);
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStop(data, LEFT);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
			}
		}
		break;
		
		case RIGHT:
		{
			if ( m_worm )
			{
				m_worm -> actionStop(Worm::MOVERIGHT);
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStop(data, RIGHT);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
			}
		}
		break;
		
		case FIRE:
		{
			if ( m_worm )
			{
				m_worm -> actionStop(Worm::FIRE);
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStop(data, FIRE);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
			}
		}
		break;
		
		case JUMP:
		{
			if ( m_worm )
			{
				m_worm -> actionStop(Worm::JUMP);
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStop(data, JUMP);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
			}
		}
		break;
		
		case NINJAROPE:
		{
			if ( m_worm )
			{
				m_worm->actionStop(Worm::NINJAROPE);
			}
			if ( m_node )
			{
				ZCom_BitStream *data = new ZCom_BitStream;
				addActionStop(data, NINJAROPE);
				m_node->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_PROXY | ZCOM_REPRULE_OWNER_2_AUTH, data);
			}
		}
		
		case DIG: break; // Doesn't stop
			
		case RESPAWN: break; // Doesn't stop
		
		case ACTION_COUNT: break; // Do nothing
	}
}


