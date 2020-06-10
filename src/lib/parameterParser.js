import { INSTANCE_TYPE } from "../config";

export const getParametersFromArgs = () => {
  const parameters = {
    instanceName: "default",
    instanceType: INSTANCE_TYPE.RQAP,
    agents: 1,
    solutionCountTarget: 100,
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
