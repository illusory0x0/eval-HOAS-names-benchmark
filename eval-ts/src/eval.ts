type Name = string;

type List<a> = {
  head: a;
  tail: List<a>;
} | null;

type Tm = Tm_Var | Tm_Lam | Tm_App;

type Tm_Var = { tag: "Var"; name: Name };

type Tm_Lam = { tag: "Lam"; parm: Name; body: Tm };

type Tm_App = { tag: "App"; fun: Tm; arg: Tm };

type Val = Val_VVar | Val_VLam | Val_VApp;

type Val_VVar = { tag: "VVar"; name: Name };

type Val_VApp = { tag: "VApp"; fun: Val; arg: Val };

type Val_VLam = { tag: "VLam"; parm: Name; closure: (x: Val) => Val };

type Env = List<[Name, Val]>;

let Cons = <a>(x: a, xs: List<a>): List<a> => {
  return { head: x, tail: xs };
};

let Lam = (parm: Name, body: Tm): Tm => {
  return {
    tag: "Lam",
    parm: parm,
    body: body,
  };
};
let App = (fun: Tm, arg: Tm): Tm => {
  return {
    tag: "App",
    fun: fun,
    arg: arg,
  };
};
let Var = (name: Name): Tm => {
  return {
    tag: "Var",
    name: name,
  };
};
let VVar = (name: Name): Val => {
  return {
    tag: "VVar",
    name: name,
  };
};
let VLam = (parm: Name, closure: (x: Val) => Val): Val => {
  return {
    tag: "VLam",
    parm: parm,
    closure: closure,
  };
};
let VApp = (fun: Val, arg: Val): Val => {
  return {
    tag: "VApp",
    fun: fun,
    arg: arg,
  };
};

// Warning: this funtion without Eq bound
let list_contains = <a>(xs: List<a>, key: a): boolean => {
  if (xs === null) {
    return false;
  } else {
    if (xs.head === key) {
      return true;
    } else {
      return list_contains(xs.tail, key);
    }
  }
};

let list_map = <a, b>(xs: List<a>, f: (x: a) => b): List<b> => {
  if (xs === null) {
    return null;
  } else {
    return Cons(f(xs.head), list_map(xs.tail, f));
  }
};

let list_lookup = <a, b>(xs: List<[a, b]>, key: a): b | null => {
  if (xs === null) {
    return null;
  } else {
    if (xs.head[0] === key) {
      return xs.head[1];
    } else {
      return list_lookup(xs.tail, key);
    }
  }
};

// This not the hot path
let fresh = (ns: List<Name>, x: Name): Name => {
  if (x === "_") {
    return "_";
  } else {
    if (list_contains(ns, x)) {
      return fresh(ns, x + "'");
    } else {
      return x;
    }
  }
};

let vApp = (t: Val, u: Val): Val => {
  if (t.tag === "VLam") {
    return t.closure(u);
  } else {
    return VApp(t, u);
  }
};

let tm_to_string = (tm: Tm): string => {
  if (tm.tag === "App") {
    return `(${tm_to_string(tm.fun)} ${tm_to_string(tm.arg)})`;
  } else if (tm.tag === "Lam") {
    return `(fun ${tm.parm} -> ${tm_to_string(tm.body)})`;
  } else {
    return tm.name;
  }
};

let five = Lam(
  "f",
  Lam(
    "x",
    App(
      Var("f"),
      App(Var("f"), App(Var("f"), App(Var("f"), App(Var("f"), Var("x")))))
    )
  )
);

let add = Lam(
  "m",
  Lam(
    "n",
    Lam(
      "f",
      Lam(
        "x",
        App(App(Var("m"), Var("f")), App(App(Var("n"), Var("f")), Var("x")))
      )
    )
  )
);
let mult = Lam("m", Lam("n", Lam("f", App(Var("m"), App(Var("n"), Var("f"))))));

let eval_ = (tm: Tm, env: Env): Val => {
  if (tm.tag === "Var") {
    return list_lookup(env, tm.name) as Val;
  } else if (tm.tag === "Lam") {
    // eta expansion
    return VLam(tm.parm, (x) =>
      eval_(tm.body, { head: [tm.parm, x], tail: env })
    );
  } else {
    return vApp(eval_(tm.fun, env), eval_(tm.arg, env));
  }
};

let quote = (val: Val, ns: List<Name>): Tm => {
  if (val.tag === "VVar") {
    return Var(val.name);
  } else if (val.tag === "VApp") {
    return App(quote(val.fun, ns), quote(val.arg, ns));
  } else {
    let parm = fresh(ns, val.parm);
    return Lam(parm, quote(val.closure(VVar(parm)), Cons(parm, ns)));
  }
};

let nf = (tm: Tm, env: Env): Tm => {
  return quote(
    eval_(tm, env),
    list_map(env, (x) => x[0])
  );
};

export {
  type Name,
  type List,
  type Tm,
  type Tm_Var,
  type Tm_Lam,
  type Tm_App,
  type Val,
  type Val_VVar,
  type Val_VLam,
  type Val_VApp,
  Lam,
  App,
  Var,
  VVar,
  VLam,
  VApp,
  list_contains,
  list_map,
  list_lookup,
  fresh,
  vApp,
  tm_to_string,
  five,
  add,
  mult,
  quote,
  eval_,
  nf
};
