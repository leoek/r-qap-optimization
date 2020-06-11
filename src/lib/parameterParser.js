import { INSTANCE_TYPE } from "../config";

export const getParametersFromArgs = () => {
  const parameters = {
    instanceName: "nug12",
    instanceType: INSTANCE_TYPE.QAP,
    agents: 1,
    solutionCountTarget: 10000,
    n: 1
  };
  process.argv.forEach((val, i) => {
    // 0 and 1 are node and the js filename
    if (i === 2) {
      parameters.instanceName = val;
    } else if (i === 3) {
      parameters.instanceType = val;
    } else if (i === 4) {
      parameters.agents = parseInt(val);
    } else if (i === 5) {
      parameters.solutionCountTarget = parseInt(val);
    } else if (i === 6) {
      parameters.n = parseInt(val);
    }
  });
  return parameters;
};

export default getParametersFromArgs;
