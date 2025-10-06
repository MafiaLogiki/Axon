#pragma once

#include "axon/types.hpp"
#include <boost/asio/ip/tcp.hpp>
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

  websocket::stream<boost::beast::tcp_stream> ws_;
  pmr::flat_buffer buf_;

  request_type request_;
  response_type response_;

  application_handler handler_;
};
}
