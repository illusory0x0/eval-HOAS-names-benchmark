#include "prelude.hxx"

template <typename A> using Rc = std::shared_ptr<A>;

template <typename F, typename R, typename... Args>
concept Fn = requires(F f, Args... args) {
  { f(args...) } -> std::same_as<R>;
};

namespace list {
template <typename A> struct Repr {
  A first;
  Rc<Repr<A>> rest;
  Repr() = delete;
  Repr(Repr const &) = default;
  Repr(Repr &&) = default;
  Repr &operator=(Repr const &) = delete;
  Repr &operator=(Repr &&) = delete;
  ~Repr() = default;
  explicit Repr(A const &x, Rc<Repr<A>> const &xs) : first(x), rest(xs) {}
};

template <typename A> using T = Rc<Repr<A>>;

template <typename A> Rc<Repr<A>> Cons(A const &x, Rc<Repr<A>> const &xs) {
  return std::make_shared<Repr<A>>(Repr{x, xs});
}
template <typename A> const Rc<Repr<A>> Nil = std::shared_ptr<Repr<A>>{nullptr};

template <typename B, typename A, Fn<B, A> F> T<B> map(T<A> const &xs, F f) {
  if (xs == Nil<A>) {
    return Nil<B>;
  } else {
    return Cons(f(xs->first), map<B>(xs->rest, f));
  }
}

template <typename A> bool contains(T<A> const &xs, A const &value) {
  if (xs == Nil<A>) {
    return false;
  } else {
    if (prelude::equals(xs->first, value)) {
      return true;
    } else {
      return contains(xs->rest, value);
    }
  }
}

template <typename A, typename B>
std::optional<B> lookup(T<std::tuple<A, B>> const &xs, A const &key) {
  if (xs == Nil<std::tuple<A, B>>) {
    return std::nullopt;
  } else {
    auto &x = std::get<0>(xs->first);
    auto &y = std::get<1>(xs->first);
    auto &rest = xs->rest;

    if (prelude::equals(x, key)) {
      return std::make_optional<B>(y);
    } else {
      return lookup(rest, key);
    }
  }
}

} // namespace list

namespace debug {
template <typename A> struct Instance<list::T<A>> {
  using Self = list::T<A>;
  template <typename Logger>
  static void output(Logger &logger, Self const &value) {
    if (value == list::Nil<A>) {
      self::output(logger, "[]");
    } else {
      self::output(logger, "[");
      self::output(logger, value->first);
      output_go(logger, value->rest);
      self::output(logger, "]");
    }
  }

private:
  template <typename Logger>
  static void output_go(Logger &logger, Self const &value) {
    if (value->rest != list::Nil<A>) {
      self::output(logger, ", ");
      self::output(logger, value->first);
      output_go(logger, value->rest);
    } else {
      // do nothing
    }
  }
};
} // namespace debug

namespace eval {

struct Tm;
using Name = Rc<std::string>;

struct Var {
  Name name;
};

struct Lam {
  Name name;
  Rc<Tm> body;
};

struct App {
  Rc<Tm> fun;
  Rc<Tm> arg;
};

struct Tm : std::variant<Var, Lam, App> {
  using std::variant<Var, Lam, App>::variant;
};

using TmRef = Rc<Tm>;
TmRef var(std::string name) {
  return std::make_shared<Tm>(
      Tm{Var{.name = std::make_shared<std::string>(std::move(name))}});
}

TmRef lam(std::string const &name, Rc<Tm> const &body) {
  return std::make_shared<Tm>(
      Tm{Lam{.name = std::make_shared<std::string>(name), .body = body}});
}

TmRef var(Name const &name) {
  return std::make_shared<Tm>(Tm{Var{.name = name}});
}

TmRef lam(Name const &name, Rc<Tm> const &body) {
  return std::make_shared<Tm>(Tm{Lam{.name = name, .body = body}});
}

TmRef app(Rc<Tm> const &fun, Rc<Tm> const &arg) {
  return std::make_shared<Tm>(Tm{App{.fun = fun, .arg = arg}});
}

struct Val;
using ValRef = Rc<Val>;

struct VVar {
  Name name;
};

using Closure = std::function<ValRef(ValRef)>;
struct VLam {
  Name name;
  Closure closure;
};

struct VApp {
  Rc<Val> fun;
  Rc<Val> arg;
};

struct Val : std::variant<VVar, VLam, VApp> {
  using std::variant<VVar, VLam, VApp>::variant;
};

ValRef vvar(std::string name) { // move argument
  return std::make_shared<Val>(
      Val{VVar{.name = std::make_shared<std::string>(std::move(name))}});
}

ValRef vlam(std::string const &name, Closure const &closure) {
  return std::make_shared<Val>(Val{
      VLam{.name = std::make_shared<std::string>(name), .closure = closure}});
}

ValRef vvar(Name const &name) {
  return std::make_shared<Val>(Val{VVar{.name = name}});
}

ValRef vlam(Name const &name, Closure closure) {
  return std::make_shared<Val>(Val{VLam{.name = name, .closure = closure}});
}

ValRef vapp(Rc<Val> const &fun, Rc<Val> const &arg) {
  return std::make_shared<Val>(Val{VApp{.fun = fun, .arg = arg}});
}

using Env = list::T<std::tuple<Name, ValRef>>;

ValRef vApp(ValRef const &t, ValRef const &u) {
  auto visitor = prelude::overloaded(
      [&](VLam const &lam) -> ValRef { return lam.closure(u); },
      [&](VVar const &_) -> ValRef { return vapp(t, u); },
      [&](VApp const &_) -> ValRef { return vapp(t, u); });
  return std::visit(visitor, *t);
}

ValRef eval(TmRef const &tm, Env const &env) {
  auto visitor = prelude::overloaded(
      [&](Lam const &value) -> ValRef {
        const auto &[x, t] = value;
        return vlam(
            x,
            [=](ValRef u)
                -> ValRef { // Clone all captured variables to ensure the lambda
                            // can safely outlive the current scope
              return eval(t, list::Cons(std::tuple{x, u}, env));
            });
      },
      [&](Var const &var) -> ValRef {
        return list::lookup(env, var.name).value();
      },
      [&](App const &app) -> ValRef {
        const auto &[t, u] = app;
        return vApp(eval(t, env), eval(u, env));
      });
  return std::visit(visitor, *tm);
}

Name fresh(list::T<Name> const &ns, Name const &x) {
  if (*x == "_") {
    return x;
  } else if (list::contains(ns, x)) {
    return fresh(ns, std::make_shared<std::string>(std::string{*x} + "'"));
  } else {
    return x;
  }
}
TmRef quote(ValRef const &val, list::T<Name> const &ns) {
  auto visitor = prelude::overloaded(
      [&](VVar const &value) -> TmRef { return var(value.name); },
      [&](VLam const &value) -> TmRef {
        const auto &[x, t] = value;
        auto x_ = fresh(ns, x);
        return lam(x_, quote(t(vvar(x_)), ns));
      },
      [&](VApp const &value) -> TmRef {
        const auto &[t, u] = value;
        return app(quote(t, ns), quote(u, ns));
      });
  return std::visit(visitor, *val);
}

TmRef nf(TmRef const &tm, Env const &env) {
  auto v = eval(tm, env);
  list::T<Name> ns = list::map<Name>(
      env, [](std::tuple<Name, ValRef> const &x) { return std::get<0>(x); });
  auto temp = quote(v, ns);
  return temp;
}

} // namespace eval
  // namespace eval

namespace debug {
template <> struct Instance<eval::Tm> {
  using Self = eval::Tm;
  template <typename Logger>
  static void output(Logger &logger, Self const &value) {
    using namespace eval;

    auto visitor = prelude::overloaded(
        [&](Var const &var) { self::output(logger, *var.name); },
        [&](Lam const &lam) {
          self::output(logger, "(fun ");
          self::output(logger, *lam.name);
          self::output(logger, " -> ");
          self::output(logger, *lam.body);
          self::output(logger, ")");
        },
        [&](App const &app) {
          self::output(logger, "(");
          self::output(logger, *app.fun);
          self::output(logger, " ");
          self::output(logger, *app.arg);
          self::output(logger, ")");
        });
    std::visit(visitor, value);
  }
};

} // namespace debug

void consume(eval::TmRef const &value) {
  using namespace eval;
  auto visitor =
      prelude::overloaded([&](Var const &var) { prelude::println("var"); },
                          [&](Lam const &lam) { prelude::println("lam"); },
                          [&](App const &app) { prelude::println("app"); });
  std::visit(visitor, *value);
}

void test_church_number() {
  using namespace prelude;

  using namespace eval;
  auto nil = list::Nil<std::tuple<Name, ValRef>>;
  auto five =
      lam("f",
          lam("x",
              app(var("f"),
                  app(var("f"),
                      app(var("f"), app(var("f"), app(var("f"), var("x"))))))));
  auto add =
      lam("m",
          lam("n",
              lam("f", lam("x", app(app(var("m"), var("f")),
                                    app(app(var("n"), var("f")), var("x")))))));
  auto mult =
      lam("m", lam("n", lam("f", app(var("m"), app(var("n"), var("f"))))));

  auto ten = nf(app(app(add, five), five), nil);
  auto num_25 = nf(app(app(mult, five), five), nil);

  auto tostr = [](TmRef const &tm) {
    std::stringstream ss;
    output(ss, *tm);
    return ss.str();
  };
  auto ten_actual = tostr(ten);
  auto num_25_actual = tostr(num_25);

  assert(ten_actual ==
         "(fun f -> (fun x -> (f (f (f (f (f (f (f (f (f (f x))))))))))))");
  assert(num_25_actual ==
         "(fun f -> (fun x -> (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f "
         "(f (f (f (f (f (f (f (f (f x)))))))))))))))))))))))))))");
}

int main() {
  using namespace prelude;
  using namespace eval;
  auto times = 1024;
  auto nil = list::Nil<std::tuple<Name, ValRef>>;
  auto five =
      lam("f",
          lam("x",
              app(var("f"),
                  app(var("f"),
                      app(var("f"), app(var("f"), app(var("f"), var("x"))))))));
  auto add =
      lam("m",
          lam("n",
              lam("f", lam("x", app(app(var("m"), var("f")),
                                    app(app(var("n"), var("f")), var("x")))))));

  auto add5 = app(add, five);

  for (int j = 0; j < 1000; ++j) {
    auto tm = five;
    for (int i = 0; i < times; ++i) {
      tm = app(add5, tm);
    }
    auto result = nf(tm, nil);
    consume(result);
  }
}
