#pragma once

#include <cstddef>
#include <tuple>

namespace axon {
namespace router {
namespace detail {

template <typename T>
struct function_traits
  : function_traits<decltype(&T::operator())>
{};

template <typename class_type, typename return_type, typename... Args>
struct function_traits<return_type(class_type::*)(Args...) const> {
  static constexpr size_t arity = sizeof...(Args);

  using result_type = return_type;
  using args_tuple = std::tuple<Args...>;
};

template <typename class_type, typename return_type, typename... Args>
struct function_traits<return_type(class_type::*)(Args...)> {
  static constexpr size_t arity = sizeof...(Args);

  using result_type = return_type;
  using args_tuple = std::tuple<Args...>;
};

template <typename return_type, typename... Args>
struct function_traits<return_type(*)(Args...)> {
  static constexpr size_t arity = sizeof...(Args);

  using result_type = return_type;
  using args_tuple = std::tuple<Args...>;
};

template <typename return_type, typename... Args>
struct function_traits<return_type(&)(Args...)> {
  static constexpr size_t arity = sizeof...(Args);

  using result_type = return_type;
  using args_tuple = std::tuple<Args...>;
};

template <typename T>
using callable_args_t = typename function_traits<T>::args_tuple;

} // namespace detail
} // namespace router
} // namespace axon

