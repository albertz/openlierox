# CMake file for GameCompiler

cmake_minimum_required(VERSION 2.4)
IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)
	cmake_policy(SET CMP0005 OLD)
ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)

MESSAGE( "CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}" )
MESSAGE( "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}" )
MESSAGE( "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}" )
MESSAGE( "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}" )

PROJECT(gamecompiler)

# main includes
#ADD_DEFINITIONS("-I-")
INCLUDE_DIRECTORIES(../../optional-includes/generated)
INCLUDE_DIRECTORIES(../../include)

# TODO: don't hardcode path here
INCLUDE_DIRECTORIES(/usr/include/libxml2)

EXEC_PROGRAM(sdl-config ARGS --cflags OUTPUT_VARIABLE SDLCFLAGS)
string(REGEX REPLACE "[\r\n]" " " SDLCFLAGS "${SDLCFLAGS}")
ADD_DEFINITIONS(${SDLCFLAGS})


SET(SRCS src/main.cpp)
SET(OLX_SRCS ../../src/common/ConfigHandler.cpp
../../src/common/CGameScript.cpp
../../src/common/Version.cpp ../../src/common/Debug.cpp
../../src/common/StringUtils.cpp ../../src/common/StringBuf.cpp)

EXEC_PROGRAM(sh ARGS ${CMAKE_CURRENT_SOURCE_DIR}/../../get_version.sh OUTPUT_VARIABLE OLXVER)
string(REGEX REPLACE "[\r\n]" " " OLXVER "${OLXVER}")
MESSAGE( "OLX_VERSION = ${OLXVER}" )
#ADD_DEFINITIONS("-DLX_VERSION=\\\"${OLXVER}\\\"") # This will trigger whole project rebuild
# Escaping backslashes overkill, yay.
EXEC_PROGRAM(sh ARGS -c echo\\ \\\\\#define\\ LX_VERSION\\ \\\\\\"${OLXVER}\\\\\\" > ../../optional-includes/generated/Version_generated.h OUTPUT_VARIABLE DUMMY)



ADD_EXECUTABLE(gamecompiler ${SRCS} ${OLX_SRCS})
EXEC_PROGRAM(mkdir ARGS -p ${CMAKE_CURRENT_SOURCE_DIR}/bin OUTPUT_VARIABLE -)
SET_TARGET_PROPERTIES(gamecompiler PROPERTIES OUTPUT_NAME bin/gamecompiler)


EXEC_PROGRAM(sdl-config ARGS --libs OUTPUT_VARIABLE SDLLIBS)
string(REGEX REPLACE "[\r\n]" " " SDLLIBS "${SDLLIBS}")
TARGET_LINK_LIBRARIES(gamecompiler ${SDLLIBS} SDL SDL_image pthread z xml2)
