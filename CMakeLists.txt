cmake_minimum_required(VERSION 3.7)
project(hoshizora)

#set(CMAKE_CXX_COMPILER clang++) # make infinit loop for some reason
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mlzcnt -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -DSPDLOG_DEBUG_ON")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories(src src/spdlog/include)

set(SOURCE_FILES src/hoshizora/main.cpp src/hoshizora/core/model/graph.h src/hoshizora/core/util/includes.h src/hoshizora/core/io/io.h src/hoshizora/app/pagerank.h src/hoshizora/core/executor/bulksync_gas_executor.h src/hoshizora/core/api/kernel.h src/hoshizora/core/executor/executor.h src/hoshizora/core/util/bulksync_thread_pool.h src/hoshizora/core/util/thread_pool.h src/hoshizora/core/util/spin_barrier.h src/hoshizora/core/util/condition_variable_barrier.h src/hoshizora/core/util/numa_allocator.h src/hoshizora/core/util/primitive_includes.h src/hoshizora/core/compress/common.h src/hoshizora/core/compress/single.h src/hoshizora/core/compress/multiple.h src/hoshizora/core/util/loop.h src/hoshizora/core/util/colle.h)

if (UNIX AND NOT APPLE)
    # linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lnuma -DSUPPORT_NUMA -fopenmp -D_GLIBCXX_PARALLEL")
    set(SOURCE_FILES ${SOURCE_FILES} src/pcm/cpucounters.cpp src/pcm/msr.cpp src/pcm/pci.cpp src/pcm/client_bw.cpp)
else ()
    # macos
endif ()

add_executable(hoshizora ${SOURCE_FILES})
