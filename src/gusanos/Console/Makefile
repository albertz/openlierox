CC= g++
SOURCES= console.cpp bindings.cpp variables.cpp text.cpp consoleitem.cpp \
	command.cpp alias.cpp special_command.cpp
INCLUDES= console.h bindings.h variables.h text.h consoleitem.h command.h \
	alias.h special_command.h
OBJECTS= console.o bindings.o variables.o text.o consoleitem.o command.o \
	alias.o special_command.o

CFLAGS= -O3 -mwindows
LIBRARY= libgcon.a

$(LIBRARY): $(OBJECTS)
	ar rs $@ $^

.cpp.o:
	$(CC) -c $< $(CFLAGS) $(DEFINES)

clean:
	del $(OBJECTS)
