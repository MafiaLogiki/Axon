set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

add_executable(run_tests
  test_matching.cpp
)

find_package(Boost REQUIRED COMPONENTS url)

target_link_libraries(run_tests PRIVATE
  router
  gtest_main
  Boost::url
)

target_compile_definitions(run_tests PRIVATE ROUTER_TEST)

target_compile_options(run_tests PRIVATE
  $<$<CONFIG:Debug>: -g3 -O0>
)
