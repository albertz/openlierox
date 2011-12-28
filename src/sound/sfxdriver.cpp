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

void SfxDriver::registerInConsole()
{
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
