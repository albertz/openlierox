#ifndef SOUNDS_H
#define SOUNDS_H

#include <allegro.h>


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
	class sound* load(const char* sound_name);
	void destroy();
};

extern class sound_list *sounds;

#endif /* SOUNDS_H */
