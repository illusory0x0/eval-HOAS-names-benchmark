#include <compare>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>
#include <cassert>
using float64_t = double;
using float32_t = float;

namespace logger {
template <typename Self> struct Instance {
  static void write_string(Self &self, std::string const &val) {
    static_assert(false, "No instance for Logger typeclass");
  }
  static void write_int32(Self &self, int32_t val) {
    static_assert(false, "No instance for Logger typeclass");
  }
  static void write_float64(Self &self, float64_t val) {
    static_assert(false, "No instance for Logger typeclass");
  }
  template <size_t N>
  static void write_string_literal(Self &self, const char (&val)[N]) {
    static_assert(false, "No instance for Logger typeclass");
  }
};

template <> struct Instance<std::ostream> {
  using Self = std::ostream;
  static void write_string(Self &self, std::string const &val) { self << val; }
  static void write_int32(Self &self, int val) { self << val; }
  static void write_float64(Self &self, float64_t val) { self << val; }
  template <size_t N>
  static void write_string_literal(Self &self, const char (&val)[N]) {
    self << val;
  }
};
template <> struct Instance<std::stringstream> {
  using Self = std::stringstream;
  static void write_string(Self &self, std::string const &val) { self << val; }
  static void write_int32(Self &self, int val) { self << val; }
  static void write_float64(Self &self, float64_t val) { self << val; }
  template <size_t N>
  static void write_string_literal(Self &self, const char (&val)[N]) {
    self << val;
  }
};
template <typename Self> void write_string(Self &self, std::string const &val) {
  Instance<Self>::write_string(self, val);
}

template <typename Self> void write_int32(Self &self, int32_t val) {
  Instance<Self>::write_int32(self, val);
}
template <typename Self> void write_float64(Self &self, float64_t val) {
  Instance<Self>::write_float64(self, val);
}
template <typename Self, size_t N>
void write_string_literal(Self &self, const char (&val)[N]) {
  Instance<Self>::write_string_literal(self, val);
}

} // namespace logger

namespace debug {

namespace self = ::debug;
template <typename Self> struct Instance {

  template <typename Logger>
  static void output(Logger &logger, Self const &value) {
    static_assert(false, "No instance for Debug typeclass");
  }
};

template <typename Logger, typename A>
void output(Logger &logger, A const &value) {
  Instance<A>::output(logger, value);
}

template <size_t N> struct Instance<char[N]> {
  using Self = char[N];
  template <typename Logger>
  static void output(Logger &logger, Self const &value) {
    logger::write_string_literal(logger, value);
  }
};

template <> struct Instance<int> {
  using Self = int;
  template <typename Logger>
  static void output(Logger &logger, Self const &value) {
    logger::write_int32(logger, value);
  }
};

template <> struct Instance<std::string> {
  using Self = std::string;
  template <typename Logger>
  static void output(Logger &logger, Self const &value) {
    logger::write_string(logger, value);
  }
};
template <> struct Instance<double> {
  using Self = double;
  template <typename Logger>
  static void output(Logger &logger, Self const &value) {
    logger::write_float64(logger, value);
  }
};

template <typename... Ts> struct Instance<std::tuple<Ts...>> {
  using Self = std::tuple<Ts...>;
  template <typename Logger>
  static void output(Logger &logger, Self const &value) {

    logger::write_string(logger, "(");

    std::apply(
        [&](auto const &fst, auto const &...rest) {
          self::output(logger, fst);
          (((logger::write_string(logger, ", "), self::output(logger, rest))),
           ...);
        },
        value);

    logger::write_string(logger, ")");
  }
};

template <typename A> struct Instance<std::vector<A>> {

  using Self = std::vector<A>;
  template <typename Logger>
  static void output(Logger &logger, Self const &value) {
    logger::write_string(logger, "[");
    auto it = value.begin();

    if (it != value.end()) {
      self::output(logger, *it);
      ++it;
    }

    for (; it != value.end(); ++it) {
      logger::write_string(logger, ", ");
      self::output(logger, *it);
    }

    logger::write_string(logger, "]");
  }
};
template <typename A> struct Instance<std::optional<A>> {
  using Self = std::optional<A>;
  template <typename Logger>
  static void output(Logger &logger, Self const &value) {
    if (value.has_value()) {
      self::output(logger, "Some(");
      self::output(logger, value.value());
      self::output(logger, ")");
    } else {
      self::output(logger, "None");
    }
  }
};
template <> struct Instance<bool> {
  using Self = bool;
  template <typename Logger>
  static void output(Logger &logger, Self const &value) {
    if (value) {
      logger::write_string_literal(logger, "true");
    } else {
      logger::write_string_literal(logger, "false");
    }
  }
};

} // namespace debug

namespace eq {
namespace self = ::eq;
template <typename Self> struct Instance {
  static bool equals(Self const &a, Self const &b) {
    static_assert(false, "No instance for Eq typeclass");
    return std::declval<Self>();
  }
};

template <typename Self> bool equals(Self const &a, Self const &b) {
  return Instance<Self>::equals(a, b);
}

template <> struct Instance<int32_t> {
  using Self = int32_t;
  static bool equals(Self const &a, Self const &b) { return a == b; }
};
template <> struct Instance<float64_t> {
  using Self = float64_t;
  static bool equals(Self const &a, Self const &b) { return a == b; }
};

template <> struct Instance<std::string> {
  using Self = std::string;
  static bool equals(Self const &a, Self const &b) { return a == b; }
};

template <typename A> struct Instance<std::vector<A>> {
  using Self = std::vector<A>;
  static bool equals(Self const &a, Self const &b) {
    return a.size() == b.size() &&
           std::equal(
               a.begin(), a.end(), b.begin(), b.end(),
               [](auto const &a, auto const &b) { return self::equals(a, b); });
  }
};
template <typename A> struct Instance<std::shared_ptr<A>> {
  using Self = std::shared_ptr<A>;
  static bool equals(Self const &a, Self const &b) { return *a == *b; }
};
} // namespace eq

namespace prelude {
using ::debug::output;
using ::eq::equals;
using ::logger::write_float64;
using ::logger::write_int32;
using ::logger::write_string;
namespace self = ::prelude;

template <typename A> void println(A const &value) {
  output(std::cout, value);
  output(std::cout, "\n");
}
template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
} // namespace prelude

struct TypedHole {
  template <typename A> operator A() const { std::declval<A>(); }
};

const TypedHole _ = TypedHole{};