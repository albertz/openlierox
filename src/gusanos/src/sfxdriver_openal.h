#ifndef SFXDRIVER_OPENAL_H
#define SFXDRIVER_OPENAL_H

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV


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
	SoundSample* load(fs::path const& filename);
};

#endif // SFXDRIVER_OPENAL_H
