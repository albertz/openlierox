PROJECT(olx_fontgenerator)

INCLUDE_DIRECTORIES(include ../../include)

AUX_SOURCE_DIRECTORY(src MAIN_SRCS)
SET(ALL_SRCS ${MAIN_SRCS})

ADD_DEFINITIONS(-ggdb -Wall)

ADD_EXECUTABLE(bin/olx_fontgenerator ${ALL_SRCS})
TARGET_LINK_LIBRARIES(bin/olx_fontgenerator SDL SDL_ttf gd)
