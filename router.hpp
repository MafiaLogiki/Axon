#include <string_view>
#include <type_traits>

template <size_t N>
struct constexpr_string {
  char data[N];

  constexpr constexpr_string(const char (&str)[N]) {
    for (size_t i = 0; i < N; ++i) {
      data[i] = str[i];
    }
  }

  constexpr operator std::string_view() const {
    return {data, N - 1};
  }
};

template <size_t N>
constexpr_string(const char (&)[N]) -> constexpr_string<N>;

template <constexpr_string str, std::enable_if_t<std::string_view(str).starts_with("int"), bool> = false>
struct type__ {
  using value = int;
};

class router {
  type__<"int"> t;
};
