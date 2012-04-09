#include "level.h"
#include "game/CMap.h"

#ifndef DEDICATED_ONLY
#include "gfx.h"
#include "blitters/context.h"
#include "CViewport.h"
#endif
#include "material.h"
#include "game/WormInputHandler.h"
#include "sprite_set.h"
#include "sprite.h"
#include "CVec.h"
#include "util/macros.h"
#include "level_effect.h"
#include "culling.h"
#include "events.h"
#include "gusgame.h"
#include "game/CWorm.h"
#include "game/Game.h"

#include "gusanos/allegro.h"
#include <string>
#include <vector>

using namespace std;

#ifndef DEDICATED_ONLY
struct AddCuller : Culler<AddCuller>
{
	AddCuller( CMap& level, ALLEGRO_BITMAP* dest, ALLEGRO_BITMAP* source, int alpha,int dOffx, int dOffy, int sOffx, int sOffy, Rect const& rect )
			: Culler<AddCuller>(rect),
			m_level(level),
			m_dest( dest ),
			m_source( source),
			m_destOffx(dOffx),
			m_destOffy(dOffy),
			m_sourceOffx(sOffx),
			m_sourceOffy(sOffy),
			m_alpha(alpha)
	{}

	bool block(int x, int y)
	{
		// doubleRes to singleRes coords
		return m_level.unsafeGetMaterial(x/2,y/2).blocks_light;
	}

	void line(int y, int x1, int x2)
	{
		drawSpriteLine_add(
		    m_dest,
		    m_source,
		    x1 - m_destOffx,
		    y - m_destOffy,
		    x1 - m_sourceOffx,
		    y - m_sourceOffy,
		    x2 - m_sourceOffx + 1,
		    m_alpha
		);
	}

private:

	CMap const &m_level;

	ALLEGRO_BITMAP* m_dest;
	ALLEGRO_BITMAP* m_source;

	int m_destOffx;
	int m_destOffy;
	int m_sourceOffx;
	int m_sourceOffy;

	int m_alpha;

};
#endif

void CMap::gusInit()
{
	m_gusLoaded = false;
	m_firstFrame = true;
	m_config = 0;

#ifndef DEDICATED_ONLY

	image = NULL;
	background = NULL;
	paralax = NULL;
	lightmap = NULL;
	watermap = NULL;
#endif

	material = NULL;

	// Rock
	m_materialList[0].worm_pass = false;
	m_materialList[0].particle_pass = false;
	m_materialList[0].draw_exps = false;
	m_materialList[0].blocks_light = true;

	// Background
	m_materialList[1].worm_pass = true;
	m_materialList[1].particle_pass = true;
	m_materialList[1].draw_exps = true;

	// Dirt
	m_materialList[2].worm_pass = false;
	m_materialList[2].particle_pass = false;
	m_materialList[2].draw_exps = false;
	m_materialList[2].destroyable = true;
	m_materialList[2].blocks_light = true;
	m_materialList[2].flows = false;

	// Special dirt
	m_materialList[3].worm_pass = true;
	m_materialList[3].particle_pass = false;
	m_materialList[3].draw_exps = false;
	m_materialList[3].destroyable = true;

	// Special rock
	m_materialList[4].worm_pass = false;
	m_materialList[4].particle_pass = true;
	m_materialList[4].draw_exps = false;

	m_materialList[7].worm_pass = false;
	m_materialList[7].particle_pass = false;
	m_materialList[7].draw_exps = false;

	for ( size_t i = 0; i < m_materialList.size() ; ++i ) {
		m_materialList[i].index = i;
		if ( m_materialList[i].flows && !m_materialList[i].is_stagnated_water && i < m_materialList.size()-1 ) {
			m_materialList[i+1] = m_materialList[i];
			m_materialList[i+1].is_stagnated_water = true;
		}
	}
}


void CMap::gusShutdown()
{
	m_gusLoaded = false;
	m_firstFrame = true;

	if(m_config) {
		delete m_config;
		m_config = 0;
	}
	
#ifndef DEDICATED_ONLY

	destroy_bitmap(image);
	image = NULL;
	destroy_bitmap(background);
	background = NULL;
	destroy_bitmap(paralax);
	paralax = NULL;
	destroy_bitmap(lightmap);
	lightmap = NULL;
	destroy_bitmap(watermap);
	watermap = NULL;
#endif

	destroy_bitmap(material);
	material = NULL;

	vectorEncoding = Encoding::VectorEncoding();
}



void CMap::checkWBorders( int x, int y )
{
	if ( getMaterial( x, y-1 ).is_stagnated_water ) {
		unsigned char mat = getMaterialIndex(x, y-1) - 1;
		m_water.push_back( WaterParticle( x, y-1, mat ) );
		putMaterial( mat, x, y-1 );
	}
	if ( getMaterial( x+1, y ).is_stagnated_water ) {
		unsigned char mat = getMaterialIndex(x+1, y) - 1;
		m_water.push_back( WaterParticle( x+1, y, mat ) );
		putMaterial( mat, x+1, y );
	}
	if ( getMaterial( x-1, y ).is_stagnated_water ) {
		unsigned char mat = getMaterialIndex(x-1, y) - 1;
		m_water.push_back( WaterParticle( x-1, y, mat ) );
		putMaterial( mat, x-1, y );
	}

}

static const float WaterSkipFactor = 0.05f;

void CMap::gusThink()
{
	if(!gusIsLoaded())
		return;

	if( m_firstFrame ) {
		m_firstFrame = false;
		if ( m_config && m_config->gameStart )
			m_config->gameStart->run(0,0,0,0);
	}
#ifndef DEDICATED_ONLY
	foreach_delete( wp, m_water ) {
		if ( getMaterialIndex( wp->x, wp->y ) != wp->mat ) {
			copypixel_solid2x2(image, background, wp->x*2, wp->y*2);
			m_water.erase(wp);
		} else
			if ( rnd() > WaterSkipFactor ) {
				unsigned char mat = getMaterialIndex( wp->x, wp->y+1 );
				if ( m_materialList[mat].particle_pass && !m_materialList[mat].flows) {
					checkWBorders( wp->x, wp->y  );
					copypixel_solid2x2(image, background, wp->x*2, wp->y*2);
					putMaterial( 1, wp->x, wp->y );
					++wp->y;
					copypixel_solid2x2(image, watermap, wp->x*2, wp->y*2);
					putMaterial( wp->mat, wp->x, wp->y );
					wp->count = 0; // Reset stagnation counter because it moved
				} else {
					char dir;
					if ( wp->dir )
						dir = 1;
					else
						dir = -1;

					mat = getMaterialIndex( wp->x+dir, wp->y );
					if ( m_materialList[mat].particle_pass && !m_materialList[mat].flows ) {
						checkWBorders( wp->x, wp->y );
						copypixel_solid2x2(image, background, wp->x*2, wp->y*2);
						putMaterial( 1, wp->x, wp->y );
						wp->x += dir;
						copypixel_solid2x2(image, watermap, wp->x*2, wp->y*2);
						putMaterial( wp->mat, wp->x, wp->y );
						wp->count = 0;
						// Reset stagnation counter because it moved
					} else {
						wp->dir = !wp->dir;
						++wp->count; // It didnt move so the stagnation counter gets incremented.
						if ( wp->count > 1 ) {
							mat = getMaterialIndex( wp->x-dir, wp->y );
							if ( !m_materialList[mat].particle_pass || m_materialList[mat].flows ) {
								putMaterial( wp->mat+1, wp->x, wp->y );
								putpixel_solid(image, wp->x, wp->y, getpixel(watermap, wp->x, wp->y) );
								m_water.erase(wp);
							}
						}
					}
				}
			}
	}
#endif
}

#ifndef DEDICATED_ONLY
void CMap::gusDraw(ALLEGRO_BITMAP* where, int x, int y)
{
	if (image) {
		if (!paralax) {
			blit(image,where,x*2,y*2,0,0,where->w,where->h);
		} else {
			int px = int(x * (paralax->w - where->w) / float( image->w - where->w ));
			int py = int(y * (paralax->h - where->h) / float( image->h - where->h ));
			blit(paralax,where,px*2,py*2,0,0,where->w,where->h);
			masked_blit(image,where,x*2,y*2,0,0,where->w,where->h);
		}

		if ( gusGame.options.showMapDebug ) {
			foreach( s, m_config->spawnPoints ) {
				int c = (s->team == 0 ? makecol( 255,0,0 ) : makecol( 0, 255, 0 ));
				circle( where, (int)(s->pos.x - x) * 2, (int)(s->pos.y - y) * 2, 8, c );
			}
		}
	}
}


// TODO: optimize this
void CMap::specialDrawSprite( Sprite* sprite, ALLEGRO_BITMAP* where, const IVec& pos, const IVec& matPos, BlitterContext const& blitter )
{
	// pos are where-coords. i.e. they are already doubleRes
	// matPos are material/world coords. i.e. singleRes

	int transCol = makecol(255,0,255); // TODO: make a gfx.getTransCol() function

	int xMatStart = matPos.x*2 - sprite->m_xPivot;
	int yMatStart = matPos.y*2 - sprite->m_yPivot;
	int xDrawStart = pos.x - sprite->m_xPivot;
	int yDrawStart = pos.y - sprite->m_yPivot;
	for ( int y = 0; y < sprite->m_bitmap->h ; ++y )
		for ( int x = 0; x < sprite->m_bitmap->w ; ++x ) {
			if ( getMaterialDoubleRes( xMatStart + x , yMatStart + y ).draw_exps ) {
				int c = getpixel( sprite->m_bitmap, x, y );
				if ( c != transCol )
					blitter.putpixel( where, xDrawStart + x, yDrawStart + y, c );
			}
		}
}

void CMap::culledDrawSprite( Sprite* sprite, CViewport* viewport, const IVec& pos, int alpha )
{
	ALLEGRO_BITMAP* renderBitmap = sprite->m_bitmap;
	IVec off = viewport->getPos() * 2;
	IVec loff(pos*2 - IVec(sprite->m_xPivot, sprite->m_yPivot));

	Rect r(0, 0, Width*2 - 1, Height*2 - 1);
	r &= Rect(renderBitmap) + loff;


	if ( r.isIntersecting( Rect( viewport->dest ) + off ) ) // Check that it can be seen
	{
		AddCuller addCuller(
		    *this,
		    viewport->dest,
		    renderBitmap,
		    alpha,
			off.x,
			off.y,
			loff.x,
			loff.y,
			r );

		addCuller.cullOmni(pos.x*2, pos.y*2);
	}

}

void CMap::culledDrawLight( Sprite* sprite, CViewport* viewport, const IVec& pos, int alpha )
{
	ALLEGRO_BITMAP* renderBitmap = sprite->m_bitmap;
	IVec off = viewport->getPos() * 2;
	IVec loff(pos * 2 - IVec(sprite->m_xPivot, sprite->m_yPivot));

	Rect r(0, 0, Width*2 - 1, Height*2 - 1);
	r &= Rect(renderBitmap) + loff;


	if ( r.isIntersecting( Rect( viewport->fadeBuffer ) + off ) ) // Check that it can be seen
	{
		// we use drawSpriteLine_add which requires the same bit depths
		assert(renderBitmap->surf->format->BitsPerPixel == 8);

		AddCuller addCuller(
		    *this,
		    viewport->fadeBuffer,
		    renderBitmap,
		    alpha,
		    off.x,
		    off.y,
		    loff.x,
		    loff.y,
		    r );

		addCuller.cullOmni(pos.x*2, pos.y*2);
	}
}

#endif

bool CMap::applyEffect(LevelEffect* effect, int drawX, int drawY )
{
	bool returnValue = false;
	if ( effect && effect->mask ) {
		drawX *= 2; drawY *= 2;
		Sprite* tmpMask = effect->mask->getSprite();
		drawX -= tmpMask->m_xPivot;
		drawY -= tmpMask->m_yPivot;
		unsigned int colour = 0;
		for( int y = 0; y < tmpMask->m_bitmap->h; ++y )
			for( int x = 0; x < tmpMask->m_bitmap->w; ++x ) {
				colour = getpixel( tmpMask->m_bitmap, x, y);
				if( ( colour == 0 ) && getMaterialDoubleRes( drawX+x, drawY+y ).destroyable ) {
					returnValue = true;
					putMaterialDoubleRes( /*background*/1, drawX+x, drawY+y );
					checkWBorders( (drawX+x)/2, (drawY+y)/2 );
#ifndef DEDICATED_ONLY
					putpixel(image, drawX+x, drawY+y, getpixel( background, drawX+x, drawY+y ) );
					putpixel(lightmap, drawX+x, drawY+y, 0);
#endif
				}
			}
		
		UpdateArea(drawX/2, drawY/2, tmpMask->m_bitmap->w/2 + 1, tmpMask->m_bitmap->h/2 + 1, true);
	}
	return returnValue;
}

namespace
{
	bool canPlayerRespawn(CWorm* worm, SpawnPoint const& point)
	{
		bool teamplay = game.isTeamPlay();
		// point.team - 1 because OLX team numbers start at 0, Gus team numbers at 1
		if(teamplay && (point.team != -1) && (point.team - 1 != worm->getTeam()))
			return false;
		return true;
	}
}

bool CMap::getPredefinedSpawnLocation(CWorm* worm, CVec* v) {
	if(m_config) {
		int alt = 0;
		foreach(i, m_config->spawnPoints) {
			if(canPlayerRespawn(worm, *i))
				++alt;
		}
		
		if(alt > 0) {
			int idx = rndInt(alt);
			foreach(i, m_config->spawnPoints) {
				if(canPlayerRespawn(worm, *i) && --idx < 0) {
					*v = CVec(i->pos);
					return true;
				}
			}
		}
	}

	return false;
}

void CMap::loaderSucceeded()
{
	m_water.clear();
	for ( int y = 0; y < material->h; ++y )
		for ( int x = 0; x < material->w; ++x ) {
			if ( unsafeGetMaterial(x,y).flows && !unsafeGetMaterial(x,y).is_stagnated_water ) {
				m_water.push_back( WaterParticle( x, y, getMaterialIndex(x,y) ) );
			}

			if ( unsafeGetMaterial(x,y).is_stagnated_water ) {
				allegro_message( "Map is using a material that is reserved for internal use on water" );
				break;
			}
		}

#ifndef DEDICATED_ONLY
	if ( !lightmap ) {
		// NOTE: doubleRes lightmap
		LocalSetColorDepth cd(8);
		lightmap = create_bitmap(material->w*2, material->h*2);
		clear_to_color(lightmap, 0); // 50 earlier. but 0 makes more sense. also matches the CMap::applyEffect value.
		for ( int x = 0; x < lightmap->w ; ++x )
			for ( int y = 0; y < lightmap->h ; ++y ) {
				if ( unsafeGetMaterial(x/2,y/2).blocks_light )
					putpixel( lightmap, x, y, 200 );
			}
	}

	if(!background) {
		background = create_bitmap_from_sdl(GetCopiedImage(image->surf));
		// Does this make sense? In CMap::applyEffect, we replace every pixel from image by it.
		//DrawRectFill(background->surf.get(), 0, 0, background->w, background->h, Color(0,0,0,120));
	}

	if ( !watermap ) {
		watermap = create_bitmap( image->w, image->h );
		blit( background, watermap, 0,0,0,0,image->w, image->h );
		gfx.setBlender(ALPHA, 150);
		rectfill( watermap, 0,0,watermap->w, watermap->h, makecol( 0, 0, 200 ) );
		solid_mode();
	}
#endif
	// Make the domain one pixel larger than the level so that things like ninjarope hook
	// can get slightly outside the level and attach.
	vectorEncoding = Encoding::VectorEncoding(Rect(-1, -1, Width + 1, Height + 1), 2048);
	intVectorEncoding = Encoding::VectorEncoding(Rect(-1, -1, Width + 1, Height + 1), 1);
	diffVectorEncoding = Encoding::DiffVectorEncoding(1024);
	//cerr << "vectorEncoding: " << vectorEncoding.totalBits() << endl;

	if(!m_config)
		m_config = new LevelConfig(); // Default config
	
	bmpDrawImage = image->surf;
	bmpBackImageHiRes = background->surf;
}



void CMap::gusUpdateMinimap(int x, int y, int w, int h) {
	void (*blitFct) ( SDL_Surface * bmpDest, SDL_Surface * bmpSrc, int sx, int sy, int dx, int dy, int sw, int sh, float xratio, float yratio);
	
	if (tLXOptions->bAntiAliasing)
		blitFct = &DrawImageResampledAdv;
	else
		blitFct = &DrawImageResizedAdv;
	
	// Calculate ratios
	const float xratio = (float)bmpMiniMap.get()->w / (float)Width;
	const float yratio = (float)bmpMiniMap.get()->h / (float)Height;
	
	const int dx = (int)((float)x * xratio);
	const int dy = (int)((float)y * yratio);

	if (paralax) {
		// Calculate ratios
		const float parxratio = (float)paralax->w / (float)image->w;
		const float paryratio = (float)paralax->h / (float)image->h;
		
		const int parx = (int)((float)x * parxratio * 2);
		const int pary = (int)((float)y * paryratio * 2);
		const int parw = (int)((float)w * parxratio * 2);
		const int parh = (int)((float)h * paryratio * 2);

		(*blitFct) (bmpMiniMap.get(), paralax->surf.get(), parx, pary, dx, dy, parw, parh, 0.5f * xratio / parxratio, 0.5f * yratio / paryratio);
	} else {
		DrawRectFill(bmpMiniMap.get(), x - 1, y - 1, x + w + 1, y + h + 1, Color());
	}
	
	(*blitFct) (bmpMiniMap.get(), image->surf.get(), (x - 1)*2, (y - 1)*2, dx, dy, (w + 1)*2, (h + 1)*2, xratio*0.5f, yratio*0.5f);
}

