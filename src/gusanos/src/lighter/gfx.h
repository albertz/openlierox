#ifndef GFX_H
#define GFX_H

#include <allegro.h>

#include <string>
#include <list>

class Gfx
{
public:

	enum Blenders
	{
		ALPHA,
		ADD,
		NONE
	};

	Gfx();
	~Gfx();
	
	void init();
	
	BITMAP* loadBitmap(const std::string &filename, RGB* palette = NULL, bool keepAlpha = false);
	bool saveBitmap(const std::string &filename, BITMAP* image, RGB* palette = NULL);
	
	inline void setBlender( Blenders blender, int alpha )
	{
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
		switch ( blender )
		{
			case ALPHA:
				set_trans_blender(255, 255, 255, alpha);
			break;
			
			case ADD:
				set_add_blender( 255,255,255, alpha);
			break;
			
			case NONE:
				solid_mode();
			break;
		}
	}
	
private:
	
};

extern Gfx gfx;

struct LocalSetColorConversion
{
	LocalSetColorConversion(int flags)
	: old(get_color_conversion())
	{
		set_color_conversion(flags);
	}
	
	~LocalSetColorConversion()
	{
		set_color_conversion(old);
	}
	
private:
	int old;
};

struct LocalSetColorDepth
{
	LocalSetColorDepth(int depth)
	: old(get_color_depth())
	{
		set_color_depth(depth);
	}
	
	~LocalSetColorDepth()
	{
		set_color_depth(old);
	}
	
private:
	int old;
};

#endif // _GFX_H_
