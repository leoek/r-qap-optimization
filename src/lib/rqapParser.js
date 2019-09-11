import bindings from "bindings";
import fs from "fs";

import { ERROR } from "../config";
import { objectValues, newMatrix } from "../helpers";

const agentaddon = bindings("agentaddon");

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
  let flowMatrix, changeOverMatrix, flow_i, co_i;
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
              probability: parseInt(match[2]), // Ausfallwahrscheinlichkeit
              capacity: parseInt(match[3]), //Kapazität 
              x: parseInt(match[4]),
              y: parseInt(match[5])
            }
          } else {
            throw new Error(ERROR.INVALID_INPUT_SYNTAX);
          }
      } else if (modes[modeIndex] === PARSE_MODE.MACHINE) {
          const match = machineMatcher.exec(line);
          if (match){
            const id = match[1];
            machines[id] = {
              size: parseInt(match[2]), //Größe
              redundancy: parseInt(match[3]) || 1 //Redundancy
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
        if (!changeOverMatrix){
          const k = objectValues(factories).length
          changeOverMatrix = newMatrix(k,k);
          co_i = 0;
        }
        const lineValues = line.split(" ");
        lineValues.forEach((value, j) => {
          changeOverMatrix[co_i][j] = value;
        })
        co_i += 1;
      }
      line = lines.shift()
  }
  return {
    factories,
    machines,
    flowMatrix,
    changeOverMatrix
  }
}

const calculateDistance = (a, b) => Math.round(Math.sqrt(Math.pow(b.x - a.x, 2) + Math.pow(b.y - a.y, 2)))
const calculateDistanceMatrix = factories => Object.keys(factories).map(idA => Object.keys(factories).map(idB => calculateDistance(factories[idA], factories[idB])))
export const enhanceWithDistanceMatrix = ({ factories, ...rest }) => ({ factories, distanceMatrix: calculateDistanceMatrix(factories), ...rest })

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

export const toNativeInstance = instance => {
  const factories = objectValues(instance.factories).map(({ probability, capacity, x, y }) => new agentaddon.Factory(probability, capacity, x, y))
  const machines = objectValues(instance.machines).map(({ size, redundancy }) => new agentaddon.Machine(size, redundancy));
  const flowMatrix = new agentaddon.Matrix(instance.flowMatrix);
  const changeOverMatrix = new agentaddon.Matrix(instance.changeOverMatrix);
  const distanceMatrix = new agentaddon.Matrix(instance.distanceMatrix);
  return {
    factories,
    machines,
    flowMatrix,
    changeOverMatrix,
    distanceMatrix
  }
}

export default RQAPParser