#include <axon/router/core.hpp>         
#include <axon/router/extract/extract.hpp>
#include <axon/types.hpp>              
#include <gtest/gtest.h>              
#include <exception>                 
#include <ostream>                  
#include <string>                  
#include <utility>                

using namespace axon;

struct first_test_data {
  std::string username;
  std::string password;

  int id;

  void deserialize(router::extract::json<first_test_data>::json_type& j) {
    username = j.get_value<std::string>("username");
    password = j.get_value<std::string>("password");
    id = j.get_value<int>("id");
  }
};

TEST(json_test, simple_json_test) {
  
  auto r = router::core::create_router();
  
  first_test_data d;
  r->GET<"/api/user">([&d](router::extract::json<first_test_data> j) {
    d = j.deserialize();
  });

  std::string json_data_string = "{\"username\": \"testuser\", \"password\": \"qwerty\", \"id\": 123}"; 

  request_type req;
  response_type res;

  boost::beast::ostream(req.body()) << json_data_string;
  req.target("/api/user");

  auto h = r->get_handler(req.target(), boost::beast::http::verb::get);

  h(std::move(req), res);

  ASSERT_EQ(d.username, "testuser");
  ASSERT_EQ(d.password, "qwerty");
  ASSERT_EQ(d.id, 123);
}


TEST(json_test, incorrect_json_test) {
  
  auto r = router::core::create_router();
  
  first_test_data d;
  r->GET<"/api/user">([&d](router::extract::json<first_test_data> j) {
    d = j.deserialize();
  });

  std::string json_data_string = "\"username\": \"testuser\", \"password\": \"qwerty\", \"id\": 123}"; 

  request_type req;
  response_type res;

  boost::beast::ostream(req.body()) << json_data_string;
  req.target("/api/user");

  auto h = r->get_handler(req.target(), boost::beast::http::verb::get);
  try {
    h(std::move(req), res);
  } catch(std::exception& e) {
    return;
  }
  FAIL() << "Code must throw exception in wrong json case" << std::endl;
}
