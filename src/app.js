import bindings from "bindings";
import RQAPParser, { enhanceWithDistanceMatrix, toNativeInstance } from "./lib/rqapParser";
import getParametersFromArgs from "./lib/parameterParser"
import { getPerformaceTools, sleep, objectValues, compose } from "./helpers";

const addon = bindings("nativeaddon");
const agentaddon = bindings("agentaddon");
const { performance } = getPerformaceTools()

const parser = new RQAPParser();
const parameters = getParametersFromArgs()
console.log("Parameters", parameters)

const main = async () => {
  // Parse the instance into a js Object
  const jsInstance = await parser.parseFile({ name: parameters.instanceName });
  // Enhance the parsed instance with the distance matrix
  // Create native instances of the entities
  const instance = compose(
    toNativeInstance,
    enhanceWithDistanceMatrix
  )(jsInstance);
  let createdSolutions = 0;
  const start = performance.now();

  let current = 0;

  const callback = (err, result) => {
    console.log("err result", err, result, createdSolutions);
    createdSolutions += 1;
    current -= 1;
  }

  const { factories, machines, flowMatrix, changeOverMatrix, distanceMatrix } = instance;
  console.log(instance);
  console.log("\n\n");
  
  const agent = new agentaddon.Agent(factories, machines, flowMatrix, changeOverMatrix, distanceMatrix)
  const solution = agent.createSolution();

  console.log(solution);

  let i = 0;
  while (++i < 1000){
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
  console.log(`Result for ParamILS: ${solved}, ${runtime}, ${runlength}, ${bestSol}, ${seed}`)

  //await sleep(100000)
}

main()

//addon.test()