#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/verb.hpp>
#include <charconv>
#include <cstdlib>
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

  static constexpr size_t npos = -1;

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
  

  constexpr size_t find(char c) const {
    for (size_t i = 0; i < N; ++i) {
      if (data[i] == c) {
        return i;
      }
    }

    return npos;
  }
};

template <size_t N>
constexpr_string(const char (&)[N]) -> constexpr_string<N>;


namespace __router_detail {

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
    // using current_type__ = std::conditional_t<pos_start != constexpr_string<0>::npos, std::tuple<typename get_type_from_string<type_and_after_structual>::type>, std::tuple<>>;
   
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
          std::tuple<const http::request<http::string_body>&, http::response<http::string_body>>, 
          parsed_types_in_tuple<str>>>()) };
  };

  template <constexpr_string str, typename Handler>
  concept match_path = invokable_with_path<str, Handler>;

  template <typename T>
  T parse_type_value(std::string_view& requested_url, std::string_view& path);

  template <>
  int parse_type_value<int>(std::string_view& requested_url, std::string_view& path) {
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
  std::string parse_type_value<std::string>(std::string_view& requested_url, std::string_view& path) {
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

}

class router {
private:

  template <constexpr_string str, typename Handler>
  void register_method(http::verb method, Handler&& h) {

    using tuple_path_types = __router_detail::parsed_types_in_tuple<str>;

    internal_handler_type internal_handler = [path = std::string(std::string_view(str)), 
                                              h = std::forward<Handler>(h), 
                                              global_middlewares = std::vector(global_middleware_storage),
                                              endpoint_middlewares = std::vector(temporary_middleware_storage)]
        (
          http::request<http::string_body>&& req,
          http::response<http::string_body>& res
        ) 
    {
      tuple_path_types path_types;
      std::string_view requested_url_view = req.target();
      std::string_view path_view = std::string_view(path);

       __router_detail::parse_path_types<0>(requested_url_view, path_view, path_types);

      std::apply(h, std::tuple_cat(std::tuple(req), std::tuple(res), path_types));
      std::ignore = res;
    };
    
    temporary_middleware_storage.clear();
    handlers[std::make_pair(std::string(std::string_view(str)), method)] = internal_handler;
  }

public:

  template <constexpr_string str, typename Handler>
  requires __router_detail::match_path<str, Handler>
  void GET(Handler&& h) {
    register_method<str>(http::verb::get, std::forward<Handler>(h));
  }

  using middleware_type = std::function<void(http::request<http::string_body>)>;

  router& with(middleware_type middleware) {
    temporary_middleware_storage.push_back(middleware);
    return *this;
  }

  using internal_handler_type = std::function<void(http::request<http::string_body>&&, http::response<http::string_body>&)>;
  
  std::map<std::pair<std::string, http::verb>, internal_handler_type> handlers;

  std::vector<middleware_type> temporary_middleware_storage;
  std::vector<middleware_type> global_middleware_storage;

};
