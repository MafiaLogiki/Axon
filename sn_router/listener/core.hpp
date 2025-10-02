#pragma once

#include <memory>
#include <memory_resource>
#include <sn_router/types.hpp>
#include <sn_router/listener/session.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace listener {
namespace detail {

template <typename session>
class base_listener
  : public std::enable_shared_from_this<base_listener<session>>{

public:
  base_listener(boost::asio::io_context& io,
         boost::asio::ip::address address,
         unsigned short port)
      : io(io), 
        acceptor(io, {address, port}) 
  {}

  base_listener(base_listener&&) = default;

  void serveHTTP() {
    do_accept();
    io.run();
  }

  void set_application_handler(sn::application_handler h) {
    handler = h;
  } 

private:

  void do_accept() {

    auto self = this->shared_from_this();

    acceptor.async_accept(
      [self](boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
          if(!ec) {
            std::make_shared<session>(std::move(socket), self->handler)->start();
            self->do_accept();
          }
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
  using core = listener::detail::base_listener<listener::http_session>;
}
