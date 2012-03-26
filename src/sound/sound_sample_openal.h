#ifndef SOUND_SAMPLE_OPENAL_H
#define SOUND_SAMPLE_OPENAL_H

#include "gusanos/resource_list.h"
#include "CVec.h"
#include "sound_sample.h"
#include <AL/al.h>

class CGameObject;
struct OpenALBuffer;
template <> void SmartPointer_ObjectDeinit<OpenALBuffer> ( OpenALBuffer * obj );

class SoundSampleOpenAL : public SoundSample 
{
public:		
	SoundSampleOpenAL(std::string const& filename);
	SoundSampleOpenAL(const SoundSampleOpenAL& s);
	~SoundSampleOpenAL();
	
	void play( float pitch,float volume);
	void play2D(const Vec& pos, float loudness, float pitch);
	void play2D( CGameObject* obj, float loudness, float pitch);
	bool isPlaying();
	void updateObjSound(Vec& vec);
	bool avail();
	SmartPointer<SoundSample> copy() { return new SoundSampleOpenAL(*this); }
	
	size_t currentSimulatiousPlays();

	std::string name();
	size_t GetMemorySize();
	
private:
	
	void initSound();
	SmartPointer<OpenALBuffer> buffer;
	ALuint m_sound;
	
};

#endif // SOUND_SAMPLE_OPENAL_H
