#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>

#include <boost/url/host_type.hpp>
#include <memory>
#include <axon/types.hpp>

namespace axon {
namespace listener {

using namespace boost::asio::ip;
using namespace boost::beast;

class http_session
  : public std::enable_shared_from_this<http_session>
{

public: 
  http_session(tcp::socket socket, application_handler h)
    : socket_(std::move(socket))
    , handler_(h)
  {}

  void start() {
    read_request();
    check_deadline();
  }

  void read_request() {
    auto self = shared_from_this();
    
    http::async_read(
        socket_,
        buffer_,
        request_,
        [self](boost::beast::error_code err, size_t bytes) {
          boost::ignore_unused(bytes);

          response_callback_type callback = [self](response_type res) {
            self->write_response(std::move(res));
          };

          if (!err) {
            self->handler_(self->request_, callback);
          }
        });
  }

  void write_response(response_type res) {

    auto self = shared_from_this();

    response_ = std::move(res);
    
    http::async_write(
        socket_,
        response_,
        [self](boost::beast::error_code err, std::size_t b)
        {
          boost::ignore_unused(err);
          boost::ignore_unused(b);

          self->socket_.shutdown(tcp::socket::shutdown_send);
          self->deadline_.cancel();
        });
  }

  void check_deadline() {

    auto self = this->shared_from_this();

    deadline_.async_wait(
        [self](boost::beast::error_code ec)
        {
          if (!ec)
            self->socket_.close();
        });
    }

  tcp::socket socket_;
  pmr::flat_buffer buffer_;

  request_type request_;
  response_type response_;

  application_handler handler_;

  boost::asio::basic_waitable_timer<std::chrono::steady_clock> deadline_ {
    socket_.get_executor(), std::chrono::seconds(60)
  };
};

} // namespace listener
} // namespace axon

