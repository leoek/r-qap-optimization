import fs from "fs";

import { ERROR } from "./config";
import { objectValues, newMatrix } from "./helpers";
const resolvePath = (...paths) => paths.join("/");

const readFile = path => new Promise((resolve, reject) => {
  fs.readFile(path, 'utf8', function(err, contents) {
      if (err){
          reject(err);
      } else {
          resolve(contents);
      }
  });
})

const PARSE_MODE = {
  FACTORY: "factory",
  MACHINE: "machine",
  FLOW: "flow",
  CHANGEOVER_COST: "changeover"
}

const modes = objectValues(PARSE_MODE);

const parseRQAPContent = content => {
  const lines = content.split("\n");
  let modeIndex = 0;
  const factories = {};
  const machines = {};
  let flowMatrix, coMatrix, flow_i, co_i;
  // id	Ausfallwahrscheinlichkeit p_i (float)	Kapazität c_i (int)	x-Koordinate x_i (int)	y-Koordinate y_i (int)
  const factoryMatcher = new RegExp('([0-9]+) ([0-9\.]+) ([0-9]+) ([0-9]+) ([0-9]+)');
  // id	Größe s_i (int)	redundancy r_i (int, default=1)
  const machineMatcher = new RegExp('([0-9]+) ([0-9]+) ?([0-9]*)');
  let line = lines.shift()
  while (line || line === "") {
      if (line === ""){
        modeIndex += 1;
      } else if (modes[modeIndex] === PARSE_MODE.FACTORY){
          const match = factoryMatcher.exec(line)
          if (match){
            const id = match[1];
            factories[id] = {
              id: parseInt(id),
              p: parseInt(match[2]),
              c: parseInt(match[3]),
              x: parseInt(match[4]),
              x: parseInt(match[5])
            }
          } else {
            throw new Error(ERROR.INVALID_INPUT_SYNTAX);
          }
      } else if (modes[modeIndex] === PARSE_MODE.MACHINE) {
          const match = machineMatcher.exec(line);
          if (match){
            const id = match[1];
            console.log("machine match", match);
            machines[id] = {
              s: parseInt(match[2]),
              r: parseInt(match[3]) || 1
            }
          } else {
            throw new Error(ERROR.INVALID_INPUT_SYNTAX);
          }
      } else if (modes[modeIndex] === PARSE_MODE.FLOW){
        if (!flowMatrix){
          const n = objectValues(machines).length
          flowMatrix = newMatrix(n,n);
          flow_i = 0;
        }
        const lineValues = line.split(" ");
        lineValues.forEach((value, j) => {
          flowMatrix[flow_i][j] = value;
        })
        flow_i += 1;
      } else if (modes[modeIndex] === PARSE_MODE.CHANGEOVER_COST){
        if (!coMatrix){
          const k = objectValues(factories).length
          coMatrix = newMatrix(k,k);
          co_i = 0;
        }
        const lineValues = line.split(" ");
        lineValues.forEach((value, j) => {
          coMatrix[co_i][j] = value;
        })
        co_i += 1;
      }
      line = lines.shift()
  }
  return {
    factories,
    machines,
    flowMatrix,
    coMatrix
  }
}

class RQAPParser {

  constructor(basePath = ".") {
      this.basePath = basePath;
  }

  parseFile = (options = {}) => {
      const { path, name = "default" } = options;
      const fullPath = path ? resolvePath(this.basePath, path) : resolvePath(this.basePath, "problems", `${name}.rqap`);
      return new Promise((resolve, reject) => {
          readFile(fullPath).then(content => {
            try {
              const result = parseRQAPContent(content);
              resolve(result);
            } catch (ex){
              reject(ex);
            }
          }).catch(error => reject(error))
      })
  }
}

export default RQAPParser