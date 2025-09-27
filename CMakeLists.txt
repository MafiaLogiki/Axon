cmake_minimum_required(VERSION 3.16)

project(router CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED COMPONENTS url)

add_executable(app main.cpp)

target_link_libraries(
  app
  Boost::url
)
