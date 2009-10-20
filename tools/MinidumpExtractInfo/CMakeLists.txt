# CMake file for GameCompiler

cmake_minimum_required(VERSION 2.4)

SET(OLXROOTDIR ../..)
include(${OLXROOTDIR}/CMakeOlxCommon.cmake)

SET(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
PROJECT(minidumpextractinfo)

ADD_EXECUTABLE(minidumpextractinfo src/main.cpp ../ExternSourceHelper/define_extern_dummies.cpp ${ALL_SRCS})

EXEC_PROGRAM(mkdir ARGS -p ${CMAKE_CURRENT_SOURCE_DIR}/bin OUTPUT_VARIABLE -)

SET_TARGET_PROPERTIES(minidumpextractinfo PROPERTIES OUTPUT_NAME bin/minidumpextractinfo)

TARGET_LINK_LIBRARIES(minidumpextractinfo ${LIBS})
