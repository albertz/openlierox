#ifndef GAME_H
#define GAME_H

#include "level.h"
//#include "base_object.h"
//#include "base_action.h"
//#include "objects_list.h"
#include "object_grid.h"
#include "message_queue.h"

#ifndef DEDSERV
#include <allegro.h>
#endif
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
using boost::shared_ptr;
namespace fs = boost::filesystem;
#include <zoidcom.h>

class BasePlayer;
class BaseWorm;
class BaseAction;
class PlayerOptions;
class WeaponType;
class Particle;
class PartType;
class Explosion;
class ZCom_BitStream;
struct LuaEventDef;
#ifndef DEDSERV
class Sound1D;
class Font;
#endif


#define USE_GRID

class Player;

struct Options
{
	void registerInConsole();
	
	float ninja_rope_shootSpeed;
	float ninja_rope_pullForce;
	float ninja_rope_startDistance;
	float ninja_rope_maxLength;
	float worm_maxSpeed;
	float worm_acceleration;
	float worm_airAccelerationFactor;
	float worm_friction;
	float worm_airFriction;
	float worm_gravity;
	float worm_bounceQuotient;
	float worm_bounceLimit;
	float worm_jumpForce;
	int worm_disableWallHugging;
	int worm_weaponHeight;
	int worm_height;
	int worm_width;
	int worm_maxClimb;
	float worm_boxRadius;
	float worm_boxTop;
	float worm_boxBottom;
	int maxRespawnTime;
	int minRespawnTime;
	int host;
	int maxWeaponsVar;
	size_t maxWeapons;
	int splitScreenVar;
	bool splitScreen;
	std::string rConPassword;
	int teamPlay;
	bool showDeathMessages;
	bool logDeathMessages;
	
	int showMapDebug;
};

struct LevelEffectEvent
{
	LevelEffectEvent( int index_, int x_, int y_ ) : index(index_), x(x_), y(y_)
	{
	}
	int index;
	int x,y;
	
};

struct ScreenMessage
{
	enum Type
	{
		Death,
		Chat,
	};
	
	ScreenMessage(Type type_, std::string const& str_, int timeOut_ = 400)
	: type(type_), str(str_), timeOut(timeOut_)
	{
	}
	
	Type type;
	std::string str;
	int timeOut;
};

class Game
{
public:
#ifndef USE_GRID
	enum ColLayer
	{
		WORMS_COLLISION_LAYER = 0,
		NO_COLLISION_LAYER = 1,
		COLLISION_LAYER_COUNT = 10,
	};
	
	enum RenderLayer
	{
		WORMS_RENDER_LAYER = 4,
		RENDER_LAYER_COUNT = 10,
	};
	
	static const int CUSTOM_COL_LAYER_START = 2;
#endif

	static const size_t MAX_LOCAL_PLAYERS = 2;
	
	
		
	static ZCom_ClassID  classID;

	enum PLAYER_TYPE
	{
		OWNER = 0,
		PROXY,
		AI,
	};
	
	enum ResetReason
	{
		ServerQuit,
		ServerChangeMap,
		Kicked,
		LoadingLevel,
		IncompatibleProtocol,
		IncompatibleData,
	};
	
	enum Error
	{
		ErrorNone = 0,
		ErrorMapNotFound,
		ErrorMapLoading,
		ErrorModNotFound,
		ErrorModLoading
	};
		
	Game();
	~Game();
	
	void init(int argc, char** argv);
	void parseCommandLine(int argc, char** argv);
	
	void think();
	
	bool setMod(const std::string& mod);
	void loadWeapons();
	void reset(ResetReason reason);
	void unload();
	void error(Error err);
	void loadMod(bool doLoadWeapons = true);
	bool isLoaded();
	void refreshResources(fs::path const& levelPath);
	void refreshLevels();
	void refreshMods();
	bool reloadModWithoutMap();
	void createNetworkPlayers();
	bool changeLevel(const std::string& level, bool refresh = true);
	bool changeLevelCmd(const std::string& level);
	bool hasLevel(std::string const& level);
	bool hasMod(std::string const& mod);
	void runInitScripts();
	void addBot( int team = -1 );
	BasePlayer* findPlayerWithID( ZCom_NodeID ID );
	BasePlayer* addPlayer( PLAYER_TYPE type, int team = -1, BaseWorm* worm = 0 );
	BaseWorm* addWorm(bool isAuthority); // Creates a worm class depending on the network condition.
	//static ZCom_Node* getNode();
	static void sendLuaEvent(LuaEventDef* event, eZCom_SendMode mode, zU8 rules, ZCom_BitStream* data, ZCom_ConnID connID);
	
	void assignNetworkRole( bool authority );
	void removeNode();
	
	void applyLevelEffect( LevelEffect* effect, int x, int y );
	
	void sendRConMsg( std::string const & message );
	void displayChatMsg( std::string const& owner, std::string const& message);
	void displayKillMsg( BasePlayer* killed, BasePlayer* killer );
	void displayMessage( ScreenMessage const& msg );
	
	Level level;
	std::vector<WeaponType*> weaponList;
	Options options;
	std::vector<shared_ptr<PlayerOptions> > playerOptions;
	std::set<std::string> modList;
	
	std::vector<Player*> localPlayers;
	std::list<BasePlayer*> players;
#ifdef USE_GRID
	Grid objects;
#else
	ObjectsList objects;
#endif

	void insertExplosion( Explosion* explosion );
	
	std::map< std::string, BaseAction*(*)( const std::vector< std::string > & ) > actionList;
	//HashTable< std::string, BaseAction*(*)( const std::vector< std::string > & ) > actionList;
	
	PartType* NRPartType;
	PartType* deathObject;
	PartType* digObject;
		
	const std::string& getMod();
	fs::path const& getModPath();
	fs::path const& getDefaultPath();

#ifndef DEDSERV
	Sound1D* chatSound;
	Font *infoFont;
#endif
	std::list<ScreenMessage> messages;

	unsigned long stringToIndex(std::string const& str);
	
	std::string const& indexToString(unsigned long idx);
	
	std::string const& getModName();
	
	static void addCRCs(ZCom_BitStream* req);
	static bool checkCRCs(ZCom_BitStream& data);
	
	MessageQueue msg;
	
	mq_define_message(ChangeLevel, 0, (std::string level_))
		: level(level_)
		{
			
		}
		
		std::string level;
	mq_end_define_message()
	
	mq_define_message(ChangeLevelReal, 1, (std::string level_))
		: level(level_)
		{
			
		}
		
		std::string level;
	mq_end_define_message()
	
	
};

extern Game game;

#endif // _GAME_H_
