import cluster from "cluster";
import getParametersFromArgs from "./lib/parameterParser";
import createQAPParser from "./lib/qapParser";

import createLogger from "./services/logger";
import config from "./config";

import masterMain from "./master";
import workerMain from "./worker";

const qapParser = createQAPParser();

const main = async () => {
  const logLevels =
    (cluster.isMaster && config.logging.master) ||
    (cluster.isWorker && config.logging.worker)
      ? config.logging.levels
      : [];
  const logger = createLogger({ logLevels });
  const parameters = getParametersFromArgs();
  logger.info("Parameters", parameters);

  /**
   * @typedef nativeInstance
   * @property factories
   * @property machines
   * @property flowMatrix
   * @property changeOverMatrix
   * @property distanceMatrix
   *
   * @type nativeInstance
   */
  let instance;
  try {
    instance = await qapParser.fileToNativeInstance({
      name: "nug12" || parameters.instanceName
    });
  } catch (error) {
    console.error("Could not create problem instance", error);
    throw error;
  }

  const { agents = 1, solutionCountTarget = 100 } = parameters;

  if (cluster.isMaster) {
    await masterMain({ logger, workerCount: agents, solutionCountTarget });
  } else if (cluster.isWorker) {
    const solutionCountTargetPerWorker =
      2 * Math.ceil(solutionCountTarget / agents);
    await workerMain({
      logger,
      instance,
      solutionCountMax: solutionCountTargetPerWorker
    });
  }
};

try {
  main();
} catch (error) {
  console.error("Exception in main method", inspect(error));
}
