#ifndef DEDSERV

#include "sfx.h"
#include "gconsole.h"
#include "base_object.h"
#include "util/macros.h"
#include <boost/assign/list_inserter.hpp>
using namespace boost::assign;

#include <vector>
#include <list>
#include <fmod.h>
#include <boost/utility.hpp>

using namespace std;

Sfx sfx;

namespace
{
	bool m_initialized = false;
	
	std::list< std::pair< int, BaseObject* > > chanObject;
	std::vector<Listener*> listeners;
	
	int m_volume;
	int m_listenerDistance;
	int m_outputMode = -1;
}

void volume( int oldValue )
{
	if (sfx) sfx.volumeChange();
}

Sfx::Sfx()
{
}

Sfx::~Sfx()
{
}

void Sfx::init()
{
	// Select a driver
	
	FSOUND_SetOutput(m_outputMode);
	FSOUND_SetDriver(0);
	
	/* We select default driver for now
	int numDrivers = FSOUND_GetNumDrivers();
	// TODO: Desired driver here: string desiredDriver(); 
	int selectedDriver = 0;
	
	for(int i = 0; i < numDrivers; ++i)
	{
		const char* driverName = FSOUND_GetDriverName(i);
		console.addLogMsg(string("* FMOD DRIVER ") + cast<string>(i) + string(": ") + driverName);
		if(string(driverName).find(desiredDriver) >= 0)
		{
			selectedDriver = i;
			break;
		}
	}
	
	FSOUND_SetDriver(selectedDriver);*/

	FSOUND_Init(44100, 32, 0);
	FSOUND_3D_SetDistanceFactor(20);
	FSOUND_3D_SetRolloffFactor(2);
	volumeChange();
	
	console.addLogMsg(string("* FMOD LIB INITIALIZED, USING DRIVER ") + FSOUND_GetDriverName(FSOUND_GetDriver()));
	m_initialized = true;
}

void Sfx::shutDown()
{
	FSOUND_Close();
}

void Sfx::registerInConsole()
{
	{
		EnumVariable::MapType outputModes;
		insert(outputModes)
			("AUTO", -1)
			("NOSFX", FSOUND_OUTPUT_NOSOUND)
#ifdef WINDOWS
			("WINMM", FSOUND_OUTPUT_WINMM)
			("DSOUND", FSOUND_OUTPUT_DSOUND)
#else //ifdef LINUX
			("A3D", FSOUND_OUTPUT_A3D) // Is this Linux, Windows or both?
			("OSS", FSOUND_OUTPUT_OSS)
			("ESD", FSOUND_OUTPUT_ESD)
			("ALSA", FSOUND_OUTPUT_ALSA)
			//("ASIO", FSOUND_OUTPUT_ASIO) //What's this
#endif

#if 0 //These aren't useful at the moment
			("XBOX", FSOUND_OUTPUT_XBOX)
			("PS2", FSOUND_OUTPUT_PS2)
			("MAC", FSOUND_OUTPUT_MAC)
			("GC", FSOUND_OUTPUT_GC)
#endif
		;

		console.registerVariables()
			("SFX_VOLUME", &m_volume, 255, volume)
			("SFX_LISTENER_DISTANCE", &m_listenerDistance, 20)
			(new EnumVariable("SFX_OUTPUT_MODE", &m_outputMode, -1, outputModes))
		;
		
		// NOTE: When/if adding a callback to sfx variables, make it do nothing if
		// sfx.operator bool() returns false.
	}
}

void Sfx::think()
{
	FSOUND_Update();
	
	for (size_t i = 0; i < listeners.size(); ++i )
	{
		FSOUND_3D_Listener_SetCurrent(i,listeners.size());
		float pos[3] = { listeners[i]->pos.x, listeners[i]->pos.y, -m_listenerDistance };
		FSOUND_3D_Listener_SetAttributes(pos,NULL,0,0,1,0,1,0);
	}
	
	//Update 3d channel that follow objects positions
	/*
	list< pair< int, BaseObject* > >::iterator obj, next;
	for ( obj = chanObject.begin(); obj != chanObject.end(); obj = next)
	{
		next = boost::next(obj);

	*/
	foreach_delete(obj, chanObject)
	{

		if( !obj->second
		||  obj->second->deleteMe
		||  !FSOUND_IsPlaying( obj->first ) )
		{
			chanObject.erase(obj);
		}
		else
		{
			float pos[3] = { obj->second->pos.x, obj->second->pos.y, 0 };
			FSOUND_3D_SetAttributes(obj->first, pos, NULL);
		}
	}

/*
	//Check for deleted objects
	for ( obj = chanObject.begin(); obj != chanObject.end(); )
	{
		if ( obj->second && ( obj->second->deleteMe || !FSOUND_IsPlaying( obj->first ) ) )
		{
			list< pair< int, BaseObject* > >::iterator tmp = obj;
			obj++;
			chanObject.erase(tmp);
		}
		else
			obj++;
	}*/

}

void Sfx::setChanObject(int chan, BaseObject* object)
{
	chanObject.push_back( pair< int, BaseObject* > ( chan, object ) );
}
	
void Sfx::clear()
{
	chanObject.clear();
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
	FSOUND_SetSFXMasterVolume(m_volume);
}

Sfx::operator bool()
{
	return m_initialized;
} 

#endif
