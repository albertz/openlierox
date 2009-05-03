#ifndef SOUND_SAMPLE_H
#define SOUND_SAMPLE_H

#ifdef DEDSERV
#error "Can't use this in dedicated server"
#endif //DEDSERV

#include "resource_list.h"
#include "util/vec.h"
#include "glua.h"
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

class BaseObject;

class SoundSample 
{
	
	public:
		
	SoundSample(fs::path const& filename);
	virtual ~SoundSample();
	
	virtual void play( float pitch,float volume)=0;
	virtual void play2D(const Vec& pos, float loudness, float pitch)=0;
	virtual void play2D( BaseObject* obj, float loudness, float pitch)=0;
	virtual bool isValid()=0;
	virtual void updateObjSound(Vec& vec)=0;
	virtual bool avail()=0;
	private:

	
};


#endif // SOUND_SAMPLE_H
