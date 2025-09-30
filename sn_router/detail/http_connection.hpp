#pragma once

#include <memory>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>


namespace router {
namespace detail {

using tcp = boost::asio::ip::tcp;
using namespace boost::beast;

template <typename Router>
class http_connection
  : public std::enable_shared_from_this<http_connection<Router>>
{

  tcp::socket socket_;
  boost::beast::flat_buffer buffer_{8192};

  http::request<http::dynamic_body> request_;
  http::response<http::dynamic_body> response_;

  Router& router_;

  boost::asio::basic_waitable_timer<std::chrono::steady_clock> deadline_ {
    socket_.get_executor(), std::chrono::seconds(60)
  };

public: 
  http_connection(tcp::socket socket, Router& router_)
    : socket_(std::move(socket)), router_(router_)
  {}

  void start() {
    read_request();
    check_deadline();
  }

  void read_request() {
    auto self = this->shared_from_this();
    
    http::async_read(
        socket_,
        buffer_,
        request_,
        [self](boost::beast::error_code err, size_t bytes) {
          boost::ignore_unused(bytes);
          if (!err) {
            try {
              auto handler = self->router_.get_handler(self->request_.target(), self->request_.method());

              handler(std::move(self->request_), self->response_);
              self->write_response();
            } catch(std::exception& e) {
              self->router_.not_found_handler(std::move(self->request_), self->response_);
              self->write_response();
            }
          }
        });
  }

  void write_response() {

    auto self = this->shared_from_this();
    
    std::stringstream str;
    str << response_.body().size();

    response_.set(http::field::content_length, str.view());

    http::async_write(
        socket_,
        response_,
        [self](boost::beast::error_code err, std::size_t b)
        {
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
};

} // namespace __detail
} // namespace router 
