#pragma once

#include <cstddef>
#include <string_view>

namespace axon {

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

} // namespace axon
