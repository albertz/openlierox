#ifndef DEDICATED_ONLY

#include "sound.h"
#include "sound/sound_sample.h"
#include "sound/sfxdriver.h"

#include "sound/sfx.h"
#include "gusanos/resource_list.h"
#include "CGameObject.h"
#include "util/math_func.h"

#include <cstdio>
#include <vector>
#include <string>



ResourceList<Sound> soundList;
ResourceList<Sound> sound1DList;

Sound::Sound():m_sound(0)
{
	// actually it should have been passed as an argument
	driver = sfx.getDriver();
}

Sound::~Sound()
{
}


bool Sound::load(std::string const& filename)
{	
	if(driver == NULL) return false;
	//cout<<"Sound::load";
	//cerr << "Loading sound: " << filename.native_file_string() << endl;
	m_sound = LoadSample(filename, 1);
	return ( m_sound.get() && m_sound->avail() );
}

void Sound::play(float volume,float pitch, float volumeVariation, float pitchVariation)
{
	float rndPitch = pitch + (float)rnd()*pitchVariation - pitchVariation / 2;
			
	float rndVolume = pitch + (float)rnd()*volumeVariation - volumeVariation / 2;
	m_sound ->play(rndPitch,rndVolume );
}

void Sound::play1D(float volume,float pitch, float volumeVariation, float pitchVariation)
{
	float rndPitch = pitch + (float)midrnd()*pitchVariation - pitchVariation / 2;
			
	float rndVolume = pitch + (float)midrnd()*volumeVariation - volumeVariation / 2;
	m_sound ->play(rndPitch,rndVolume );
}

void Sound::play2D(const Vec& pos, float loudness, float pitch, float pitchVariation)
{
	float rndPitch = pitch + (float)rnd()*pitchVariation - pitchVariation / 2;
	m_sound ->play2D(pos,loudness, rndPitch );
}

void Sound::play2D(CGameObject* obj, float loudness, float pitch, float pitchVariation)
{
	//cout<<"Play 2d(obj)"<<endl;
	float rndPitch = pitch + (float)rnd()*pitchVariation - pitchVariation / 2;
	m_sound ->play2D(obj,loudness, rndPitch );
	m_obj=obj;
	
}

bool Sound::isValid()
{
	if (m_obj && !m_obj->deleteMe)
	{
		return m_sound ->isValid();
	}
	return false;
}

void Sound::updateObjSound()
{
	Vec v(m_obj->pos());
	return m_sound->updateObjSound(v);
}

#endif
