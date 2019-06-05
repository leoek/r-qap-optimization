import bindings from "bindings";
import RQAPParser from "./rqapParser";
import getParametersFromArgs from "./lib/parameterParser"
import { getPerformaceTools } from "./helpers";

const addon = bindings("nativeaddon");
const { performance } = getPerformaceTools()

const parser = new RQAPParser();
const parameters = getParametersFromArgs()
console.log("Parameters", parameters)

const main = async () => {
  const instance = await parser.parseFile({ name: parameters.instanceName });
  let createdSolutions = 0;
  const start = performance.now();

  const end = performance.now();

  const solved = true;
  const runtime = end - start;
  const runlength = createdSolutions;
  const seed = null;
  const bestSol = 0;
  console.log(`Result for ParamILS: ${solved}, ${runtime}, ${runlength}, ${bestSol}, ${seed}`)
}

main()

addon.test()