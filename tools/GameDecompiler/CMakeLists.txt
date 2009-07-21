# CMake file for GameCompiler

cmake_minimum_required(VERSION 2.4)
IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)
	cmake_policy(SET CMP0005 OLD)
ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)

MESSAGE( "CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}" )
MESSAGE( "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}" )
MESSAGE( "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}" )
MESSAGE( "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}" )

PROJECT(gamedecompiler)

# main includes
#ADD_DEFINITIONS("-I-")
INCLUDE_DIRECTORIES(../../optional-includes/generated)
INCLUDE_DIRECTORIES(../../include)
INCLUDE_DIRECTORIES(../../libs/pstreams)
INCLUDE_DIRECTORIES(../../libs/hawknl/include)

# TODO: don't hardcode path here
INCLUDE_DIRECTORIES(/usr/include/libxml2)

EXEC_PROGRAM(sdl-config ARGS --cflags OUTPUT_VARIABLE SDLCFLAGS)
string(REGEX REPLACE "[\r\n]" " " SDLCFLAGS "${SDLCFLAGS}")
ADD_DEFINITIONS(${SDLCFLAGS})


AUX_SOURCE_DIRECTORY(../../src/client CLIENT_SRCS)
AUX_SOURCE_DIRECTORY(../../src/client/DeprecatedGUI CLIENT_SRCS_GUI_OLD)
AUX_SOURCE_DIRECTORY(../../src/client/SkinnedGUI CLIENT_SRCS_GUI_NEW)
AUX_SOURCE_DIRECTORY(../../src/common COMMON_SRCS)
AUX_SOURCE_DIRECTORY(../../src/server SERVER_SRCS)

SET(SRCS src/main.cpp ${CLIENT_SRCS} ${CLIENT_SRCS_GUI_OLD} ${CLIENT_SRCS_GUI_NEW} ${COMMON_SRCS} ${SERVER_SRCS})

SET(SRCS ${SRCS} 
../../libs/hawknl/src/crc.c 
../../libs/hawknl/src/errorstr.c
../../libs/hawknl/src/nl.c
../../libs/hawknl/src/sock.c
../../libs/hawknl/src/group.c
../../libs/hawknl/src/loopback.c
../../libs/hawknl/src/err.c
../../libs/hawknl/src/thread.c
../../libs/hawknl/src/mutex.c
../../libs/hawknl/src/condition.c
../../libs/hawknl/src/nltime.c)

INCLUDE_DIRECTORIES(../../libs/libzip)
AUX_SOURCE_DIRECTORY(../../libs/libzip LIBZIP_SRCS)
SET(SRCS ${SRCS} ${LIBZIP_SRCS})


EXEC_PROGRAM(sh ARGS ${CMAKE_CURRENT_SOURCE_DIR}/../../get_version.sh OUTPUT_VARIABLE OLXVER)
string(REGEX REPLACE "[\r\n]" " " OLXVER "${OLXVER}")
MESSAGE( "OLX_VERSION = ${OLXVER}" )
#ADD_DEFINITIONS("-DLX_VERSION=\\\"${OLXVER}\\\"") # This will trigger whole project rebuild
# Escaping backslashes overkill, yay.
EXEC_PROGRAM(sh ARGS -c echo\\ \\\\\#define\\ LX_VERSION\\ \\\\\\"${OLXVER}\\\\\\" > ../../optional-includes/generated/Version_generated.h OUTPUT_VARIABLE DUMMY)


ADD_EXECUTABLE(gamedecompiler ${SRCS})
EXEC_PROGRAM(mkdir ARGS -p ${CMAKE_CURRENT_SOURCE_DIR}/bin OUTPUT_VARIABLE -)
SET_TARGET_PROPERTIES(gamedecompiler PROPERTIES OUTPUT_NAME bin/gamedecompiler)


EXEC_PROGRAM(sdl-config ARGS --libs OUTPUT_VARIABLE SDLLIBS)
string(REGEX REPLACE "[\r\n]" " " SDLLIBS "${SDLLIBS}")
TARGET_LINK_LIBRARIES(gamedecompiler ${SDLLIBS} SDL SDL_image pthread z xml2 SDL_mixer gd)
