add_executable(run_tests
  test_matching.cpp
  test_listener.cpp
  test_path.cpp
  test_json.cpp
  test_header.cpp
)

find_package(Boost REQUIRED COMPONENTS url)

target_link_libraries(run_tests PRIVATE
  axon 
  GTest::gtest_main
  Boost::url
)

target_compile_definitions(run_tests PRIVATE ROUTER_TEST)

target_compile_options(run_tests PRIVATE
  $<$<CONFIG:Debug>: -g3 -O0>
)
