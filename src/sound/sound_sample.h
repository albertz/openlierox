#ifndef SOUND_SAMPLE_H
#define SOUND_SAMPLE_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include "gusanos/resource_list.h"
#include "util/vec.h"
#include "gusanos/glua.h"

class CGameObject;

class SoundSample 
{
public:
		
	SoundSample(std::string const& filename);
	virtual ~SoundSample();
	
	virtual void play( float pitch,float volume)=0;
	virtual void play2D(const Vec& pos, float loudness, float pitch)=0;
	virtual void play2D( CGameObject* obj, float loudness, float pitch)=0;
	virtual bool isValid()=0;
	virtual void updateObjSound(Vec& vec)=0;
	virtual bool avail()=0;


	virtual size_t GetMemorySize() = 0;
};


#endif // SOUND_SAMPLE_H
