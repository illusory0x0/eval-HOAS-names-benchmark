#![allow(dead_code)]
#![allow(non_snake_case)]
#![allow(unused_imports)]

use std::fmt::Debug;
use std::rc::Rc;

type Name = Rc<String>;

type TmRef = Rc<Tm>;
type ValRef = Rc<Val>;

type Closure = Rc<dyn Fn(ValRef) -> ValRef>;

enum Tm {
    Var(Name),
    Lam(Name, TmRef),
    App(TmRef, TmRef),
}

enum Val {
    VVar(Name),
    VApp(ValRef, ValRef),
    VLam(Name, Closure),
}

enum List<A> {
    Nil,
    Cons(A, Rc<List<A>>),
}

use expect_test::expect;

use crate::List::*;

type Env = Rc<List<(Name, ValRef)>>;

impl<A: PartialEq, B: Clone> List<(A, B)> {
    fn lookup(&self, key: &A) -> Option<B> {
        match self {
            Nil => None,
            Cons((x, y), _) if x == key => Some(y.clone()),
            Cons(_, xs) => xs.lookup(key),
        }
    }
}

impl<A: PartialEq> List<A> {
    fn containes(&self, key: &A) -> bool {
        match self {
            Nil => false,
            Cons(x, _) if x == key => true,
            Cons(_, xs) => xs.containes(key),
        }
    }
}

impl<A: Clone> List<A> {
    fn map<F, B>(xs: Rc<List<A>>, f: F) -> List<B>
    where
        F: Fn(A) -> B,
    {
        match *xs {
            Nil => Nil,
            Cons(ref x, ref xs) => Cons(f(x.clone()), Rc::new(List::map(xs.clone(), f))),
        }
    }
}

use crate::Tm::*;
use crate::Val::*;

fn vApp(t: ValRef, u: ValRef) -> ValRef {
    match &*t {
        VLam(_, t) => t(u),
        _ => Rc::new(VApp(t, u)),
    }
}

fn eval(tm: TmRef, env: Env) -> ValRef {
    match &*tm {
        Var(x) => env.lookup(x).unwrap(),
        App(t, u) => vApp(eval(t.clone(), env.clone()), eval(u.clone(), env.clone())),
        Lam(x, t) => {
            let x_ = x.clone();
            let t_ = t.clone();
            let env_ = env.clone();
            Rc::new(VLam(
                x.clone(),
                Rc::new(move |u| {
                    let env = Rc::new(Cons((x_.clone(), u), env_.clone()));
                    eval(t_.clone(), env)
                }),
            ))
        }
    }
}

fn quote(val: ValRef, ns: Rc<List<Name>>) -> TmRef {
    match &*val {
        VVar(x) => Rc::new(Var(x.clone())),
        VApp(t, u) => Rc::new(App(
            quote(t.clone(), ns.clone()),
            quote(u.clone(), ns.clone()),
        )),
        VLam(x, t) => Rc::new(Lam(
            fresh(&*ns, x.clone()),
            quote(t(Rc::new(VVar(x.clone()))), ns.clone()),
        )),
    }
}

fn fresh(ns: &List<Name>, x: Name) -> Name {
    if *x == "_" {
        x
    } else {
        if ns.containes(&x) {
            let x = Rc::new((*x).clone() + "'");
            fresh(ns, x)
        } else {
            x
        }
    }
}
fn nf(tm: TmRef, env: Env) -> TmRef {
    let v = eval(tm, env.clone());
    let ns = Rc::new(List::map(env, |x| x.0));
    quote(v, ns)
}

fn var(x: &str) -> TmRef {
    Rc::new(Var(Rc::new(String::from(x))))
}
fn app(t: TmRef, u: TmRef) -> TmRef {
    Rc::new(App(t, u))
}
fn lam(x: &str, t: TmRef) -> TmRef {
    Rc::new(Lam(Rc::new(String::from(x)), t))
}

impl Debug for Tm {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match &*self {
            Var(name) => write!(f, "{}", *name),
            Lam(x, t) => {
                write!(f, "(fun {} -> {:?}))", *x, *t)
            }
            App(t, u) => write!(f, "({:?} {:?})", *t, *u),
        }
    }
}

#[test]
fn test_example() {
    let nil: Env = Rc::new(Nil);

    let five = lam(
        "f",
        lam(
            "x",
            app(
                var("f"),
                app(
                    var("f"),
                    app(var("f"), app(var("f"), app(var("f"), var("x")))),
                ),
            ),
        ),
    );
    let add = lam(
        "m",
        lam(
            "n",
            lam(
                "f",
                lam(
                    "x",
                    app(
                        app(var("m"), var("f")),
                        app(app(var("n"), var("f")), var("x")),
                    ),
                ),
            ),
        ),
    );

    let ten = app(app(add, five.clone()), five.clone());
    let ten = nf(ten.clone(), nil.clone());

    let ten_actual = format!("{ten:?}");
    let ten_expected = expect!["(fun f -> (fun x -> (f (f (f (f (f (f (f (f (f (f x))))))))))))))"];
    ten_expected.assert_eq(&ten_actual);

    let mult = lam(
        "m",
        lam("n", lam("f", app(var("m"), app(var("n"), var("f"))))),
    );

    let num_25 = app(app(mult, five.clone()), five.clone());
    let num_25 = nf(num_25.clone(), nil.clone());

    let num_25_actual = format!("{num_25:?}");
    let num_25_expected = expect![
        "(fun f -> (fun x -> (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f x)))))))))))))))))))))))))))))"
    ];
    num_25_expected.assert_eq(&num_25_actual);
}

fn consume(x: TmRef) {
    let s = match *x {
        Lam(..) => "lam",
        Var(..) => "var",
        App(..) => "app",
    };
    println!("{s}");
}

fn main() {
    let times = 1024;
    let nil: Env = Rc::new(Nil);

    let five = lam(
        "f",
        lam(
            "x",
            app(
                var("f"),
                app(
                    var("f"),
                    app(var("f"), app(var("f"), app(var("f"), var("x")))),
                ),
            ),
        ),
    );
    let add = lam(
        "m",
        lam(
            "n",
            lam(
                "f",
                lam(
                    "x",
                    app(
                        app(var("m"), var("f")),
                        app(app(var("n"), var("f")), var("x")),
                    ),
                ),
            ),
        ),
    );

    let add5 = app(add, five.clone());

    for _ in 0..1000 {
        let mut tm = five.clone();

        for _ in 0..times {
            tm = app(add5.clone(), tm.clone());
        }

        let result = nf(tm, nil.clone());
        consume(result);
    }
}
