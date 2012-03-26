#ifndef SOUND_SAMPLE_H
#define SOUND_SAMPLE_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include <string>
#include "SmartPointer.h"
#include "gusanos/resource_list.h"
#include "CVec.h"

class CGameObject;

class SoundSample 
{
public:
	size_t maxSimultaniousPlays; // only for LX sounds
	
	SoundSample() : maxSimultaniousPlays(0) {}
	virtual ~SoundSample() {}
	
	virtual void play( float pitch,float volume)=0;
	virtual void play2D(const Vec& pos, float loudness, float pitch)=0;
	virtual void play2D( CGameObject* obj, float loudness, float pitch)=0;
	virtual bool isPlaying()=0;
	virtual void updateObjSound(Vec& vec)=0;
	virtual bool avail()=0;

	virtual size_t currentSimulatiousPlays() = 0; // only for LX
	virtual SmartPointer<SoundSample> copy() = 0;

	virtual std::string name() { return "<unnamed>"; }
	virtual size_t GetMemorySize() = 0;
};


#endif // SOUND_SAMPLE_H
