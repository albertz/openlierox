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
	//notes<<"Sfx::shutDown()"<<endl;
	driver->shutDown();
}

void Sfx::registerInConsole()
{
	notes<<"Sfx::registerInConsole()"<<endl;
	if (m_initialized ) {
	driver->registerInConsole();
	}
}

void Sfx::think()
{
	driver->setListeners(listeners);
	driver->think();
}

SfxDriver* Sfx::getDriver()
{
	return driver;
}
	
void Sfx::clear()
{
	driver->clear();
}

Listener* Sfx::newListener()
{
	listeners.push_back( new Listener );
	return listeners.back();
}

void Sfx::freeListener(Listener* listener)
{
	vector<Listener*>::iterator i;
	for ( i = listeners.begin(); i != listeners.end(); ++i )
	{
		if ( listener == *i )
		{
			delete *i;
			listeners.erase(i);
			break;
		}
	}
}

void Sfx::volumeChange()
{
	driver->volumeChange();
}

Sfx::operator bool()
{
	return m_initialized;
} 

#endif
