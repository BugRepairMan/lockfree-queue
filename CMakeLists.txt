cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mcx16 -std=c++11 -pthread -g -Wall -Wextra -Wnon-virtual-dtor")

#add_definitions(-std=c++11)

set(CMAKE_VERBOSE_MAKEFILE on)

#if(UNIX)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
#endif(UNIX)

#if(CYGWIN OR MINGW)
#    string(REGEX REPLACE "-Wl,--out-implib,[^ ]+ " " " CMAKE_C_CREATE_SHARED_LIBRARY "${CMAKE_C_CREATE_SHARED_LIBRARY}")
#    string(REGEX REPLACE "-Wl,--out-implib,[^ ]+ " " " CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY}")
#endif(CYGWIN OR MINGW)

project(fifo_queue)

file(GLOB srcs *.cpp)
file(GLOB hdrs *.hpp)
#file(GLOB in_hdrs include/dspatch/*.h)

#include_directories(
#    ${CMAKE_SOURCE_DIR}/include
#)

# Build shared (LGPL)
#add_library(
#    ${PROJECT_NAME} SHARED
#    ${srcs}
#    ${hdrs}
#    ${in_hdrs}
#)

#target_link_libraries(
#    ${PROJECT_NAME}
#    -static-libgcc
#    -static-libstdc++
#)

add_executable(
	${PROJECT_NAME}
	${srcs}
	${hdrs}
)

# Link pthread and dl on Unix
if(UNIX)
    target_link_libraries(
        ${PROJECT_NAME}
        pthread
        dl
		#	-fsanitize=thread 
    )
endif(UNIX)
