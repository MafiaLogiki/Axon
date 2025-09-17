#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/verb.hpp>
#include <string_view>
#include <string>
#include <tuple>
#include <type_traits>
#include <cstdio>
#include <boost/beast/http.hpp>
#include <map>
#include <utility>

using namespace boost::beast;

template <size_t N>
struct constexpr_string {
  char data[N];

  constexpr constexpr_string(const char (&str)[N]) {
    for (size_t i = 0; i < N; ++i) {
      data[i] = str[i];
    }
  }

  constexpr constexpr_string(std::string_view str) {
    for (size_t i = 0; i < N; ++i) {
      data[i] = str[i];
    }
  }

  constexpr operator std::string_view() const {
    return {data, N - 1};
  }

  constexpr size_t size() const {
    return N;
  }
};

template <size_t N>
constexpr_string(const char (&)[N]) -> constexpr_string<N>;


namespace __router_detail {

  template<constexpr_string str, typename = void>
  struct get_type_from_string;

  template <constexpr_string str>
  struct get_type_from_string<str, std::enable_if_t<std::string_view(str).starts_with("int")>> {
    using type = int;
  };

  template <constexpr_string str>
  struct get_type_from_string<str, std::enable_if_t<std::string_view(str).starts_with("string")>> {
    using type = std::string;
  };

  template <constexpr_string str, size_t pos>
  struct path_parser {
    static constexpr size_t pos_start = std::string_view(str).find("{");
    
    static constexpr std::string_view type_and_after = std::string_view(str).substr(pos_start + 1);
    
    static constexpr constexpr_string<type_and_after.size()> type_and_after_structual = type_and_after;
    using current_type__ = std::conditional_t<pos_start != std::string_view::npos, std::tuple<typename get_type_from_string<type_and_after_structual>::type>, std::tuple<>>;

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
}

template <size_t max_endpoints_count = 1024>
class router_config {
  static constexpr size_t max_endpoints = max_endpoints_count;
};

template <typename Config = router_config<>>
class router {
public:
  template <constexpr_string str, typename handler>
  consteval void GET(handler&& h) {

    using types = __router_detail::parsed_types_in_tuple<str>;

    static_assert(
      __router_detail::function_matches_tuple<handler, types>::value,
      "Handler signature not match path types"
    );
    
    internal_handler_type internal_handler = [h = std::forward<handler>(h)](
          http::request<http::string_body>&& req,
          http::response<http::string_body>& res
        ) 
    {

    };
    
    handlers[std::make_pair("", http::verb::get)] = internal_handler;
  }

private:
  // using internal_handler_type = std::function<void(http::request<http::string_body>&&, http::response<http::string_body>&)>;

  using internal_handler_type = void(*)(http::request<http::string_body>&&, http::response<http::string_body>&);
  
  Config config;

  std::map<std::pair<std::string, http::verb>, internal_handler_type> handlers;
};
