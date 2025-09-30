cmake_minimum_required(VERSION 3.14)

project(router CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(router INTERFACE)
target_include_directories(router INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

find_package(Boost REQUIRED COMPONENTS url)

target_link_libraries(router INTERFACE 
  Boost::url
  nlohmann_json::nlohmann_json
)

target_compile_options(router INTERFACE
  $<$<CXX_COMPILER_ID:GNU,Clang>:
    -Wall
    -Wextra
  >

  $<$<CXX_COMPILER_ID:MSVC>:
    /w4
  >
)

option(BUILD_TESTING "Build the project's tests" ON)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF)
 
enable_testing()

if(BUILD_TESTING)
  FetchContent_MakeAvailable(googletest)
  add_subdirectory(tests)
endif()
