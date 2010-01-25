#ifndef SOUND_SAMPLE_OPENAL_H
#define SOUND_SAMPLE_OPENAL_H

#include "gusanos/resource_list.h"
#include "util/vec.h"
#include "gusanos/glua.h"
#include "sound_sample.h"
#include <AL/al.h>

class CGameObject;

class SoundSampleOpenAL : public SoundSample 
{
public:		
	SoundSampleOpenAL(std::string const& filename);
	~SoundSampleOpenAL();
	
	void play( float pitch,float volume);
	void play2D(const Vec& pos, float loudness, float pitch);
	void play2D( CGameObject* obj, float loudness, float pitch);
	bool isValid();
	void updateObjSound(Vec& vec);
	bool avail();
	
	size_t GetMemorySize() { return size; }
	
private:
	
	ALuint m_sound;
	size_t size;
	
};

#endif // SOUND_SAMPLE_OPENAL_H
