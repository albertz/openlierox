#ifndef SFXDRIVER_H
#define SFXDRIVER_H

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV

#include "sfx.h"

#include <vector>
using namespace std;

#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

class SoundSample;


class SfxDriver
{
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
	virtual SoundSample *load(fs::path const& filename)=0;

protected:
	std::vector<Listener*> listeners;
	int m_volume;
	int m_listenerDistance;

	const int MAX_VOLUME;

private:

};

#endif // SFXDRIVER_H
