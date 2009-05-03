#include "bindings-gfx.h"

#include "luaapi/types.h"
#include "luaapi/macros.h"
#include "luaapi/classes.h"

#include "../glua.h"
#include "util/log.h"

#ifndef DEDSERV
#include "../viewport.h"
#include "../gfx.h"
#endif

#include <cmath>
#include <iostream>
#include <allegro.h>
using std::cerr;
using std::endl;
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
using boost::lexical_cast;

namespace LuaBindings
{
	
#ifndef DEDSERV
LuaReference ViewportMetaTable;
LuaReference BITMAPMetaTable;
BlitterContext blitter;
#endif

/*! gfx_draw_box(bitmap, x1, y1, x2, y2, r, g, b)

	//This function is deprecated in 0.9c and later. Use the draw_box method of Bitmap instead//.
*/

/*! Bitmap:draw_box(bitmap, x1, y1, x2, y2, color)

	Draws a filled box with the corners (x1, y1) and (x2, y2)
	with the color //color// using the currently selected blender.
*/
int l_gfx_draw_box(lua_State* L)
{
#ifndef DEDSERV
	LuaContext context(L);
	//BITMAP* b = *static_cast<BITMAP **>(lua_touserdata(L, 1));
	BITMAP* b = ASSERT_OBJECT(BITMAP, 1);
	
	int x1 = lua_tointeger(L, 2);
	int y1 = lua_tointeger(L, 3);
	int x2 = lua_tointeger(L, 4);
	int y2 = lua_tointeger(L, 5);
	int c = lua_tointeger(L, 6);
#ifndef NO_DEPRECATED
	if(lua_gettop(L) >= 8) // Deprecated
	{
		LUA_WLOG_ONCE("The r, g, b version of draw_box is deprecated, replace the r, g, b parameters by color(r, g, b)");
		int g = lua_tointeger(L, 7);
		int b = lua_tointeger(L, 8);
		c = makecol(c, g, b);
	}
#endif
	
	blitter.rectfill(b, x1, y1, x2, y2, c);
#endif
	return 0;
}

#ifndef NO_DEPRECATED
int l_gfx_draw_box_depr(lua_State* L)
{
	LuaContext context(L);
	LUA_WLOG_ONCE("gfx_draw_box is deprecated, use the draw_box method of Bitmap instead");
	return l_gfx_draw_box(L);
}
#endif

//! version 0.9c

/*! Bitmap:line(x1, y1, x2, y2, color)

	Draws a line from (x1, y1) to (x2, y2)
	with the color //color// using the currently selected blender.
*/
int l_gfx_line(lua_State* L)
{
#ifndef DEDSERV
	LuaContext context(L);
	BITMAP* b = ASSERT_OBJECT(BITMAP, 1);
	
	int x1 = lua_tointeger(L, 2);
	int y1 = lua_tointeger(L, 3);
	int x2 = lua_tointeger(L, 4);
	int y2 = lua_tointeger(L, 5);
	int c = lua_tointeger(L, 6);
	
	blitter.line(b, x1, y1, x2, y2, c);
#endif
	return 0;
}

/*! Bitmap:linewu(x1, y1, x2, y2, color)

	Draws a Wu-line from (x1, y1) to (x2, y2)
	with the color //colour// using the currently selected blender.
*/
int l_gfx_linewu(lua_State* L)
{
#ifndef DEDSERV
	LuaContext context(L);
	BITMAP* b = ASSERT_OBJECT(BITMAP, 1);
	
	lua_Number x1 = lua_tonumber(L, 2);
	lua_Number y1 = lua_tonumber(L, 3);
	lua_Number x2 = lua_tonumber(L, 4);
	lua_Number y2 = lua_tonumber(L, 5);
	int c = lua_tointeger(L, 6);
	
	blitter.linewu(b, x1, y1, x2, y2, c);
#endif
	return 0;
}

/*! Bitmap:putpixelwu(x1, y1, color)

	Draws a Wu-pixel at position (x1, y1)
	with the color //color// using the currently selected blender.
*/
int l_gfx_putpixelwu(lua_State* L)
{
#ifndef DEDSERV
	LuaContext context(L);
	BITMAP* b = ASSERT_OBJECT(BITMAP, 1);
	
	lua_Number x = lua_tonumber(L, 2);
	lua_Number y = lua_tonumber(L, 3);
	int c = lua_tointeger(L, 4);
	
	blitter.putpixelwu(b, x, y, c);
#endif
	return 0;
}

/*! Bitmap:putpixel(x1, y1, color)

	Draws a pixel at position (x1, y1)
	with the color //color// using the currently selected blender.
*/
int l_gfx_putpixel(lua_State* L)
{
#ifndef DEDSERV
	LuaContext context(L);
	BITMAP* b = ASSERT_OBJECT(BITMAP, 1);
	
	int x = lua_tointeger(L, 2);
	int y = lua_tointeger(L, 3);
	int cr = lua_tointeger(L, 4);
	int cg = lua_tointeger(L, 5);
	int cb = lua_tointeger(L, 6);
	
	blitter.putpixel(b, x, y, makecol(cr, cg, cb));
#endif
	return 0;
}

/*! Bitmap:hline(x1, y1, x2, color)

	Draws a horizontal line from (x1, y1) to (x2, y1)
	with the color //color// using the currently selected blender.
*/
int l_gfx_hline(lua_State* L)
{
#ifndef DEDSERV
	LuaContext context(L);
	BITMAP* b = ASSERT_OBJECT(BITMAP, 1);
	
	int x1 = lua_tointeger(L, 2);
	int y1 = lua_tointeger(L, 3);
	int x2 = lua_tointeger(L, 4);
	Pixel c = lua_tointeger(L, 5);
	
	blitter.hline(b, x1, y1, x2, c);
#endif
	return 0;
}

/*! color(r, g, b)

	Returns a color
*/
int l_color(lua_State* L)
{
	int r = lua_tointeger(L, 1);
	int g = lua_tointeger(L, 2);
	int b = lua_tointeger(L, 3);
	lua_pushinteger(L, makecol(r, g, b));
	return 1;
}

//! version any

/*! gfx_set_alpha(alpha)

	Activates the alpha blender.
	//alpha// is a value in [0, 255] that specifies the opacity
	of things drawn after this is called.
*/
int l_gfx_set_alpha(lua_State* L)
{
#ifndef DEDSERV
	int alpha = lua_tointeger(L, 1);
	blitter.set(BlitterContext::alpha(), alpha);
#endif
	return 0;
}

//! version 0.9c

/*! gfx_set_alphach(alpha)

	Activates the alphach blender.
	//alpha// is a value in [0, 255] that specifies the opacity
	of things drawn after this is called.
*/
int l_gfx_set_alphach(lua_State* L)
{
#ifndef DEDSERV
	int alpha = lua_tointeger(L, 1);
	blitter.set(BlitterContext::AlphaChannel, alpha);
#endif
	return 0;
}

//! version any

/*! gfx_set_add(alpha)

	Activates the add blender.
	//alpha// is a value in [0, 255] that specifies the scaling factor
	of things drawn after this is called.
*/
int l_gfx_set_add(lua_State* L)
{
#ifndef DEDSERV
	int alpha = lua_tointeger(L, 1);
	blitter.set(BlitterContext::add(), alpha);
#endif
	return 0;
}

/*! gfx_reset_blending()

	Deactivates any blender that was active.
	Everything drawn after this is called will be drawn solid.
*/
int l_gfx_reset_blending(lua_State* L)
{
#ifndef DEDSERV
	blitter.set(BlitterContext::none());
#endif
	return 0;
}

#ifndef DEDSERV

/*! Viewport:bitmap()

	(Known as get_bitmap before 0.9c)
	
	Returns the HUD bitmap of this viewport.
*/
METHOD(Viewport, viewport_getBitmap,
	context.pushFullReference(*p->hud, BITMAPMetaTable);
	return 1;
)

#ifndef NO_DEPRECATED
int l_viewport_getBitmap_depr(lua_State* L)
{
	LuaContext context(L);
	LUA_WLOG_ONCE("get_bitmap is deprecated, use the bitmap method instead");
	return l_viewport_getBitmap(L);
}
#endif

METHOD(Viewport, viewport_getGameBitmap,
	context.pushFullReference(*p->dest, BITMAPMetaTable);
	return 1;
)

//! version 0.9c

/*! Viewport:from_map(x, y)

	Converts the map coordinates (x, y) to
	coordinates relative to the viewport bitmap and returns them.
*/
METHOD(Viewport, viewport_fromMap,
	lua_Integer x = lua_tointeger(context, 2);
	lua_Integer y = lua_tointeger(context, 3);
	
	IVec v(p->convertCoords(IVec(x, y)));
	context.push(v.x);
	context.push(v.y);
	return 2;
)

//! version any

/*! Bitmap:w()

	Returns the width of this bitmap.
*/
METHOD(BITMAP, bitmap_w,
	lua_pushinteger(context, p->w);
	return 1;
)

/*! Bitmap:h()

	Returns the width of this bitmap.
*/
METHOD(BITMAP, bitmap_h,
	lua_pushinteger(context, p->h);
	return 1;
)

#endif


void initGfx()
{
	LuaContext& context = lua;

	context.functions()
#ifndef NO_DEPRECATED
		("gfx_draw_box", l_gfx_draw_box_depr)
#endif
		/*
		("gfx_line", l_gfx_line)
		("gfx_linewu", l_gfx_linewu)
		("gfx_putpixelwu", l_gfx_putpixelwu)
		("gfx_putpixel", l_gfx_putpixel)
		("gfx_hline", l_gfx_hline)*/
		//("gfx_vline", l_gfx_vline)
		("color", l_color)
		("gfx_set_alpha", l_gfx_set_alpha)
		("gfx_set_alphach", l_gfx_set_alphach)
		("gfx_set_add", l_gfx_set_add)
		("gfx_reset_blending", l_gfx_reset_blending)
	;


#ifndef DEDSERV
	// Viewport method and metatable
	
	CLASS(Viewport,
#ifndef NO_DEPRECATED
		("get_bitmap", l_viewport_getBitmap_depr)
#endif
		("bitmap", l_viewport_getBitmap)
		("game_bitmap", l_viewport_getGameBitmap)
		("from_map", l_viewport_fromMap)
	)

	// Bitmap method and metatable
	
	CLASS(BITMAP,
		("w", l_bitmap_w)
		("h", l_bitmap_h)
		("draw_box", l_gfx_draw_box)
		("line", l_gfx_line)
		("linewu", l_gfx_linewu)
		("putpixelwu", l_gfx_putpixelwu)
		("putpixel", l_gfx_putpixel)
		("hline", l_gfx_hline)
	)
	
#endif	
}

}
