#include <axon/router/core.hpp>
#include <axon/types.hpp>

#include <gtest/gtest.h>

#include <deque>

using namespace boost::beast;
using namespace axon;

TEST(matching_test, simple_int_path_test) {

  auto r = router::core::create_router();
  
  std::deque<int> d;

  r->GET<"/test/{int:id}/path">([&d](router::extract::path<int> p){
    d.push_back(std::get<0>(p));
  });

  r->GET<"/test/{int:id}/path2">([&d](router::extract::path<int> p){
    d.push_back(std::get<0>(p));
  });

  request_type  req; 
  response_type res; 

  request_type  req2; 
  response_type res2;

  req.target("/test/1/path");
  req2.target("/test/3/path2");

  auto h1 = r->get_handler(req.target(), http::verb::get);
  auto h2 = r->get_handler(req2.target(), http::verb::get);

  h1(std::move(req), res);
  h2(std::move(req2), res2);

  ASSERT_EQ(d.front(), 1);
  d.pop_front();

  ASSERT_EQ(d.front(), 3);
}


TEST(matching_test, multy_variable_path_test) {

  auto r = router::core::create_router();
  
  std::deque<int> d;

  r->GET<"/test/{int:id}/path/{int:id2}">([&d](router::extract::path<int, int> p){
    d.push_back(std::get<0>(p));
    d.push_back(std::get<1>(p));
  });

  r->GET<"/test/{int:id}/path2/{int:id2}">([&d](router::extract::path<int, int> p){
    d.push_back(std::get<0>(p));
    d.push_back(std::get<1>(p));
  });

  request_type  req; 
  response_type res; 

  request_type  req2; 
  response_type res2;

  req.target("/test/1/path/2");
  req2.target("/test/3/path2/4");

  auto h1 = r->get_handler(req.target(), http::verb::get);
  auto h2 = r->get_handler(req2.target(), http::verb::get);

  h1(std::move(req), res);
  h2(std::move(req2), res2);

  ASSERT_EQ(d.front(), 1);
  d.pop_front();

  ASSERT_EQ(d.front(), 2);
  d.pop_front();

  ASSERT_EQ(d.front(), 3);
  d.pop_front();

  ASSERT_EQ(d.front(), 4);
}
