#ifndef DEDICATED_ONLY

#include "gusanos/gconsole.h"
#include "util/macros.h"
#include "sfxdriver.h"
#include "MathLib.h"

#include <vector>

using namespace std;



SfxDriver::SfxDriver():MAX_VOLUME(255)
{
	m_volume=MAX_VOLUME;
	m_listenerDistance=0;
}

SfxDriver::~SfxDriver()
{
}

void SfxDriver::setListeners(std::vector<Listener*> &_listeners)
{
//aaaa
	listeners=_listeners;
}

void SfxDriver::registerInConsole()
{
//aaaaaaa
	console.registerVariables()
			("SFX_VOLUME", &m_volume, MAX_VOLUME , update_volume)
			("SFX_LISTENER_DISTANCE", &m_listenerDistance, 20)
		;
		
		// NOTE: When/if adding a callback to sfx variables, make it do nothing if
		// sfx.operator bool() returns false.

}

void SfxDriver::setVolume(float val) {
	m_volume = Round(val * MAX_VOLUME);
	volumeChange();
}

float SfxDriver::volume() const {
	return float(m_volume) / MAX_VOLUME;
}

#endif
