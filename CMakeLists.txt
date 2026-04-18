cmake_minimum_required(VERSION 3.20)
project(sharded_lru_cache VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SHARDLRU_BUILD_TESTS "Build unit tests" ON)
option(SHARDLRU_BUILD_BENCHMARKS "Build benchmarks" ON)

add_library(sharded_lru_cache INTERFACE)
target_include_directories(sharded_lru_cache INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(sharded_lru_example examples/basic_usage.cpp)
target_link_libraries(sharded_lru_example PRIVATE sharded_lru_cache)

if(SHARDLRU_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(SHARDLRU_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
