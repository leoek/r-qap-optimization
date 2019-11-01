import bindings from "bindings";
import cluster from "cluster";
import getParametersFromArgs from "./lib/parameterParser";
import { getPerformaceTools, sleep, objectValues, compose } from "./helpers";
import createRQAPParser from "./lib/rqapParser";
import createQAPParser from "./lib/qapParser";

const addon = bindings("nativeaddon");
const agentaddon = bindings("agentaddon");
const { performance } = getPerformaceTools();

const qapParser = createQAPParser();
const rqapParser = createQAPParser();
const parameters = getParametersFromArgs();
console.log("Parameters", parameters);

const main = async () => {
  /**
   * @typedef nativeInstance
   * @property factories
   * @property machines
   * @property flowMatrix
   * @property changeOverMatrix
   * @property distanceMatrix
   * 
   * @type nativeInstance
   */
  let instance;
  try {
    instance = await qapParser.fileToNativeInstance({
      name: "nug12" || parameters.instanceName
    });
  } catch (error) {
    console.error("Could not create problem instance", error);
    throw error;
  }

  const {
    factories,
    machines,
    flowMatrix,
    changeOverMatrix,
    distanceMatrix
  } = instance;
  const agent = new agentaddon.Agent(
    factories,
    machines,
    flowMatrix,
    changeOverMatrix,
    distanceMatrix
  );
  const solution = agent.createSolution();
  //console.log(solution);

  let createdSolutions = 0;
  const workerCount = 20;
  const start = performance.now();

  let best = null;

  let executionDoneCallback = () => undefined;
  const executionDonePromise = new Promise(resolve => {
    executionDoneCallback = resolve;
  })

  if (cluster.isMaster){

    console.log(`Master ${process.pid} is running`);
    const createdWorkerSolutions = Array(workerCount).fill(0);

    for (let i=0; i < workerCount; i++){
      cluster.fork();
    }

    cluster.on('exit', (worker, code, signal) => {
      console.log("MASTER",`worker ${worker.process.pid} died`);
      console.log("MASTER:", { createdSolutions, quality: best.quality, workersLeft: objectValues(cluster.workers).length });
      if (objectValues(cluster.workers).length === 0){
        executionDoneCallback()
      }
    });

    // Receive Solutions from Workers
    const messageHandler = (msg) => {
      if (msg.cmd && msg.cmd.createdSolutions){
        createdWorkerSolutions[msg.cmd.workerId] = msg.cmd.createdSolutions
        createdSolutions = createdWorkerSolutions.reduce((sum, val) => sum += val, 0);
      }
      if (msg.cmd && msg.cmd.solution) {
        if (!best || best.quality > msg.cmd.solution.quality){
          best = msg.cmd.solution
        }
      }
      //console.log("MASTER", { quality: best.quality, createdSolutions, permutation: best.permutation })
    }

    for (const id in cluster.workers) {
      cluster.workers[id].on('message', messageHandler);
    }
  } else {
    console.log(`Worker ${process.pid} started`);

    let i = 0;
    while (++i < 1000000000) {
      const solution = { quality: 1, permutation: []}
      agent.createSolution();
      if (!best || best.quality > solution.quality){
        best = solution
        //console.log("WORKER", { cmd: { quality: solution.quality, workerId: cluster.worker.id, i } })
        process.send({ cmd: { solution, workerId: cluster.worker.id, createdSolutions: i } });
      }
    }
    createdSolutions = i;
    process.send({ cmd: { solution: best, workerId: cluster.worker.id, createdSolutions } });
    executionDoneCallback();
    process.exit(0)
  }

  console.log("MASTER", "waiting for execution promise to resolve")
  await executionDonePromise;
  console.log("MASTER", "execution promise resolved")
  /*

  const sol = new agentaddon.Solution([1,2,3,4,5], 9.78);
  sol.add(4);

  console.log("\n\n")
  const factory = new agentaddon.Factory(1, 2, 1, 1);
  console.log(factory.useCapacity(1));
  console.log(factory.useCapacity(1));
  console.log(factory.useCapacity(1));
  /*
  
  /*


  let current = 0;

  const callback = (err, result) => {
    console.log("err result", err, result, createdSolutions);
    createdSolutions += 1;
    current -= 1;
  };

  while (createdSolutions < parameters.maxSolutions){
    if (current < parameters.agents){
      current += 1;
      addon.createSolution(1, callback);
    } else {
      await sleep(1000)
    }
  }
  */

  const end = performance.now();

  const solved = true;
  const runtime = end - start;
  const runlength = createdSolutions;
  const seed = null;
  const bestSol = 0;
  console.log(`Human readable result \n`, {
    solved,
    runtime: `${runtime} ms`,
    runlength,
    bestSol
  });
  console.log(
    `Result for ParamILS: ${solved}, ${runtime}, ${runlength}, ${bestSol}, ${seed}`
  );

  //await sleep(100000)
};

try {
  main();
} catch (error) {
  console.error("Exception in main method", inspect(error));
}

//addon.test()
