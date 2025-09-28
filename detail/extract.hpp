#pragma once

#include <tuple>
#include <type_traits>

namespace router {
namespace extract {

template <typename... Ts>
struct path {
  using value_type = std::tuple<Ts...>;

  explicit path(value_type&& data): data_(std::move(data)) {}

  const value_type& get() const {
    return data_;
  }

private:
  value_type data_;
};

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
  
