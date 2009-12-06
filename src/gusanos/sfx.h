#ifndef SFX_H
#define SFX_H

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV

#include "util/vec.h"
#include "sound.h"

void volume( int oldValue );


struct Listener
{
	Vec pos;
	Vec spd;
};


class SfxDriver;
	
class Sfx
{
public:
		
	Sfx();
	~Sfx();
	
	void init();
	void shutDown();
	void registerInConsole();
	void think();
	void setChanObject(int chan, Sound* sound);
	void clear();
	Listener* newListener();
	void freeListener(Listener* listener);
	void volumeChange();
	SfxDriver* getDriver();
	
	operator bool(); // Returns true if it's safe to use this object
private:
	SfxDriver *driver;
};

extern Sfx sfx;

#endif // _GFX_H_
