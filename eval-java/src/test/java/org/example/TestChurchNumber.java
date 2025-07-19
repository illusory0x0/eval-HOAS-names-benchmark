package org.example;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.example.Eval.*;

public class TestChurchNumber {
    @Test
    public void clauses() {
        var ten = nf(App(App(add, five), five), List.nil());
        var num_25 = nf(App(App(mult, five), five), List.nil());

        var ten_expected = "(fun f -> (fun x -> (f (f (f (f (f (f (f (f (f (f x))))))))))))";
        var num_25_expected = "(fun f -> (fun x -> (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f (f x)))))))))))))))))))))))))))";

        assertEquals(ten_expected, Eval.toString(ten));
        assertEquals(num_25_expected, Eval.toString(num_25));
    }
}
