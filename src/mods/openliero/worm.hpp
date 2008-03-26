#ifndef LIERO_WORM_HPP
#define LIERO_WORM_HPP

#include "math.hpp"
#include <SDL/SDL.h>
#include <string>
#include <cstring>

struct Worm;

struct Ninjarope
{
	Ninjarope()
	: out(false)
	, attached(false)
	, anchor(0)
	{
	}
	
	bool out;            //Is the ninjarope out?
	bool attached;
	Worm* anchor;			// If non-zero, the worm the ninjarope is attached to
	fixed x, y, velX, velY; //Ninjarope props
	// Not needed as far as I can tell: fixed forceX, forceY;
	int length, curLen;
	
	void process(Worm& owner);
};

struct Controls
{
	bool up, down, left, right;
	bool fire, change, jump;
};

struct WormWeapon
{
	WormWeapon()
	: id(0)
	, ammo(0)
	, delayLeft(0)
	, loadingLeft(0)
	, available(true)
	{
	}
	
	int id;
	int ammo;
	int delayLeft;
	int loadingLeft;
	bool available;			
};

struct WormSettings
{
	enum
	{
		Up, Down, Left, Right,
		Fire, Change, Jump
	};
	
	WormSettings()
	: health(100)
	, controller(0)
	, randomName(true)
	, colour(0)
	{
		rgb[0] = 26;
		rgb[1] = 26;
		rgb[2] = 62;
		
		std::memset(weapons, 0, sizeof(weapons));
	}
	
	int health;
	int controller; // CPU / Human
	Uint32 controls[7];
	int weapons[5]; // TODO: Adjustable
	std::string name;
	int rgb[3];
	bool randomName;
	
	int colour;
	int selWeapX;
};

/*
typedef struct _settings
{
 long m_iHealth[2];
 char m_iController[2];
	char m_iWeapTable[40];
 long m_iMaxBonuses;
 long m_iBlood;
 long m_iTimeToLose;
 long m_iFlagsToWin;
 char m_iGameMode;
 bool m_bShadow;
 bool m_bLoadChange;
 bool m_bNamesOnBonuses;
 bool m_bRegenerateLevel;
 BYTE m_iControls[2][7];
 BYTE m_iWeapons[2][5];
	long m_iLives;
 long m_iLoadingTime;
	bool m_bRandomLevel;
 char m_bWormName[2][21];
 char m_bLevelFile[13];
 BYTE m_iWormRGB[2][3];
 bool m_bRandomName[2];
 bool m_bMap;
 bool m_bScreenSync;
} SETTINGS, *PSETTINGS;
*/

struct Viewport;

struct Worm
{
	enum
	{
		RFDown,
		RFLeft,
		RFUp,
		RFRight
	};
		
	// ----- Changed when importing to OLX -----
	Worm(WormSettings* settings=NULL, int index=-1, int wormSoundID=-1)
	// ----- Changed when importing to OLX -----
	: x(0), y(0), velX(0), velY(0)
	, hotspotX(0), hotspotY(0)
	, aimingAngle(0), aimingSpeed(0)
	, ableToJump(false), ableToDig(false)   //The previous state of some keys
	, keyChangePressed(false)
	, movable(false)
	, animate(false)                 //Should the worm be animated?
	, visible(false)                 //Is the worm visible?
	, ready(false)                   //Is the worm ready to play?
	, flag(false)                    //Has the worm a flag?
	, makeSightGreen(false)          //Changes the sight color
	, health(0)                  //Health left
	, lives(0)                   //lives left
	, kills(0)                   //Kills made
	, timer(0)                   //Timer for GOT
	, killedTimer(0)             //Time until worm respawns
	, currentFrame(0)
	, flags(0)                   //How many flags does this worm have?
	, currentWeapon(0)
	, fireConeActive(false)
	, lastKilledBy(0)
	, fireCone(0)
	, leaveShellTimer(0)
	, settings(settings)
	, viewport(0)
	, index(index)
	, wormSoundID(wormSoundID)
	, direction(0)
	
	
	{
	}
	
	int keyLeft()
	{
		return settings->controls[WormSettings::Left];
	}
	
	int keyRight()
	{
		return settings->controls[WormSettings::Right];
	}
	
	int keyUp()
	{
		return settings->controls[WormSettings::Up];
	}
	
	int keyDown()
	{
		return settings->controls[WormSettings::Down];
	}
	
	int keyFire()
	{
		return settings->controls[WormSettings::Fire];
	}
	
	int keyChange()
	{
		return settings->controls[WormSettings::Change];
	}
	
	int keyJump()
	{
		return settings->controls[WormSettings::Jump];
	}
	
	void beginRespawn();
	void doRespawning();
	void process();
	void processWeapons();
	void processPhysics();
	void processMovement();
	void processTasks();
	void processAiming();
	void processWeaponChange();
	void processSteerables();
	void fire();
	void processSight();
	void calculateReactionForce(int newX, int newY, int dir);
	
	void processLieroAI(); // Move?
	
	fixed x, y;                    //Worm position    
	fixed velX, velY;              //Worm velocity
	
	int hotspotX, hotspotY;      //Hotspots for laser, laser sight, etc.
	fixed aimingAngle, aimingSpeed;
 
	Controls controls;
	bool ableToJump, ableToDig;   //The previous state of some keys
	bool keyChangePressed;
	bool movable;
 
	bool animate;                 //Should the worm be animated?
	bool visible;                 //Is the worm visible?
	bool ready;                   //Is the worm ready to play?
	bool flag;                    //Has the worm a flag?
	bool makeSightGreen;          //Changes the sight color
	int health;                  //Health left
	int lives;                   //lives left
	int kills;                   //Kills made
	
	int timer;                   //Timer for GOT
	int killedTimer;             //Time until worm respawns
	int currentFrame;
 
	int flags;                   //How many flags does this worm have?

	Ninjarope ninjarope;
	
	int currentWeapon;           //The selected weapon
	bool fireConeActive;          //Is the firecone showing
	Worm* lastKilledBy;          // What worm that last killed this worm
	int fireCone;                //How much is left of the firecone
	int leaveShellTimer;         //Time until next shell drop
	
	WormSettings* settings;
	Viewport* viewport;
	int index; // 0 or 1
	int wormSoundID;
	
	int reacts[4];
	WormWeapon weapons[5];
	int direction;
};

bool checkForWormHit(int x, int y, int dist, Worm* ownWorm);
bool checkForSpecWormHit(int x, int y, int dist, Worm& w);

#endif // LIERO_WORM_HPP
