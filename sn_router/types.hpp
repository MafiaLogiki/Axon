#pragma once

#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/basic_dynamic_body.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http.hpp>
#include <functional>
#include <memory_resource>

namespace sn {
namespace pmr {
  //using allocator_type = std::pmr::polymorphic_allocator<char>;
  using allocator_type = std::allocator<char>;

  using multi_buffer = boost::beast::basic_multi_buffer<allocator_type>;
  using dynamic_body = boost::beast::http::basic_dynamic_body<multi_buffer>;

  using flat_buffer = boost::beast::basic_flat_buffer<allocator_type>;

  using fields = boost::beast::http::basic_fields<allocator_type>; 
} // namespace pmr
  
  using body_type = pmr::dynamic_body;

  using request_type = boost::beast::http::request<body_type, pmr::fields>;
  using response_type = boost::beast::http::response<body_type, pmr::fields>;

  using response_callback_type = std::function<void(response_type)>;
  using application_handler = std::function<void(const request_type&, response_callback_type)>;

} // namespace pmr 
