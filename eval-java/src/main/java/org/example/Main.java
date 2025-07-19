package org.example;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.example.Pair;

import java.util.Optional;
import java.util.function.Function;
import java.util.function.Predicate;

import static org.example.Eval.*;

class Eval {
    sealed interface Tm permits Var, Lam, App {

    }

    record Var(@NotNull String name) implements Tm {
    }

    record Lam(@NotNull String name, @NotNull Tm body) implements Tm {
    }

    record App(@NotNull Tm fun, @NotNull Tm arg) implements Tm {
    }

    sealed interface Val permits VVar, VApp, VLam {

    }

    record VVar(@NotNull String name) implements Val {
    }

    record VApp(@NotNull Val fun, @NotNull Val arg) implements Val {
    }

    record VLam(@NotNull String name, @NotNull Function<Val, Val> closure) implements Val {
    }

    static Tm Var(@NotNull String name) {
        return new Var(name);
    }

    static Tm Lam(@NotNull String name, @NotNull Tm body) {
        return new Lam(name, body);
    }

    static Tm App(@NotNull Tm fun, @NotNull Tm arg) {
        return new App(fun, arg);
    }

    static Val VVar(@NotNull String name) {
        return new VVar(name);
    }

    static Val VApp(@NotNull Val fun, @NotNull Val arg) {
        return new VApp(fun, arg);
    }

    static Val VLam(@NotNull String name, @NotNull Function<Val, Val> closure) {
        return new VLam(name, closure);
    }

    static String fresh(@NotNull List<String> ns, @NotNull String x) {
        if (x.equals("_")) {
            return "_";
        } else {
            if (List.contains(ns, x)) {
                return fresh(ns, x + "'");
            } else {
                return x;
            }
        }
    }

    static Val vApp(@NotNull Val t, @NotNull Val u) {
        switch (t) {
            case VLam(var _, var t_) -> {
                return t_.apply(u);
            }
            default -> {
                return VApp(t, u);
            }
        }
    }

    static @NotNull Val eval(@NotNull Tm tm, @NotNull List<Pair<String, Val>> env) {
        switch (tm) {
            case App(var t, var u) -> {
                return vApp(eval(t, env), eval(u, env));
            }
            case Lam(var x, var t) -> {
                return VLam(x, u -> eval(t, List.cons(Pair.pair(x, u), env)));
            }
            case Var(var x) -> {
                return List.lookup(env, x).get();
            }
        }
    }

    static @NotNull Tm quote(@NotNull Val val, @NotNull List<String> ns) {
        switch (val) {
            case VVar(var x) -> {
                return Var(x);
            }
            case VApp(var t, var u) -> {
                return App(quote(t, ns), quote(u, ns));
            }
            case VLam(var x, var t) -> {
                var x_ = fresh(ns, x);
                return Lam(x_, quote(t.apply(VVar(x_)), List.cons(x_, ns)));
            }
        }
    }

    static void output(@NotNull Tm tm, @NotNull StringBuilder sb) {
        switch (tm) {
            case Var(var x) -> sb.append(x);

            case Lam(var x, var t) -> {
                sb.append("(fun ");
                sb.append(x);
                sb.append(" -> ");
                output(t, sb);
                sb.append(")");
            }
            case App(var t, var u) -> {
                sb.append("(");
                output(t, sb);
                sb.append(" ");
                output(u, sb);
                sb.append(")");
            }
        }
    }

    static Tm nf(@NotNull Tm tm, @NotNull List<Pair<String, Val>> env) {
        return quote(eval(tm, env), List.map(env, Pair::fst));
    }

    static String toString(@NotNull Tm tm) {
        var sb = new StringBuilder();
        output(tm, sb);
        return sb.toString();
    }

    static final @NotNull Tm five = Lam("f", Lam("x", App(Var("f"), App(Var("f"), App(Var("f"), App(Var("f"), App(Var("f"), Var("x"))))))));
    static final @NotNull Tm add = Lam("m", Lam("n", Lam("f", Lam("x", App(App(Var("m"), Var("f")), App(App(Var("n"), Var("f")), Var("x")))))));
    static final @NotNull Tm mult = Lam("m", Lam("n", Lam("f", App(Var("m"), App(Var("n"), Var("f"))))));
}


public class Main {
    static void consume(@NotNull Eval.Tm tm) {
        switch (tm) {
            case Eval.Var(var x) -> System.out.println("var");
            case Eval.Lam(var x, var t) -> System.out.println("lam");
            case Eval.App(var t, var u) -> System.out.println("app");
        }
    }

    public static void main(@NotNull String[] args) {
        var add5 = App(add, five);
        var times = 8192;
        var tm = five;

        for (int i = 0; i < times; i++) {
            tm = App(add5, tm);
        }
        var result = nf(tm, List.nil());
        consume(result);
    }
}