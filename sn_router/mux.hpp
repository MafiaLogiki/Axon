#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <sn_router/router/core.hpp>
#include <sn_router/listener/core.hpp>
#include <sn_router/types.hpp>

template <typename router_engine, typename listener_engine>
class base_mux {
public:
  base_mux(boost::asio::io_context& io, boost::asio::ip::address address, unsigned short port)
    : router_(), listener_(io, address, port)
  {}

private:
  router_engine router_;
  listener_engine listener_;
};

using mux = base_mux<router::core, listener::core>;
