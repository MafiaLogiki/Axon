#pragma once

#include <memory>
#include <tuple>
#include <type_traits>
#include <nlohmann/json.hpp>

namespace router {
namespace extract {
namespace detail {

template <typename T, typename... Args>
std::true_type test_has_method_deserialize(decltype(std::declval<T>().deserialize(std::declval<Args>()...), nullptr));

template <typename...>
std::false_type test_has_method_deserialize(...);


struct json_unit {
  using json_type = nlohmann::json;

  json_unit(std::string&& data): j(std::move(data)) {}
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

  T& deserialize() {
    obj->deserialize(j);
    return *obj;
  }

private:
  json_type j;
  std::unique_ptr<T> obj;
};

template <typename T, typename... Args>
struct has_method_deserialize: decltype(detail::test_has_method_deserialize<T, Args...>(nullptr)) {};

} // namespace extract
} // namespace router

namespace std {

template<typename... Ts>
struct tuple_size<router::extract::path<Ts...>>
  : std::integral_constant<size_t, sizeof...(Ts)> {};

template <size_t N, typename... Ts>
struct tuple_element<N, router::extract::path<Ts...>> {
  using type = std::tuple_element<N, std::tuple<Ts...>>::type;
};

template <size_t N, typename... Ts>
decltype(auto) get(const router::extract::path<Ts...>& p) {
  return std::get<N>(p.get());
}

} //namespace std 
