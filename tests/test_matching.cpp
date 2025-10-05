#include <axon/types.hpp>
#include <axon/mux.hpp>

#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>

#include <deque>
#include <gtest/gtest.h>
#include <boost/beast.hpp>

using namespace axon;
using namespace boost::beast;

TEST(matching_test, simple_match_test) {

  auto r = router::core::create_router();
  

  std::deque<int> d;

  r->GET<"/test/path">([&d](){
    d.push_back(1);
  });


  r->GET<"/test/path2">([&d](){
    d.push_back(2);
  });

  request_type req;
  response_type res;

  request_type req2; 
  response_type res2;

  req.target("/test/path");
  req2.target("/test/path2");

  auto h1 = r->get_handler(req.target(), http::verb::get);
  auto h2 = r->get_handler(req2.target(), http::verb::get);

  h1(std::move(req), res);
  h2(std::move(req2), res2);

  ASSERT_EQ(d.front(), 1);
  d.pop_front();
  ASSERT_EQ(d.front(), 2);
}

TEST(matching_test, match_path_with_parametr) {

  auto r = router::core::create_router();
  
  std::deque<int> d;

  r->GET<"/test/{int:id}/path">([&d](){
    d.push_back(1);
  });


  r->GET<"/test/{int:id}/path2">([&d](){
    d.push_back(2);
  });

  request_type req;
  response_type res;

  request_type req2; 
  response_type res2;

  req.target("/test/1/path");
  req2.target("/test/1/path2");

  auto h1 = r->get_handler(req.target(), http::verb::get);
  auto h2 = r->get_handler(req2.target(), http::verb::get);

  h1(std::move(req), res);
  h2(std::move(req2), res2);

  ASSERT_EQ(d.front(), 1);
  d.pop_front();

  ASSERT_EQ(d.front(), 2);
}


