#ifndef SOUNDS_H
#define SOUNDS_H

#include <ctype.h>
#include <allegro.h>
#ifdef WINDOWS
#include "winalleg.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "engine.h"

class sound
{
  public:
	SAMPLE *snd ;
	char name[512];
	class sound* next;
	class sound* prev;
  ~sound();
};

class sound_list
{
  public:
	class sound *start, *end;
	sound_list();
  ~sound_list();
	class sound* load(char* sound_name);
	void destroy();
};

extern class sound_list *sounds;

#endif /* SOUNDS_H */
