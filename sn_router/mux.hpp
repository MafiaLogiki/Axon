#pragma once

#include <sn_router/router/core.hpp>
#include <sn_router/listener/core.hpp>

template <typename router_engine, typename listener_engine>
class base_mux {
  using handler_type = decltype(router_engine::get_handler);
};

using mux = base_mux<router::core, listener::core>;
