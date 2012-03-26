#ifndef SFX_H
#define SFX_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include "CVec.h"
#include "gusanos/sound.h"



struct Listener
{
	Vec pos;
	Vec spd;
};

class SoundSample;
class SfxDriver;
class Sound;

class Sfx
{
public:
		
	Sfx();
	~Sfx();
	
	bool init();
	void shutDown();
	void registerInConsole();
	void think();
	void clear();
	void registerListener(Listener* listener);
	void removeListener(Listener* listener);
	void volumeChange();
	SfxDriver* getDriver();
	
	void playSimpleGlobal(SoundSample* snd);
	void playSimple2D(SoundSample* snd, CVec pos);
	
	operator bool(); // Returns true if it's safe to use this object
private:
	SfxDriver *driver;
};

extern Sfx sfx;

#endif // _GFX_H_
