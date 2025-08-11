import * as E from "./eval";

let consume = (x: E.Tm) => {
  if (x.tag === "Var") {
    console.log("Var");
  } else if (x.tag === "App") {
    console.log("App");
  } else {
    console.log("Lam");
  }
};

const main = () => {
  let times = 1024;

  let five = E.five;
  let add = E.add;

  let add5 = E.App(add, five);

  for (let i = 0; i < 1000; ++i) {
    let tm = five;

    for (let j = 0; j < times; ++j) {
      tm = E.App(add5, tm);
    }
    let result = E.nf(tm, null);
    consume(result);
  }
};

main();
