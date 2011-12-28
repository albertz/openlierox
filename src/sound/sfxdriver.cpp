#ifndef DEDICATED_ONLY

#include "gusanos/gconsole.h"
#include "util/macros.h"
#include "sfxdriver.h"
#include "MathLib.h"
#include "Options.h"

#include <vector>
#include <list>

using namespace std;

static const float SFX_LISTENER_DISTANCE = 20.0f;

SfxDriver::SfxDriver()
{
	m_volume = 1.0f;
	m_listenerDistance = SFX_LISTENER_DISTANCE;
}

SfxDriver::~SfxDriver()
{
}

void SfxDriver::setListeners(std::vector<Listener*> &_listeners)
{
	listeners=_listeners;
}


string wrapper__guscon_sfx_volume(const list<string> &args)
{
	if(args.size() >= 1) {
		warnings << "Gus con sfx_volume: ignored overwrite with " << *args.begin() << endl;
		return ""; // just ignore
	}
	
	// Gusanos volume range is 0-255
	// LX volume range is 0-100
	return itoa(Round(float(tLXOptions->iSoundVolume) * 0.01f * 255.0f));
}

string wrapper__guscon_sfx_listener_distance(const list<string> &args)
{
	if(args.size() >= 1) {
		warnings << "Gus con sfx_listener_distance: ignored overwrite with " << *args.begin() << endl;
		return ""; // just ignore
	}
	
	return ftoa(SFX_LISTENER_DISTANCE);
}


void SfxDriver::registerInConsole()
{
	console.registerCommands()
			("SFX_VOLUME", wrapper__guscon_sfx_volume)
			("SFX_LISTENER_DISTANCE", wrapper__guscon_sfx_listener_distance)
			;

	// NOTE: When/if adding a callback to sfx variables, make it do nothing if
	// sfx.operator bool() returns false.
}

void SfxDriver::setVolume(float val) {
	m_volume = val;
	volumeChange();
}

float SfxDriver::volume() const {
	return m_volume;
}

#endif
