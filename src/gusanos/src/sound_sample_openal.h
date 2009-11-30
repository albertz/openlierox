#ifndef SOUND_SAMPLE_OPENAL_H
#define SOUND_SAMPLE_OPENAL_H

#include "resource_list.h"
#include "util/vec.h"
#include "glua.h"
#include "sound_sample.h"
#include <OpenAL/al.h>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

class BaseObject;

class SoundSampleOpenAL : public SoundSample 
{
	
	public:
		
	SoundSampleOpenAL(fs::path const& filename);
	~SoundSampleOpenAL();
	
	void play( float pitch,float volume);
	void play2D(const Vec& pos, float loudness, float pitch);
	void play2D( BaseObject* obj, float loudness, float pitch);
	bool isValid();
	void updateObjSound(Vec& vec);
	bool avail();
	private:
	
	ALuint m_sound;
	
};

#endif // SOUND_SAMPLE_OPENAL_H
