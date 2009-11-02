# CMake common file for OpenLieroX
# sets up the source lists and vars

cmake_minimum_required(VERSION 2.4)
IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)
	if(COMMAND CMAKE_POLICY)
		cmake_policy(VERSION 2.4)
		cmake_policy(SET CMP0005 OLD)
		cmake_policy(SET CMP0003 OLD)
		# Policy CMP0011 was introduced in 2.6.3.
		# We cannot do if(POLCY CMP0011) as a check because 2.4 would fail then.
		if(${CMAKE_MAJOR_VERSION} GREATER 2 OR ${CMAKE_MINOR_VERSION} GREATER 6 OR ${CMAKE_PATCH_VERSION} GREATER 2)
			# We explicitly want to export variables here.
			cmake_policy(SET CMP0011 OLD)
		endif(${CMAKE_MAJOR_VERSION} GREATER 2 OR ${CMAKE_MINOR_VERSION} GREATER 6 OR ${CMAKE_PATCH_VERSION} GREATER 2)
	endif(COMMAND CMAKE_POLICY)
	include(${OLXROOTDIR}/PCHSupport_26.cmake)
ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)


SET(SYSTEM_DATA_DIR "/usr/share/games" CACHE STRING "system data dir")
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
OPTION(BREAKPAD "Google Breakpad support" Yes)

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


MESSAGE( "SYSTEM_DATA_DIR = ${SYSTEM_DATA_DIR}" )
MESSAGE( "DEBUG = ${DEBUG}" )
MESSAGE( "DEDICATED_ONLY = ${DEDICATED_ONLY}" )
MESSAGE( "G15 = ${G15}" )
MESSAGE( "X11 = ${X11}" )
MESSAGE( "HAWKNL_BUILTIN = ${HAWKNL_BUILTIN}" )
MESSAGE( "LIBZIP_BUILTIN = ${LIBZIP_BUILTIN}" )
MESSAGE( "STLPORT = ${STLPORT}" )
MESSAGE( "GCOREDUMPER = ${GCOREDUMPER}" )
MESSAGE( "BREAKPAD = ${BREAKPAD}" )
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
INCLUDE_DIRECTORIES(${OLXROOTDIR}/optional-includes/generated)
INCLUDE_DIRECTORIES(${OLXROOTDIR}/include)
INCLUDE_DIRECTORIES(${OLXROOTDIR}/libs/pstreams)

IF(ADVASSERT)
	INCLUDE_DIRECTORIES(${OLXROOTDIR}/optional-includes/advanced-assert)
ENDIF(ADVASSERT)

# TODO: don't hardcode path here
IF(NOT WIN32)
	INCLUDE_DIRECTORIES(/usr/include/libxml2)
	INCLUDE_DIRECTORIES(/usr/local/include/libxml2)
ENDIF(NOT WIN32)

AUX_SOURCE_DIRECTORY(${OLXROOTDIR}/src/client CLIENT_SRCS)
AUX_SOURCE_DIRECTORY(${OLXROOTDIR}/src/client/DeprecatedGUI CLIENT_SRCS_GUI_OLD)
AUX_SOURCE_DIRECTORY(${OLXROOTDIR}/src/client/SkinnedGUI CLIENT_SRCS_GUI_NEW)
AUX_SOURCE_DIRECTORY(${OLXROOTDIR}/src/common COMMON_SRCS)
AUX_SOURCE_DIRECTORY(${OLXROOTDIR}/src/server SERVER_SRCS)

SET(ALL_SRCS ${CLIENT_SRCS} ${CLIENT_SRCS_GUI_OLD} ${CLIENT_SRCS_GUI_NEW} ${COMMON_SRCS} ${SERVER_SRCS})

IF(APPLE)
	SET(ALL_SRCS ${OLXROOTDIR}/src/MacMain.m ${ALL_SRCS})
ENDIF(APPLE)

SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/ExtractInfo.cpp ${ALL_SRCS})
IF (BREAKPAD)
	INCLUDE_DIRECTORIES(${OLXROOTDIR}/src/breakpad/external/src)
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/DumpSyms.cpp ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/BreakPad.cpp ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/on_demand_symbol_supplier.cpp ${ALL_SRCS})

	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/client/minidump_file_writer.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/client/linux/handler/exception_handler.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/client/linux/handler/linux_thread.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/client/linux/minidump_writer/linux_dumper.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/client/linux/minidump_writer/minidump_writer.cc ${ALL_SRCS})

	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/convert_UTF.c ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/md5.c ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/string_conversion.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/linux/file_id.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/linux/guid_creator.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/linux/dump_symbols.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/linux/module.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/linux/stabs_reader.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/dwarf/bytereader.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/dwarf/dwarf2diehandler.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/dwarf/dwarf2reader.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/common/linux/dump_dwarf.cc ${ALL_SRCS})

	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/basic_code_modules.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/basic_source_line_resolver.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/call_stack.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/logging.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/minidump.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/minidump_processor.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/pathname_stripper.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/process_state.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/simple_symbol_supplier.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/stackwalker_amd64.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/stackwalker_ppc.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/stackwalker_sparc.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/stackwalker_x86.cc ${ALL_SRCS})
	SET(ALL_SRCS ${OLXROOTDIR}/src/breakpad/external/src/processor/stackwalker.cc ${ALL_SRCS})
ELSE (BREAKPAD)
	ADD_DEFINITIONS(-DNBREAKPAD)
ENDIF (BREAKPAD)

IF (GCOREDUMPER)
	INCLUDE_DIRECTORIES(${OLXROOTDIR}/libs/coredumper/src)
	ADD_DEFINITIONS(-DGCOREDUMPER)
	AUX_SOURCE_DIRECTORY(${OLXROOTDIR}/libs/coredumper/src COREDUMPER_SRCS)
	SET(ALL_SRCS ${ALL_SRCS} ${COREDUMPER_SRCS})
ENDIF (GCOREDUMPER)

IF (HAWKNL_BUILTIN)
	INCLUDE_DIRECTORIES(${OLXROOTDIR}/libs/hawknl/include)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/crc.c)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/errorstr.c)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/nl.c)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/sock.c)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/group.c)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/loopback.c)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/err.c)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/thread.c)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/mutex.c)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/condition.c)
	SET(ALL_SRCS ${ALL_SRCS} ${OLXROOTDIR}/libs/hawknl/src/nltime.c)
ELSE (HAWKNL_BUILTIN)
	INCLUDE_DIRECTORIES(/usr/include/hawknl)
ENDIF (HAWKNL_BUILTIN)

IF (LIBZIP_BUILTIN)
	INCLUDE_DIRECTORIES(${OLXROOTDIR}/libs/libzip)
	AUX_SOURCE_DIRECTORY(${OLXROOTDIR}/libs/libzip LIBZIP_SRCS)
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
	ADD_DEFINITIONS(-include ${OLXROOTDIR}/optional-includes/memdebug/memstats.h)
ENDIF(MEMSTATS)



IF(WIN32)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -DHAVE_BOOST -DZLIB_WIN32_NODLL)
	SET(OPTIMIZE_COMPILER_FLAG /Ox /Ob2 /Oi /Ot /GL)
	IF(DEBUG)
		ADD_DEFINITIONS(-DUSE_DEFAULT_MSC_DELEAKER)
	ELSE(DEBUG)
		ADD_DEFINITIONS(${OPTIMIZE_COMPILER_FLAG})
	ENDIF(DEBUG)
	INCLUDE_DIRECTORIES(${OLXROOTDIR}/libs/hawknl/include
				${OLXROOTDIR}/libs/hawknl/src
				${OLXROOTDIR}/libs/libzip
				${OLXROOTDIR}/libs/boost_process)
ELSE(WIN32)
	ADD_DEFINITIONS(-Wall)

	EXEC_PROGRAM(sh ARGS ${CMAKE_CURRENT_SOURCE_DIR}/get_version.sh OUTPUT_VARIABLE OLXVER)
	string(REGEX REPLACE "[\r\n]" " " OLXVER "${OLXVER}")
	MESSAGE( "OLX_VERSION = ${OLXVER}" )

	ADD_DEFINITIONS("-pthread")

	SET(OPTIMIZE_COMPILER_FLAG -O3)
ENDIF(WIN32)

IF(OPTIM_PROJECTILES)
	#Always optimize these files - they make debug build unusable otherwise
	SET_SOURCE_FILES_PROPERTIES(	${OLXROOTDIR}/src/common/PhysicsLX56_Projectiles.cpp 
						PROPERTIES COMPILE_FLAGS ${OPTIMIZE_COMPILER_FLAG})
ENDIF(OPTIM_PROJECTILES)

if(WIN32)
elseif(APPLE)
	INCLUDE_DIRECTORIES(/Library/Frameworks/SDL.framework/Headers)
	INCLUDE_DIRECTORIES(/Library/Frameworks/SDL_image.framework/Headers)
	INCLUDE_DIRECTORIES(/Library/Frameworks/SDL_mixer.framework/Headers)
	INCLUDE_DIRECTORIES(/Library/Frameworks/UnixImageIO.framework/Headers)
	INCLUDE_DIRECTORIES(/Library/Frameworks/GD.framework/Headers)
else()
	EXEC_PROGRAM(sdl-config ARGS --cflags OUTPUT_VARIABLE SDLCFLAGS)
	string(REGEX REPLACE "[\r\n]" " " SDLCFLAGS "${SDLCFLAGS}")
	ADD_DEFINITIONS(${SDLCFLAGS})
endif(WIN32)


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

SET(LIBS ${PYTHONLIBS})
SET(LIBS ${LIBS} curl)

if(APPLE)
	FIND_PACKAGE(SDL REQUIRED)
	FIND_PACKAGE(SDL_image REQUIRED)
	SET(LIBS ${LIBS} ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})
	SET(LIBS ${LIBS} "-framework Cocoa" "-framework Carbon")
else(APPLE)
	SET(LIBS ${LIBS} SDL SDL_image)
endif(APPLE)

if(WIN32)
	SET(LIBS ${LIBS} SDL_mixer wsock32 wininet dbghelp
				"${OLXROOTDIR}/build/msvc/libs/SDLmain.lib"
				"${OLXROOTDIR}/build/msvc/libs/libxml2.lib"
				"${OLXROOTDIR}/build/msvc/libs/NLstatic.lib"
				"${OLXROOTDIR}/build/msvc/libs/libzip.lib"
				"${OLXROOTDIR}/build/msvc/libs/zlib.lib"
				"${OLXROOTDIR}/build/msvc/libs/bgd.lib")
elseif(APPLE)
	link_directories(/Library/Frameworks/SDL_mixer.framework)
	link_directories(/Library/Frameworks/SDL_image.framework)
	link_directories(/Library/Frameworks/UnixImageIO.framework)
else()
	EXEC_PROGRAM(sdl-config ARGS --libs OUTPUT_VARIABLE SDLLIBS)
	STRING(REGEX REPLACE "[\r\n]" " " SDLLIBS "${SDLLIBS}")
endif(WIN32)

if(UNIX)
	IF (NOT HAWKNL_BUILTIN)
		SET(LIBS ${LIBS} NL)
	ENDIF (NOT HAWKNL_BUILTIN)
	IF (NOT LIBZIP_BUILTIN)
		SET(LIBS ${LIBS} zip)
	ENDIF (NOT LIBZIP_BUILTIN)
	IF(X11)
		SET(LIBS ${LIBS} X11)
	ENDIF(X11)
	IF (STLPORT)
		SET(LIBS ${LIBS} stlportstlg)
	ENDIF (STLPORT)
	IF (G15)
		SET(LIBS ${LIBS} g15daemon_client g15render)
	ENDIF (G15)

	SET(LIBS ${LIBS} ${SDLLIBS} pthread xml2 z)
endif(UNIX)

IF (NOT DEDICATED_ONLY)
	if(APPLE)
		FIND_PACKAGE(SDL_mixer REQUIRED)
		SET(LIBS ${LIBS} ${SDLMIXER_LIBRARY})
		SET(LIBS ${LIBS} "-framework UnixImageIO" "-framework GD")
	elseif(UNIX)
		SET(LIBS ${LIBS} SDL_mixer gd)
	endif(APPLE)
ENDIF (NOT DEDICATED_ONLY)


ADD_DEFINITIONS('-D SYSTEM_DATA_DIR=\"${SYSTEM_DATA_DIR}\"')
