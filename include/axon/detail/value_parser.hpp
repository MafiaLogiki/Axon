#pragma once

#include <string_view>
#include <string>
#include <charconv>

namespace router {
namespace detail {

  template <typename T>
  T parse_type_value(std::string_view& requested_url, std::string_view& path);

  template <>
  inline int parse_type_value<int>(std::string_view& requested_url, std::string_view& path) {
    size_t pos_start = path.find("{");
    size_t pos_end_of_type_cell = path.find("}");

    size_t end = requested_url.find("/", pos_start);

    std::string_view number;
    if (end == std::string_view::npos) {
      number = requested_url.substr(pos_start);
    } else {
      number = requested_url.substr(pos_start, end - pos_start + 1);
      requested_url.remove_prefix(end);
      path.remove_prefix(pos_end_of_type_cell + 1);
    }

    int num;
    std::from_chars(number.data(), number.data() + number.size(), num);

    return num;
  }
   
  template <>
  inline std::string parse_type_value<std::string>(std::string_view& requested_url, std::string_view& path) {
    size_t pos_start = path.find("{");
    size_t pos_end_of_type_cell = path.find("}");

    size_t end = requested_url.find("/", pos_start);

    std::string_view result_string;
    if (end == std::string_view::npos) {
      result_string = requested_url.substr(pos_start);
    } else {
      result_string = requested_url.substr(pos_start, end - pos_start);
      requested_url.remove_prefix(end);
      path.remove_prefix(pos_end_of_type_cell + 1);
    }

    return std::string(result_string.data(), result_string.size());
  }

  template <size_t N, typename... Args>
  void parse_path_types(std::string_view requested_url, std::string_view path, std::tuple<Args...>& result_tuple) {

    if constexpr (N >= sizeof...(Args)) {
      return; 
    } else {
      using head = std::tuple_element_t<N, std::tuple<Args...>>;
      std::get<N>(result_tuple) = parse_type_value<head>(requested_url, path); 
      parse_path_types<N + 1>(requested_url, path, result_tuple);
    }
  }
  
} // namespace router 
} // namespace detail
