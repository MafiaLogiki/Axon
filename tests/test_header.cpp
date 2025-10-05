#include <axon/router/core.hpp>
#include <axon/types.hpp>

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <gtest/gtest.h>

namespace http = boost::beast::http;
using namespace axon;

TEST(Header, simple_header)
{

  auto r = router::core::create_router();

  request_type req;
  response_type res;

  req.method(http::verb::get);
  req.target("/");
  req.set("Authorization", "some-token");
  
  std::string token;

  r->GET<"/">([&token](router::extract::header<"Authorization"> h){
    token = h.get_value();
  });

  auto h1 = r->get_handler(req.target(), http::verb::get);

  h1(std::move(req), res);

  ASSERT_EQ(token, "some-token");
}


TEST(Header, empty_header)
{

  auto r = router::core::create_router();

  request_type req;
  response_type res;

  req.method(http::verb::get);
  req.target("/");
  req.set("Authorization", "some-token");
  
  std::string token;

  r->GET<"/">([&token](router::extract::header<"empty-field"> h){
    token = h.get_value();
  });

  auto h1 = r->get_handler(req.target(), http::verb::get);

  h1(std::move(req), res);

  ASSERT_EQ(token, "");
}
