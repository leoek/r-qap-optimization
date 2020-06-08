import cluster from "cluster";
import { inspect } from "util";

import sum from "lodash/sum";

import getParametersFromArgs from "./lib/parameterParser";
import createQAPParser from "./lib/qapParser";
import createRQAPParser from "./lib/rqapParser";

import createLogger from "./services/logger";
import config, { INSTANCE_TYPE } from "./config";

import masterMain from "./master";
import workerMain from "./worker";
import { objectValues } from "./helpers";

const qapParser = createQAPParser();
const rqapParser = createRQAPParser();

const main = async () => {
  const logLevels =
    (cluster.isMaster && config.logging.master) ||
    (cluster.isWorker && config.logging.worker)
      ? config.logging.levels
      : [];
  const logger = createLogger({ logLevels });
  const parameters = getParametersFromArgs();
  logger.info("Parameters", parameters);
  const {
    agents,
    solutionCountTarget,
    n,
    instanceType,
    instanceName
  } = parameters;

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
  const parser = instanceType === INSTANCE_TYPE.QAP ? qapParser : rqapParser;
  try {
    instance = await parser.fileToNativeInstance({
      name: instanceName
    });
  } catch (error) {
    console.error("Could not create problem instance", error);
    throw error;
  }

  const solutionCountTargetPerWorker = Math.ceil(solutionCountTarget / agents);

  if (cluster.isMaster) {
    let i = 0;
    const qualities = [];
    let createdSolutionCount = 0;
    let overallBest = null;
    while (i++ < n) {
      const { best, bestQuality, createdSolutions } = await masterMain({
        logger,
        workerCount: agents,
        solutionCountTarget,
        overallProgress: i,
        overallProgressTotal: 100,
        overallSolutionCount: createdSolutionCount,
        overallSolutionCountTotal:
          createdSolutionCount + (n - i) * solutionCountTargetPerWorker
      });
      createdSolutionCount += createdSolutions;
      qualities.push(bestQuality);
      if (!overallBest || best.quality < overallBest.quality) {
        overallBest = best;
      }
    }
    if (n > 1) {
      logger.log(
        inspect(
          {
            n,
            avgQuality: sum(qualities) / n,
            overallBest,
            createdSolutionCount
          },
          false,
          null
        )
      );
    }
  } else if (cluster.isWorker) {
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
