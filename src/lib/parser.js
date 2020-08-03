import bindings from "bindings";
import fs from "fs";
import { objectValues } from "../helpers";

const agentaddon = bindings("agentaddon");

const resolvePath = (...paths) => paths.join("/");

const readFile = path =>
  new Promise((resolve, reject) => {
    fs.readFile(path, "utf8", function(err, contents) {
      if (err) {
        reject(err);
      } else {
        resolve(contents);
      }
    });
  });

export const toNativeInstance = instance => {
  const factories = objectValues(instance.factories).map(
    ({ probability, capacity, x, y }) =>
      new agentaddon.Factory(probability, capacity, x, y)
  );
  const machines = objectValues(instance.machines).map(
    ({ size, redundancy }) => new agentaddon.Machine(size, redundancy)
  );
  const flowMatrix = new agentaddon.Matrix(instance.flowMatrix);
  const changeOverMatrix = new agentaddon.Matrix(instance.changeOverMatrix);
  const distanceMatrix = new agentaddon.Matrix(instance.distanceMatrix);
  return {
    factories,
    machines,
    flowMatrix,
    changeOverMatrix,
    distanceMatrix
  };
};

export const createParser = ({
  basePath = ".",
  fileExtension = "",
  parseFn = id => id
}) => {
  const parseFile = (options = {}) => {
    const { path, name = "default" } = options;
    const fullPath = path
      ? resolvePath(basePath, path)
      : resolvePath(
          basePath,
          "problems",
          fileExtension && !name.includes(fileExtension)
            ? `${name}.${fileExtension}`
            : name
        );
    return new Promise((resolve, reject) => {
      readFile(fullPath)
        .then(content => {
          try {
            const result = parseFn(content);
            resolve(result);
          } catch (ex) {
            reject(ex);
          }
        })
        .catch(error => reject(error));
    });
  };
  return {
    parseFile
  };
};

export default createParser;
