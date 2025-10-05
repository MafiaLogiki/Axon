#pragma once

#include <axon/types.hpp>
#include <axon/detail/constexpr_string.hpp>

#include <memory>
#include <tuple>
#include <type_traits>
#include <nlohmann/json.hpp>

namespace axon {
namespace router {
namespace extract {
namespace detail {

template <typename T, typename... Args>
std::true_type test_has_method_deserialize(decltype(std::declval<T>().deserialize(std::declval<Args>()...), nullptr));

template <typename...>
std::false_type test_has_method_deserialize(...);


struct json_unit {
  using json_type = nlohmann::json;

  json_unit(std::string&& data): j(nlohmann::json::parse(data)) {}
  json_unit() = default;
  json_unit(json_unit&&) = default;

  template <typename T>
  T get_value(std::string str) {
    return j.at(str).get<T>();
  }

private:
  nlohmann::json j;
};

} // namespace detail

template <typename... Ts>
struct path {
  using value_type = std::tuple<Ts...>;

  explicit path(value_type&& data): data_(std::move(data)) {}
  path(): data_() {}

  const value_type& get() const {
    return data_;
  }

private:
  value_type data_;
};

template <typename T>
struct json {
  
  using json_type = detail::json_unit;

  json(std::string&& data): j(std::move(data))
  {};

  json() = default;
  json(json&& other) = default;

  T deserialize() {
    obj = std::make_unique<T>();
    obj->deserialize(j);
    return *obj;
  }

private:
  json_type j;
  std::unique_ptr<T> obj;
};


template <constexpr_string field>
struct header {
  
  header(request_type& req) {
    if (req.count(std::string_view(field))) {
      header_field_value = req[std::string_view(field)];
    }
  }

  header() = default;
  header(header&&) = default;

  std::string& get_value() {
    return header_field_value;
  }

private:
  std::string header_field_value;
};

template <typename T, typename... Args>
struct has_method_deserialize: decltype(detail::test_has_method_deserialize<T, Args...>(nullptr)) {};

} // namespace extract
} // namespace router
} // namespace axon
  
namespace std {

template<typename... Ts>
struct tuple_size<axon::router::extract::path<Ts...>>
  : std::integral_constant<size_t, sizeof...(Ts)> {};

template <size_t N, typename... Ts>
struct tuple_element<N, axon::router::extract::path<Ts...>> {
  using type = std::tuple_element<N, std::tuple<Ts...>>::type;
};

template <size_t N, typename... Ts>
decltype(auto) get(const axon::router::extract::path<Ts...>& p) {
  return std::get<N>(p.get());
}

} //namespace std 
