import bindings from "bindings";
import RQAPParser, {
  enhanceWithDistanceMatrix,
  toNativeInstance
} from "./lib/rqapParser";
import getParametersFromArgs from "./lib/parameterParser";
import { getPerformaceTools, sleep, objectValues, compose } from "./helpers";
import createRQAPParser from "./lib/rqapParser";

const addon = bindings("nativeaddon");
const agentaddon = bindings("agentaddon");
const { performance } = getPerformaceTools();

const parser = createRQAPParser();
const parameters = getParametersFromArgs();
console.log("Parameters", parameters);

const main = async () => {
  /**
   * @typedef nativeInstance
   * @property factories
   * @property machines
   * @property flowMatrix
   * @property changeOverMatrix
   * @propertydistanceMatrix
   * 
   * @type nativeInstance
   */
  let instance;
  try {
    instance = await parser.fileToNativeInstance({
      name: parameters.instanceName
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
  console.log(instance);
  console.log("\n\n");

  const agent = new agentaddon.Agent(
    factories,
    machines,
    flowMatrix,
    changeOverMatrix,
    distanceMatrix
  );
  const solution = agent.createSolution();

  console.log(solution);

  let createdSolutions = 0;
  const start = performance.now();

  let current = 0;

  const callback = (err, result) => {
    console.log("err result", err, result, createdSolutions);
    createdSolutions += 1;
    current -= 1;
  };

  let i = 0;
  while (++i < 1000) {
    const solution = agent.createSolution();
    //console.log(solution);
  }

  /*l

  const sol = new agentaddon.Solution([1,2,3,4,5], 9.78);
  sol.add(4);

  console.log("\n\n")
  const factory = new agentaddon.Factory(1, 2, 1, 1);
  console.log(factory.useCapacity(1));
  console.log(factory.useCapacity(1));
  console.log(factory.useCapacity(1));
  /*
  
  /*
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
