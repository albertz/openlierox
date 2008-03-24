#ifndef LIERO_KEYS_HPP
#define LIERO_KEYS_HPP

#include <SDL/SDL.h>

//extern int SDLToLieroKeys[SDLK_LAST];
//extern int lieroToSDLKeys[177];

void initKeys();

Uint32 SDLToDOSKey(SDL_keysym const& keysym);
Uint32 SDLToDOSKey(SDLKey key);
SDLKey DOSToSDLKey(Uint32 scan);

#endif // LIERO_KEYS_HPP
