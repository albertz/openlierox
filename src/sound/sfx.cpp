#ifndef DEDICATED_ONLY

#include <boost/assign/list_inserter.hpp>
using namespace boost::assign;

#include <vector>
#include <list>
#include <AL/al.h>
#include <boost/utility.hpp>

#include "sfx.h"
#include "sfxdriver.h"
#include "sfxdriver_openal.h"
#include "gusanos/gconsole.h"
#include "CGameObject.h"
#include "util/macros.h"
#include "sound_sample.h"

#include "Debug.h"


using namespace std;

Sfx sfx;

namespace
{
	bool m_initialized = false;
	
	std::vector<Listener*> listeners;
	
	typedef std::list< SmartPointer<SoundSample> > ChanSounds;
	ChanSounds simpleChanSounds;
}

void update_volume( int oldValue )
{
	notes<<"update_volume"<<endl;
	if (sfx)
		sfx.volumeChange();
}


Sfx::Sfx() : driver(NULL) {}
Sfx::~Sfx() {}

bool Sfx::init()
{
	//notes<<"Sfx::init()"<<endl;
	driver = new SfxDriverOpenAL();
	m_initialized = driver->init();
	return m_initialized;
}

void Sfx::shutDown()
{
	simpleChanSounds.clear();
	if(driver) {
		driver->shutDown();
		delete driver;
		driver = NULL;
	}
	m_initialized = false;
}

void Sfx::registerInConsole()
{
	notes<<"Sfx::registerInConsole()"<<endl;
	if (driver && m_initialized) {
		driver->registerInConsole();
	}
}

void Sfx::think()
{
	if(driver) {
		driver->setListeners(listeners);
		driver->think();
	}
	
	for(ChanSounds::iterator i = simpleChanSounds.begin(); i != simpleChanSounds.end(); ) {
		if(!(*i)->isPlaying())
			i = simpleChanSounds.erase(i);
		else
			++i;
	}
}

SfxDriver* Sfx::getDriver()
{
	return driver;
}
	
void Sfx::clear()
{
	if(driver)
		driver->clear();
}

void Sfx::registerListener(Listener* l)
{
	listeners.push_back(l);
}

void Sfx::removeListener(Listener* listener)
{
	vector<Listener*>::iterator i;
	for ( i = listeners.begin(); i != listeners.end(); ++i )
	{
		if ( listener == *i )
		{
			listeners.erase(i);
			break;
		}
	}
}

void Sfx::volumeChange()
{
	if(driver)
		driver->volumeChange();
}

Sfx::operator bool()
{
	return m_initialized;
}

void Sfx::playSimpleGlobal(SoundSample* s) {
	if(s->currentSimulatiousPlays() >= s->maxSimultaniousPlays) return;
	
	SmartPointer<SoundSample> snd = s->copy();
	simpleChanSounds.push_back(snd);
	snd->play(1.0f, 1.0f);
}

void Sfx::playSimple2D(SoundSample* s, CVec pos) {
	if(s->currentSimulatiousPlays() >= s->maxSimultaniousPlays) return;

	SmartPointer<SoundSample> snd = s->copy();
	simpleChanSounds.push_back(snd);
	snd->play2D(Vec(pos), 100, 1);
}


#endif
