cmake_minimum_required(VERSION 3.10)

project(router CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

find_package(Boost REQUIRED COMPONENTS url)

add_executable(app main.cpp)

target_compile_options(app PRIVATE
  $<$<CONFIG:Debug>: -g3 -O0>
)

target_link_libraries(app PRIVATE
  Boost::url
  nlohmann_json::nlohmann_json
)
