#pragma once

#include <sn_router/router/core.hpp>

template <typename router_engine, typename listener_engine>
class base_mux {

};

using mux = base_mux<router::core, int>;
