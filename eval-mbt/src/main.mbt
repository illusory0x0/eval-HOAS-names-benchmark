///|
typealias String as Name

///|
enum List[A] {
  Nil
  Cons(A, List[A])
}

///|
enum Tm {
  /// x
  Var(Name)
  /// fun x -> t
  Lam(Name, Tm)
  /// t u
  App(Tm, Tm)
}

///|
typealias List[(Name, Val)] as Env

///|
enum Val {
  VVar(Name)
  VApp(Val, Val)
  VLam(Name, (Val) -> Val)
}

///|
fn[A : Eq] List::contains(self : List[A], key : A) -> Bool {
  loop self {
    Nil => false
    Cons(x, _) if x == key => true
    Cons(_, xs) => continue xs
  }
}

///|
fn[A, B] List::map(self : List[A], f : (A) -> B) -> List[B] {
  match self {
    Nil => Nil
    Cons(x, xs) => Cons(f(x), List::map(xs, f))
  }
}

///|
fn fresh(ns : List[Name], x : Name) -> Name {
  loop x {
    "_" => "_"
    _ as x if ns.contains(x) => continue x + "'"
    _ as x => x
  }
}

///|
fn[A : Eq, B] lookup(self : List[(A, B)], key : A) -> B? {
  loop self {
    Nil => None
    Cons((x, y), _) if x == key => Some(y)
    Cons(_, xs) => continue xs
  }
}

///|
fn vApp(t : Val, u : Val) -> Val {
  match t {
    VLam(_, t) => t(u)
    _ => VApp(t, u)
  }
}

///|
fn eval(tm : Tm, env : Env) -> Val {
  match tm {
    App(t, u) => vApp(eval(t, env), eval(u, env))
    Lam(x, t) => VLam(x, u => eval(t, Cons((x, u), env)))
    Var(x) => env.lookup(x).unwrap()
  }
}

///|
fn quote(v : Val, ns : List[Name]) -> Tm {
  match v {
    VApp(t, u) => App(quote(t, ns), quote(u, ns))
    VVar(x) => Var(x)
    VLam(x, t) => {
      // eta expansion
      let x = fresh(ns, x)
      let t = quote(t(VVar(x)), Cons(x, ns))
      Lam(x, t)
    }
  }
}

///|
fn nf(tm : Tm, env : Env) -> Tm {
  let v = eval(tm, env)
  let ns = env.map(x => x.0)
  quote(v, ns)
}

///|
test "ex2" {
  let nil : Env = Nil

  // \ f x. f(f(f(f(f x))))
  let five = Lam(
    "f",
    Lam(
      "x",
      App(
        Var("f"),
        App(Var("f"), App(Var("f"), App(Var("f"), App(Var("f"), Var("x"))))),
      ),
    ),
  )

  // \ m n f x. m f (n f x)
  let add = Lam(
    "m",
    Lam(
      "n",
      Lam(
        "f",
        Lam(
          "x",
          App(App(Var("m"), Var("f")), App(App(Var("n"), Var("f")), Var("x"))),
        ),
      ),
    ),
  )

  // \ m n. m (n f)
  let mult = Lam(
    "m",
    Lam("n", Lam("f", App(Var("m"), App(Var("n"), Var("f"))))),
  )

  // \ n f x. f (n f x)
  let succ = Lam(
    "n",
    Lam(
      "f",
      Lam(
        "x",
        {
          let n = App(App(Var("n"), Var("f")), Var("x")) // apply f to x, n times
          App(Var("f"), n) // apply f to x, (n + 1) times
        },
      ),
    ),
  )
  // \f x. x 
  let zero = Lam("f", Lam("x", Var("x"))) // apply f to x, 0 times
  let one = Lam("f", Lam("x", App(Var("f"), Var("x")))) // appl f to x. 1 times
  inspect(zero, content="(fun f -> (fun x -> x))")
  inspect(nf(zero, nil), content="(fun f -> (fun x -> x))")
  inspect(one, content="(fun f -> (fun x -> (f x)))")
  inspect(nf(one, nil), content="(fun f -> (fun x -> (f x)))")
  inspect(succ, content="(fun n -> (fun f -> (fun x -> (f ((n f) x)))))")
  let one = nf(App(succ, zero), nil)
  inspect(one, content="(fun f -> (fun x -> (f x)))")
  let ten = nf(App(App(add, five), five), nil)
  inspect(
    ten,
    content="(fun f -> (fun x -> (f (f (f (f (f (f (f (f (f (f x))))))))))))",
  )
  let num_25 = nf(App(App(mult, five), five), nil)
  inspect(
    num_25,
    content="(fun f -> (fun x -> (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f x)))))))))))))))))))))))))))",
  )
}

///|
impl Show for Tm with output(self, logger) {
  match self {
    App(t, u) => {
      logger.write_char('(')
      logger.write_object(t)
      logger.write_char(' ')
      logger.write_object(u)
      logger.write_char(')')
    }
    Lam(x, t) => {
      logger.write_string("(fun ")
      logger.write_string(x)
      logger.write_string(" -> ")
      logger.write_object(t)
      logger.write_char(')')
    }
    Var(name) => logger.write_string(name)
  }
}

///|
fn consume(x : Tm) -> Unit {
  let s = match x {
    Lam(_) => "lam"
    Var(_) => "var"
    App(_) => "app"
  }
  println(s)
}

///|
fn main {
  let times = 1024
  let five = Lam(
    "f",
    Lam(
      "x",
      App(
        Var("f"),
        App(Var("f"), App(Var("f"), App(Var("f"), App(Var("f"), Var("x"))))),
      ),
    ),
  )
  let add = Lam(
    "m",
    Lam(
      "n",
      Lam(
        "f",
        Lam(
          "x",
          App(App(Var("m"), Var("f")), App(App(Var("n"), Var("f")), Var("x"))),
        ),
      ),
    ),
  )
  let add5 = App(add, five)
  for _ in 0..<1000 {
    let mut tm = five
    for _ in 0..<times {
      tm = App(add5, tm)
    }
    // Stack overflow in MoonBit JavaScript backend
    // Requires v8 JIT optimization to avoid stack overflow.
    let result = nf(tm, Nil)
    consume(result)
  }
}
