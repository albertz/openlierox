#ifndef SFXDRIVER_H
#define SFXDRIVER_H

#ifdef DEDICATED_ONLY
#error "Can't use this in dedicated server"
#endif //DEDICATED_ONLY

#include "SmartPointer.h"
#include "sfx.h"

#include <vector>
#include <string>

class SoundSample;

static const float SFX_LISTENER_DISTANCE = 20.0f;

class SfxDriver
{
	friend SmartPointer<SoundSample> LoadSample(const std::string& _filename, int maxplaying);

public:
	
	SfxDriver();
	virtual ~SfxDriver();
	virtual bool init()=0;
	virtual void shutDown()=0;
	virtual void registerInConsole();
	virtual void think()=0;
	virtual void clear()=0;
	virtual void volumeChange()=0;
	void setListeners(std::vector<Listener*> &_listeners);

	float volume() const;
	
protected:
	virtual SmartPointer<SoundSample> load(std::string const& filename)=0;

	std::vector<Listener*> listeners;

private:

};

#endif // SFXDRIVER_H
