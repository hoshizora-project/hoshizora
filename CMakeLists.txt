cmake_minimum_required(VERSION 3.7)
project(hoshizora)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -g -DSPDLOG_DEBUG_ON")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -g")

include_directories(src src/external)

if (UNIX AND NOT APPLE)
    # linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lnuma -DSUPPORT_NUMA -fopenmp -D_GLIBCXX_PARALLEL")
    set(SOURCE_FILES src/hoshizora/main.cpp src/hoshizora/core/model/graph.h src/hoshizora/core/util/includes.h src/hoshizora/core/io/io.h src/hoshizora/app/pagerank.h src/hoshizora/core/dispatcher/bulksyncdispatcher.h src/hoshizora/core/api/kernel.h src/hoshizora/core/dispatcher/dispatcher.h src/hoshizora/core/util/bulksync_thread_pool.h src/hoshizora/core/util/thread_pool.h src/hoshizora/core/util/sense_reversing_barrier.h src/hoshizora/core/util/numa_allocator.h src/hoshizora/core/util/primitive_includes.h src/external/pcm/cpucounters.cpp src/external/pcm/msr.cpp src/external/pcm/pci.cpp src/external/pcm/client_bw.cpp)
else()
    # macos
    set(SOURCE_FILES src/hoshizora/main.cpp src/hoshizora/core/model/graph.h src/hoshizora/core/util/includes.h src/hoshizora/core/io/io.h src/hoshizora/app/pagerank.h src/hoshizora/core/dispatcher/bulksyncdispatcher.h src/hoshizora/core/api/kernel.h src/hoshizora/core/dispatcher/dispatcher.h src/hoshizora/core/util/bulksync_thread_pool.h src/hoshizora/core/util/thread_pool.h src/hoshizora/core/util/sense_reversing_barrier.h src/hoshizora/core/util/numa_allocator.h src/hoshizora/core/util/primitive_includes.h)
endif ()

add_executable(hoshizora ${SOURCE_FILES})
