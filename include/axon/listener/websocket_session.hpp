#pragma once

#include <axon/types.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket.hpp>

#include <memory>

namespace axon {

using namespace boost::beast;

class websocket_session
  : std::enable_shared_from_this<websocket_session>
{
public:

  explicit websocket_session(boost::asio::ip::tcp&& sock)
    : ws_(std::move(sock))
  {}

  void start() {
    ws_.set_option(
        websocket::stream_base::timeout::suggested(
          boost::beast::role_type::server
    ));
  }

  void on_accept(error_code ec) {
    if (ec)
      return;
    
    do_read();
  }


  void do_read() {
    ws_.async_read(
      buf_,
      bind_front_handler(
        &websocket_session::on_read, 
        shared_from_this())
    );
  }

  void on_read() {

  }

private:
  websocket::stream<boost::beast::tcp_stream> ws_;
  pmr::flat_buffer buf_;

  request_type request_;
  response_type response_;

  application_handler handler_;
};
}
