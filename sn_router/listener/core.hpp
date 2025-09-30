#pragma once

#include "sn_router/types.hpp"
#include <sn_router/detail/http_connection.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace listener {
namespace detail {

template <typename session>
class base_listener {
  
  base_listener(boost::asio::io_context& io,
         boost::asio::ip::address address,
         unsigned short port)
      : io(io), 
        acceptor(io, {address, port}) 
  {}

public:

  base_listener(base_listener&&) = default;

  void serveHTTP() {
    start_http_server();
    io.run();
  }

  void set_application_handler(sn::application_handler h) {
    handler = h;
  } 

private:

  void start_http_server() {
    acceptor.async_accept(
      [this](boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
          if(!ec)
            std::make_shared<session>(std::move(socket), *this)->start();
          start_http_server();
      }
    ); 
  }

  boost::asio::io_context& io;
  boost::asio::ip::tcp::acceptor acceptor;

  sn::application_handler handler;
};

} // namespace detail
} // namespace listener

namespace listener {
  using core = listener::detail::base_listener<router::detail::http_connection<int>>;
}
