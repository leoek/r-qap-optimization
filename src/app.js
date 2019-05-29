import bindings from "bindings";

const addon = bindings("nativeaddon");

console.log("test");

function printResult(type, pi, ms) {
  console.log(type, 'method:');
  console.log('\tπ ≈ ' + pi +
              ' (' + Math.abs(pi - Math.PI) + ' away from actual)');
  console.log('\tTook ' + ms + 'ms');
  console.log();
}

var calculations = 10000000000;

const runAsync = () => {
  var batches = 16;
  var ended = 0;
  var total = 0;
  var start = Date.now();

  const done = (err, result) => {
    total += result;

    // have all the batches finished executing?
    if (++ended === batches) {
      printResult('Async', total / batches, Date.now() - start);
    }
  }

  for (var i = 0; i < batches; i++) {
    addon.createSolution(calculations / batches, done);
  }

}

runAsync();

let flag = true;
let counter = 0;

setTimeout(() => { flag = false; console.log("done")}, 10000);

while(flag){
  counter = counter + Math.log(Math.sqrt(counter));
}