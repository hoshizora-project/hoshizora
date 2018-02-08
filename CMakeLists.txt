cmake_minimum_required(VERSION 3.8)
project(hoshizora)

#set(CMAKE_CXX_COMPILER clang++) # make infinit loop for some reason
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mlzcnt -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -DSPDLOG_DEBUG_ON")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(PYTHON_VERSION 3.6)
set(PYBIND11_PYTHON_VERSION ${PYTHON_VERSION})
set(PYTHON_EXECUTABLE python${PYTHON_VERSION})

file(GLOB SOURCES src/hoshizora/**/*.h src/hoshizora/**/**/*.h)
include_directories(src)
include_directories(SYSTEM
        src/spdlog/include
        src/pcm)

if (UNIX AND NOT APPLE)
    # linux
    ## pcm
    set(PCM_SOURCES
        src/pcm/cpucounters.cpp
        src/pcm/msr.cpp
        src/pcm/pci.cpp
        src/pcm/client_bw.cpp)
    set(LIB_PCM pcm)
    add_library(${LIB_PCM} STATIC ${PCM_SOURCES})
    set_property(TARGET ${LIB_PCM} PROPERTY POSITION_INDEPENDENT_CODE ON)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -D_GLIBCXX_PARALLEL")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSUPPORT_NUMA -fopenmp -D_GLIBCXX_PARALLEL")
else ()
    # macos
endif ()


# dependencies
## pybind11
add_subdirectory(src/pybind11)

## FastPFor
#add_subdirectory(src/FastPFor)


# cli
set(HOSHIZORA_CLI hoshizora-cli)
add_executable(${HOSHIZORA_CLI} ${SOURCES} src/hoshizora/cli.cpp)
#target_link_libraries(${SHOSHIZORA_CLI} FastPFor)
if (UNIX AND NOT APPLE)
    target_link_libraries(${HOSHIZORA_CLI} numa pthread ${LIB_PCM})
endif()


# pybind
set(HOSHIZORA hoshizora)
pybind11_add_module(${HOSHIZORA} SHARED ${SOURCES} src/hoshizora/pybind.cpp)
target_link_libraries(${HOSHIZORA} ${LIB_PCM})
#target_link_libraries(${HOSHIZORA} PRIVATE numa)
