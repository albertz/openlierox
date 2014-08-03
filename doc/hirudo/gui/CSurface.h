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
	byte			getOverallAlpha()	{ return (tSurface) ? (byte)tSurface->format->alpha : 0; }
	void			setOverallAlpha(byte _a) { if (tSurface) SDL_SetAlpha(tSurface, SDL_SRCALPHA, _a); }
	void			RemoveAlpha()	{ if (tSurface) SDL_SetAlpha(tSurface, 0, 0); }
	void			SetColorKey()	{ if (tSurface) SDL_SetColorKey(tSurface, 1, MakeColor(255, 0, 255)); }
	Uint32			MakeColor(Uint8 r, Uint8 g, Uint8 b) {}

	void			Fill(Color c) {}
	void			FillRect(int x, int y, int w, int h);

	void			DrawImage(CSurface &src, int x, int y)  {}
	void			DrawImageTiled(CSurface &src, int x, int y, int w, int h) {}
};
