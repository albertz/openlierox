# CMake file for development of OLX

cmake_minimum_required(VERSION 2.4)
IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)
	cmake_policy(SET CMP0005 OLD)
	cmake_policy(SET CMP0003 OLD)
	include(PCHSupport_26.cmake)
ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)


include(CMakeOlxCommon.cmake)

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
