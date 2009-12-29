#include "gusgame.h"

#include "CWorm.h"
#include "worm.h"
#include "part_type.h"
#include "exp_type.h"
#include "weapon_type.h"
#include "CGameObject.h"
#include "CWormHuman.h"
#include "player_options.h"
#include "particle.h"
#include "explosion.h"
#include "exp_type.h"
#include "level_effect.h"
#include "level.h"
#include "gconsole.h"
#include "game_actions.h"
#include "game/WormInputHandler.h"
#include "proxy_player.h"
#include "gfx.h"
#include "sprite_set.h"
#include "util/macros.h"
#include "util/log.h"
#ifndef DEDICATED_ONLY
#include "sound/sfx.h"
#include "sound.h"
#include "font.h"
#include "menu.h"
#include "keyboard.h"
#include "player_input.h"
#include "CViewport.h"
#endif //DEDICATED_ONLY
#include "player_ai.h"
#include "net_worm.h"
#include "network.h"
#include "script.h"
#include "ninjarope.h"
#include "hash_table.h"

#include "loaders/vermes.h"
#include "loaders/LieroXLevelLoader.h"
#include "loaders/liero.h"
#include "loaders/losp.h"
#include "glua.h"
#include "lua/bindings.h"
#include "Debug.h"
#include "FindFile.h"
#include "CClient.h"
#include "CServer.h"
#include "game/Game.h"

#include "gusanos/allegro.h"
#include <string>
#include <algorithm>
#include <list>
#include <iostream>
#include <sstream> //TEMP

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;

//using namespace std; // Conflicting
using std::string;
using std::list;
using std::vector;
using std::cerr;
using std::cout;
using std::endl;

namespace
{
	enum NetEvents
	{
		eHole = 0,
		// Add here
		LuaEvent,
		//RegisterLuaEvents, //oobsol33t
		NetEventsCount,
	};
	
	void addEvent(Net_BitStream* data, NetEvents event)
	{
		Encoding::encode( *data, static_cast<int>(event), NetEventsCount );
	}
	
	std::list<LevelEffectEvent> appliedLevelEffects;

	std::string nextMod;
	std::string    m_modPath;
	std::string m_modName;
	std::string    m_defaultPath;
	bool loaded;
	Net_Node *m_node;
	bool m_isAuthority;
	HashTable<std::string, unsigned long> stringToIndexMap;
	std::vector<std::string> indexToStringMap;
	
	uint32_t getWeaponCRC()
	{
		uint32_t v = 0;
		foreach(i, gusGame.weaponList)
		{
			v ^= (*i)->crc;
			++v;
		}
		
		return v;
	}
}

Net_ClassID GusGame::classID = Net_Invalid_ID;

GusGame gusGame;

string mapCmd(const list<string> &args)
{
	return "Gusanos MAP not available";
	/*if (!args.empty())
	{
		string tmp = *args.begin();
		std::transform(tmp.begin(), tmp.end(), tmp.begin(), (int(*)(int)) tolower);
		gusGame.changeLevelCmd( tmp );
		return "";
	}*/
	return "MAP <MAPNAME> : LOAD A MAP";
}

struct MapIterGetText
{
	template<class IteratorT>
	std::string const& operator()(IteratorT i) const
	{
		return i->first;
	}
};

string mapCompleter(Console* con, int idx, std::string const& beginning)
{
	if(idx != 0)
		return beginning;
		
	return shellComplete(
		levelLocator.getMap(),
		beginning.begin(),
		beginning.end(),
		MapIterGetText(),
		ConsoleAddLines(*con)
	);
}

string gameCmd(const list<string> &args)
{
	if (!args.empty())
	{
		string tmp = *args.begin();
		std::transform(tmp.begin(), tmp.end(), tmp.begin(), (int(*)(int)) tolower);
		if(!gusGame.setMod( tmp ))
			return "MOD " + tmp + " NOT FOUND";
		return "THE GAME WILL CHANGE THE NEXT TIME YOU CHANGE MAP";
	}
	return "GAME <MODNAME> : SET THE MOD TO LOAD THE NEXT MAP CHANGE";
}

struct GameIterGetText
{
	template<class IteratorT>
	std::string const& operator()(IteratorT i) const
	{
		return *i;
	}
};

string gameCompleter(Console* con, int idx, std::string const& beginning)
{
	if(idx != 0)
		return beginning;
		
	return shellComplete(
		gusGame.modList,
		beginning.begin(),
		beginning.end(),
		GameIterGetText(),
		ConsoleAddLines(*con)
	);
}

string addbotCmd(const list<string> &args)
{
	if ( !network.isClient() )
	{
		int team = -1;
		list<string>::const_iterator i = args.begin();
		if(i != args.end())
		{
			team = cast<int>(*i);
			++i;
		}
		gusGame.addBot(team);
		return "";
	}else
	{
		return "You cant add bots as client";
	}
}

string connectCmd(const list<string> &args)
{
	return "Error: Gusanos connect command not supported";
}

string rConCmd(const list<string> &args)
{
	if ( !args.empty() && network.isClient() )
	{
		
		list<string>::const_iterator iter = args.begin();
		string tmp = *iter++;
		for (; iter != args.end(); ++iter )
		{
			tmp += " \"" + *iter + '"';
		}
		gusGame.sendRConMsg( tmp );
		return "";
	}
	return "";
}

string rConCompleter(Console* con, int idx, std::string const& beginning)
{
	if(idx != 0)
		return beginning;
		
	return con->completeCommand(beginning);
}

CWormInputHandler* findPlayerByName(std::string const& name)
{
	//CWormInputHandler* player2Kick = 0;
	//for ( std::list<CWormInputHandler*>::iterator iter = game.players.begin(); iter != game.players.end(); iter++)
	foreach(iter, game.players)
	{
		if ( (*iter)->m_name == name )
		{
			return *iter;
		}
	}
	
	return 0;
}

string banCmd(list<string> const& args)
{
	return "Gusanos ban command not available";
}

string kickCmd(const list<string> &args)
{
	return "Gusanos kick command not available";
}

struct BasePlayerIterGetText
{
	template<class IteratorT>
	std::string const& operator()(IteratorT i) const
	{
		return (*i)->m_name;
	}
};

string kickCompleter(Console* con, int idx, std::string const& beginning)
{
	if(idx != 0)
		return beginning;
		
	return shellComplete(
		game.players,
		beginning.begin(),
		beginning.end(),
		BasePlayerIterGetText(),
		ConsoleAddLines(*con)
	);
}

void Options::registerInConsole()
{
	console.registerVariables()
		("SV_NINJAROPE_SHOOT_SPEED", &ninja_rope_shootSpeed, 2)
		("SV_NINJAROPE_PULL_FORCE", &ninja_rope_pullForce, 0.031)
		//("SV_NINJAROPE_PULL_FORCE", &ninja_rope_pullForce, (4.0f / 3.0f) * (70.0f * 70.0f / (100.0f * 100.0f * 16.0f)))
		
		//("SV_NINJAROPE_START_DISTANCE", &ninja_rope_startDistance, 20)
		("SV_NINJAROPE_START_DISTANCE", &ninja_rope_startDistance, 4000.0f / 16.f - 1.f)
		("SV_NINJAROPE_MAX_LENGTH", &ninja_rope_maxLength, 2000.f)

		("SV_WORM_MAX_SPEED", &worm_maxSpeed, 0.45)
		("SV_WORM_ACCELERATION", &worm_acceleration, 0.03)
		("SV_WORM_AIR_ACCELERATION_FACTOR", &worm_airAccelerationFactor, 1.f)
		//("SV_WORM_FRICTION", &worm_friction, 0.02)
		("SV_WORM_FRICTION", &worm_friction, pow(0.89, 0.7))
		("SV_WORM_AIR_FRICTION", &worm_airFriction, 1.f)
		("SV_WORM_GRAVITY", &worm_gravity, 0.009)
		("SV_WORM_DISABLE_WALL_HUGGING", &worm_disableWallHugging, 0)
		//("SV_WORM_BOUNCE_QUOTIENT", &worm_bounceQuotient, 0.3)
		("SV_WORM_BOUNCE_QUOTIENT", &worm_bounceQuotient, 0.333)
		("SV_WORM_BOUNCE_LIMIT", &worm_bounceLimit, 2)
		//("SV_WORM_BOUNCE_LIMIT", &worm_bounceLimit, 0.56875f)
		
		("SV_WORM_JUMP_FORCE", &worm_jumpForce, 0.6)
		("SV_WORM_WEAPON_HEIGHT", &worm_weaponHeight, 5)
		("SV_WORM_HEIGHT", &worm_height, 9)
		("SV_WORM_WIDTH", &worm_width, 3)
		("SV_WORM_MAX_CLIMB", &worm_maxClimb, 4)
 		("SV_WORM_BOX_RADIUS", &worm_boxRadius, 2)
		("SV_WORM_BOX_TOP", &worm_boxTop, 3)
		("SV_WORM_BOX_BOTTOM", &worm_boxBottom, 4)

		("SV_MAX_RESPAWN_TIME", &maxRespawnTime, -1 )
		("SV_MIN_RESPAWN_TIME", &minRespawnTime, 100 )
		("SV_TEAM_PLAY", &teamPlay, 0)

		("HOST", &host, 0)
		
		("SV_MAX_WEAPONS", &maxWeaponsVar, 5)
		("CL_SPLITSCREEN", &splitScreenVar, 0)
			
		("RCON_PASSWORD", &rConPassword, "" )
		("CL_SHOW_MAP_DEBUG", &showMapDebug, 0 )
		("CL_SHOW_DEATH_MESSAGES", &showDeathMessages, true )
		("CL_LOG_DEATH_MESSAGES", &logDeathMessages, false )
	;
	maxWeapons = 5;
	splitScreen = false;
	
	console.registerCommands()
		("MAP", mapCmd, mapCompleter)
		("GAME", gameCmd, gameCompleter)
		("ADDBOT", addbotCmd)
		("CONNECT", connectCmd)
		("RCON", rConCmd, rConCompleter)
		("KICK", kickCmd, kickCompleter)
		("BAN", banCmd, kickCompleter)
	;
}

GusGame::GusGame()
{
	NRPartType = NULL;
	deathObject = NULL;
	loaded = false;
	m_node = NULL;
}

GusGame::~GusGame()
{

}

void GusGame::parseCommandLine(int argc, char** argv)
{
	for(int i = 0; i < argc; ++i)
	{
		const char* arg = argv[i];
		if(arg[0] == '-')
		{
			switch(arg[1])
			{
				case 'c':
					if(++i >= argc)
						return;
						
					console.parseLine(argv[i]);
				break;
			}
		}
	}
}

bool GusGame::init()
{
	if(!allegro_init()) return false;
	
	levelLocator.registerLoader(&VermesLevelLoader::instance);
	levelLocator.registerLoader(&LieroXLevelLoader::instance);
	levelLocator.registerLoader(&LieroLevelLoader::instance);
	
#ifndef DEDICATED_ONLY
	fontLocator.registerLoader(&VermesFontLoader::instance);
	fontLocator.registerLoader(&LOSPFontLoader::instance);
	fontLocator.registerLoader(&LieroFontLoader::instance);
	
	xmlLocator.registerLoader(&XMLLoader::instance);
	gssLocator.registerLoader(&GSSLoader::instance);
#endif
	
	scriptLocator.registerLoader(&LuaLoader::instance);
	
	LuaBindings::init();

	m_defaultPath = "Gusanos";
	m_modPath = "Gusanos";
	m_modName = "Gusanos";
	if(!setMod("Gusanos")) {
		errors << "default Gusanos mod not found" << endl;
		return false;
	}
	refreshResources("Gusanos");
	
	console.init();
#ifndef DEDICATED_ONLY
	//OmfgGUI::menu.init();
	
	sfx.registerInConsole();
#endif
	gfx.registerInConsole();
	options.registerInConsole();
	network.registerInConsole();
	
	for ( size_t i = 0; i< MAX_LOCAL_PLAYERS; ++i)
	{
		shared_ptr<PlayerOptions> options(new PlayerOptions);
		options->registerInConsole(i);
		playerOptions.push_back(options);
	}
	
#ifndef DEDICATED_ONLY
	console.executeConfig("config.cfg");
#else
	console.executeConfig("config-ded.cfg");
#endif
	
	//parseCommandLine(argc, argv);
	
	gfx.init();
#ifndef DEDICATED_ONLY
	
	keyHandler.init();
	//mouseHandler.init();
#endif
	
	network.init();
	registerGameActions();
#ifndef DEDICATED_ONLY
	registerPlayerInput();
#endif
	
	return true;
}

void GusGame::sendLuaEvent(LuaEventDef* event, eNet_SendMode mode, Net_U8 rules, Net_BitStream* userdata, Net_ConnID connID)
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

void GusGame::think()
{
	mq_process_messages(msg)
		mq_case(ChangeLevel)
			
			if(!network.isDisconnected())
			{
				if(!network.isDisconnecting())
				{
					if( network.isHost() && options.host )
						network.disconnect( Network::ServerMapChange );
					else
						network.disconnect();
				}
				
				mq_delay(); // Wait until network is disconnected
			}

			// Network is disconnected

			refreshLevels();
			if(!levelLocator.exists(data.level))
				break;
			if(!changeLevel( data.level, false ))
				break;
			
			if ( options.host && !network.isClient() )
			{
				network.olxHost();
			}
			
			runInitScripts();
				
			// All this is temporal, dont be scared ;D
			if ( loaded && level().gusIsLoaded() ) 
			{
				if ( network.isHost() )
				{
					createNetworkPlayers();
				}
				else if ( !network.isClient() )
				{
					if(true)
					{
						CWorm* worm = addWorm(true);
						addPlayer ( OWNER, -1, worm );
						//player->assignWorm(worm);
					}
					if(options.splitScreen)
					{
						CWorm* worm = addWorm(true);
						addPlayer ( OWNER, -1, worm );
						//player->assignWorm(worm);
					}
				}
			}
		mq_end_case()
	mq_end_process_messages()

#ifndef DEDICATED_ONLY
	if(!messages.empty())
	{
		int size = messages.size();
		int step = 1 + (size / 3);
		if((messages.front().timeOut -= step) < 0)
			messages.erase(messages.begin());
	}
#endif

	if(isLevelLoaded())
		level().gusThink();

	if ( !m_node )
		return;
	
	while ( m_node->checkEventWaiting() )
	{
		eNet_Event type;
		eNet_NodeRole    remote_role;
		Net_ConnID       conn_id;
		
		Net_BitStream* data = m_node->getNextEvent(&type, &remote_role, &conn_id);
		switch(type)
		{
			case eNet_EventUser:
			if ( data )
			{
				NetEvents event = (NetEvents)Encoding::decode(*data, NetEventsCount);
				switch ( event )
				{
					case eHole:
					{
						int index = Encoding::decode(*data, levelEffectList.size());
						BaseVec<int> v = level().intVectorEncoding.decode<BaseVec<int> >(*data);
						level().applyEffect( levelEffectList[index], v.x, v.y );
					}
					break;
					
/*
					case RegisterLuaEvents:
					{
						for(int t = Network::LuaEventGroup::GusGame;
							t < Network::LuaEventGroup::Max; ++t)
						{
							int c = data->getInt(8);
							for(int i = 0; i < c; ++i)
							{
								char const* name = data->getStringStatic();
								DLOG("Got lua event: " << name);
								network.indexLuaEvent((Network::LuaEventGroup::type)t, name);
							}
						}
					}
					break;
*/
					case LuaEvent:
					{
						int index = data->getInt(8);
						DLOG("Got lua event index " << index);
						if(LuaEventDef* event = network.indexToLuaEvent(Network::LuaEventGroup::GusGame, index))
						{
							event->call(data);
						}
					}
					break;
					
					case NetEventsCount: break;
				}
			}
			break;
			
			case eNet_EventInit:
			{
				// Call this first since level effects will hog the message queue
				EACH_CALLBACK(i, gameNetworkInit)
				{
					(lua.call(*i), conn_id)();
				}
				
				list<LevelEffectEvent>::iterator iter = appliedLevelEffects.begin();
				for( ; iter != appliedLevelEffects.end() ; ++iter )
				{
					Net_BitStream *data = new Net_BitStream;
					addEvent(data, eHole);
					Encoding::encode(*data, iter->index, levelEffectList.size());
					level().intVectorEncoding.encode(*data, BaseVec<int>(iter->x, iter->y));
					
					m_node->sendEventDirect(eNet_ReliableOrdered, data, conn_id );
				}
				
				
			}
			break;
			
			default: break; // Annoying warnings >:O
		}
	}

}

void GusGame::applyLevelEffect( LevelEffect* effect, int x, int y )
{
	if ( !network.isClient() )
	{
		if ( level().applyEffect( effect, x, y ) && m_node && network.isHost() )
		{
			Net_BitStream *data = new Net_BitStream;

			addEvent(data, eHole);
			Encoding::encode(*data, effect->getIndex(), levelEffectList.size());
			level().intVectorEncoding.encode(*data, BaseVec<int>(x, y));

			m_node->sendEvent(eNet_ReliableOrdered, Net_REPRULE_AUTH_2_ALL, data);
			
			appliedLevelEffects.push_back( LevelEffectEvent(effect->getIndex(), x, y ) );
		}
	}
}

void GusGame::loadWeapons()
{
	std::string path = m_modPath + "/weapons";
	
	if ( gusExists( path ) )
	{		
		for( Iterator<std::string>::Ref iter = gusFileListIter(path); iter->isValid(); iter->next())
		{
			if( gusExistsFile(path + "/" + iter->get()) )
			{
				if ( fs::extension(iter->get()) == ".wpn")
				{
					WeaponType* weapon = new WeaponType;
					weapon->load(path + "/" + iter->get());
					weaponList.push_back(weapon);
				}
			}
		}
		
		WeaponOrder comp;
		std::sort(weaponList.begin(), weaponList.end(), comp); 
		
		for ( size_t i = 0; i < weaponList.size(); ++i )
		{
			weaponList[i]->setIndex(i);
		}
	}
};

bool GusGame::loadMod(bool doLoadWeapons)
{
	options.maxWeapons = options.maxWeaponsVar;
	options.splitScreen = ( options.splitScreenVar != 0 );
	console.loadResources();
	gfx.loadResources();
	
	NRPartType = partTypeList.load("ninjarope.obj");
	deathObject = partTypeList.load("death.obj");
	digObject = partTypeList.load("wormdig.obj");
#ifndef DEDICATED_ONLY
	chatSound = sound1DList.load("chat.ogg");
	if (!chatSound)
		sound1DList.load("chat.wav");
	infoFont = fontLocator.load("minifont");
	if(infoFont == NULL) {
		errors << "Gusanos GusGame::loadMod: cannot load minifont" << endl;
		return false;
	}
	
#endif
	if(doLoadWeapons)
	{
		loadWeapons();
		if (weaponList.size() > 0 )
		{
			loaded = true;
		}
		else
		{
			loaded = false;
			console.addLogMsg("ERROR: NO WEAPONS FOUND IN MOD FOLDER");
		}
	}
	console.executeConfig("mod.cfg");
	
	if(!loaded)
		error(ErrorModLoading);
	
	return true;
}

void GusGame::runInitScripts()
{
	Script* modScript = scriptLocator.load(m_modName);
	if(!modScript) {
		notes << "init script " << m_modName << ".lua not found, trying common.lua" << endl;
		modScript = scriptLocator.load("common");
		if(!modScript) notes << "common.lua also not found" << endl;
	}
	if(modScript)
	{
		LuaReference ref = modScript->createFunctionRef("init");
		(lua.call(ref))();
	}

	if(isLevelLoaded()) {
		Script* levelScript = scriptLocator.load("map_" + level().getName());
		if(levelScript)
		{
			LuaReference ref = levelScript->createFunctionRef("init");
			(lua.call(ref))();
		}
	}
	levelEffectList.indexate();
	partTypeList.indexate();
}

void GusGame::reset(ResetReason reason)
{
	game.reset();
	
	appliedLevelEffects.clear();
	
	// OLX manages this
	//level().gusUnload();
	
	if(reason != LoadingLevel)
	{
		EACH_CALLBACK(i, gameEnded)
		{
			(lua.call(*i), static_cast<int>(reason))();
		}
	}
}

void GusGame::unload()
{
	//cerr << "Unloading..." << endl;
	loaded = false;
#ifndef DEDICATED_ONLY
	OmfgGUI::menu.destroy();
	sfx.clear();
#endif
	
	console.clearTemporaries();
	
	reset(LoadingLevel);
/*
	// Delete all players
	for ( list<CWormInputHandler*>::iterator iter = players.begin(); iter != players.end(); ++iter)
	{
		(*iter)->deleteThis();
	}
	players.clear();
	localPlayers.clear();
	
	// Delete all objects
#ifdef USE_GRID
	objects.clear();
#else
	for ( ObjectsList::Iterator iter = objects.begin(); (bool)iter; ++iter)
	{
		(*iter)->deleteThis();
	}
	objects.clear();
#endif

	appliedLevelEffects.clear();
	
	level.unload();
*/
	for ( vector<WeaponType*>::iterator iter = weaponList.begin(); iter != weaponList.end(); ++iter)
	{
		luaDelete(*iter);
	}
	weaponList.clear();
	
	partTypeList.clear();
	expTypeList.clear();
#ifndef DEDICATED_ONLY
	soundList.clear();
	sound1DList.clear();
#endif
	spriteList.clear();
	levelEffectList.clear();

#ifndef DEDICATED_ONLY
	fontLocator.clear();
	xmlLocator.clear();
	gssLocator.clear();
#endif
	scriptLocator.clear();

	network.clear();
	lua.reset();
	luaCallbacks = LuaCallbacks(); // Reset callbacks
	LuaBindings::init();
#ifndef DEDICATED_ONLY
	OmfgGUI::menu.clear();
#endif
}

bool GusGame::isLoaded()
{
	return loaded;
}

void GusGame::refreshResources(std::string const& levelPath)
{
#ifndef DEDICATED_ONLY
	fontLocator.addPath(levelPath + "/fonts");
	fontLocator.addPath(m_defaultPath + "/fonts");
	fontLocator.addPath(std::string(nextMod) + "/fonts");
	fontLocator.refresh();

	xmlLocator.addPath(m_defaultPath + "/gui");
	xmlLocator.addPath(std::string(nextMod) + "/gui");
	xmlLocator.refresh();
	
	gssLocator.addPath(m_defaultPath + "/gui");
	gssLocator.addPath(std::string(nextMod) + "/gui");
	gssLocator.refresh();
#endif
	
	scriptLocator.addPath(levelPath + "/scripts");
	scriptLocator.addPath(m_defaultPath + "/scripts");
	scriptLocator.addPath(std::string(nextMod) + "/scripts");
	scriptLocator.refresh();
	
	// These are added in reverse order compared to
	// the resource locator paths! Fix maybe?
	partTypeList.addPath(levelPath + "/objects");
	partTypeList.addPath(std::string(nextMod) + "/objects");
	partTypeList.addPath(m_defaultPath + "/objects");
	
	expTypeList.addPath(levelPath + "/objects");
	expTypeList.addPath(std::string(nextMod) + "/objects");
	expTypeList.addPath(m_defaultPath + "/objects");
	
#ifndef DEDICATED_ONLY
	soundList.addPath(levelPath + "/sounds");
	soundList.addPath(std::string(nextMod) + "/sounds");
	soundList.addPath(m_defaultPath + "/sounds");
	
	sound1DList.addPath(levelPath + "/sounds");
	sound1DList.addPath(std::string(nextMod) + "/sounds");
	sound1DList.addPath(m_defaultPath + "/sounds");
#endif
	
	spriteList.addPath(levelPath + "/sprites");
	spriteList.addPath(std::string(nextMod) + "/sprites");
	spriteList.addPath(m_defaultPath + "/sprites");
	
	levelEffectList.addPath(levelPath + "/mapeffects");
	levelEffectList.addPath(std::string(nextMod) + "/mapeffects");
	levelEffectList.addPath(m_defaultPath + "/mapeffects");
	
	refreshMods();
}

void GusGame::refreshLevels()
{
	levelLocator.clear();
	levelLocator.addPath("levels");
	//levelLocator.addPath(std::string(nextMod) + "/maps");
	levelLocator.refresh();
}

void GusGame::refreshMods()
{
	modList.clear();
	for( Iterator<std::string>::Ref i = gusFileListIter("."); i->isValid(); i->next())
	{
		if( gusIsDirectory(i->get()) )
		{
			if ( gusExists(i->get() + "/weapons"))
			{
				modList.insert(i->get());
			}
		}
	}
}

void GusGame::createNetworkPlayers()
{
	CWorm* worm = addWorm(true);
	CWormInputHandler* player = addPlayer ( OWNER, -1, worm );
	player->assignNetworkRole(true);
	//player->assignWorm(worm);

	if(options.splitScreen)
	{
		// TODO: Factorize all this out, its being duplicated on client.cpp also :O
		CWorm* worm = addWorm(true); 
		CWormInputHandler* player = addPlayer ( OWNER, -1, worm );
		player->assignNetworkRole(true);
		//player->assignWorm(worm);
	}
}


bool GusGame::changeLevelCmd(const std::string& levelName )
{
	warnings << "GusGame::changeLevelCmd not supported anymore" << endl;
	return false;
}

bool GusGame::reloadModWithoutMap()
{
	unload();
	//level.gusUnload();
	refreshResources("Gusanos");
	loadMod(false);
	runInitScripts();
	
	return true;
}

void GusGame::error(Error err)
{
	EACH_CALLBACK(i, gameError)
	{
		(lua.call(*i), static_cast<int>(err))();
	}
}

bool GusGame::hasLevel(std::string const& level)
{
	return levelLocator.exists(level);
}

bool GusGame::hasMod(std::string const& mod)
{
	return modList.find(mod) != modList.end();
}

bool GusGame::changeLevel(const std::string& levelName, bool refresh)
{
	if(refresh)
		refreshLevels();
	
	ResourceLocator<CMap>::NamedResourceMap::const_iterator i = levelLocator.getMap().find(levelName);
	if(i == levelLocator.getMap().end())
		return false;
	
	return changeLevel(i->second.loader, i->second.path);
}

bool GusGame::changeLevel(ResourceLocator<CMap>::BaseLoader* loader, const std::string& levelPath, CMap* m)
{
	notes << "GusGame::changeLevel: " << levelPath << " with mod " << nextMod << endl;
	
	unload();
	
	m_modName = nextMod;
	m_modPath = nextMod;
	
	//level.setName(levelName);
	refreshResources(levelPath);
	//cerr << "Loading level" << endl;
	
	if(!m) m = &level();
	
	if(!loader->load(m, levelPath))
	{
		warnings << "GusGame::changeLevel: error while loading map" << endl;
		reloadModWithoutMap();
		error(ErrorMapLoading);
		return false;
	}
	
#ifdef USE_GRID
	game.objects.resize(0, 0, m->GetWidth(), m->GetHeight());
#endif
	
	//cerr << "Loading mod" << endl;
	loadMod();

	// earlier, this was in GusGame::think() for the ChangeLevel event
	runInitScripts();
	
	return true;
}

void GusGame::assignNetworkRole( bool authority )
{
	m_node = new Net_Node;
	
	m_node->beginReplicationSetup(2);
		//m_node->addReplicationInt( (Net_S32*)&deaths, 32, false, Net_REPFLAG_MOSTRECENT, Net_REPRULE_AUTH_2_ALL , 0);
	m_node->addReplicationInt( (Net_S32*)&options.worm_gravity, 32, false, Net_REPFLAG_MOSTRECENT | Net_REPFLAG_RARELYCHANGED, Net_REPRULE_AUTH_2_ALL );
	m_node->addReplicationInt( (Net_S32*)&options.teamPlay, 1, false, Net_REPFLAG_MOSTRECENT | Net_REPFLAG_RARELYCHANGED, Net_REPRULE_AUTH_2_ALL );
	
	m_node->endReplicationSetup();

	m_isAuthority = authority;
	if( authority)
	{
		m_node->setEventNotification(true, false); // Enables the eEvent_Init.
		if( !m_node->registerNodeUnique(classID, eNet_RoleAuthority, network.getNetControl() ) )
			ELOG("Unable to register gusGame authority node.");
	}else
	{
		if( !m_node->registerNodeUnique( classID, eNet_RoleProxy, network.getNetControl() ) )
			ELOG("Unable to register gusGame requested node.");
	}

	m_node->applyForNetLevel(1);
}

void GusGame::sendRConMsg( string const& message )
{
	Net_BitStream *req = new Net_BitStream;
	req->addInt(Network::RConMsg, 8);
	req->addString( options.rConPassword.c_str() );
	req->addString( message.c_str() );
	network.getNetControl()->Net_sendData( network.getServerID(), req, eNet_ReliableOrdered );
}

void GusGame::removeNode()
{
	delete m_node;
	m_node = NULL;
}

bool GusGame::setMod( const string& modname )
{
	if( gusExists(modname) )
	{
		nextMod = modname;
		m_modName = nextMod;
		m_modPath = nextMod;		
	}
	else
	{
		nextMod = m_modName;
		return false;
	}
	
	refreshLevels();
	
	return true;
}

void GusGame::displayChatMsg( std::string const& owner, std::string const& message)
{
	console.addLogMsg('<' + owner + "> " + message);
	displayMessage(ScreenMessage(ScreenMessage::Chat, '{' + owner + "}: " + message, 800));
	
#ifndef DEDICATED_ONLY
	if ( chatSound ) chatSound->play1D();
#endif
}

void GusGame::displayKillMsg( CWormInputHandler* killed, CWormInputHandler* killer )
{
	std::string str = "{" + killed->m_name + "} ";
	
	if(killed != killer)
	{
		str += "got killed by";
		if(killer)
			str += " {" + killer->m_name + '}';
		else
			str += " {\01305anonymous}";
	}
	else
	{
		str += "commited suicide";
	}
	
	if ( options.showDeathMessages ) displayMessage(ScreenMessage(ScreenMessage::Death, str, 400));
	if ( options.logDeathMessages ) console.addLogMsg(str);
}

void GusGame::displayMessage( ScreenMessage const& msg )
{
	messages.push_back(msg);
}

const string& GusGame::getMod()
{
	return m_modName;
}

std::string const& GusGame::getModPath()
{
	return m_modPath;
}

std::string const& GusGame::getDefaultPath()
{
	return m_defaultPath;
}

CWormInputHandler* GusGame::findPlayerWithID( Net_NodeID ID )
{
	list<CWormInputHandler*>::iterator playerIter;
	for ( playerIter = game.players.begin(); playerIter != game.players.end(); ++playerIter)
	{
		if ( (*playerIter)->getNodeID() == ID )
		{
			return (*playerIter);
		}
	}
	return NULL;
}

void GusGame::insertExplosion( Explosion* explosion )
{
#ifdef USE_GRID
	game.objects.insert( explosion, Grid::NoColLayer, explosion->getType()->renderLayer);
#else
	game.objects.insert( NO_COLLISION_LAYER, explosion->getType()->renderLayer, explosion );
#endif
}

CWormInputHandler* GusGame::addPlayer( PLAYER_TYPE type, int team, CWorm* worm )
{
	CWormInputHandler* p = 0;
	switch(type)
	{
		
		case OWNER:
		{
			if ( game.localPlayers.size() >= MAX_LOCAL_PLAYERS ) allegro_message("OMFG Too much local players");
			// TODO: gusGame::addplayer
			CWormHumanInputHandler* player = NULL; // new CWormHumanInputHandler( playerOptions[localPlayers.size()], worm );
#ifndef DEDICATED_ONLY
			CViewport* viewport = new CViewport;
			if ( options.splitScreen )
			{
				viewport->setDestination(gfx.buffer,game.localPlayers.size()*160,0,160,240);
			}else
			{
				viewport->setDestination(gfx.buffer,0,0,320,240);
			}
			player->assignViewport(viewport);
#endif
			game.onNewPlayer( player );
			game.onNewHumanPlayer( player );
			game.onNewHumanPlayer_Lua( player );
			p = player;
		}
		break;
				
		case PROXY:
		{
			ProxyPlayer* player = new ProxyPlayer(worm);
			game.onNewPlayer( player );
			p = player;
		}
		break;
		
		case AI:
		{
			PlayerAI* player = new PlayerAI(team, worm);
			game.onNewPlayer( player );
			p = player;
		}
	}
	
	if(p)
		game.onNewPlayer_Lua(p);
	
	return p;
}

CWorm* GusGame::addWorm(bool isAuthority)
{
	CWorm* returnWorm = NULL;
	if ( network.isHost() || network.isClient() )
	{
		NetWorm* netWorm = new NetWorm(isAuthority);
		returnWorm = netWorm;
	}else
	{
		Worm* worm = new Worm();
		returnWorm = worm;
	}
	if ( !returnWorm ) allegro_message("moo");

	game.onNewWorm(returnWorm);
	
	return returnWorm;
}

void GusGame::addBot(int team)
{
	if ( loaded && level().gusIsLoaded() )
	{
		CWorm* worm = addWorm(true); 
		CWormInputHandler* player = addPlayer(AI, team, worm);
		if ( network.isHost() ) player->assignNetworkRole(true);
		//player->assignWorm(worm);
	}
}

unsigned long GusGame::stringToIndex(std::string const& str)
{
	HashTable<std::string, unsigned long>::iterator i = stringToIndexMap.find(str);
	if(i != stringToIndexMap.end())
		return i->second;
	i = stringToIndexMap.insert(str);
	int idx = indexToStringMap.size();
	i->second = idx;
	indexToStringMap.push_back(str);
	return idx;
}

std::string const& GusGame::indexToString(unsigned long idx)
{
	return indexToStringMap.at(idx);
}

std::string const& GusGame::getModName()
{
	return m_modName;
}

void GusGame::addCRCs(Net_BitStream* req)
{
	req->addInt(partTypeList.crc(), 32);
	req->addInt(expTypeList.crc(), 32);
	req->addInt(getWeaponCRC(), 32);
	req->addInt(levelEffectList.crc(), 32);
}

bool GusGame::checkCRCs(Net_BitStream& data)
{
	uint32_t particleCRC = data.getInt(32);
	uint32_t expCRC = data.getInt(32);
	uint32_t weaponCRC = data.getInt(32);
	uint32_t levelEffectCRC = data.getInt(32);
	
	if(particleCRC != partTypeList.crc()
	|| expCRC != expTypeList.crc()
	|| weaponCRC != getWeaponCRC()
	|| levelEffectCRC != levelEffectList.crc())
	{
		return false;
	}
	
	return true;
}
/*
Net_Node* GusGame::getNode()
{
	return m_node;
}*/

static CMap* getMap() {
	if(tLX) {
		if(tLX->iGameType == GME_JOIN) return cClient->getMap();
		return cServer->getMap();
	}
	return NULL;
}

CMap& GusGame::level() {
	return *getMap();
}

bool GusGame::isLevelLoaded() {
	return getMap() && getMap()->isLoaded();
}

bool GusGame::isEngineNeeded() {
	return (game.gameScript() && game.gameScript()->gusEngineUsed()) || (getMap() && getMap()->gusIsLoaded());
}

