#ifndef DEDSERV

#include "sound_sample_openal.h"
#include "resource_list.h"
#include "base_object.h"

#include <vorbis/vorbisfile.h>
#include <OpenAL/al.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string.hpp>
namespace fs = boost::filesystem;


using namespace std;


namespace
{
	const int BUFFER_SIZE = 32768;       // 32 KB buffers
}


// This function loads a .ogg file into a memory buffer and returns
// the format and frequency.
void LoadOGG(const char *fileName, vector<char> &buffer, ALenum &format, ALsizei &freq)
    {
    int endian = 0;                         // 0 for Little-Endian, 1 for Big-Endian
    int bitStream;
    long bytes;
    char array[BUFFER_SIZE];                // Local fixed size array
    FILE *f;

    // Open for binary reading
    f = fopen(fileName, "rb");

    if (f == NULL)
        {
        //cerr << "Cannot open " << fileName << " for reading..." << endl;
        throw -1;
        }
    // end if

    vorbis_info *pInfo;
    OggVorbis_File oggFile;

    // Try opening the given file
    if (ov_open(f, &oggFile, NULL, 0) != 0)
        {
        cerr << "Error opening " << fileName << " for decoding..." << endl;
        throw -1;
        }
    // end if

    // Get some information about the OGG file
    pInfo = ov_info(&oggFile, -1);

    // Check the number of channels... always use 16-bit samples
    if (pInfo->channels == 1)
        format = AL_FORMAT_MONO16;
    else
        format = AL_FORMAT_STEREO16;
    // end if

    // The frequency of the sampling rate
    freq = pInfo->rate;

    // Keep reading until all is read
    do
        {
        // Read up to a buffer's worth of decoded sound data
        bytes = ov_read(&oggFile, array, BUFFER_SIZE, endian, 2, 1, &bitStream);

        if (bytes < 0)
            {
            ov_clear(&oggFile);
            cerr << "Error decoding " << fileName << "..." << endl;
        throw -1;
            }
        // end if

        // Append to end of buffer
        buffer.insert(buffer.end(), array, array + bytes);
        }
    while (bytes > 0);

    // Clean up!
    ov_clear(&oggFile);
    }
// end of LoadOGG


ALuint  LoadSoundFromFile( const char* inSoundFile ) 
{
    ALuint bufferID;                        // The OpenAL sound buffer ID
    ALenum format;                          // The sound data format
    ALsizei freq;                           // The frequency of the sound data
    vector<char> bufferData;                // The sound buffer data from file

    // Create sound buffer and source
    alGenBuffers(1, &bufferID);

	try{
    // Load the OGG file into memory
    LoadOGG(inSoundFile, bufferData, format, freq);
	}
	catch 
	 (int error) {
		return 0;
	}
    // Upload sound data to buffer
    alBufferData(bufferID, format, &bufferData[0], static_cast<ALsizei>(bufferData.size()), freq);
    return bufferID;
  
}


SoundSampleOpenAL::SoundSampleOpenAL(fs::path const& filename):SoundSample(filename)
{
    ALuint bufferID;                        // The OpenAL sound buffer ID
	string name = filename.native_file_string();
	
	if (boost::iends_with(name, ".ogg"))
	{
		bufferID=LoadSoundFromFile( name.c_str());
		if (bufferID==0)
		{
			m_sound=0;
			return;
		}
	}
	else
	{
		bufferID=alutCreateBufferFromFile (name.c_str());
		if (bufferID==AL_NONE)
		{
			m_sound=0;
			return;
		}
	}
    ALuint sourceID;                        // The OpenAL sound source
    alGenSources(1, &sourceID);
    // Attach sound buffer to source
    alSourcei(sourceID, AL_BUFFER, bufferID);
	alSourcef(	 sourceID,AL_ROLLOFF_FACTOR,2);
	alSourcef(sourceID,AL_GAIN,0.7f);
	//alSourcef(	 mALSource,AL_REFERENCE_DISTANCE,20);
    /*alSource3f(mALSource, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(mALSource, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(mALSource, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef (mALSource, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcei (mALSource, AL_SOURCE_RELATIVE, AL_TRUE      );*/
	m_sound=sourceID ;

}

SoundSampleOpenAL::~SoundSampleOpenAL()
{
	if ( m_sound)  
	{
		ALint mALBufferi;
		alGetSourcei(m_sound, AL_BUFFER ,&mALBufferi);
		ALuint mALBufferu = (ALuint )mALBufferi;
		alDeleteBuffers(1,&mALBufferu);
		alDeleteSources(1,&m_sound);
	}
	
}


bool SoundSampleOpenAL::avail()
{
	return (m_sound!=0);
}


void SoundSampleOpenAL::play( float pitch,float volume)
{
	if( m_sound ) 
	{
		//cout<<"Play "<<endl;
			alSourcef(	 m_sound ,AL_PITCH,pitch);
			alSourcef(	 m_sound ,AL_GAIN,volume );
			alSourcePlay( m_sound) ;			
	}
}


void SoundSampleOpenAL::play2D(const Vec& pos, float loudness, float pitch)
{
	if( m_sound ) 
	{
		//cout<<"Play 2D"<<endl;
			float _pos[3] = { pos.x, pos.y, 0 };
			//cout<<"sound: x,y: "<<pos.x<<" "<<pos.y<<endl;
			alSourcefv( m_sound , AL_POSITION, _pos);
			alSourcef(	 m_sound ,AL_PITCH,pitch);
			alSourcef(	 m_sound ,AL_REFERENCE_DISTANCE,loudness/100*50); //ok?
			alSourcePlay( m_sound) ;			
	}
}

void SoundSampleOpenAL::play2D(BaseObject* obj, float loudness, float pitch)
{
	//cout<<"Play 2d(obj)"<<endl;
	Vec pos( obj->pos.x, obj->pos.y) ;
	play2D(pos,loudness, pitch);
	
}

bool SoundSampleOpenAL::isValid()
{
	ALint state;
	alGetSourcei(m_sound,AL_SOURCE_STATE,&state);
	if (state==AL_PLAYING) 
	{
		return true;
	}
	return false; //I think
}

void SoundSampleOpenAL::updateObjSound(Vec& vec)
{
	ALfloat pos[3] = { vec.x, vec.y, 0 };
	alSourcefv( m_sound , AL_POSITION, pos);
	
}



#endif
