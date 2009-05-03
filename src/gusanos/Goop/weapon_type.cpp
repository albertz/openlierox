#include "weapon_type.h"

#include "events.h"
#include "sprite_set.h"
#include "util/text.h"
#include "parser.h"
#include "resource_base.h"
#include "game_actions.h"
#include "omfg_script.h"
#include "util/macros.h"
#include "timer_event.h"
#include "luaapi/context.h"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
namespace fs = boost::filesystem;

using namespace std;

LuaReference WeaponType::metaTable;

WeaponType::WeaponType() : ResourceBase()
{
	ammo = 1;
	reloadTime = 0;
#ifndef DEDSERV
	firecone = NULL;
	skin = 0;
#endif

	syncHax = false;
	syncReload = true;
	
	laserSightColour = makecol(255,0,0);
	laserSightRange = -1;
	laserSightIntensity = 0;
	laserSightAlpha = 255;
	laserSightBlender = NONE;

	primaryShoot = NULL;
	primaryPressed = NULL;
	primaryReleased = NULL;
	outOfAmmo = NULL;
	reloadEnd = NULL;
}

WeaponType::~WeaponType()
{
	delete primaryShoot;
	delete primaryPressed;
	delete primaryReleased;
	delete outOfAmmo;
	delete reloadEnd;
	
	foreach( t, timer )
	{
		delete (*t);
	}
	
	foreach( t, activeTimer )
	{
		delete (*t);
	}
	
	foreach( t, shootTimer )
	{
		delete (*t);
	}
}


namespace EventID
{
enum type
{
	PrimaryShoot,
	PrimaryPress,
	PrimaryRelease,
	OutOfAmmo,
	ReloadEnd,
	Timer,
	ActiveTimer
};
}

bool WeaponType::load(fs::path const& filename)
{
	fs::ifstream fileStream(filename, std::ios::binary | std::ios::in);

	if (!fileStream )
		return false;
		
	fileName = filename;
	
	OmfgScript::Parser parser(fileStream, gameActions, filename.native_file_string());
	
	namespace af = OmfgScript::ActionParamFlags;
		
	parser.addEvent("primary_shoot", EventID::PrimaryShoot, af::Weapon | af::Object);
	parser.addEvent("primary_press", EventID::PrimaryPress, af::Weapon | af::Object);
	parser.addEvent("primary_release", EventID::PrimaryRelease, af::Weapon | af::Object);
	parser.addEvent("out_of_ammo", EventID::OutOfAmmo, af::Weapon | af::Object);
	parser.addEvent("reload_end", EventID::ReloadEnd, af::Weapon | af::Object);
	
	parser.addEvent("timer", EventID::Timer, af::Weapon | af::Object)
		("delay")
		("delay_var")
		("max_trigger")
		("start_delay")
	;
	
	parser.addEvent("active_timer", EventID::ActiveTimer, af::Weapon | af::Object)
		("delay")
		("delay_var")
		("max_trigger")
		("start_delay")
	;
	
	if(!parser.run())
	{
		if(parser.incomplete())
			parser.error("Trailing garbage");
		return false;
	}
	
	crc = parser.getCRC();

	ammo = parser.getInt("ammo", 1);
	name = parser.getString("name");
	reloadTime = parser.getInt("reload_time", 0);
	syncHax = parser.getBool("sync_shot", false);
	syncReload = parser.getBool("sync_reload", true);
	laserSightIntensity = parser.getDouble("laser_sight_intensity", 0.0);
	laserSightRange = parser.getInt("laser_sight_range", -1);
	laserSightAlpha = parser.getInt("laser_sight_alpha", 255);
	{
		std::string const& str = parser.getString("laser_sight_blender", "none");
		if(str == "add") laserSightBlender = ADD;
		else if(str == "alpha") laserSightBlender = ALPHA;
		else if(str == "none") laserSightBlender = NONE;
	}

	laserSightColour = parser.getProperty("laser_sight_colour", "laser_sight_color")->toColor(255, 0, 0);
#ifndef DEDSERV
	{
		OmfgScript::TokenBase* v = parser.getProperty("firecone");
		if(!v->isDefault())
			firecone = spriteList.load(v->toString());
		
		v = parser.getProperty("skin");
		if(!v->isDefault())
			skin = spriteList.load(v->toString());
	}
#endif
	
	OmfgScript::Parser::EventIter i(parser);
	for(; i; ++i)
	{
		std::vector<OmfgScript::TokenBase*> const& p = i.params();
		switch(i.type())
		{
			case EventID::PrimaryShoot:
				primaryShoot = new Event(i.actions());
			break;
			
			case EventID::PrimaryPress:
				primaryPressed = new Event(i.actions());
			break;
			
			case EventID::PrimaryRelease:
				primaryReleased = new Event(i.actions());
			break;
			
			case EventID::OutOfAmmo:
				outOfAmmo = new Event(i.actions());
			break;
			
			case EventID::ReloadEnd:
				reloadEnd = new Event(i.actions());
			break;
			
			case EventID::Timer:
				timer.push_back(new TimerEvent(i.actions(), p[0]->toInt(100), p[1]->toInt(0), p[2]->toInt(0), p[3]->toInt(0) ) );
			break;
			
			case EventID::ActiveTimer:
				activeTimer.push_back(new TimerEvent(i.actions(), p[0]->toInt(100), p[1]->toInt(0), p[2]->toInt(0), p[3]->toInt(0) ) );
			break;
		}
	}
	
	return true;
}

void WeaponType::makeReference()
{
	lua.pushFullReference(*this, metaTable);
}

void WeaponType::finalize()
{
	delete primaryShoot; primaryShoot = 0;
	delete primaryPressed; primaryPressed = 0;
	delete primaryReleased; primaryReleased = 0;
	delete outOfAmmo; outOfAmmo = 0;
	delete reloadEnd; reloadEnd = 0;
	
	foreach( t, timer )
	{
		delete (*t);
	}
	timer.clear();
	
	foreach( t, activeTimer )
	{
		delete (*t);
	}
	activeTimer.clear();
	
	foreach( t, shootTimer )
	{
		delete (*t);
	}
	shootTimer.clear();
}

