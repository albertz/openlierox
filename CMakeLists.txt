PROJECT(openlierox)

# TODO: don't hardcode libxml2 path
INCLUDE_DIRECTORIES(include /usr/include/libxml2)

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
