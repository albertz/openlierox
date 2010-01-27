#ifndef DEDICATED_ONLY

#include <boost/assign/list_inserter.hpp>
using namespace boost::assign;

#include <vector>
#include <list>
#include <AL/al.h>
#include <boost/utility.hpp>

#include "sound/sfx.h"
#include "sound/sfxdriver.h"
#include "sound/sfxdriver_openal.h"
#include "gusanos/gconsole.h"
#include "CGameObject.h"
#include "util/macros.h"

#include "Debug.h"


using namespace std;

Sfx sfx;

namespace
{
	bool m_initialized = false;
	
	std::vector<Listener*> listeners;
}

void update_volume( int oldValue )
{
	notes<<"update_volume"<<endl;
	if (sfx)
		sfx.volumeChange();
}


Sfx::Sfx():driver(0)
{
}

Sfx::~Sfx()
{
	if (driver) {
		delete driver;
		driver = NULL;
		m_initialized = false;
	}
}

bool Sfx::init()
{
	//notes<<"Sfx::init()"<<endl;
	driver = new SfxDriverOpenAL();
	m_initialized = driver->init();
	return m_initialized;
}

void Sfx::shutDown()
{
	if(driver)
		driver->shutDown();
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

#endif
