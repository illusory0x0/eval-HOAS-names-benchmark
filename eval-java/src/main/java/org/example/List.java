package org.example;

import org.jetbrains.annotations.NotNull;

import java.util.Optional;
import java.util.function.Function;

public sealed interface List<A> permits List.Nil, List.Cons {
    record Nil<A>() implements List<A> {
    }

    record Cons<A>(@NotNull A first, @NotNull List<A> rest) implements List<A> {
    }

    static <A> @NotNull List<A> nil() {
        return new Nil<>();
    }

    static <A> @NotNull List<A> cons(@NotNull A x, @NotNull List<A> xs) {
        return new Cons<>(x, xs);
    }

    static <A, B> @NotNull List<B> map(@NotNull List<A> self, @NotNull Function<A, B> f) {
        switch (self) {
            case Nil<A> _ -> {
                return nil();
            }
            case Cons<A>(var x, var xs) -> {
                return cons(f.apply(x), map(xs, f));
            }
        }
    }

    static <A extends Comparable<A>> boolean contains(@NotNull List<A> self, @NotNull A key) {
        switch (self) {
            case Nil<A> _ -> {
                return false;
            }
            case Cons<A>(A x, List<A> xs) -> {
                if (x.compareTo(key) == 0) {
                    return true;
                } else {
                    return contains(xs, key);
                }
            }
        }
    }

    static <A extends Comparable<A>, B> @NotNull Optional<B> lookup(@NotNull List<Pair<A, B>> self, @NotNull A key) {
        switch (self) {
            case Nil<Pair<A, B>> _ -> {
                return Optional.empty();
            }
            case Cons<Pair<A, B>>(var x, var xs) -> {
                if (x.fst().compareTo(key) == 0) {
                    return Optional.of(x.snd());
                } else {
                    return lookup(xs, key);
                }
            }
        }
    }
}