import compact from "lodash/compact"
import { objectValues, asyncCompose } from "../helpers";
import createParser from "./parser"

const PARSE_MODE = {
  N: "n",
  A: "A", // Flow Matrix
  B: "B", // Distance Matrix
}

const modes = objectValues(PARSE_MODE);

const parseQAPContent = content => {
  const lines = content.split("\n");
  let modeIndex = 0;
  let n, flowMatrix, distanceMatrix;
  let line = lines.shift()
  while (line || line === "") {
      if (line === ""){
        modeIndex += 1;
      } else if (modes[modeIndex] === PARSE_MODE.N){
        n = parseInt(line);
      } else if (modes[modeIndex] === PARSE_MODE.A){
        flowMatrix.push(compact(line.split(" ")));
      } else if (modes[modeIndex] === PARSE_MODE.B){
        distanceMatrix.push(compact(line.split(" ")))
      }
      line = lines.shift()
  }
  return {
    n,
    flowMatrix,
    distanceMatrix
  }
}

const transformQAPContent = ({ n, flowMatrix, distanceMatrix }) => {
    return {
        factories: [],
        machines: [],
        flowMatrix: [],
        changeOverMatrix: []
    }
}

const createQAPParser = (options = {}) => {

  const parser = createParser({ fileExtension: "dat", parseFn: parseQAPContent, ...options })
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
  }
}

export default createQAPParser