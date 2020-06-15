import set from "lodash/set";
import { INSTANCE_TYPE } from "../config";

/**
 * agentOptions type defintion
 * @typedef {object} agentOptions Configuration Options per agent
 * @property {number} agentOptions.maxPersonalBest int, size of personal population
 * @property {number} agentOptions.maxGlobalBest int, size of personal population
 * @property {number} agentOptions.pBestPopulationWeight int
 * @property {number} agentOptions.gBestPopulationWeight int
 * @property {number} agentOptions.rndWeight int
 *
 * parameters type definition
 * @typedef {object} parameters
 * @property {string} parameters.instanceName
 * @property {string} parameters.instanceType expects oneof INSTANCE_TYPE
 * @property {number} parameters.agents int, number of agents/workers
 * @property {number} parameters.solutionCountTarget int, number of solutions to create
 * @property {number} parameters.n int, number of runs to get best and average
 * @property {agentOptions} parameters.agentOptions Configuration Options per agent
 * @property {number} parameters.seed ignored
 */

/**
 * accepts ordered parameters
 * @param {parameters} parameters
 * @returns {parameters}
 */
const indexParser = parameters => {
  process.argv.forEach((val, i) => {
    // 0 and 1 are node and the js filename
    if (i === 3) {
      parameters.instanceName = val;
    } else if (i === 4) {
      parameters.instanceType = val;
    } else if (i === 5) {
      parameters.agents = parseInt(val);
    } else if (i === 6) {
      parameters.solutionCountTarget = parseInt(val);
    } else if (i === 7) {
      parameters.n = parseInt(val);
    } else if (i === 8) {
      parameters.agentOptions.maxPersonalBest = parseInt(val);
    } else if (i === 9) {
      parameters.agentOptions.maxGlobalBest = parseInt(val);
    } else if (i === 10) {
      parameters.agentOptions.pBestPopulationWeight = parseInt(val);
    } else if (i === 11) {
      parameters.agentOptions.gBestPopulationWeight = parseInt(val);
    } else if (i === 12) {
      parameters.agentOptions.rndWeight = parseInt(val);
    }
  });
  return parameters;
};

/**
 * accepts parameters in the format used by paramils
 * @param {parameters} parameters
 * @returns {parameters}
 */
const paramilsParser = parameters => {
  const paramNames = Object.keys(parameters);
  const agentOptionParamNames = Object.keys(parameters.agentOptions);
  let lastParamPath;
  process.argv.forEach((val, i) => {
    // 0 and 1 are node and the js filename
    if (i === 3) {
      parameters.instanceName = val;
    } else if (i === 4) {
      parameters.instanceType = val;
    } else if (i === 5) {
      parameters.cutoff_time = val;
    } else if (i === 6) {
      parameters.cutoff_length = val;
    } else if (i === 7) {
      parameters.seed = val;
    } else if (val.startsWith("-") && paramNames.includes(val.substr(1))) {
      lastParamPath = val.substr(1);
    } else if (
      val.startsWith("-") &&
      agentOptionParamNames.includes(val.substr(1))
    ) {
      lastParamPath = `agentOptions.${val.substr(1)}`;
    } else if (lastParamPath) {
      set(parameters, lastParamPath, parseInt(val));
      lastParamPath = null;
    }
  });
  return parameters;
};

/**
 * @returns {parameters}
 */
export const getParametersFromArgs = () => {
  /**
   * @type parameters
   */
  const parameters = {
    instanceName: "nug30",
    instanceType: INSTANCE_TYPE.QAP,
    agents: 1,
    solutionCountTarget: 10000,
    n: 3,
    agentOptions: {
      maxPersonalBest: 10,
      maxGlobalBest: 10,
      pBestPopulationWeight: 10,
      gBestPopulationWeight: 10,
      rndWeight: 1
    },
    seed: -1
  };
  // 0 and 1 are node and the js filename
  if (process.argv[2] === "paramils") {
    return paramilsParser(parameters);
  }
  return indexParser(parameters);
};

export default getParametersFromArgs;
