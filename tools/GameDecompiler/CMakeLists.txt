# CMake file for GameCompiler

cmake_minimum_required(VERSION 2.4)

SET(OLXROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
include(${OLXROOTDIR}/CMakeOlxCommon.cmake)

SET(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
PROJECT(gamedecompiler)

ADD_EXECUTABLE(gamedecompiler src/main.cpp ${ALL_SRCS})

EXEC_PROGRAM(mkdir ARGS -p ${CMAKE_CURRENT_SOURCE_DIR}/bin OUTPUT_VARIABLE -)

SET_TARGET_PROPERTIES(gamedecompiler PROPERTIES OUTPUT_NAME bin/gamedecompiler)

TARGET_LINK_LIBRARIES(gamedecompiler ${LIBS})
