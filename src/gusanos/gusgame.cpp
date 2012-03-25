#include "gusgame.h"

#include "game/CWorm.h"
#include "part_type.h"
#include "exp_type.h"
#include "weapon_type.h"
#include "game/CGameObject.h"
#include "CWormHuman.h"
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
#include "player_input.h"
#include "CViewport.h"
#endif //DEDICATED_ONLY
#include "net_worm.h"
#include "network.h"
#include "script.h"
#include "hash_table.h"

#include "loaders/vermes.h"
#include "loaders/liero.h"
#include "loaders/losp.h"
#include "glua.h"
#include "LuaCallbacks.h"
#include "lua/bindings.h"
#include "Debug.h"
#include "FindFile.h"
#include "CClient.h"
#include "CServer.h"
#include "game/Game.h"
#include "CGameScript.h"

#include "gusanos/allegro.h"
#include <string>
#include <algorithm>
#include <list>
#include <iostream>
#include <sstream> //TEMP

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
	
	void addEvent(BitStream* data, NetEvents event)
	{
		Encoding::encode( *data, static_cast<int>(event), NetEventsCount );
	}
	
	std::list<LevelEffectEvent> appliedLevelEffects;

	std::string nextMod;
	std::string m_modPath;
	static const std::string C_DefaultModPath = "Gusanos";
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

Net_ClassID GusGame::classID = INVALID_CLASS_ID;

GusGame gusGame;

static string wrapper__sv_team_play(const list<string> &args)
{
	if(args.size() >= 1) {
		warnings << "Gus SV_TEAM_PLAY overwrite ignored" << endl;
		return ""; // just ignore
	}
	
	return game.isTeamPlay() ? "1" : "0";
}

static string dummy_rcon_varcmd(const list<string> &args)
{
	warnings << "Gus RCON_PASSWORD obsolete/unused" << endl;
	if(args.size() >= 1)
		return "";
	return "";
}

static string dummy_splitscreen_varcmd(const list<string> &args)
{
	warnings << "Gus CL_SPLITSCREEN ignored" << endl;
	if(args.size() >= 1)
		return "";
	return "0";
}

void Options::registerInConsole()
{
	console.registerVariables()
		(ninja_rope_shootSpeed.gusVar("SV_NINJAROPE_SHOOT_SPEED", 2))
		(ninja_rope_pullForce.gusVar("SV_NINJAROPE_PULL_FORCE", 0.031))
		(ninja_rope_restLength.gusVar("SV_NINJAROPE_REST_LENGTH", 450.f / 16.f - 1.0f))
	// Note: ninja_rope_startDistance now maps to FT_RopeMaxLength. This is
	// because it mostly determines how long the rope can get after shooting when in air.
	// The old Gus ninja_rope_maxLength was removed here because it meant again
	// somewhat something different (which might just be because the implementation
	// might just have been buggy) and was not really used.
		(ninja_rope_startDistance.gusVar("SV_NINJAROPE_START_DISTANCE", 4000.0f / 16.f - 1.f))
		(worm_maxSpeed.gusVar("SV_WORM_MAX_SPEED", 0.45f))
		(worm_acceleration.gusVar("SV_WORM_ACCELERATION", 0.03f))
		(worm_airAccelerationFactor.gusVar("SV_WORM_AIR_ACCELERATION_FACTOR", 1.f))
		(worm_friction.gusVar("SV_WORM_FRICTION", pow(0.89f, 0.7f)))
		(worm_airFriction.gusVar("SV_WORM_AIR_FRICTION", 1.f))
		(worm_gravity.gusVar("SV_WORM_GRAVITY", 0.009f))
		(worm_disableWallHugging.gusVar("SV_WORM_DISABLE_WALL_HUGGING", 0))
		(worm_bounceQuotient.gusVar("SV_WORM_BOUNCE_QUOTIENT", 0.333f))
		(worm_bounceLimit.gusVar("SV_WORM_BOUNCE_LIMIT", 2))
		(worm_jumpForce.gusVar("SV_WORM_JUMP_FORCE", 0.6f))
		(worm_weaponHeight.gusVar("SV_WORM_WEAPON_HEIGHT", 5))
		(worm_height.gusVar("SV_WORM_HEIGHT", 9))
		(worm_width.gusVar("SV_WORM_WIDTH", 3))
		(worm_maxClimb.gusVar("SV_WORM_MAX_CLIMB", 4))
 		(worm_boxRadius.gusVar("SV_WORM_BOX_RADIUS", 2))
		(worm_boxTop.gusVar("SV_WORM_BOX_TOP", 3))
		(worm_boxBottom.gusVar("SV_WORM_BOX_BOTTOM", 4))

		(minRespawnTime.gusVar("SV_MIN_RESPAWN_TIME", 100))
		(maxRespawnTime.gusVar("SV_MAX_RESPAWN_TIME", -1))
		
		("SV_MAX_WEAPONS", &maxWeaponsVar, 5)
			
		("CL_SHOW_MAP_DEBUG", &showMapDebug, 0 )
		("CL_SHOW_DEATH_MESSAGES", &showDeathMessages, true )
		("CL_LOG_DEATH_MESSAGES", &logDeathMessages, false )
	;
	maxWeapons = 5;
	
	console.registerCommands()
	("SV_TEAM_PLAY", wrapper__sv_team_play)
	("RCON_PASSWORD", dummy_rcon_varcmd )
	("CL_SPLITSCREEN", dummy_splitscreen_varcmd)
	;
}

GusGame::GusGame()
{
	NRPartType = NULL;
	deathObject = NULL;
	digObject = NULL;
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
	
#ifndef DEDICATED_ONLY
	fontLocator.registerLoader(&VermesFontLoader::instance);
	fontLocator.registerLoader(&LOSPFontLoader::instance);
	fontLocator.registerLoader(&LieroFontLoader::instance);
	
	xmlLocator.registerLoader(&XMLLoader::instance);
	gssLocator.registerLoader(&GSSLoader::instance);
#endif
	
	scriptLocator.registerLoader(&LuaLoader::instance);
	
	lua.init();
	LuaBindings::init();

	m_modPath = "Gusanos";
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
		
#ifndef DEDICATED_ONLY
	console.executeConfig("config.cfg");
#else
	console.executeConfig("config-ded.cfg");
#endif
	
	//parseCommandLine(argc, argv);
	
	gfx.init();
	network.init();
	registerGameActions();
#ifndef DEDICATED_ONLY
	registerPlayerInput();
#endif
	
	return true;
}

void GusGame::sendLuaEvent(LuaEventDef* event, eNet_SendMode mode, Net_U8 rules, BitStream* userdata, Net_ConnID connID)
{
	if(!m_node) {
		errors << "GusGame::sendLuaEvent: we dont have network node" << endl;
		return;
	}
	
	BitStream* data = new BitStream;
	addEvent(data, LuaEvent);
	data->addInt(event->idx, 8);
	if(userdata)
	{
		data->addBitStream(*userdata);
	}
	if(!connID)
		m_node->sendEvent(mode, rules, data);
	else
		m_node->sendEventDirect(mode, data, connID);
}

void GusGame::think()
{
#ifndef DEDICATED_ONLY
	if(!messages.empty())
	{
		int size = messages.size();
		int step = 1 + (size / 3);
		if((messages.front().timeOut -= step) < 0)
			messages.erase(messages.begin());
	}
#endif

	if(game.isMapReady())
		game.gameMap()->gusThink();

	if ( !m_node )
		return;
	
	while ( m_node->checkEventWaiting() )
	{
		eNet_Event type;
		eNet_NodeRole    remote_role;
		Net_ConnID       conn_id;
		
		BitStream* data = m_node->getNextEvent(&type, &remote_role, &conn_id);
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
						VectorD2<int> v = game.gameMap()->intVectorEncoding.decode<VectorD2<int> >(*data);
						game.gameMap()->applyEffect( levelEffectList[index], v.x, v.y );
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
						else
							notes << "GusGame: Lua event index " << index << " unknown" << endl;
					}
					break;
					
					case NetEventsCount: break;
				}
			}
			break;
			
			case eNet_EventInit:
			{
				// Call this first since level effects will hog the message queue
				LUACALLBACK(gameNetworkInit).call()(conn_id)();
				
				list<LevelEffectEvent>::iterator iter = appliedLevelEffects.begin();
				for( ; iter != appliedLevelEffects.end() ; ++iter )
				{
					BitStream *data = new BitStream;
					addEvent(data, eHole);
					Encoding::encode(*data, iter->index, levelEffectList.size());
					game.gameMap()->intVectorEncoding.encode(*data, VectorD2<int>(iter->x, iter->y));
					
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
		if ( game.gameMap()->applyEffect( effect, x, y ) && m_node && network.isHost() )
		{
			BitStream *data = new BitStream;

			addEvent(data, eHole);
			Encoding::encode(*data, effect->getIndex(), levelEffectList.size());
			game.gameMap()->intVectorEncoding.encode(*data, VectorD2<int>(x, y));

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
				if ( GetFileExtensionWithDot(iter->get()) == ".wpn")
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

bool GusGame::_loadMod(bool doLoadWeapons)
{
	options.maxWeapons = options.maxWeaponsVar;
	console.loadResources();
	gfx.loadResources();
	
	NRPartType = partTypeList.load("ninjarope.obj");
	deathObject = partTypeList.load("death.obj");
	digObject = partTypeList.load("wormdig.obj");
#ifndef DEDICATED_ONLY
	if(sfx) {
		chatSound = sound1DList.load("chat.ogg");
		if (!chatSound)
			chatSound = sound1DList.load("chat.wav");
	}
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
	
	return loaded;
}

void GusGame::runInitScripts()
{
	{
		Script* modScript = scriptLocator.load(m_modPath);
		if(!modScript) {
			notes << "init script " << m_modPath << ".lua not found, trying common.lua" << endl;
			modScript = scriptLocator.load("common");
			if(!modScript) notes << "common.lua also not found" << endl;
		}
		if(modScript)
		{
			notes << "running " << modScript->table << ".init" << endl;
			LuaReference ref = modScript->createFunctionRef("init");
			(lua.call(ref))();
		}
	}
	
	if(game.isMapReady()) {
		const std::string mapscript = "map_" + game.gameMap()->getName();
		Script* levelScript = scriptLocator.load(mapscript);
		if(!levelScript) {
			notes << "init script " << mapscript << ".lua not found, trying map_common.lua" << endl;
			levelScript = scriptLocator.load("map_common.lua");
			if(!levelScript) notes << "map_common.lua also not found" << endl;
		}
		if(levelScript)
		{
			notes << "running " << levelScript->table << ".init" << endl;
			LuaReference ref = levelScript->createFunctionRef("init");
			(lua.call(ref))();
		}
	}
	
	levelEffectList.indexate();
	partTypeList.indexate();
}

void GusGame::unload()
{
	// TODO: we must also remove the added console bindings (from Lua or custom `alias`)
	
	//cerr << "Unloading..." << endl;
	loaded = false;
#ifndef DEDICATED_ONLY
	OmfgGUI::menu.destroy();
	sfx.clear();
#endif
	
	console.clearTemporaries();
	
	appliedLevelEffects.clear();
	
	
	//level.unload();
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
	luaCallbacks.cleanup(); // remove invalidated callbacks
	LuaBindings::init();
#ifndef DEDICATED_ONLY
	OmfgGUI::menu.clear();
#endif

	NRPartType = NULL;
	deathObject = NULL;
	digObject = NULL;
	chatSound = NULL;
	infoFont = NULL;
}

bool GusGame::isLoaded()
{
	return loaded;
}

void GusGame::refreshResources(std::string const& levelPath)
{
#ifndef DEDICATED_ONLY
	fontLocator.addPath(levelPath + "/fonts");
	fontLocator.addPath(C_DefaultModPath + "/fonts");
	fontLocator.addPath(std::string(nextMod) + "/fonts");
	fontLocator.refresh();

	xmlLocator.addPath(C_DefaultModPath + "/gui");
	xmlLocator.addPath(std::string(nextMod) + "/gui");
	xmlLocator.refresh();
	
	gssLocator.addPath(C_DefaultModPath + "/gui");
	gssLocator.addPath(std::string(nextMod) + "/gui");
	gssLocator.refresh();
#endif
	
	scriptLocator.addPath(levelPath + "/scripts");
	scriptLocator.addPath(C_DefaultModPath + "/scripts");
	scriptLocator.addPath(std::string(nextMod) + "/scripts");
	scriptLocator.refresh();
	
	// These are added in reverse order compared to
	// the resource locator paths! Fix maybe?
	partTypeList.addPath(levelPath + "/objects");
	partTypeList.addPath(std::string(nextMod) + "/objects");
	partTypeList.addPath(C_DefaultModPath + "/objects");
	
	expTypeList.addPath(levelPath + "/objects");
	expTypeList.addPath(std::string(nextMod) + "/objects");
	expTypeList.addPath(C_DefaultModPath + "/objects");
	
#ifndef DEDICATED_ONLY
	soundList.addPath(levelPath + "/sounds");
	soundList.addPath(std::string(nextMod) + "/sounds");
	soundList.addPath(C_DefaultModPath + "/sounds");
	
	sound1DList.addPath(levelPath + "/sounds");
	sound1DList.addPath(std::string(nextMod) + "/sounds");
	sound1DList.addPath(C_DefaultModPath + "/sounds");
#endif
	
	spriteList.addPath(levelPath + "/sprites");
	spriteList.addPath(std::string(nextMod) + "/sprites");
	spriteList.addPath(C_DefaultModPath + "/sprites");
	
	levelEffectList.addPath(levelPath + "/mapeffects");
	levelEffectList.addPath(std::string(nextMod) + "/mapeffects");
	levelEffectList.addPath(C_DefaultModPath + "/mapeffects");
}

void GusGame::_prepareLoad(const std::string& path) {	
	unload();

	foreach( i, console.getItems() )
		if(Variable* v = dynamic_cast<Variable*>(i->second))
			// reset variable to Gusanos default
			v->reset();	
	
	m_modPath = nextMod;
	
	//level.setName(levelName);
	refreshResources(path);
	
	nextMod = "Gusanos"; // we must explicitly set the mod each time we load a new level	
}

void GusGame::_finishLoad() {
	if(game.isMapReady())
		game.objects.resize(0, 0, game.gameMap()->GetWidth(), game.gameMap()->GetHeight());
	
	//cerr << "Loading mod" << endl;
	_loadMod();
}

// NOTE: From OLX; this is called when we have a LX-level.
// In case we have a Gus mod, we have called setMod(<GUS-mod>).
// Otherwise, we have called setMod(C_DefaultModPath /* == "Gusanos" */).
// In case we have a Gus level, we don't need to call this because we
// loaded the Gus-mod already in changeLevel, which is called from the level
// loading code.
// WARNING: This is all hacky and might be temporarely, so this might
// need some update.
bool GusGame::loadModWithoutMap() {
	notes << "GusGame::loadModWithoutMap: " << nextMod << endl;
	_prepareLoad( /* dummy path */ C_DefaultModPath );
	_finishLoad();
	return true;
}

bool GusGame::changeLevel(ResourceLocator<CMap>::BaseLoader* loader, const std::string& levelPath, CMap* m)
{
	notes << "GusGame::changeLevel: " << levelPath << " with mod " << nextMod << endl;
	
	_prepareLoad(levelPath);
	
	if(!m) m = game.gameMap();
	
	if(!loader->load(m, levelPath))
	{
		warnings << "GusGame::changeLevel: error while loading map" << endl;
		unload(); // not necessarily needed but useful to just free some memory/resources
		return false;
	}
	
	_finishLoad();
	return true;
}

void GusGame::assignNetworkRole( bool authority )
{
	m_node = new Net_Node;
	
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
		m_modPath = nextMod;		
	}
	else
	{
		nextMod = m_modPath;
		return false;
	}
	
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
	std::string str = "{" + killed->worm()->getName() + "} ";
	
	if(killed != killer)
	{
		str += "got killed by";
		if(killer)
			str += " {" + killer->worm()->getName() + '}';
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

std::string const& GusGame::getModPath()
{
	return m_modPath;
}

std::string const& GusGame::getDefaultPath()
{
	return C_DefaultModPath;
}

CWormInputHandler* GusGame::findPlayerWithID( Net_NodeID ID )
{
	vector<CWormInputHandler*>::iterator playerIter;
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
	game.objects.insert( explosion, Grid::NoColLayer, explosion->getType()->renderLayer);
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

void GusGame::addCRCs(BitStream* req)
{
	req->addInt(partTypeList.crc(), 32);
	req->addInt(expTypeList.crc(), 32);
	req->addInt(getWeaponCRC(), 32);
	req->addInt(levelEffectList.crc(), 32);
}

bool GusGame::checkCRCs(BitStream& data)
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

bool GusGame::isEngineNeeded() {
	return (game.gameScript() && game.gameScript()->gusEngineUsed()) || (game.gameMap() && game.gameMap()->gusIsLoaded());
}

