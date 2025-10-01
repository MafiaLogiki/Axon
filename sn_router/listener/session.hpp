#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>

#include <boost/url/host_type.hpp>
#include <memory>
#include <sn_router/types.hpp>

namespace listener {

using namespace boost::asio::ip;
using namespace boost::beast;

class http_session
  : public std::enable_shared_from_this<http_session>
{

public: 
  http_session(tcp::socket socket, sn::application_handler h, std::pmr::polymorphic_allocator<> alloc)
    : alloc_(alloc)
    , socket_(std::move(socket))
    , request_(sn::request_type::header_type(alloc_), sn::request_type::fields_type(alloc_))
    , response_(sn::response_type::header_type(alloc_, sn::response_type::fields_type(alloc_)))
    , handler_(std::move(h))
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

          sn::response_callback_type callback = [self](sn::response_type res) {
            self->write_response(std::move(res));
          };

          if (!err) {
            self->handler_(self->request_, callback);
          }

        });
  }

  void write_response(sn::response_type res) {

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

  std::pmr::polymorphic_allocator<> alloc_;

  tcp::socket socket_;
  boost::beast::flat_buffer buffer_{8192};

  sn::request_type request_;
  sn::response_type response_;

  sn::application_handler handler_;

  boost::asio::basic_waitable_timer<std::chrono::steady_clock> deadline_ {
    socket_.get_executor(), std::chrono::seconds(60)
  };
};

}

