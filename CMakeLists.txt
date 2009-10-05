# CMake file for development of OLX
# this file is not intendent (yet) as a system for building final releases
# use compile.sh instead

cmake_minimum_required(VERSION 2.4)
IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)
	cmake_policy(SET CMP0005 OLD)
	cmake_policy(SET CMP0003 OLD)
	include(PCHSupport_26.cmake)
ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)



OPTION(DEBUG "enable debug build" Yes)
OPTION(DEDICATED_ONLY "dedicated_only - without gfx and sound" No)
OPTION(G15 "G15 support" No)
OPTION(X11 "X11 clipboard / notify" Yes)
OPTION(HAWKNL_BUILTIN "HawkNL builtin support" Yes)
OPTION(LIBZIP_BUILTIN "LibZIP builtin support" No)
OPTION(STLPORT "STLport support" No)
OPTION(GCOREDUMPER "Google Coredumper support" No)
OPTION(PCH "Precompiled header (CMake 2.6 only)" No)
OPTION(ADVASSERT "Advanced assert" No)
OPTION(PYTHON_DED_EMBEDDED "Python embedded in dedicated server"  No)
OPTION(OPTIM_PROJECTILES "Enable optimisations for projectiles" Yes)
OPTION(MEMSTATS "Enable memory statistics and debugging" No)

IF (DEBUG)
	SET(CMAKE_BUILD_TYPE Debug)
ELSE (DEBUG)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF (DEBUG)


IF (DEDICATED_ONLY)
	SET(X11 No)
	SET(WITH_G15 No)
ENDIF (DEDICATED_ONLY)

# Platform specific things can be put here
# Compilers and other specific variables can be found here:
# http://www.cmake.org/Wiki/CMake_Useful_Variables
IF(UNIX)
	IF(APPLE)
	ELSE(APPLE)
	ENDIF(APPLE)

	IF (CYGWIN)
		SET(WITH_G15 OFF) # Linux library as of now
	ELSE (CYGWIN)
	ENDIF (CYGWIN)
ELSE(UNIX)
	IF(WIN32)
		SET(G15 OFF)
		SET(HAWKNL_BUILTIN OFF) # We already have prebuilt HawkNL library
		SET(X11 OFF)
	ELSE(WIN32)
	ENDIF(WIN32)
ENDIF(UNIX)


MESSAGE( "DEBUG = ${DEBUG}" )
MESSAGE( "DEDICATED_ONLY = ${DEDICATED_ONLY}" )
MESSAGE( "G15 = ${G15}" )
MESSAGE( "X11 = ${X11}" )
MESSAGE( "HAWKNL_BUILTIN = ${HAWKNL_BUILTIN}" )
MESSAGE( "LIBZIP_BUILTIN = ${LIBZIP_BUILTIN}" )
MESSAGE( "STLPORT = ${STLPORT}" )
MESSAGE( "GCOREDUMPER = ${GCOREDUMPER}" )
MESSAGE( "CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}" )
MESSAGE( "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}" )
MESSAGE( "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}" )
MESSAGE( "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}" )
# commented out because only devs need these options anyway
#MESSAGE( "PCH = ${PCH} (Precompiled header, CMake 2.6 only)" )
#MESSAGE( "ADVASSERT = ${ADVASSERT}" )
#MESSAGE( "PYTHON_DED_EMBEDDED = ${PYTHON_DED_EMBEDDED}" )

PROJECT(openlierox)

EXEC_PROGRAM(mkdir ARGS -p bin OUTPUT_VARIABLE DUMMY)

# main includes
INCLUDE_DIRECTORIES(optional-includes/generated)
INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES(libs/pstreams)

IF(ADVASSERT)
	INCLUDE_DIRECTORIES(optional-includes/advanced-assert)
ENDIF(ADVASSERT)

# TODO: don't hardcode path here
IF(NOT WIN32)
	INCLUDE_DIRECTORIES(/usr/include/libxml2)
	INCLUDE_DIRECTORIES(/usr/local/include/libxml2)
ENDIF(NOT WIN32)

AUX_SOURCE_DIRECTORY(src/client CLIENT_SRCS)
AUX_SOURCE_DIRECTORY(src/client/DeprecatedGUI CLIENT_SRCS_GUI_OLD)
AUX_SOURCE_DIRECTORY(src/client/SkinnedGUI CLIENT_SRCS_GUI_NEW)
AUX_SOURCE_DIRECTORY(src/common COMMON_SRCS)
AUX_SOURCE_DIRECTORY(src/server SERVER_SRCS)

SET(ALL_SRCS src/main.cpp ${CLIENT_SRCS} ${CLIENT_SRCS_GUI_OLD} ${CLIENT_SRCS_GUI_NEW} ${COMMON_SRCS} ${SERVER_SRCS})

IF (GCOREDUMPER)
	INCLUDE_DIRECTORIES(libs/coredumper/src)
	ADD_DEFINITIONS(-DGCOREDUMPER)
	AUX_SOURCE_DIRECTORY(libs/coredumper/src COREDUMPER_SRCS)
	SET(ALL_SRCS ${ALL_SRCS} ${COREDUMPER_SRCS})
ENDIF (GCOREDUMPER)

IF (HAWKNL_BUILTIN)
	INCLUDE_DIRECTORIES(libs/hawknl/include)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/crc.c)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/errorstr.c)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/nl.c)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/sock.c)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/group.c)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/loopback.c)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/err.c)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/thread.c)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/mutex.c)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/condition.c)
	SET(ALL_SRCS ${ALL_SRCS} libs/hawknl/src/nltime.c)
ELSE (HAWKNL_BUILTIN)
	INCLUDE_DIRECTORIES(/usr/include/hawknl)
ENDIF (HAWKNL_BUILTIN)

IF (LIBZIP_BUILTIN)
	INCLUDE_DIRECTORIES(libs/libzip)
	AUX_SOURCE_DIRECTORY(libs/libzip LIBZIP_SRCS)
	SET(ALL_SRCS ${ALL_SRCS} ${LIBZIP_SRCS})
ENDIF (LIBZIP_BUILTIN)

IF (STLPORT)
	INCLUDE_DIRECTORIES(/usr/include/stlport)
	ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64) # hm, don't know, at least it works for me (ppc32/amd32)
# debugging stuff for STLport
	ADD_DEFINITIONS(-D_STLP_DEBUG=1)
	ADD_DEFINITIONS(-D_STLP_DEBUG_LEVEL=_STLP_STANDARD_DBG_LEVEL)
	ADD_DEFINITIONS(-D_STLP_SHRED_BYTE=0xA3)
	ADD_DEFINITIONS(-D_STLP_DEBUG_UNINITIALIZED=1)
	ADD_DEFINITIONS(-D_STLP_DEBUG_ALLOC=1)
ENDIF (STLPORT)


IF(DEBUG)
	ADD_DEFINITIONS(-DDEBUG=1 -D_AI_DEBUG)
ENDIF(DEBUG)

IF(MEMSTATS)
	ADD_DEFINITIONS(-include optional-includes/memdebug/memstats.h)
ENDIF(MEMSTATS)



IF(WIN32)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -DHAVE_BOOST -DZLIB_WIN32_NODLL)
	SET(OPTIMIZE_COMPILER_FLAG /Ox /Ob2 /Oi /Ot /GL)
	IF(DEBUG)
		ADD_DEFINITIONS(-DUSE_DEFAULT_MSC_DELEAKER)
	ELSE(DEBUG)
		ADD_DEFINITIONS(${OPTIMIZE_COMPILER_FLAG})
	ENDIF(DEBUG)
	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libs/hawknl/include
				${CMAKE_SOURCE_DIR}/libs/hawknl/src
				${CMAKE_SOURCE_DIR}/libs/libzip
				${CMAKE_SOURCE_DIR}/libs/boost_process)
ELSE(WIN32)
	ADD_DEFINITIONS(-Wall)

	EXEC_PROGRAM(sh ARGS ${CMAKE_CURRENT_SOURCE_DIR}/get_version.sh OUTPUT_VARIABLE OLXVER)
	string(REGEX REPLACE "[\r\n]" " " OLXVER "${OLXVER}")
	MESSAGE( "OLX_VERSION = ${OLXVER}" )

	EXEC_PROGRAM(sdl-config ARGS --cflags OUTPUT_VARIABLE SDLCFLAGS)
	string(REGEX REPLACE "[\r\n]" " " SDLCFLAGS "${SDLCFLAGS}")
	ADD_DEFINITIONS(${SDLCFLAGS})

	ADD_DEFINITIONS("-pthread")

	SET(OPTIMIZE_COMPILER_FLAG -O3)
ENDIF(WIN32)

IF(OPTIM_PROJECTILES)
	#Always optimize these files - they make debug build unusable otherwise
	SET_SOURCE_FILES_PROPERTIES(	src/common/PhysicsLX56_Projectiles.cpp 
						PROPERTIES COMPILE_FLAGS ${OPTIMIZE_COMPILER_FLAG})
ENDIF(OPTIM_PROJECTILES)


IF(X11)
	ADD_DEFINITIONS("-DX11")
ENDIF(X11)
IF(DEDICATED_ONLY)
	ADD_DEFINITIONS("-DDEDICATED_ONLY")
ENDIF(DEDICATED_ONLY)


IF(G15)
	ADD_DEFINITIONS("-DWITH_G15")
ENDIF(G15)

SET(PYTHONLIBS "")
IF(PYTHON_DED_EMBEDDED)
	ADD_DEFINITIONS("-DPYTHON_DED_EMBEDDED")
	EXEC_PROGRAM(python2.5-config ARGS --includes OUTPUT_VARIABLE PYTHONFLAGS)
	ADD_DEFINITIONS(${PYTHONFLAGS})
	EXEC_PROGRAM(python2.5-config ARGS --libs OUTPUT_VARIABLE PYTHONLIBS)
ENDIF(PYTHON_DED_EMBEDDED)

ADD_EXECUTABLE(openlierox ${ALL_SRCS})

IF(PCH)
	EXEC_PROGRAM(./update_precompiled_header.sh OUTPUT_VARIABLE NULL)
	ADD_PRECOMPILED_HEADER(openlierox include/PrecompiledHeader.hpp 1)
ENDIF(PCH)


TARGET_LINK_LIBRARIES(openlierox SDL SDL_image ${PYTHONLIBS})

IF(WIN32)
	TARGET_LINK_LIBRARIES(openlierox SDL_mixer wsock32 wininet dbghelp
				"${CMAKE_SOURCE_DIR}/build/msvc/libs/SDLmain.lib"
				"${CMAKE_SOURCE_DIR}/build/msvc/libs/libxml2.lib"
				"${CMAKE_SOURCE_DIR}/build/msvc/libs/NLstatic.lib"
				"${CMAKE_SOURCE_DIR}/build/msvc/libs/libzip.lib"
				"${CMAKE_SOURCE_DIR}/build/msvc/libs/zlib.lib"
				"${CMAKE_SOURCE_DIR}/build/msvc/libs/bgd.lib")

	SET_TARGET_PROPERTIES(openlierox PROPERTIES OUTPUT_NAME "../../../bin/OpenLieroX")
	
	# Add debug info to release build in linker flags
	SET_TARGET_PROPERTIES(openlierox PROPERTIES LINK_FLAGS "${LINK_FLAGS} 
		/LTCG
		/DEBUG
		/MAP:\"${CMAKE_SOURCE_DIR}/bin/OpenLieroX.map\"
		/MAPINFO:EXPORTS
		/NODEFAULTLIB:\"LIBC\"
		/INCREMENTAL:NO")
ELSE(WIN32)
	EXEC_PROGRAM(sdl-config ARGS --libs OUTPUT_VARIABLE SDLLIBS)
	STRING(REGEX REPLACE "[\r\n]" " " SDLLIBS "${SDLLIBS}")

	TARGET_LINK_LIBRARIES(openlierox ${SDLLIBS} pthread xml2 z)

	SET_TARGET_PROPERTIES(openlierox PROPERTIES OUTPUT_NAME bin/openlierox)

	IF (NOT DEDICATED_ONLY)
		TARGET_LINK_LIBRARIES(openlierox SDL_mixer gd)
	ENDIF (NOT DEDICATED_ONLY)
	IF (NOT HAWKNL_BUILTIN)
		TARGET_LINK_LIBRARIES(openlierox NL)
	ENDIF (NOT HAWKNL_BUILTIN)
	IF (NOT LIBZIP_BUILTIN)
		TARGET_LINK_LIBRARIES(openlierox zip)
	ENDIF (NOT LIBZIP_BUILTIN)
	IF(X11)
		TARGET_LINK_LIBRARIES(openlierox X11)
	ENDIF(X11)
	IF (STLPORT)
		TARGET_LINK_LIBRARIES(openlierox stlportstlg)
	ENDIF (STLPORT)
	IF (G15)
		TARGET_LINK_LIBRARIES(openlierox g15daemon_client g15render)
	ENDIF (G15)

ENDIF(WIN32)
