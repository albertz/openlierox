#ifndef DEDICATED_ONLY

#include "sound.h"
#include "sound_sample.h"
#include "sfxdriver.h"

#include "sfx.h"
#include "gusanos/resource_list.h"
#include "base_object.h"
#include "util/math_func.h"

#include <cstdio>
#include <vector>
#include <string>



ResourceList<GusSound> soundList;
ResourceList<GusSound> sound1DList;

GusSound::GusSound():m_sound(0)
{
	// actually it should have been passed as an argument
	driver = sfx.getDriver();
}

GusSound::~GusSound()
{
	delete m_sound;
}


bool GusSound::load(std::string const& filename)
{	
	//cout<<"GusSound::load";
	//cerr << "Loading sound: " << filename.native_file_string() << endl;
	m_sound = driver->load("gusanos/" + filename);
	return ( m_sound->avail());
}

void GusSound::play(float volume,float pitch, float volumeVariation, float pitchVariation)
{
	float rndPitch = pitch + rnd()*pitchVariation - pitchVariation / 2;
			
	float rndVolume = pitch + rnd()*volumeVariation - volumeVariation / 2;
	m_sound ->play(rndPitch,rndVolume );
}

void GusSound::play1D(float volume,float pitch, float volumeVariation, float pitchVariation)
{
	float rndPitch = pitch + midrnd()*pitchVariation - pitchVariation / 2;
			
	float rndVolume = pitch + midrnd()*volumeVariation - volumeVariation / 2;
	m_sound ->play(rndPitch,rndVolume );
}

void GusSound::play2D(const Vec& pos, float loudness, float pitch, float pitchVariation)
{
	float rndPitch = pitch + rnd()*pitchVariation - pitchVariation / 2;
	m_sound ->play2D(pos,loudness, rndPitch );
}

void GusSound::play2D(BaseObject* obj, float loudness, float pitch, float pitchVariation)
{
	//cout<<"Play 2d(obj)"<<endl;
	float rndPitch = pitch + rnd()*pitchVariation - pitchVariation / 2;
	m_sound ->play2D(obj,loudness, rndPitch );
	m_obj=obj;
	
}

bool GusSound::isValid()
{
	if (m_obj && !m_obj->deleteMe)
	{
		return m_sound ->isValid();
	}
	return false;
}

void GusSound::updateObjSound()
{
	Vec v(m_obj->pos.x, m_obj->pos.y);
	return m_sound->updateObjSound(v);
}

#endif
