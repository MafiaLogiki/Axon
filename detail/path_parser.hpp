#pragma once

#include <tuple>
#include <boost/beast/http.hpp>

#include "constexpr_string.hpp"

namespace __router_detail {

using namespace boost::beast;

template<constexpr_string str, typename = void>
struct get_type_from_string;

template <constexpr_string str>
struct get_type_from_string<str, std::enable_if_t<std::string_view(str).starts_with("int:")>> {
  using type = int;
};

template <constexpr_string str>
struct get_type_from_string<str, std::enable_if_t<std::string_view(str).starts_with("string:")>> {
  using type = std::string;
};

template <constexpr_string str, size_t pos>
struct path_parser {
  static constexpr size_t pos_start = str.find('{');
  
  static constexpr std::string_view type_and_after = (pos_start != constexpr_string<0>::npos ? std::string_view(str).substr(pos_start + 1) : std::string_view("asdasd"));
  
  static constexpr constexpr_string<type_and_after.size()> type_and_after_structual = type_and_after;
 
  using current_type__ = decltype([]() {
    if constexpr (pos_start != constexpr_string<0>::npos) {
      return std::tuple<typename get_type_from_string<type_and_after_structual>::type>();
    } else {
      return std::tuple<>();
    }
  }());

  using remaining_types__ = path_parser<type_and_after_structual, pos_start>::types;
  
  using types = decltype(std::tuple_cat(
    std::declval<current_type__>(),
    std::declval<remaining_types__>()
  ));
};

template <constexpr_string str>
struct path_parser<str, std::string_view::npos> {
  using types = std::tuple<>;
};

template <constexpr_string str>
using parsed_types_in_tuple = path_parser<str, 1>::types;

template <typename f, typename... Args>
struct function_matches_tuple;

template <typename f, typename... Args>
struct function_matches_tuple<f, std::tuple<Args...>> {
  static constexpr bool value = std::is_invocable_v<f, Args...>;
};

template <typename... tuples>
using tuple_cat_t = decltype(std::tuple_cat(std::declval<tuples>()...));

template <constexpr_string str, typename Handler>
concept invokable_with_path = requires (Handler handler) {
  { std::apply(handler, std::declval<
      tuple_cat_t<
        std::tuple<const http::request<http::dynamic_body>&, http::response<http::dynamic_body>>, 
        parsed_types_in_tuple<str>>>()) };
};

template <constexpr_string str, typename Handler>
concept match_path = invokable_with_path<str, Handler>;

} // __router_detail
