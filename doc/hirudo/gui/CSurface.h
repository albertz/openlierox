// example for Hirudo

// Surface class

#include <SDL.h>


class CSurface  {
public:
	// Constructors and destructors
	CSurface();
	CSurface(int x, int y);
	~CSurface();

private:
	SDL_Surface		*tSurface;

public:
	bool operator !()			{ return (tSurface == NULL); }
	operator bool()				{ return (tSurface != NULL); }

public:
	int				getWidth()  { return (tSurface) ? tSurface->w : 0; }
	int				getHeight()  { return (tSurface) ? tSurface->h : 0; }
	byte			getBytesPerPixel()  { return (tSurface) ? (byte)tSurface->format->BytesPerPixel : 0; }
	byte			getOverallAlpha()	{ if(tSurface) { byte alpha = 255; SDL_GetSurfaceAlphaMod(tSurface, alpha); return alpha; } return 255; }
	void			setOverallAlpha(byte _a) { if (tSurface) SDL_SetSurfaceAlphaMod(tSurface, _a); }
	void			RemoveAlpha()	{ if (tSurface) SDL_SetSurfaceAlphaMod(tSurface, 255); }
	void			SetColorKey()	{ if (tSurface) SDL_SetColorKey(tSurface, 1, MakeColor(255, 0, 255)); }
	Uint32			MakeColor(Uint8 r, Uint8 g, Uint8 b) {}

	void			Fill(Color c) {}
	void			FillRect(int x, int y, int w, int h);

	void			DrawImage(CSurface &src, int x, int y)  {}
	void			DrawImageTiled(CSurface &src, int x, int y, int w, int h) {}
};
