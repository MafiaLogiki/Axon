#include <algorithm>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/url.hpp>
#include <boost/url/segments_view.hpp>
#include <boost/url/url_view.hpp>
#include <charconv>
#include <cstdlib>
#include <exception>
#include <memory>
#include <string_view>
#include <string>
#include <iostream>
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
          std::tuple<const http::request<http::dynamic_body>&, http::response<http::dynamic_body>>, 
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
  
  using tcp = boost::asio::ip::tcp;
  
  template <typename Router>
  class http_connection
    : public std::enable_shared_from_this<http_connection<Router>>
  {

    tcp::socket socket_;
    boost::beast::flat_buffer buffer_{8192};

    http::request<http::dynamic_body> request_;
    http::response<http::dynamic_body> response_;

    Router& router_;

    boost::asio::basic_waitable_timer<std::chrono::steady_clock> deadline_ {
      socket_.get_executor(), std::chrono::seconds(60)
    };

  public: 
    http_connection(tcp::socket socket, Router& router_)
      : socket_(std::move(socket)), router_(router_)
    {}

    void start() {
      read_request();
      check_deadline();
    }

    void read_request() {
      auto self = this->shared_from_this();
      
      http::async_read(
          socket_,
          buffer_,
          request_,
          [self](boost::beast::error_code err, size_t bytes) {
            boost::ignore_unused(bytes);
            if (!err) {
              try {
                auto handler = self->router_.get_handler(self->request_.target(), self->request_.method());

                handler(std::move(self->request_), self->response_);
                self->write_response();
              } catch(std::exception& e) {
                std::cout << e.what() << std::endl;
                self->router_.not_found_handler(std::move(self->request_), self->response_);
                self->write_response();
              }
            }
          });
    }

    void write_response() {

      auto self = this->shared_from_this();
      
      std::stringstream str;
      str << response_.body().size();

      response_.set(http::field::content_length, str.view());

      http::async_write(
          socket_,
          response_,
          [self](boost::beast::error_code err, std::size_t b)
          {
            self->socket_.shutdown(tcp::socket::shutdown_send);
            self->deadline_.cancel();
          });
    }

    void check_deadline() {

      auto self = this->shared_from_this();

      deadline_.async_wait(
          [self](boost::beast::error_code ec)
          {
            if (!ec)
              self->socket_.close();
          });
      }
  };

}

class router
  : public std::enable_shared_from_this<router> {
public:

  struct parameter_storage {
  private:
    std::map<std::string, std::string> storage;
  public:
    std::string get(const std::string& key) {
      return storage[key];

    }

    void set(const std::string& key, std::string value) {
      storage[key] = value;
    }
  };

  using middleware_type = std::function<void(parameter_storage)>;

private:
  using internal_handler_type = std::function<void(http::request<http::dynamic_body>&&, http::response<http::dynamic_body>&)>;

public:
  using not_found_handler_function = std::function<void(http::request<http::dynamic_body>&&, http::response<http::dynamic_body>&)>;

private:
  router::parameter_storage parse_types_for_middleware(std::string_view requested_url, std::string_view path) {
    router::parameter_storage res;

    std::vector<std::string_view> parsed_path = split_path_view(path);
    auto requested_url_segments = boost::urls::url_view(requested_url).segments();

    size_t i = 0;
    for (auto seg : requested_url_segments) {
      if (parsed_path[i][0] == '{') {
        size_t start_pos = parsed_path[i].find(':');
        size_t end_pos = parsed_path[i].find('}', start_pos);
        string_view name = parsed_path[i].substr(start_pos + 1, end_pos - start_pos - 1);

        res.set(std::string(name), seg);
      }
      ++i;
    }
    
    return res;
  }


  template <constexpr_string str, typename Handler>
  void register_method(http::verb method, Handler&& h) {

    using tuple_path_types = __router_detail::parsed_types_in_tuple<str>;

    auto self = shared_from_this();

    internal_handler_type internal_handler = [self,
                                              path = std::string(std::string_view(str)), 
                                              h = std::forward<Handler>(h), 
                                              global_middlewares = std::vector(global_middleware_storage),
                                              endpoint_middlewares = std::vector(temporary_middleware_storage)]
        (
          http::request<http::dynamic_body>&& req,
          http::response<http::dynamic_body>& res
        ) 
    {
      tuple_path_types path_types;

      std::string_view requested_url_view = req.target();
      std::string_view path_view = std::string_view(path);

      parameter_storage storage = self->parse_types_for_middleware(requested_url_view, path);
      
      for(auto& func : global_middlewares) {
        func(storage);
      }

      for(auto& func : endpoint_middlewares) {
        func(storage);
      }

       __router_detail::parse_path_types<0>(requested_url_view, path_view, path_types);

      std::apply(h, std::tuple_cat(std::tuple(req), std::tuple(res), path_types));
    };
    
    temporary_middleware_storage.clear();
    if (std::tuple_size_v<tuple_path_types>) {
      path_with_parameter_handlers[std::make_pair(std::string(std::string_view(str)), method)] = internal_handler;
    } else {
      non_parameter_handlers[std::make_pair(std::string(std::string_view(str)), method)] = internal_handler;
    }
  }
  
  void start_http_server() {
    acceptor.async_accept(
      [this](boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
          if(!ec)
            std::make_shared<__router_detail::http_connection<router>>(std::move(socket), *this)->start();
          start_http_server();
      }
    ); 
  }

private:

  internal_handler_type get_handler(std::string_view requested_url, http::verb method) {
    if (non_parameter_handlers.count(std::make_pair(std::string(requested_url), method))) {
      return non_parameter_handlers[std::make_pair(std::string(requested_url), method)];
    }

    return parse_param_handlers(requested_url, method);
  }

  internal_handler_type parse_param_handlers(std::string_view requested_url, http::verb method) {
    for (auto& [data, handler] : path_with_parameter_handlers) {
      const std::string& path = data.first;
      http::verb path_method = data.second;

      if (is_parameter_path_math_url(requested_url, path)) {
        return handler;
      }
    }
    
    return {};
  }

  std::vector<std::string_view> split_path_view(std::string_view path) {
    std::vector<std::string_view> segments;
    size_t start = path.find_first_not_of('/'); 
      
    while (start != std::string_view::npos) {
      size_t end = path.find('/', start);
          
      if (end == std::string_view::npos) {
        segments.push_back(path.substr(start));
        break;
      }
      segments.push_back(path.substr(start, end - start));
      start = path.find_first_not_of('/', end);
    }
      
    return segments;
  }

  bool is_parameter_path_math_url(std::string_view requested_url, std::string_view path) {
    auto requested_url_view = boost::urls::url_view(requested_url);
    auto requested_url_segments = requested_url_view.segments();
    
    std::vector<std::string_view> path_segments = split_path_view(path);

    if (path_segments.size() != requested_url_segments.size())
      return false;

    size_t i = 0;
    for (auto seg : requested_url_segments) {
      if (path_segments[i][0] == '{') {
        continue;
      }

      if (!std::equal(seg.begin(), seg.end(), path_segments[i].begin(), path_segments[i].end())) {
        return false;
      }

      ++i;
    }

    return true;
  }
  router(boost::asio::io_context& io,
         boost::asio::ip::address address,
         unsigned short port)
      : io(io), 
        acceptor(io, {address, port}) 
  {}


  friend class __router_detail::http_connection<router>;

  template <typename T, typename... Args>
  friend std::shared_ptr<T> std::make_shared(Args&&...);

public:

  router(router&& r)
    : io(r.io),
      acceptor(std::move(r.acceptor))
  {}

  template <constexpr_string str, typename Handler>
  requires __router_detail::match_path<str, Handler>
  void GET(Handler&& h) {
    register_method<str>(http::verb::get, std::forward<Handler>(h));
  }
   
  void serveHTTP() {
    start_http_server();
    io.run();
  }

  router& with(middleware_type middleware) {
    temporary_middleware_storage.push_back(middleware);
    return *this;
  }

  void use(middleware_type middleware) {
    global_middleware_storage.push_back(middleware);
  }

  void set_not_found_handler(not_found_handler_function&& func) {
    not_found_handler = func;
  }
  
  static std::shared_ptr<router> create_router(boost::asio::io_context& io,
         boost::asio::ip::address address = boost::asio::ip::make_address("0.0.0.0"),
         unsigned short port = 8080) {
    router r(io, address, port);
    return std::make_shared<router>(std::move(r));
  }


  boost::asio::io_context& io;
  boost::asio::ip::tcp::acceptor acceptor;

  std::map<std::pair<std::string, http::verb>, internal_handler_type> non_parameter_handlers;
  std::map<std::pair<std::string, http::verb>, internal_handler_type> path_with_parameter_handlers;

  not_found_handler_function not_found_handler = [](http::request<http::dynamic_body>&& req, http::response<http::dynamic_body>& res){
    res.result(http::status::not_found);
    res.version(req.version());

    res.set(http::field::server, "server");
    res.set(http::field::content_type, "text/plain");

    boost::beast::ostream(res.body()) << "404 Not Found: The resource you requested could not be found.";
    
    res.prepare_payload(); 
  };

  std::vector<middleware_type> temporary_middleware_storage;
  std::vector<middleware_type> global_middleware_storage;
};
