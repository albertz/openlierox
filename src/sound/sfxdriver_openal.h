#ifndef SFXDRIVER_OPENAL_H
#define SFXDRIVER_OPENAL_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY


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
	SoundSample* load(std::string const& filename);
};

#endif // SFXDRIVER_OPENAL_H
