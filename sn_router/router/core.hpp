#pragma once

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
#include <cstdlib>
#include <functional>
#include <memory>
#include <memory_resource>
#include <string_view>
#include <string>
#include <tuple>
#include <cstdio>
#include <boost/beast/http.hpp>
#include <map>
#include <utility>

#include <sn_router/detail/constexpr_string.hpp>
#include <sn_router/detail/path_parser.hpp>
#include <sn_router/detail/value_parser.hpp>
#include <sn_router/detail/args_builder.hpp>
#include <sn_router/types.hpp>

#ifdef ROUTER_TEST
  #define PRIVATE_IF_NOT_TEST public
#else
  #define PRIVATE_IF_NOT_TEST private
#endif


namespace router {
namespace detail {

using namespace boost::beast;

class __router
  : public std::enable_shared_from_this<__router> {
public:
  struct parameter_storage {
    std::string get(const std::string& key) {
      return storage[key];
    }

    void set(const std::string& key, std::string value) {
      storage[key] = value;
    }

  private:
    std::map<std::string, std::string> storage;
  };

  using middleware_type = std::function<void(parameter_storage)>;
  using not_found_handler_function = std::function<void(sn::request_type&&, sn::response_type&)>;

  using handler_type = std::function<void(sn::request_type&&, sn::response_type&)>;

private:
  using internal_handler_type = std::function<void(sn::request_type&&, sn::response_type&)>;


  __router::parameter_storage parse_types_for_middleware(std::string_view requested_url, std::string_view path) {
    __router::parameter_storage res;

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

    using path_types_tuple = router::detail::parsed_types<str>;

    static_assert(are_all_arguments_valid_v<path_types_tuple, callable_args_t<Handler>>, "Invalid handler arguments");

    auto self = shared_from_this();

    internal_handler_type internal_handler = [self,
                                              path = std::string(std::string_view(str)), 
                                              h = std::forward<Handler>(h), 
                                              global_middlewares = std::vector(global_middleware_storage),
                                              endpoint_middlewares = std::vector(temporary_middleware_storage)]
        (
          sn::request_type&& req,
          sn::response_type& res
        ) 
    {
      std::string_view requested_url_view = req.target();
      std::string_view path_view = std::string_view(path);

      parameter_storage storage = self->parse_types_for_middleware(requested_url_view, path);
      
      for(auto& func : global_middlewares) {
        func(storage);
      }

      for(auto& func : endpoint_middlewares) {
        func(storage);
      }

      using handler_args = callable_args_t<Handler>;
      
      auto values_tuple = build_args_from_type_tuple<handler_args>(req, res, path_view);

      std::apply(h, std::move(values_tuple));
    };
    
    temporary_middleware_storage.clear();
    if (std::tuple_size<path_types_tuple>::value) {
      path_with_parameter_handlers[std::make_pair(std::string(std::string_view(str)), method)] = internal_handler;
    } else {
      non_parameter_handlers[std::make_pair(std::string(std::string_view(str)), method)] = internal_handler;
    }
  }
  

PRIVATE_IF_NOT_TEST:

  internal_handler_type get_handler(std::string_view requested_url, http::verb method) {
    if (non_parameter_handlers.count(std::make_pair(std::string(requested_url), method))) {
      return non_parameter_handlers[std::make_pair(std::string(requested_url), method)];
    }

    return parse_param_handlers(requested_url, method);
  }

private:
  internal_handler_type parse_param_handlers(std::string_view requested_url, http::verb method) {
    for (auto& [data, handler] : path_with_parameter_handlers) {
      const std::string& path = data.first;
      http::verb path_method = data.second;

      if (is_parameter_path_match_url(requested_url, path) && path_method == method) {
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

  bool is_parameter_path_match_url(std::string_view requested_url, std::string_view path) {
    auto requested_url_view = boost::urls::url_view(requested_url);
    auto requested_url_segments = requested_url_view.segments();
    
    std::vector<std::string_view> path_segments = split_path_view(path);

    if (path_segments.size() != requested_url_segments.size())
      return false;

    size_t i = 0;
    for (auto seg : requested_url_segments) {
      if (path_segments[i][0] == '{') {
        ++i;
        continue;
      }

      if (!std::equal(seg.begin(), seg.end(), path_segments[i].begin(), path_segments[i].end())) {
        return false;
      }

      ++i;
    }

    return true;
  }

  __router() = default;
  
public:

  __router(__router&& r) = default;

  template <constexpr_string str, typename Handler>
  void GET(Handler&& h) {
    register_method<str>(http::verb::get, std::forward<Handler>(h));
  }

  __router& with(middleware_type middleware) {
    temporary_middleware_storage.push_back(middleware);
    return *this;
  }

  void use(middleware_type middleware) {
    global_middleware_storage.push_back(middleware);
  }

  void set_not_found_handler(not_found_handler_function&& func) {
    not_found_handler = func;
  }
  
  static std::shared_ptr<__router> create_router() {
    __router r;
    return std::make_shared<__router>(std::move(r));
  }

private:

  std::pmr::map<std::pair<std::string, http::verb>, internal_handler_type> non_parameter_handlers;
  std::pmr::map<std::pair<std::string, http::verb>, internal_handler_type> path_with_parameter_handlers;

  not_found_handler_function not_found_handler = [](sn::request_type&& req, sn::response_type& res){
    res.result(http::status::not_found);
    res.version(req.version());

    res.set(http::field::server, "server");
    res.set(http::field::content_type, "text/plain");

    boost::beast::ostream(res.body()) << "404 Not Found: The resource you requested could not be found.\n";
    
    res.prepare_payload(); 
  };

  std::pmr::vector<middleware_type> temporary_middleware_storage;
  std::pmr::vector<middleware_type> global_middleware_storage;
};

} // namespace router
} // namespace detail

namespace router {
  using core = router::detail::__router;
}

