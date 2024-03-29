import cluster from "cluster";

const allLogLevels = ["error", "warn", "info", "debug"];

const getLogPrefix = () => {
  if (cluster.isMaster) {
    return `\nProcess ${process.pid}: MASTER`;
  } else if (cluster.isWorker) {
    return `\nProcess ${process.pid}: WORKER ${cluster.worker.id}`;
  } else {
    return "";
  }
};

/**
 * Creates a simple js console compatible logger
 *
 * @param {object} options
 * @param {string[]} options.logLevels List of levels which should be logged
 */
const create = ({ logLevels = allLogLevels }) => {
  const logger = {
    log: (...args) => console.log(getLogPrefix(), ...args)
  };
  allLogLevels.forEach(level => {
    if (logLevels.includes(level)) {
      logger[level] = (...args) => console[level](getLogPrefix(), ...args);
    } else {
      logger[level] = () => undefined;
    }
  });
  return logger;
};

export default create;
