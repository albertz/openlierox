// PlayerOptions class.
// This holds all options and changes associated with a player:
// name, colour, team, ID etc.

#include "player_options.h"
#include "gconsole.h"
#include <boost/bind.hpp>
#include "util/text.h"
#include <cmath>
#include <list>
#include "gusanos/allegro.h"

using namespace std;

PlayerOptions::PlayerOptions(std::string const& name_)
		: name(name_),
		uniqueID(0),
		team((unsigned int)-1),
		m_nameChanged(false),
		m_colorChanged(false)
{
	aimAcceleration = AngleDiff(0.1);
	//aimFriction = 0.05;
	aimFriction = (float)pow(0.89, 0.7);
	aimMaxSpeed = AngleDiff(1);
	viewportFollowFactor = 1;
	ropeAdjustSpeed = 0.5;
	colour = universalColor(100, 100, 250);

}

void PlayerOptions::registerInConsole(int index)
{
	std::string sindex = cast<string>(index);

	console.registerVariables()
	("P" + sindex + "_AIM_ACCEL", &aimAcceleration, AngleDiff(0.17))
	("P" + sindex + "_AIM_FRICTION", &aimFriction, pow(0.89, 0.7))
	("P" + sindex + "_AIM_SPEED", &aimMaxSpeed, AngleDiff(1.7))
	("P" + sindex + "_VIEWPORT_FOLLOW", &viewportFollowFactor, 0.1)
	("P" + sindex + "_ROPE_ADJUST_SPEED", &ropeAdjustSpeed, 0.5)
	("P" + sindex + "_NAME", &name, "GusPlayer",
	 boost::bind( &PlayerOptions::nameChange, this ) )
	;

	console.registerCommands()
	//("P" + sindex + "_COLOR", boost::bind(&PlayerOptions::setColour, this, _1))
	("P" + sindex + "_COLOUR", boost::bind(&PlayerOptions::setColour, this, _1))
	("P" + sindex + "_TEAM", boost::bind(&PlayerOptions::setTeam, this, _1))
	;
}

string PlayerOptions::setColour(list<string> const& args)
{
	if(args.size() >= 3) {
		list<string>::const_iterator i = args.begin();
		int r = cast<int>(*i++);
		int g = cast<int>(*i++);
		int b = cast<int>(*i++);
		colour = universalColor(r, g, b);
		m_colorChanged = true;
		return "";
	}
	return "PX_COLOUR <R> <G> <B> : SETS THE COLOUR OF THE WORM BELONGING TO THIS PLAYER";
}

string PlayerOptions::setTeam(list<string> const& args)
{
	if(args.size() >= 1) {
		list<string>::const_iterator i = args.begin();
		team = cast<int>(*i++);
		m_teamChanged = true;
		return "";
	}
	return "PX_TEAM <T> : SETS THE TEAM NUMBER OF THIS PLAYER";
}

///////////////////
// Changes the name of the player to the one passed as argument.
void PlayerOptions::changeName(std::string const& name_)
{
	if(name_ == name)
		return;
	name = name_;
	nameChange();
}

///////////////////
// Marks a change of name.
void PlayerOptions::nameChange()
{
	m_nameChanged = true;
}

///////////////////
// Checks whether the player changed his name.
bool PlayerOptions::nameChanged()
{
	bool res = m_nameChanged;
	m_nameChanged = false;
	return res;
}

///////////////////
// Checks whether the player changed colour.
bool PlayerOptions::colorChanged()
{
	bool res = m_colorChanged;
	m_colorChanged = false;
	return res;
}

///////////////////
// Checks whether the player changed teams.
bool PlayerOptions::teamChanged()
{
	bool res = m_teamChanged;
	m_teamChanged = false;
	return res;
}

///////////////////
// Sets change flags to default.
void PlayerOptions::clearChangeFlags()
{
	m_nameChanged = false;
	m_colorChanged = false;
	m_teamChanged = false;
}