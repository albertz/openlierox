#include <allegro.h>

#include "level.h"
#include "culling.h"
#include "loaders/vermes.h"
#include "../util/macros.h"
#include "../util/vec.h"
#include "../util/rect.h"
#include "../util/text.h"
#include "gfx.h"

#ifdef WINDOWS
	#include <winalleg.h>
#endif

#include <boost/progress.hpp>

#include <string>
#include <vector>
#include <list>
#include <iostream>
using std::cout;
using std::endl;

#ifdef POSIX
#include <unistd.h>
#endif

Level level;

using namespace std;

struct TestCuller
{
	TestCuller(int* dest_, int* light_, int* dirs_, int myDir_, double f_, int lightX_, int lightY_)
	: dest(dest_), light(light_), dirs(dirs_), myDir(myDir_), lightX(lightX_), lightY(lightY_), f(f_)
	{
		
	}
	
	bool block(int x, int y)
	{
		return !level.unsafeGetMaterial(x, y).worm_pass;
	}
	
	void line(int y, int x1, int x2)
	{
		//hline_add(dest, x1 + scrOffX, y + scrOffY, x2 + scrOffX + 1, makecol(50, 50, 50), 255);
		
		for(int x = x1; x <= x2; ++x)
		{
			Vec v;
			if(x != lightX || y != lightY)
			{
				v = Vec(lightX - x, lightY - y);
			}
			else
				v = Vec(1.f, 1.f);

			double l = v.length();
			double add = (f * 10000.0) / l;
			
			switch(myDir)
			{
				case 0: break; // Do nothing
				case 1: add *= double(abs(lightX - x)) / l; break;
				case 2: add *= double(abs(lightX - x)) / l; break;
				case 3: add *= double(abs(lightY - y)) / l; break;
				case 4: add *= double(abs(lightY - y)) / l; break;
			}
			
			dest[(x) + (y)*level.width()] += add;
			
			int dir = 0;
			bool f = false;
			if(x < lightX && block(x - 1, y)) { f = true; dir = 1; }
			if(x > lightX && block(x + 1, y)) { f = true; dir = 2; }
			if(y < lightY && block(x, y - 1)) { f = true; dir = 3; }
			if(y > lightY && block(x, y + 1)) { f = true; dir = 4; }
			
			if(f)
			{
				light[(x) + (y)*level.width()] += add * 0.53;
				dirs[(x) + (y)*level.width()] = dir;
			}
				
		}
	}
	
	int* dest;
	int* light;
	int* dirs;
	int myDir;

	int lightX;
	int lightY;
	double f;
};

double expose(double light, double exposure)
{
	return (1.0 - exp(-light * exposure)) * 255.0;
}

int main(int argc, char **argv)
{

	float fadeDistance= 10000;
	float prob = 1;

	/* Broken
	for(int i = 0; i < argc; ++i)
	{
		const char* arg = argv[i];
		if(arg[0] == '-')
		{
			switch(arg[1])
			{
				case 'f':
					if(++i >= argc)
						break;
						
					fadeDistance = cast<float>(argv[i]);
				break;
				
				case 'p':
					if(++i >= argc)
						break;
						
					prob = cast<float>(argv[i]);
				break;
			}
		}
	}*/
	allegro_init();
	install_keyboard();
	
	gfx.init();
	
	//set_gfx_mode (GFX_AUTODETECT, 320, 240, 0, 0);
	
	VermesLevelLoader::instance.load( &level, "./" );
	
	if ( level.isLoaded() )
	{
		BITMAP* lightmap = create_bitmap_ex(24, level.material->w, level.material->h);
#if 0
		std::vector<int> lightsource(level.width() * level.height(), 0);
		std::vector<int> lightsourcedir(level.width() * level.height(), 0);
		std::vector<int> lightsourcedest(level.width() * level.height(), 0);
		
		for ( int x = 365; x < 368 ; ++x )
		for ( int y = 131; y < 134 ; ++y )
		{
			lightsource[y*level.width() + x] = 30000/9;
		}
		
		double div = 0.0;
		
		Rect r(0, 0, level.width() - 1, level.height() - 1);
		
		for(int i = 0; i < 5; ++i)
		{
			boost::progress_display show_progress( level.height() );
			
			for ( int y = 1; y < level.height() - 1 ; ++y, ++show_progress )
			for ( int x = 1; x < level.width() - 1 ; ++x )
			{
				int v = lightsource[y*level.width() + x];
				int dir = lightsourcedir[y*level.width() + x];
				
				if(v > 0)
				{
					//cout << ":o" << endl;
					
					int radius = v;
					
					Rect area(x - radius, y - radius, x + radius, y + radius);
					
					area &= r;
					
					Culler<TestCuller> culler(TestCuller(&lightsourcedest[0], &lightsource[0], &lightsourcedir[0], dir, double(v) / 10000.0, x, y), area);
					
					culler.cullOmni(x, y);
				}
				
				lightsource[y*level.width() + x] = 0;
				lightsourcedir[y*level.width() + x] = 0;
			}
		}
		
		for ( int x = 0; x < level.width() ; ++x )
		for ( int y = 0; y < level.height() ; ++y )
		{
			//int v = lightsource[y*level.width() + x] / (div * 1280000.0 / 255.0);
			int v = expose(lightsourcedest[y*level.width() + x], 0.0005);
			if ( v > 255 ) v = 255;
			putpixel( lightmap, x, y, makecol(v, v, v) );
		}
#else
		vector<IVec> lightSources;
	
		BITMAP* lightsource = gfx.loadBitmap("lightsource", 0);
		
		for ( int x = 0; x < lightsource->w ; ++x )
		for ( int y = 0; y < lightsource->h ; ++y )
		{
			if ( getpixel( lightsource, x,y ) != 0 )
			{
				lightSources.push_back(IVec(x,y));
			}
		}
		
		destroy_bitmap(lightsource);
	
		Level::ParticleBlockPredicate pred;
		
		//lightmap = create_bitmap_ex(24,level.material->w, level.material->h);
		
		float xCoord;
		float yCoord;
		
		boost::progress_display show_progress( lightmap->h );
		
		for ( int y = 0; y < lightmap->h; ++y )
		{
			for ( int x = 0; x < lightmap->w; ++x )
			{
				int color = 0;
				float minDistanceSqr = -1;
				int appliedCount = 0;
				for ( int n = 0; n < lightSources.size() ; ++n )
				{
					if ( true ) //rnd() < prob )
					{
						++appliedCount;
						if ( !level.preciseTrace( lightSources[n].x+0.5f, lightSources[n].y + 0.5f, x, y, pred ) )
						{
							float tmpDist = ( Vec(lightSources[n]) - Vec(x,y) ).lengthSqr();
							if ( tmpDist < minDistanceSqr || minDistanceSqr < 0 ) minDistanceSqr = tmpDist;
							color += 255;
						}
					}
				}
				if ( color != 0 )
				{
					float fade = 1 - minDistanceSqr/(fadeDistance*fadeDistance);
					//float fade = sqrt( 1 / (sqrt(minDistanceSqr)*0.1f) );
					if ( fade < 0 ) fade = 0;
					//color *= fade;
					color /= appliedCount;
					if ( color > 255 ) color = 255;
				}
				putpixel( lightmap,x,y,makecol(color,color,color) );
			}
			++show_progress;
		}
#endif

		bool success = gfx.saveBitmap( "newLightmap.png",lightmap,0);
	}
	
	allegro_message("done");

	allegro_exit();

	return(0);
}
END_OF_MAIN();

