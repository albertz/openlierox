PROJECT(openlierox)

# TODO: don't hardcode libxml2 path
INCLUDE_DIRECTORIES(include /usr/include/libxml2)

# STLport stuff begins here
#INCLUDE_DIRECTORIES(/usr/include/stlport)
#ADD_DEFINITIONS(-D_PTHREADS)
#ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64) # hm, don't know, at least it works for me (ppc32/amd32)
# debugging stuff for STLport
#ADD_DEFINITIONS(-D_STLP_DEBUG=1)
#ADD_DEFINITIONS(-D_STLP_DEBUG_LEVEL=_STLP_STANDARD_DBG_LEVEL)
#ADD_DEFINITIONS(-D_STLP_SHRED_BYTE=0xA3)
#ADD_DEFINITIONS(-D_STLP_DEBUG_UNINITIALIZED=1)
#ADD_DEFINITIONS(-D_STLP_DEBUG_ALLOC=1)
# STLport stuff ends here

AUX_SOURCE_DIRECTORY(src/client CLIENT_SRCS)
AUX_SOURCE_DIRECTORY(src/common COMMON_SRCS)
AUX_SOURCE_DIRECTORY(src/server SERVER_SRCS)
SET(ALL_SRCS src/main.cpp ${CLIENT_SRCS} ${COMMON_SRCS} ${SERVER_SRCS})

IF (WIN32)
	# equivalent here?
ELSE (WIN32)
	ADD_DEFINITIONS(-ggdb -Wall)
ENDIF (WIN32)

ADD_DEFINITIONS(-DDEBUG=1 -D_AI_DEBUG)
ADD_DEFINITIONS("`pkg-config --cflags sdl`")

ADD_EXECUTABLE(bin/openlierox ${ALL_SRCS})
TARGET_LINK_LIBRARIES(bin/openlierox SDL pthread SDL_image SDL_mixer NL gd z xml2)

# needed for STLport
#TARGET_LINK_LIBRARIES(bin/openlierox stlportstlg)

