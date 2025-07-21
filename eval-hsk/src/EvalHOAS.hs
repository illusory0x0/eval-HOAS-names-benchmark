module EvalHOAS where

import System.CPUTime (getCPUTime)
import Control.Monad (replicateM, replicateM_)

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

data List a = Nil | Cons a (List a)
  deriving (Eq, Show)

type Env = List (Name, Val)

lookupEnv :: Eq a => a -> List (a, b) -> Maybe b
lookupEnv _ Nil = Nothing
lookupEnv key (Cons (k, v) xs)
  | key == k  = Just v
  | otherwise = lookupEnv key xs

containes :: Eq a => a -> List a -> Bool
containes _ Nil = False
containes x (Cons y ys)
  | x == y    = True
  | otherwise = containes x ys

mapList :: (a -> b) -> List a -> List b
mapList _ Nil = Nil
mapList f (Cons x xs) = Cons (f x) (mapList f xs)

vApp :: Val -> Val -> Val
vApp (VLam _ f) u = f u
vApp f u = VApp f u

eval :: Tm -> Env -> Val
eval (Var x) env =
  case lookupEnv x env of
    Just v  -> v
    Nothing -> error ("Unbound variable: " ++ x)
eval (App t u) env = vApp (eval t env) (eval u env)
eval (Lam x t) env = VLam x (\u -> eval t (Cons (x, u) env))

fresh :: Name -> List Name -> Name
fresh "_" _ = "_"
fresh x ns
  | containes x ns = fresh (x ++ "'") ns
  | otherwise = x

quote :: Val -> List Name -> Tm
quote (VVar x) _ = Var x
quote (VApp f x) ns = App (quote f ns) (quote x ns)
quote (VLam x f) ns =
  let x' = fresh x ns
  in Lam x' (quote (f (VVar x')) (Cons x' ns))

nf :: Tm -> Env -> Tm
nf tm env = quote (eval tm env) (mapList fst env)

var :: String -> Tm
var = Var

app :: Tm -> Tm -> Tm
app = App

lam :: String -> Tm -> Tm
lam = Lam

-- Timing helper

timeIt :: String -> IO a -> IO a
timeIt label action = do
  start <- getCPUTime
  result <- action
  end <- getCPUTime
  let durationPs = end - start
  putStrLn $ label ++ " took " ++ show durationPs ++ " ps"
  return result

benchmarkWithWarmup :: Int -> String -> IO () -> IO ()
benchmarkWithWarmup runs label action = do
  putStrLn $ "\nWarming up (" ++ show warmupRuns ++ " runs)..."
  replicateM_ warmupRuns action

  times <- replicateM runs $ do
    start <- getCPUTime
    action
    end <- getCPUTime
    return (end - start)

  let avg = sum times `div` fromIntegral runs
  putStrLn $ label ++ " average over " ++ show runs ++ " runs: " ++ show avg ++ " ps"
  where
    warmupRuns = 10


-- Multi-run benchmarking
multiRunAdd :: Int -> Int -> Tm -> Tm -> Env -> IO ()
multiRunAdd rounds depth base addFn env =
  let buildNested 0 tm = tm
      buildNested k tm = buildNested (k - 1) (app addFn tm)
      loop 0 = return ()
      loop n = do
        let expr = buildNested depth base
        let result = nf expr env
        let _ = result  -- force evaluation
        loop (n - 1)
  in timeIt ("Running " ++ show rounds ++ " rounds of add5 depth " ++ show depth) (loop rounds)

