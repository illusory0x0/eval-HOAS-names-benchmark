package org.example;

import org.jetbrains.annotations.Contract;
import org.jetbrains.annotations.NotNull;

public record Pair<A, B>(@NotNull A fst, @NotNull B snd) {
    @Contract("_, _ -> new")
    public static <A, B> @NotNull Pair<A, B> pair(@NotNull A fst, @NotNull B snd) {
        return new Pair<>(fst, snd);
    }
}
