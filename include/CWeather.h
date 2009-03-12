/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Weather class
// Created 14/3/03
// Jason Boettcher


#ifndef __CWEATHER_H__
#define __CWEATHER_H__

#include <SDL.h>
#include "CVec.h"
#include "SmartPointer.h"

class CMap;
class CViewport;


// Weather types
enum {
    wth_snow,
    wth_rain,
    wth_hail
};


// Particle types
enum {
    wpt_snowpart,
    wpt_raindrop,
    wpt_hailbit
};


// Weather particle
class wthpart_t { public:
    bool    bUsed;
    int     nType;
    int     nType2;
    CVec    cPos;
    CVec    cVel;
};


#define     MAX_WEATHERPARTS    1024


class CWeather {
private:
    // Attributes

    // Particles
    int         m_nType;
    wthpart_t   *m_psParticles;

    // Snow details
    AbsTime       m_fNextSnow;
    float       m_fWind;

    // Graphics
    SmartPointer<SDL_Surface> m_psSnowPart;


public:
    // Methods
    CWeather();

    bool        Initialize(int nType);
    void        Shutdown(void);
    
    void        Simulate(float dt);
    void        SimulateSnow(float dt);
    void        SpawnParticle(int nType, int nType2, CVec cVel, CVec cPos);

    void        Draw(SDL_Surface * psDest, CViewport *view);

};



#endif  //  __CWEATHER_H__
