cmake_minimum_required(VERSION 3.14)
project(SocialNetworkAnalysis LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source layout
set(PROJECT_INCLUDE_DIR \${CMAKE_SOURCE_DIR}/include)
set(PROJECT_SRC_DIR \${CMAKE_SOURCE_DIR}/src)

# executable
if(EXISTS \${PROJECT_SRC_DIR}/main.cpp)
  add_executable(sna \${PROJECT_SRC_DIR}/main.cpp)
  target_include_directories(sna PRIVATE \${PROJECT_INCLUDE_DIR})
endif()

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Tests
enable_testing()
file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(runTests \${TEST_SOURCES})
target_include_directories(runTests PRIVATE \${PROJECT_INCLUDE_DIR})
target_link_libraries(runTests PRIVATE gtest_main)
add_test(NAME AllTests COMMAND runTests)
EOF