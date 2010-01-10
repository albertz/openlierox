#ifndef SOUND_H
#define SOUND_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include <string>
#include "gusanos/resource_list.h"
#include "util/vec.h"
#include "gusanos/glua.h"

class CGameObject;
class SfxDriver;
class SoundSample;

class Sound : public LuaObject
{
	
	public:
		
	Sound();
	~Sound();
	
	bool load(std::string const& filename);
	void play( float volume = 1,float pitch = 1,float volumeVariation = 1,float pitchVariation = 1);
	void play1D( float volume = 1,float pitch = 1,float volumeVariation = 0,float pitchVariation = 0);
	void play2D(const Vec& pos, float loudness = 100, float pitch = 1, float pitchVariation = 1);
	void play2D( CGameObject* obj, float loudness = 100, float pitch = 1, float pitchVariation = 1);
	bool isValid();
	void updateObjSound();
	
	private:
	
	SfxDriver* driver;
	SoundSample *m_sound;
	CGameObject* m_obj;
	
};

extern ResourceList<Sound> soundList;
extern ResourceList<Sound> sound1DList;

#endif // _SOUND_H_
