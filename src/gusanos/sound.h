#ifndef SOUND_H
#define SOUND_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include <string>
#include "gusanos/resource_list.h"
#include "util/vec.h"
#include "gusanos/glua.h"

class BaseObject;
class SfxDriver;
class SoundSample;

class GusSound : public LuaObject
{
	
	public:
		
	GusSound();
	~GusSound();
	
	bool load(std::string const& filename);
	void play( float volume = 1,float pitch = 1,float volumeVariation = 1,float pitchVariation = 1);
	void play1D( float volume = 1,float pitch = 1,float volumeVariation = 0,float pitchVariation = 0);
	void play2D(const Vec& pos, float loudness = 100, float pitch = 1, float pitchVariation = 1);
	void play2D( BaseObject* obj, float loudness = 100, float pitch = 1, float pitchVariation = 1);
	bool isValid();
	void updateObjSound();
	
	private:
	
	SfxDriver* driver;
	SoundSample *m_sound;
	BaseObject* m_obj;
	
};

extern ResourceList<GusSound> soundList;
extern ResourceList<GusSound> sound1DList;

#endif // _SOUND_H_
