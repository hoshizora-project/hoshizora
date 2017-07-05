cmake_minimum_required(VERSION 3.7)
project(hoshizora)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -g")
set(CMAKE_CXX_FLAGS "-O3")

include_directories(src)
set(SOURCE_FILES src/hoshizora/main.cpp src/hoshizora/core/model/graph.h src/hoshizora/core/util/includes.h src/hoshizora/core/io/io.h src/hoshizora/app/pagerank.h src/hoshizora/core/dispatcher/bulksyncdispatcher.h src/hoshizora/core/allocator/sharedmemoryallocator.h src/hoshizora/core/api/kernel.h src/hoshizora/core/dispatcher/dispatcher.h src/hoshizora/core/allocator/allocator.h src/hoshizora/core/util/thread_pool.h)
add_executable(hoshizora ${SOURCE_FILES})