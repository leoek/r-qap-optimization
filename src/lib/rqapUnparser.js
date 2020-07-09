import "./typedefs";
import fs from "fs";
import config from "../config";
import { objectValues } from "../helpers";

/**
 *
 * @param {instance} instance
 * @returns {string[]} instance content as array of strings
 */
export const unparseInstance = instance => {
  const {
    factories,
    machines,
    changeOverMatrix,
    flowMatrix,
    distanceMatrix
  } = instance;

  console.log(instance);

  const result = [];
  objectValues(factories).forEach(({ id, probability, capacity, x, y }) => {
    result.push(`${id} ${probability} ${capacity} ${x} ${y}`);
  });
  result.push("");
  objectValues(machines).forEach(({ id, size, redundancy }) => {
    result.push(`${id} ${size} ${redundancy}`);
  });
  result.push("");
  [flowMatrix, changeOverMatrix, distanceMatrix].forEach(matrix => {
    matrix.forEach(row => {
      const line = [];
      row.forEach(val => {
        line.push(`${val}`);
      });
      result.push(line.join(" "));
    });
    result.push("");
  });

  return result;
};

/**
 *
 * @param {string[]} content the instance content, each item will be one line
 * @returns {Promise} resolves when the content is successfully written
 */
export const writeInstanceContent = async (content, options = {}) => {
  const {
    path = config.convertedInstancesDir,
    filename = "converted.rqap"
  } = options;

  if (!fs.existsSync(path)) {
    fs.mkdirSync(path);
  }

  if (fs.existsSync(`${path}/${filename}`)) {
    fs.unlinkSync(`${path}/${filename}`);
  }

  const writeStream = fs.createWriteStream(`${path}/${filename}`, {
    flags: "a"
  });

  const writeAsync = val =>
    new Promise((resolve, reject) => {
      writeStream.write(val, (err, bytesWritten) => {
        if (err) {
          reject(err);
        } else {
          resolve(bytesWritten);
        }
      });
    });

  for (const line of content) {
    await writeAsync(line);
    await writeAsync("\n");
  }
};
