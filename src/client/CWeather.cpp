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
// Created 30/3/03
// Jason Boettcher


#include "LieroX.h"

#include "GfxPrimitives.h"
#include "CViewport.h"
#include "CWeather.h"
#include "MathLib.h"


///////////////////
// CWeather constructor
CWeather::CWeather()
{
    m_nType = 0;
    m_psParticles = NULL;
    
    m_psSnowPart = NULL;
    
    m_fNextSnow = -9999;
    m_fWind = 0;
}


///////////////////
// Initialize the weather
bool CWeather::Initialize(int nType)
{
    m_nType = nType;
    
    m_psParticles = new wthpart_t[MAX_WEATHERPARTS];
    if( !m_psParticles )
        return false;

    // Clear all the particles to unused
    for(int i=0; i<MAX_WEATHERPARTS; i++)
        m_psParticles[i].bUsed = false;

    // Load the graphics
    m_psSnowPart = LoadGameImage("data/gfx/snowpart.png");
    if( !m_psSnowPart.get() )
        return false;

    m_fNextSnow = -9999;
    m_fWind = 0;

    return true;
}


///////////////////
// Shutdown the weather
void CWeather::Shutdown(void)
{
    if( m_psParticles ) {
        delete[] m_psParticles;
        m_psParticles = NULL;
    }
}


///////////////////
// Simulate the weather
void CWeather::Simulate(float dt, CMap *pcMap)
{
    switch( m_nType ) {

        // Snow
        case wth_snow:
            SimulateSnow(dt,pcMap);
            break;
    }
}


///////////////////
// Simulate the snow
void CWeather::SimulateSnow(float dt, CMap *pcMap)
{
    // Spawn a new particle along the top of the map
    // TODO: Do this every n seconds

    if( tLX->fCurTime > m_fNextSnow ) {
        for( int n=0; n<5; n++) {
            float x = (float)GetRandomInt(pcMap->GetWidth());
            SpawnParticle(wpt_snowpart, 0, CVec(GetRandomNum()*10,(float)GetRandomInt(10)+15),CVec(x,2));
        }
        m_fNextSnow = tLX->fCurTime+0.1f;
    }

    // Wind swaying from side to side
    m_fWind += dt*5;
    if( m_fWind > 360 )
        m_fWind -= 360;
    float wind = (float) (sin( m_fWind * PI/180 ) * 1);


    // Simulate the particles
    wthpart_t *psPart = m_psParticles;

    for(int i=0; i<MAX_WEATHERPARTS; i++, psPart++) {
        if( !psPart->bUsed )
            continue;

        // Wind
        psPart->cVel += CVec(wind,0)*dt;
        
        //psPart->cVel += CVec(0,50)*dt;      // Gravity
        psPart->cPos += psPart->cVel*dt;

        switch( psPart->nType ) {

            // Snow particle
            case wpt_snowpart:
                // Have we hit some dirt/rock?
                byte flag = pcMap->GetPixelFlag( (int)psPart->cPos.x, (int)psPart->cPos.y );
                if( flag & PX_DIRT || flag & PX_ROCK ) {
                    // Leave a white dot
                    pcMap->PutImagePixel( (int)psPart->cPos.x, (int)psPart->cPos.y, tLX->clWhite );
                    psPart->bUsed = false;
                }
                break;
        }
    }
}


///////////////////
// Spawn a weather particle
void CWeather::SpawnParticle(int nType, int nType2, CVec cVel, CVec cPos)
{
    // Find an unused particle    
    wthpart_t *psPart = m_psParticles;

	int i;
    for(i=0; i<MAX_WEATHERPARTS; i++, psPart++) {
        if( !psPart->bUsed )
            break;
    }

    // No free particles
    if( i==MAX_WEATHERPARTS-1 )
        return;

    psPart->bUsed = true;
    psPart->cPos = cPos;
    psPart->cVel = cVel;
    psPart->nType = nType;
    psPart->nType2 = nType2;
}


///////////////////
// Draw the weather
void CWeather::Draw(SDL_Surface * psDest, CViewport *view)
{
    int wx = view->GetWorldX();
	int wy = view->GetWorldY();
	int l = view->GetLeft();
	int t = view->GetTop();	

    // Draw the particles
    wthpart_t *psPart = m_psParticles;

    for(int i=0; i<MAX_WEATHERPARTS; i++, psPart++) {
        if( !psPart->bUsed )
            continue;

        int x=((int)psPart->cPos.x-wx)*2+l;
    	int y=((int)psPart->cPos.y-wy)*2+t;


        switch( psPart->nType ) {

            // Snow particle
            case wpt_snowpart:
                DrawImage( psDest, m_psSnowPart, x,y );
                break;
        }

    }


}
