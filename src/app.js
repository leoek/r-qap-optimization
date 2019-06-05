import bindings from "bindings";
import RQAPParser from "./rqapParser";
import getParametersFromArgs from "./lib/parameterParser"

const addon = bindings("nativeaddon");
const parser = new RQAPParser();
const parameters = getParametersFromArgs()
console.log("Parameters", parameters)

const main = async () => {
  const parsed = await parser.parseFile({ name: parameters.instanceName });
  console.log(parsed);
}

main()

console.log("test");

addon.test()