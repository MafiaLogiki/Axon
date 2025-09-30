#pragma once

#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <functional>

namespace sn {
  using body_type = boost::beast::http::dynamic_body;

  using request_type = boost::beast::http::request<body_type>;
  using response_type = boost::beast::http::response<body_type>;

  using response_callback_type = std::function<void(response_type)>;
  using application_handler = std::function<void(const request_type&, response_callback_type)>;
}
