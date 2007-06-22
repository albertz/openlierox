PROJECT(openlierox)

# TODO: don't hardcode libxml2 path
INCLUDE_DIRECTORIES(include /usr/include/libxml2)

# remove the following stuff if you don't want STLport
INCLUDE_DIRECTORIES(/usr/include/stlport)
ADD_DEFINITIONS(-D_PTHREADS)
ADD_DEFINITIONS(-D_STLP_DEBUG)

AUX_SOURCE_DIRECTORY(src MAIN_SRCS)
AUX_SOURCE_DIRECTORY(src/client CLIENT_SRCS)
AUX_SOURCE_DIRECTORY(src/common COMMON_SRCS)
AUX_SOURCE_DIRECTORY(src/server SERVER_SRCS)
SET(ALL_SRCS ${MAIN_SRCS} ${CLIENT_SRCS} ${COMMON_SRCS} ${SERVER_SRCS})

IF (WIN32)
	# equivalent here?
ELSE (WIN32)
	ADD_DEFINITIONS(-ggdb -Wall --no-inline)
ENDIF (WIN32)

ADD_EXECUTABLE(bin/openlierox ${ALL_SRCS})
TARGET_LINK_LIBRARIES(bin/openlierox SDL SDL_image SDL_mixer NL gd z xml2)

# needed for STLport
TARGET_LINK_LIBRARIES(bin/openlierox stlportstlg stlport)

