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
    instanceName,
    agentOptions,
    seed
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

  logger.info("native instance", inspect(instance, false, null));

  if (cluster.isMaster) {
    let i = 0;
    const qualities = [];
    let overallCreatedSolutions = 0;
    let overallBest = null;
    let overallRuntime = 0;
    while (i++ < n) {
      const { best, bestQuality, createdSolutions, runtime } = await masterMain(
        {
          logger,
          workerCount: agents,
          solutionCountTarget,
          addToProgressBar: n > 1 ? `| ${i}/${n}` : ""
        }
      );
      qualities.push(bestQuality);
      overallCreatedSolutions += createdSolutions;
      if (!overallBest || best.quality < overallBest.quality) {
        overallBest = best;
      }
      overallRuntime += runtime;
    }
    logger.log(
      "overall result:\n",
      inspect(
        {
          qualities,
          n,
          overallCreatedSolutions,
          overallRuntime,
          avgQuality: sum(qualities) / n,
          overallBest
        },
        false,
        null
      )
    );
    // Assmble result for Paramils
    const solved = "SAT";
    logger.log(
      `Result for ParamILS: ${solved}, ${Math.floor(
        overallRuntime
      )}, ${overallCreatedSolutions}, ${overallBest.quality}, ${seed}`
    );
  } else if (cluster.isWorker) {
    const solutionCountTargetPerWorker = Math.ceil(
      solutionCountTarget / agents
    );
    await workerMain({
      logger,
      instance,
      solutionCountMax: solutionCountTargetPerWorker,
      agentOptions
    });
  }
};

try {
  main();
} catch (error) {
  console.error("Exception in main method", inspect(error));
}
