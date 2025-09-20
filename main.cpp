#include "router.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/verb.hpp>
#include <iostream>

void handler(http::request<http::dynamic_body> req, http::response<http::dynamic_body> res, std::string a, int b) {
  std::cout << a << ' ' << b << std::endl;
}

int main() {
  boost::asio::io_context io;
  router r(io);
  r.GET<"/api/v1/{string:id}/{int:id2}">(handler);
  
  http::request<http::dynamic_body> req;
  http::response<http::dynamic_body> res;
  req.method(http::verb::get);

  req.target("/api/v1/42/aaa");

  r.handlers[std::make_pair("/api/v1/{string:id}/{int:id2}", http::verb::get)](std::move(req), res);
}
