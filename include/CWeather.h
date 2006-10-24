/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Weather class
// Created 14/3/03
// Jason Boettcher


#ifndef __CWEATHER_H__
#define __CWEATHER_H__


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
typedef struct {
    bool    bUsed;
    int     nType;
    int     nType2;
    CVec    cPos;
    CVec    cVel;
} wthpart_t;


#define     MAX_WEATHERPARTS    1024


class CWeather {
private:
    // Attributes

    // Particles
    int         m_nType;
    wthpart_t   *m_psParticles;

    // Snow details
    float       m_fNextSnow;
    float       m_fWind;

    // Graphics
    SDL_Surface *m_psSnowPart;


public:
    // Methods
    CWeather();

    bool        Initialize(int nType);
    void        Shutdown(void);
    
    void        Simulate(float dt, CMap *pcMap);
    void        SimulateSnow(float dt, CMap *pcMap);
    void        SpawnParticle(int nType, int nType2, CVec cVel, CVec cPos);

    void        Draw(SDL_Surface *psDest, CViewport *view);

};



#endif  //  __CWEATHER_H__