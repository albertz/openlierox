#ifndef DEDICATED_ONLY

#include <vector>
#include <list>
#include <AL/al.h>
#include <AL/alut.h>
#include <boost/utility.hpp>

#include "Debug.h"

#include "gusanos/gconsole.h"
#include "CGameObject.h"
#include "util/macros.h"
#include "sfxdriver_openal.h"
#include "sound_sample_openal.h"
#include "sound_sample.h"

#include <boost/assign/list_inserter.hpp>
using namespace boost::assign;

using namespace std;


namespace
{
	std::list< Sound* > chanObject;
}


bool SfxDriverOpenAL::init()
{
	ALboolean init=alutInit(NULL,NULL);
	if (init==AL_FALSE) 
	{
	   errors << "SfxDriverOpenAL: ALUT error: " << alutGetErrorString (alutGetError ()) << endl;
	   return false;
	}
	volumeChange();
	// orientation doesn't change during the game
	ALfloat listenerOri[]={0.0,0.0,-1.0, 0.0,1.0,0.0};
	alListenerfv(AL_ORIENTATION,listenerOri);

	hints << "OpenAL lib initialized" << endl;
	return true;
}

void SfxDriverOpenAL::shutDown()
{
	alutExit();
}


void SfxDriverOpenAL::think()
{
	
	for (size_t i = 0; i < listeners.size(); ++i )
	{
		ALfloat listenerPos[]={listeners[i]->pos.x,listeners[i]->pos.y,(ALfloat)-m_listenerDistance };
		//cout<<"listener x,y,z "<<listenerPos[0]<<" "<<listenerPos[1]<<" "<<listenerPos[2]<<endl;
		alListenerfv(AL_POSITION,listenerPos);
		//multi listeners are not supported in OpenAL
		break;
	}
	
	foreach_delete(obj, chanObject)
	{

		if( !(*obj)->isValid())
		{
			chanObject.erase(obj);
		}
		else
		{
			(*obj)->updateObjSound();
		}
	}

}
	
void SfxDriverOpenAL::clear()
{
	chanObject.clear();
}


void SfxDriverOpenAL::volumeChange()
{
	notes << "sfx volume: " << m_volume << endl;
	//multi listeners are not supported in OpenAL
	alListenerf(AL_GAIN,(float)m_volume);
}

SmartPointer<SoundSample> SfxDriverOpenAL::load(std::string const& filename)
{
	return new SoundSampleOpenAL(filename);
		
}
#endif
