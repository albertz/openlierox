#ifndef SFXDRIVER_OPENAL_H
#define SFXDRIVER_OPENAL_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include "SmartPointer.h"
#include "sfxdriver.h"



class SfxDriverOpenAL : public SfxDriver
{
public:
		
	bool init();
	void shutDown();
	void think();
	void volumeChange();
	void setChanObject(Sound*);
	void clear();
	SmartPointer<SoundSample> load(std::string const& filename);
};

#endif // SFXDRIVER_OPENAL_H
