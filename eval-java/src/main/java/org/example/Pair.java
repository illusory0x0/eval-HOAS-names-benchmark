package org.example;

import org.jetbrains.annotations.NotNull;

public record Pair<A, B>(@NotNull A fst, @NotNull B snd) {
    public static <A, B> Pair<A, B> pair(@NotNull A fst, @NotNull B snd) {
        return new Pair<>(fst, snd);
    }
}
