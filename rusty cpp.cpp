// http://www.club.cc.cmu.edu/~ajo/disseminate/2017-02-15-Optional-From-Scratch.pdf

#include <format>
#include <iostream>
#include <optional>
#include <type_traits>
#include <variant>

void println(const std::string &str = "") { std::cout << str << "\n"; }

template <typename... Args>
void println(std::format_string<Args...> str, Args &&...args) {
  println(std::format(str, std::forward<Args>(args)...));
}

template <typename... branches_t> struct cases : branches_t... {
  using branches_t::operator()...;
};
template <typename... branches_t> cases(branches_t...) -> cases<branches_t...>;

template <typename variant_t, typename... branches_t>
auto match(variant_t &var, cases<branches_t...> &&matcher) {
  return std::visit(matcher, var);
}

template <typename T>
concept trivially_destructible = std::is_trivially_destructible_v<T>;

// https://stackoverflow.com/questions/41897418/significance-of-trivial-destruction

template <typename T, typename E = void> struct test {
  ~test() { 1 + 1; }
};
template <trivially_destructible T> struct test<T, T> {
  ~test() = default;
};

constexpr bool a = trivially_destructible<test<int, void>>;
constexpr bool b = trivially_destructible<test<int>>;

template <typename T, typename E = void> struct optional_storage {
  union {
    char dummy_;
    T val_;
  };
  bool engaged_;
  ~optional_storage() {
    if (engaged_)
      val_.~T(); // this destructor is not trivial
  }
};
template <typename T> // partial specialization
struct optional_storage<T,
                        std::enable_if_t<std::is_trivially_destructible_v<T>>> {
  union {
    char dummy_;
    T val_;
  };
  bool engaged_;
  ~optional_storage() = default; // this destructor is trivial
};

template <typename T> struct optional {
  optional_storage<T> storage;
  ~optional() = default; // this destructor is sometimes trivial
};

int main() {
  std::optional<int> opt = 0;
  println("{}", opt.value());
}