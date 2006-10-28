/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Graphics header file
// Created 30/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
//#include "corona.h"
//#include "libpng/png.h"


gfxgui_t	gfxGUI;
gfxgame_t	gfxGame;

///////////////////
// Load the graphics
int LoadGraphics(void)
{
	int i;
	LOAD_IMAGE_BPP(gfxGUI.bmpMouse[0], "data/frontend/mouse.png");
	LOAD_IMAGE_BPP(gfxGUI.bmpMouse[1], "data/frontend/mouse_hand.png");
	LOAD_IMAGE_BPP(gfxGUI.bmpMouse[2], "data/frontend/mouse_text.png");

	LOAD_IMAGE_BPP(gfxGUI.bmpScrollbar,"data/frontend/scrollbar.png");
	LOAD_IMAGE_BPP(gfxGUI.bmpSliderBut,"data/frontend/sliderbut.png");

	LOAD_IMAGE_BPP(gfxGame.bmpCrosshair,"data/gfx/crosshair.bmp");
	LOAD_IMAGE_BPP(gfxGame.bmpMuzzle,"data/gfx/muzzle.bmp");
	LOAD_IMAGE_BPP(gfxGame.bmpExplosion,"data/gfx/explosion.png");
	LOAD_IMAGE_BPP(gfxGame.bmpSmoke,"data/gfx/smoke.png");
	LOAD_IMAGE_BPP(gfxGame.bmpChemSmoke,"data/gfx/chemsmoke.png");
	LOAD_IMAGE_BPP(gfxGame.bmpSpawn,"data/gfx/spawn.png");
	LOAD_IMAGE_BPP(gfxGame.bmpHook,"data/gfx/hook.bmp");
	LOAD_IMAGE(gfxGame.bmpGameover,"data/gfx/gameover.png");
	LOAD_IMAGE_BPP(gfxGame.bmpInGame,"data/gfx/ingame.png");
	LOAD_IMAGE(gfxGame.bmpScoreboard,"data/gfx/scoreboard.png");
    LOAD_IMAGE_BPP(gfxGame.bmpViewportMgr,"data/gfx/viewportmgr.png");
	LOAD_IMAGE_BPP(gfxGame.bmpSparkle, "data/gfx/sparkle.png");
	LOAD_IMAGE(gfxGame.bmpInfinite,"data/gfx/infinite.png");
	LOAD_IMAGE_BPP(gfxGame.bmpLag, "data/gfx/lag.png");

	LOAD_IMAGE_BPP(gfxGame.bmpBonus, "data/gfx/bonus.png");
	LOAD_IMAGE_BPP(gfxGame.bmpHealth, "data/gfx/health.png");


	if(!tLX->cFont.Load("data/gfx/font.png",true,15))
		return false;
	if(!tLX->cOutlineFont.Load("data/gfx/out_font.png",true,15))
		return false;
	if(!tLX->cOutlineFontGrey.Load("data/gfx/out_fontgrey.png",false,15))
		return false;

	tLX->cOutlineFont.SetOutline(true);
	//tLX->cOutlineFontGrey.SetOutline(true);

	// Set the colour keys
	Uint32 pink = MakeColour(255,0,255);
	for(i=0;i<3;i++)
		SDL_SetColorKey(gfxGUI.bmpMouse[i], SDL_SRCCOLORKEY, pink);

	SDL_SetColorKey(gfxGame.bmpCrosshair, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpMuzzle, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpExplosion, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpSmoke, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpChemSmoke, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpSpawn, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpHook, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpBonus, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpHealth, SDL_SRCCOLORKEY, pink);
	SDL_SetColorKey(gfxGame.bmpSparkle, SDL_SRCCOLORKEY, pink);
    SDL_SetColorKey(gfxGame.bmpViewportMgr, SDL_SRCCOLORKEY, pink);


	return true;
}


/*int write_png(char *file_name, png_bytep *rows, int w, int h, int colortype,int bitdepth) {
  png_structp png_ptr;
  png_infop info_ptr;
  FILE *fp = fopen_i(file_name, "wb");
  char *doing = "open for writing";

  if (!(fp = fopen_i(file_name, "wb"))) goto fail;
  doing = "create png write struct";
  if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) goto fail;
  doing = "create png info struct";
  if (!(info_ptr = png_create_info_struct(png_ptr))) goto fail;
  if (setjmp(png_jmpbuf(png_ptr))) goto fail;
  doing = "init IO";
  png_init_io(png_ptr, fp);
  doing = "write header";
  png_set_IHDR(png_ptr, info_ptr, w, h, bitdepth, colortype, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  /*doing = "write info";
  png_write_info(png_ptr, info_ptr);
  doing = "write image";
  png_write_image(png_ptr, rows);
  doing = "write end";
  png_write_end(png_ptr, NULL);
  return 0;

 fail:
  printf("Write_png: could not %s\n", doing);
  return -1;
}*/

///////////////////
// Saves the surface to a PNG file, returns true if success
bool SavePng(char *FileName, SDL_Surface *img)
{
/*	corona::Image *dest_img;

	//dest_img = corona::CreateImage(img->w,img->h,corona::PF_R8G8B8);
	dest_img = corona::CreateImage(img->w,img->h,corona::PF_I8,256,corona::PF_R8G8B8);

	if (!dest_img)
		return false;

	// Copy the pixels
	memcpy( dest_img->getPixels(),img->pixels, dest_img->getWidth()*dest_img->getHeight());

	return corona::SaveImage(FileName,corona::FF_PNG,dest_img);*/

/*	unsigned char** img_rows;

	img_rows = (unsigned char**)malloc(sizeof(unsigned char*) * img->h * 24);

	for(int i = 0; i < img->h; i++) {
      img_rows[i] = ((unsigned char*)img->pixels) + i * img->pitch;
    }

	return write_png(FileName, img_rows, img->w, img->h, PNG_COLOR_TYPE_RGB, 8) == 0;*/
	return 0;
}


///////////////////
// Shutdown the graphics
void ShutdownGraphics(void)
{
	tLX->cFont.Shutdown();
	tLX->cOutlineFont.Shutdown();
	tLX->cOutlineFontGrey.Shutdown();
}
