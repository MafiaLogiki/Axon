#include <boost/asio/io_context.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/url/url_view.hpp>
#include <iostream>

#include "router.hpp"

using namespace boost::beast;

void handler(http::request<http::dynamic_body> req, http::response<http::dynamic_body> res) {
  std::cout << "Hello!" << std::endl;
}

void handler2(http::request<http::dynamic_body> req, http::response<http::dynamic_body> res, int id, int id2) {
  std::cout << "Hello2! " << id << ' ' << id2 << std::endl;
}

void handler3(router::extract::path<int>) {
  std::cout << "test";
}

void middleware1(Router::parameter_storage storage) {
  std::cout << storage.get("id") << std::endl;
  std::cout << storage.get("id2") << std::endl;
}

int main() {
  boost::asio::io_context io;
  auto r = Router::create_router(io);

  //r->GET<"/api/v1/test">(handler);
  //r->with(middleware1).GET<"/api/v1/{int:id}/{int:id2}">(handler2);
  r->GET<"/api/v1/test/{int:id}">(handler3);

  r->serveHTTP();
}
