#include "router.hpp"
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/verb.hpp>
#include <iostream>

void handler(int a, int b) {
  std::cout << a << ' ' << b << std::endl;
}

int main() {
  router r;
  r.GET<"/api/v1/{int:id}/{int:id2}">(handler);
  
  http::request<http::string_body> req;
  http::response<http::string_body> res;
  req.method(http::verb::get);

  req.target("/api/v1/42/43/");

  r.handlers[std::make_pair("/api/v1/{int:id}/{int:id2}", http::verb::get)](std::move(req), res);
}
