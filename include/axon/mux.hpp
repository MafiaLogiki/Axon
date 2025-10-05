#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>

#include <axon/router/core.hpp>
#include <axon/listener/core.hpp>
#include <axon/types.hpp>

#include <utility>

namespace axon {

template <typename router_engine, typename listener_engine>
class base_mux {
public:
  base_mux(boost::asio::io_context& io, boost::asio::ip::address address, unsigned short port)
    : listener_(io, address, port)
  {}

  void serveHTTP() {

    application_handler handler = [this](const request_type& req, response_callback_type callback) {
      typename router_engine::handler_type h = router_->get_handler(req.target().data(), req.method());
      response_type res;

      h(std::move(req), res); 

      callback(res);
    };

    listener_.set_application_handler(handler);

    listener_.serveHTTP();
  }

  template <constexpr_string str, typename Handler>
  void GET(Handler&& h) {
    router_.template GET<str>(std::forward<Handler>(h));
  }

private:

  router_engine router_;
  listener_engine listener_;
};

using mux = base_mux<router::core, listener::core>;

}
