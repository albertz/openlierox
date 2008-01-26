# CMake file for development of OLX
# this file is not intendent (yet) as a system for building final releases
# use compile.sh instead

OPTION(CMAKE_BUILD_TYPE "build type" Debug)
OPTION(DEDICATED_ONLY "dedicated_only - without gfx and sound" No)
OPTION(X11CLIPBOARD "X11 clipboard" Yes)
OPTION(HAWKNL_BUILTIN "HawkNL builtin support" No)
OPTION(STLPORT "STLport support" No)

IF (DEDICATED_ONLY)
	SET(X11CLIPBOARD No)
ENDIF (DEDICATED_ONLY)

MESSAGE( "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
MESSAGE( "DEDICATED_ONLY = ${DEDICATED_ONLY}" )
MESSAGE( "X11CLIPBOARD = ${X11CLIPBOARD}" )
MESSAGE( "HAWKNL_BUILTIN = ${HAWKNL_BUILTIN}" )
MESSAGE( "STLPORT = ${STLPORT}" )

PROJECT(openlierox)

# main includes
INCLUDE_DIRECTORIES(include pstreams)

# TODO: don't hardcode libxml2 path
INCLUDE_DIRECTORIES(/usr/include/libxml2)
IF (HAWKNL_BUILTIN)
	INCLUDE_DIRECTORIES(hawknl/include)
ENDIF (HAWKNL_BUILTIN)

IF (STLPORT)
	INCLUDE_DIRECTORIES(/usr/include/stlport)
	ADD_DEFINITIONS(-D_PTHREADS)
	ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64) # hm, don't know, at least it works for me (ppc32/amd32)
	# debugging stuff for STLport
	ADD_DEFINITIONS(-D_STLP_DEBUG=1)
	ADD_DEFINITIONS(-D_STLP_DEBUG_LEVEL=_STLP_STANDARD_DBG_LEVEL)
	ADD_DEFINITIONS(-D_STLP_SHRED_BYTE=0xA3)
	ADD_DEFINITIONS(-D_STLP_DEBUG_UNINITIALIZED=1)
	ADD_DEFINITIONS(-D_STLP_DEBUG_ALLOC=1)
ENDIF (STLPORT)	

AUX_SOURCE_DIRECTORY(src/client CLIENT_SRCS)
AUX_SOURCE_DIRECTORY(src/common COMMON_SRCS)
AUX_SOURCE_DIRECTORY(src/server SERVER_SRCS)
SET(ALL_SRCS src/main.cpp ${CLIENT_SRCS} ${COMMON_SRCS} ${SERVER_SRCS})

IF (HAWKNL_BUILTIN)
	SET(ALL_SRCS ${ALL_SRCS} "
        hawknl/src/crc.c
        hawknl/src/errorstr.c
        hawknl/src/nl.c
        hawknl/src/sock.c
        hawknl/src/group.c
        hawknl/src/loopback.c
        hawknl/src/err.c
        hawknl/src/thread.c
        hawknl/src/mutex.c
        hawknl/src/condition.c
        hawknl/src/nltime.c")
ENDIF (HAWKNL_BUILTIN)

ADD_DEFINITIONS(-Wall)
IF (WIN32)
	ADD_DEFINITIONS(-g)
ELSE (WIN32)
	ADD_DEFINITIONS(-ggdb)
ENDIF (WIN32)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
	ADD_DEFINITIONS(-DDEBUG=1 -D_AI_DEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)
ADD_DEFINITIONS("-DLX_VERSION=\\\"`cat VERSION`_r`head -n 4 .svn/entries | tail -n 1`\\\"")
IF(X11CLIPBOARD)
	ADD_DEFINITIONS("-DX11CLIPBOARD")
ENDIF(X11CLIPBOARD)
IF(DEDICATED_ONLY)
	ADD_DEFINITIONS("-DDEDICATED_ONLY")
ENDIF(DEDICATED_ONLY)

ADD_DEFINITIONS("`pkg-config --cflags sdl`")
ADD_DEFINITIONS("-pthread")

ADD_EXECUTABLE(openlierox ${ALL_SRCS})
SET_TARGET_PROPERTIES(openlierox PROPERTIES OUTPUT_NAME bin/openlierox)

TARGET_LINK_LIBRARIES(openlierox SDL SDL_image pthread z xml2)

IF (NOT DEDICATED_ONLY)
	TARGET_LINK_LIBRARIES(openlierox SDL_mixer gd)
ENDIF (NOT DEDICATED_ONLY)
IF (NOT HAWKNL_BUILTIN)
	TARGET_LINK_LIBRARIES(openlierox NL)
ENDIF (HAWKNL_BUILTIN)
IF(X11CLIPBOARD)
	TARGET_LINK_LIBRARIES(openlierox X11)
ENDIF(X11CLIPBOARD)
IF (STLPORT)
	TARGET_LINK_LIBRARIES(openlierox stlportstlg)
ENDIF (STLPORT)
