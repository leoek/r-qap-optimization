import { ERROR } from "../config";
import { objectValues, newMatrix, compose, asyncCompose } from "../helpers";
import createParser, { toNativeInstance } from "./parser";

const PARSE_MODE = {
  FACTORY: "factory",
  MACHINE: "machine",
  FLOW: "flow",
  CHANGEOVER_COST: "changeover"
};

const modes = objectValues(PARSE_MODE);

/**
 * Js factory type definition
 * @typedef {object} jsFactory
 * @property {number} jsFactory.id
 * @property {number} jsFactory.probability
 * @property {number} jsFactory.capacity
 * @property {number} jsFactory.x
 * @property {number} jsFactory.y
 */

/**
 * @typedef {object} qapContent
 * @property {object} factories object containing jsFactories
 * @property {object} machines object containing jsMachines
 * @property {array} flowMatrix 2-dimensional matrix array
 * @property {array} changeOverMatrix 2-dimensional matrix array
 */

/**
  * 
  * @param {string} content File content
  * @returns {qapContent}
  */

const parseRQAPContent = content => {
  const lines = content.split("\n");
  let modeIndex = 0;
  const factories = {};
  const machines = {};
  let flowMatrix, changeOverMatrix, flow_i, co_i;
  // id	Ausfallwahrscheinlichkeit p_i (float)	Kapazität c_i (int)	x-Koordinate x_i (int)	y-Koordinate y_i (int)
  const factoryMatcher = new RegExp(
    "([0-9]+) ([0-9.]+) ([0-9]+) ([0-9]+) ([0-9]+)"
  );
  // id	Größe s_i (int)	redundancy r_i (int, default=1)
  const machineMatcher = new RegExp("([0-9]+) ([0-9]+) ?([0-9]*)");
  let line = lines.shift();
  while (line || line === "") {
    if (line === "") {
      modeIndex += 1;
    } else if (modes[modeIndex] === PARSE_MODE.FACTORY) {
      const match = factoryMatcher.exec(line);
      if (match) {
        const id = match[1];
        factories[id] = {
          id: parseInt(id),
          probability: parseInt(match[2]), // Ausfallwahrscheinlichkeit
          capacity: parseInt(match[3]), //Kapazität
          x: parseInt(match[4]),
          y: parseInt(match[5])
        };
      } else {
        throw new Error(ERROR.INVALID_INPUT_SYNTAX);
      }
    } else if (modes[modeIndex] === PARSE_MODE.MACHINE) {
      const match = machineMatcher.exec(line);
      if (match) {
        const id = match[1];
        machines[id] = {
          size: parseInt(match[2]), //Größe
          redundancy: parseInt(match[3]) || 1 //Redundancy
        };
      } else {
        throw new Error(ERROR.INVALID_INPUT_SYNTAX);
      }
    } else if (modes[modeIndex] === PARSE_MODE.FLOW) {
      if (!flowMatrix) {
        const n = objectValues(machines).length;
        flowMatrix = newMatrix(n, n);
        flow_i = 0;
      }
      const lineValues = line.split(" ");
      lineValues.forEach((value, j) => {
        flowMatrix[flow_i][j] = value;
      });
      flow_i += 1;
    } else if (modes[modeIndex] === PARSE_MODE.CHANGEOVER_COST) {
      if (!changeOverMatrix) {
        const k = objectValues(factories).length;
        changeOverMatrix = newMatrix(k, k);
        co_i = 0;
      }
      const lineValues = line.split(" ");
      lineValues.forEach((value, j) => {
        changeOverMatrix[co_i][j] = value;
      });
      co_i += 1;
    }
    line = lines.shift();
  }
  return {
    factories,
    machines,
    flowMatrix,
    changeOverMatrix
  };
};

const calculateDistance = (a, b) =>
  Math.round(Math.sqrt(Math.pow(b.x - a.x, 2) + Math.pow(b.y - a.y, 2)));
const calculateDistanceMatrix = factories =>
  Object.keys(factories).map(idA =>
    Object.keys(factories).map(idB =>
      calculateDistance(factories[idA], factories[idB])
    )
  );
const enhanceWithDistanceMatrix = ({ factories, ...rest }) => ({
  factories,
  distanceMatrix: calculateDistanceMatrix(factories),
  ...rest
});

const createRQAPParser = (options = {}) => {
  const parser = createParser({
    fileExtension: "rqap",
    parseFn: parseRQAPContent,
    ...options
  });

  const toInstance = enhanceWithDistanceMatrix;

  /**
   *  1. Parse the instance into a js Object
   *  2. Enhance the parsed instance with the distance matrix
   *  3. Create native instances of the entities
   */
  const fileToNativeInstance = asyncCompose(
    toNativeInstance,
    toInstance,
    parser.parseFile
  );

  return {
    ...parser,
    toNativeInstance,
    toInstance,
    fileToNativeInstance
  };
};

export default createRQAPParser;
