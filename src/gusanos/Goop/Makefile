CC= g++
SOURCES= gusanos.cpp keyboard.cpp gconsole.cpp keys.cpp resource_list.cpp sprite.cpp \
	font.cpp vec.cpp level.cpp game.cpp material.cpp viewport.cpp \
	base_object.cpp worm.cpp part_type.cpp particle.cpp \
	base_action.cpp events.cpp game_actions.cpp parser.cpp sound.cpp gfx.cpp \
	sfx.cpp base_animator.cpp animators.cpp distortion.cpp player.cpp player_input.cpp \
	base_player.cpp player_options.cpp weapon.cpp weapon_type.cpp player_ai.cpp ninjarope.cpp \
	server.cpp client.cpp network.cpp base_worm.cpp net_worm.cpp liero.cpp
CFLAGS= -O3 -mwindows -frtti -I../Console/
LIBS= -lldpng -lpng -lz -lalleg -lgcon -lfmod -lzoidcom_mw -L../Console/ \
      -lboost_filesystem
DEFINES= -DWINDOWS
BIN= ../gusanos.exe
OBJECTS= gusanos.o keyboard.o gconsole.o keys.o resource_list.o sprite.o \
	font.o vec.o level.o game.o material.o viewport.o base_object.o \
	worm.o part_type.o particle.o base_action.o events.o \
	game_actions.o parser.o sound.o gfx.o sfx.o base_animator.o animators.o distortion.o \
	player.o player_input.o base_player.o player_options.o weapon.o weapon_type.o player_ai.o \
	ninjarope.o gui.o guilist.o server.o client.o network.o base_worm.o net_worm.o liero.o \
	loaders/gusanos.o loaders/liero.o loaders/lierox.o

$(BIN): makeloader $(OBJECTS)
	$(CC) -o $(BIN) $(OBJECTS) $(CFLAGS) $(LIBS)

makeloader:
	$(MAKE) -C loaders

.cpp.o:
	$(CC) -c $< $(CFLAGS) $(DEFINES)

clean:
	del $(OBJECTS)
