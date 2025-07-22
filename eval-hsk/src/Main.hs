-- Main.hs - Run tests for EvalHOAS

module Main where

import EvalHOAS

-- Empty environment
nil :: Env
nil = Nil

-- Church numeral 5
five :: Tm
five = lam "f" $ lam "x" $ 
          app (var "f") $ 
          app (var "f") $ 
          app (var "f") $ 
          app (var "f") $ 
          app (var "f") (var "x")

-- Church addition
add :: Tm
add = lam "m" $ lam "n" $ 
        lam "f" $ lam "x" $ 
          app (app (var "m") (var "f")) 
              (app (app (var "n") (var "f")) (var "x"))

-- Church multiplication
mult :: Tm
mult = lam "m" $ lam "n" $ 
          lam "f" $ 
            app (var "m") (app (var "n") (var "f"))

-- Nested addN (add 5 applied n times)
makeNestedAdd :: Int -> Tm
makeNestedAdd n = go n five
  where
    add5 = app add five
    go 0 tm = tm
    go k tm = go (k - 1) (app add5 tm)

-- Consume prints term constructor
consume :: Tm -> String
consume (Var _) = "var"
consume (Lam _ _) = "lam"
consume (App _ _) = "app"

-- Helpers for printing
printTm :: Tm -> IO ()
printTm = print

main :: IO ()
main = do
  putStrLn "Evaluating 5 + 5:"
  let ten = nf (app (app add five) five) nil
  printTm ten

  putStrLn "Evaluating 5 * 5:"
  let twentyFive = nf (app (app mult five) five) nil
  printTm twentyFive

  benchmarkWithWarmup 20 "add5 applied 1000 times" $ do
    let tm = makeNestedAdd 1000
    let result = nf tm nil
    let _ = result 
    return ()

  putStrLn "\nRunning 100 rounds of add5 depth 500:"
  multiRunAdd 100 500 five (app add five) nil

