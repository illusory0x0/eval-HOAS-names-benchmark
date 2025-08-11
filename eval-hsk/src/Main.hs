{-# LANGUAGE Strict #-}

module Main where

import Control.Monad (replicateM_)

type Name = String

data Tm
  = Var Name
  | Lam Name Tm
  | App Tm Tm
  deriving (Eq, Show)

data Val
  = VVar Name
  | VApp Val Val
  | VLam Name (Val -> Val)

type Env = [(Name, Val)]

five :: Tm
five =
  lam "f" $
    lam "x" $
      app (var "f") $
        app (var "f") $
          app (var "f") $
            app (var "f") $
              app (var "f") (var "x")

add :: Tm
add =
  lam "m" $
    lam "n" $
      lam "f" $
        lam "x" $
          app
            (app (var "m") (var "f"))
            (app (app (var "n") (var "f")) (var "x"))

mult :: Tm
mult =
  lam "m" $
    lam "n" $
      lam "f" $
        app (var "m") (app (var "n") (var "f"))

vApp :: Val -> Val -> Val
vApp (VLam _ f) u = f u
vApp f u = VApp f u

eval :: Tm -> Env -> Val
eval (Var x) env =
  case lookup x env of
    Just v -> v
    Nothing -> error ("Unbound variable: " ++ x)
eval (App t u) env = vApp (eval t env) (eval u env)
eval (Lam x t) env = VLam x (\u -> eval t ((x, u) : env))

fresh :: Name -> [Name] -> Name
fresh "_" _ = "_"
fresh x ns
  | x `elem` ns = fresh (x ++ "'") ns
  | otherwise = x

quote :: Val -> [Name] -> Tm
quote (VVar x) _ = Var x
quote (VApp f x) ns = App (quote f ns) (quote x ns)
quote (VLam x f) ns =
  let x' = fresh x ns
   in Lam x' (quote (f (VVar x')) (x' : ns))

nf :: Tm -> Env -> Tm
nf tm env = quote (eval tm env) (map fst env)

var :: String -> Tm
var = Var

app :: Tm -> Tm -> Tm
app = App

lam :: String -> Tm -> Tm
lam = Lam

consume :: Tm -> IO ()
consume (Lam _ _) = putStrLn "lam"
consume (Var _) = putStrLn "var"
consume (App _ _) = putStrLn "app"

main :: IO ()
main = do
  let times = 1024
  let add5 = app add five
  replicateM_ 1000 $ do
    -- let tm = foldl (\t _ -> app add5 t) five [1..times]
    let tm = iterate (app add5) five !! times
    let result = nf tm []
    consume result
