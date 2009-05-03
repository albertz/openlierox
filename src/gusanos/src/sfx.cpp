#ifndef DEDSERV

#include "sfx.h"
#include "gconsole.h"
#include "base_object.h"
#include "sfxdriver.h"
#include "sfxdriver_openal.h"
#include "util/macros.h"
#include <boost/assign/list_inserter.hpp>
using namespace boost::assign;

#include <vector>
#include <list>
#include <AL/al.h>
#include <AL/alut.h>
#include <boost/utility.hpp>

using namespace std;

Sfx sfx;

namespace
{
	bool m_initialized = false;
	
	std::vector<Listener*> listeners;
}

void volume( int oldValue )
{
	cout<<"volume"<<endl;
	if (sfx)
		sfx.volumeChange();
}


Sfx::Sfx():driver(0)
{
	cerr<<"Sfx::Sfx()"<<endl;
}

Sfx::~Sfx()
{
	if (driver) {
		delete driver;
	}
}

void Sfx::init()
{
	//cerr<<"Sfx::init()"<<endl;
	driver = new SfxDriverOpenAL();
	m_initialized = driver->init();
	cout<<m_initialized <<endl;
}

void Sfx::shutDown()
{
	//cerr<<"Sfx::shutDown()"<<endl;
	driver->shutDown();
}

void Sfx::registerInConsole()
{
	cerr<<"Sfx::registerInConsole()"<<endl;
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
