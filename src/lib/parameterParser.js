import { INSTANCE_TYPE } from "../config";

export const getParametersFromArgs = () => {
  const parameters = {
    instanceName: "nug12",
    instanceType: INSTANCE_TYPE.QAP,
    agents: 1,
    solutionCountTarget: 10000,
    n: 1,
    agentOptions: {
      maxPersonalBest: 10,
      maxGlobalBest: 10,
      pBestPopulationWeight: 10,
      gBestPopulationWeight: 10,
      rndWeight: 1
    }
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
    } else if (i === 7) {
      parameters.agentOptions.maxPersonalBest = parseInt(val);
    } else if (i === 8) {
      parameters.agentOptions.maxGlobalBest = parseInt(val);
    } else if (i === 9) {
      parameters.agentOptions.pBestPopulationWeight = parseInt(val);
    } else if (i === 10) {
      parameters.agentOptions.gBestPopulationWeight = parseInt(val);
    } else if (i === 11) {
      parameters.agentOptions.rndWeight = parseInt(val);
    }
  });
  return parameters;
};

export default getParametersFromArgs;
