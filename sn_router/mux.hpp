#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <sn_router/router/core.hpp>
#include <sn_router/listener/core.hpp>
#include <sn_router/types.hpp>
#include <utility>

template <typename router_engine, typename listener_engine>
class base_mux {
public:
  base_mux(boost::asio::io_context& io, boost::asio::ip::address address, unsigned short port)
    : router_(), listener_(io, address, port)
  {}

  void serveHTTP() {

    sn::application_handler handler = [this](const sn::request_type& req, sn::response_callback_type callback) {
      typename router_engine::handler_type h = router_->get_handler(req.target().data(), req.method());
      sn::response_type res;

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
