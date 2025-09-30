#pragma once

#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <string_view>
#include <nlohmann/json.hpp>

#include "function_traits.hpp"
#include "extract.hpp"
#include "nlohmann/json_fwd.hpp"
#include "path_parser.hpp"

namespace router {
namespace detail {

using namespace boost::beast;

template <typename Handler>
using handler_args = callable_args_t<Handler>;

template <typename... Args>
struct is_argument_valid
  : std::false_type
{};

template <typename... Args>
struct is_argument_valid<router::extract::path<Args...>, std::tuple<Args...>>
  : std::true_type
{};

template <typename... Args>
struct is_argument_valid<http::request<http::dynamic_body>&&, std::tuple<Args...>>
  : std::true_type
{};

template <typename... Args>
struct is_argument_valid<http::response<http::dynamic_body>&, std::tuple<Args...>>
  : std::true_type
{};

template <typename T, typename... Args>
struct is_argument_valid<router::extract::json<T>, std::tuple<Args...>>
  : router::extract::has_method_deserialize<T, typename router::extract::json<T>::json_type&>
{};

template <typename tuple, typename... Args>
struct are_all_arguments_valid;

template <typename tuple, typename... Args>
struct are_all_arguments_valid<tuple, std::tuple<Args...>>
  : std::conjunction<is_argument_valid<Args, tuple>...>
{};


template <typename tuple, typename... Args>
inline static constexpr bool are_all_arguments_valid_v = are_all_arguments_valid<tuple, Args...>::value;

template <typename T>
struct argument_creator;

template<>
struct argument_creator<http::request<http::dynamic_body>&&> {
  static http::request<http::dynamic_body>&& create(http::request<http::dynamic_body>& req, http::response<http::dynamic_body>& res, std::string_view path) {
    return std::move(req);
  }
};

template<>
struct argument_creator<http::response<http::dynamic_body>&&> {
  static http::response<http::dynamic_body>& create(http::request<http::dynamic_body>& req, http::response<http::dynamic_body>& res, std::string_view path) {
    return res;
  }
};

template<typename... Args>
struct argument_creator<router::extract::path<Args...>> {
  static router::extract::path<Args...> create(http::request<http::dynamic_body>& req, http::response<http::dynamic_body>& res, std::string_view path) {

    std::tuple<Args...> values;
    parse_path_types<0>(req.target(), path, values);
  
    return router::extract::path<Args...>(std::move(values));
  }
};

template<typename T>
struct argument_creator<router::extract::json<T>> {
  static router::extract::json<T> create(http::request<http::dynamic_body>& req, http::response<http::dynamic_body>& res, std::string_view path) {
    std::string body_str = boost::beast::buffers_to_string(req.body().data());

    return router::extract::json<T>(std::move(body_str));
  }
};

template <typename... Args>
std::tuple<Args...> build_tuple_of_args(http::request<http::dynamic_body>& req, http::response<http::dynamic_body>& res, std::string_view path) {
  return std::make_tuple(argument_creator<Args>::create(req, res, path)...);
}

template <typename tuple_of_args>
auto build_args_from_type_tuple(http::request<http::dynamic_body>& req, http::response<http::dynamic_body>& res, std::string_view path) {
  auto unpacker = [&]<typename... Args>(std::tuple<Args...>){
    return build_tuple_of_args<Args...>(req, res, path);
  };

  return unpacker(tuple_of_args{});
}

} // namespace router
} // namespace detail 
