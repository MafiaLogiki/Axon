#pragma once

#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <functional>
#include <memory_resource>

namespace sn {
namespace pmr {
  using fields = boost::beast::http::basic_fields<std::pmr::polymorphic_allocator<char>>; 
} // namespace pmr
  
  using body_type = boost::beast::http::dynamic_body;

  using request_type = boost::beast::http::request<body_type, pmr::fields>;
  using response_type = boost::beast::http::response<body_type, pmr::fields>;

  using response_callback_type = std::function<void(response_type)>;
  using application_handler = std::function<void(const request_type&, response_callback_type)>;

} // namespace pmr 
