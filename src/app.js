import bindings from "bindings";
import RQAPParser, { enhanceWithDistanceMatrix } from "./lib/rqapParser";
import getParametersFromArgs from "./lib/parameterParser"
import { getPerformaceTools, sleep, objectValues } from "./helpers";

const addon = bindings("nativeaddon");
const agentaddon = bindings("agentaddon")
const { performance } = getPerformaceTools()

const parser = new RQAPParser();
const parameters = getParametersFromArgs()
console.log("Parameters", parameters)

const main = async () => {
  const instance = enhanceWithDistanceMatrix(await parser.parseFile({ name: parameters.instanceName }));
  let createdSolutions = 0;
  const start = performance.now();

  let current = 0;

  const callback = (err, result) => {
    console.log("err result", err, result, createdSolutions);
    createdSolutions += 1;
    current -= 1;
  }

  console.log(instance);

  console.log("\n\n");
  const factories = objectValues(instance.factories).map(({ probability, capacity, x, y }) => new agentaddon.Factory(probability, capacity, x, y))
  const machines = objectValues(instance.machines).map(({ size, redundancy }) => new agentaddon.Machine(size, redundancy));
  const flowMatrix = new agentaddon.Matrix(instance.flowMatrix);
  const changeOverMatrix = new agentaddon.Matrix(instance.changeOverMatrix);
  const distanceMatrix = new agentaddon.Matrix(instance.distanceMatrix); 

  console.log(flowMatrix, changeOverMatrix, distanceMatrix);

  const agent = new agentaddon.Agent(factories, machines)

  const solution = agent.createSolution();
  console.log(solution);

  let i = 0;
  while (++i < 1000){
    const solution = agent.createSolution();
    //console.log(solution);
  }

  console.log(agent);

  const sol = new agentaddon.Solution([1,2,3,4,5], 9.78);
  sol.add(4);

  console.log("\n\n")
  const factory = new agentaddon.Factory(1, 2, 1, 1);
  console.log(factory.useCapacity(1));
  console.log(factory.useCapacity(1));
  console.log(factory.useCapacity(1));
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

addon.test()