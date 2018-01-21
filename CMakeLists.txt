cmake_minimum_required(VERSION 3.7)
project(hoshizora)

#set(CMAKE_CXX_COMPILER clang++) # make infinit loop for some reason
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mlzcnt -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -DSPDLOG_DEBUG_ON")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

file(GLOB SOURCES src/hoshizora/**/*.h src/hoshizora/**/**/*.h)
include_directories(src)
include_directories(SYSTEM
        src/spdlog/include)

if (UNIX AND NOT APPLE)
    # linux
    # pcm
    file(GLOB PCM_SOURCES src/pcm/*.h src/pcm/cpucounters.cpp src/pcm/msr.cpp src/pcm/pci.cpp src/pcm/client_bw.cpp)
    add_library(LIB_PCM SHARED ${PCM_SOURCES})

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lnuma -DSUPPORT_NUMA -fopenmp -D_GLIBCXX_PARALLEL")
#    set(SOURCE_FILES ${SOURCES} src/pcm/cpucounters.cpp src/pcm/msr.cpp src/pcm/pci.cpp src/pcm/client_bw.cpp)
    include_directories(SYSTEM src/pcm)
else ()
    # macos
endif ()


# cli
set(HOSHIZORA_CLI hoshizora-cli)
add_executable(${HOSHIZORA_CLI} ${SOURCES} src/hoshizora/main.cpp)
if (UNIX AND NOT APPLE)
    target_link_libraries(${HOSHIZORA_CLI} ${LIB_PCM})
endif()
