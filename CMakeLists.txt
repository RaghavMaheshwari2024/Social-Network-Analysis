cmake_minimum_required(VERSION 3.14)
project(SocialNetworkAnalysis VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include headers
include_directories(\${CMAKE_SOURCE_DIR}/include)

if(EXISTS src/main.cpp)
  add_executable(sna src/main.cpp)
  target_include_directories(sna PRIVATE \${CMAKE_SOURCE_DIR}/include)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Use relative paths for test sources so CMake resolves them reliably
add_executable(runTests
    tests/test_graph.cpp
    tests/test_betweenness.cpp
    tests/test_recommendation.cpp
)
target_include_directories(runTests PRIVATE include)
target_link_libraries(runTests PRIVATE gtest_main)

add_test(NAME AllTests COMMAND runTests)
