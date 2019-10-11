import compact from "lodash/compact";
import { objectValues, asyncCompose } from "../helpers";
import createParser, { toNativeInstance } from "./parser";

const PARSE_MODE = {
  N: "n",
  A: "A", // Flow Matrix
  B: "B" // Distance Matrix
};

const modes = objectValues(PARSE_MODE);

const parseQAPContent = content => {
  const lines = content.split("\n");
  let modeIndex = 0;
  let n, flowMatrix = [], distanceMatrix = [];
  let line = lines.shift();
  while (line || line === "") {
    if (line === "") {
      modeIndex += 1;
    } else if (modes[modeIndex] === PARSE_MODE.N) {
      n = parseInt(line);
    } else if (modes[modeIndex] === PARSE_MODE.A) {
      flowMatrix.push(compact(line.split(" ")).map(val => parseInt(val)));
    } else if (modes[modeIndex] === PARSE_MODE.B) {
      distanceMatrix.push(compact(line.split(" ")).map(val => parseInt(val)));
    }
    line = lines.shift();
  }
  return {
    n,
    flowMatrix,
    distanceMatrix
  };
};

const toObject = (arr, keyResolver = item => item.id) => {
    const result = {}
    arr.forEach(item => {
        result[keyResolver(item)] = item
    })
    return result;
}

const transformQAPContent = ({ n, flowMatrix, distanceMatrix }) => {
  const factories = toObject([...Array(n)].map((_, i) => ({
      id: i,
      probability: 0,
      capacity: 1,
      x: 0,
      y: 0
  })))
  const machines = toObject([...Array(n)].map((_, i) => ({
      id: i,
      size: 1,
      redundancy: 1
  })))
  const changeOverCost = 0;
  const changeOverMatrix = [...Array(n)].map(_ => [...Array(n)].map(_ => changeOverCost))
  return {
    factories,
    machines,
    changeOverMatrix,
    flowMatrix,
    distanceMatrix
  };
};

const createQAPParser = (options = {}) => {
  const parser = createParser({
    fileExtension: "dat",
    parseFn: parseQAPContent,
    ...options
  });
  const toInstance = transformQAPContent;

  /**
   *  1. Parse the file into a js content Object
   *  2. Parse the content into a js problem instance
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

export default createQAPParser;
