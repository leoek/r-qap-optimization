import cluster from "cluster";
import { inspect } from "util";
import fs from "fs";

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
    qualityTarget,
    n,
    instanceType,
    instanceName,
    agentOptions,
    seed,
    randomizeAgentOptions,
    warmupSolutions,
    pResetAfterBatch,
    iraceOutputFileName
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
    if (!fs.existsSync(config.outDir)) {
      fs.mkdirSync(config.outDir);
    }
    const jsonLogWriteStream = fs.createWriteStream(
      `${config.outDir}/${new Date().toISOString()}.jsonlog`,
      { flags: "a" }
    );
    jsonLogWriteStream.write(JSON.stringify({ type: "parameters", parameters }));
    jsonLogWriteStream.write("\n");
    jsonLogWriteStream.write(JSON.stringify({ type: "instance", instance }));
    jsonLogWriteStream.write("\n");
    let i = 0;
    const qualities = [];
    const qualityComponents = [];
    let overallCreatedSolutions = 0;
    let overallBest = null;
    let overallRuntime = 0;
    while (i++ < n) {
      const { best, bestQuality, createdSolutions, runtime } = await masterMain(
        {
          logger,
          workerCount: agents,
          solutionCountTarget,
          qualityTarget,
          addToProgressBar: n > 1 ? `| ${i}/${n}` : "",
          newSolutionCallback: ({ workerId, solution, createdSolutions }) => {
            jsonLogWriteStream.write(
              JSON.stringify({
                type: "solution",
                workerId,
                solution,
                createdSolutions
              })
            );
            jsonLogWriteStream.write("\n");
          }
        }
      );
      qualities.push(bestQuality);
      qualityComponents.push({
        fd: best.flowDistanceSum,
        risk: best.failureRiskSum,
        fFailureScore: best.singleFactoryFailureScore
      });
      overallCreatedSolutions += createdSolutions;
      if (!overallBest || best.quality < overallBest.quality) {
        overallBest = best;
      }
      overallRuntime += runtime;
    }
    jsonLogWriteStream.end();
    logger.log(
      "overall result:\n",
      inspect(
        {
          qualities,
          qualityComponents,
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
    // irace result
    const iraceResult = `${overallBest.quality} ${Math.floor(overallRuntime)}`;
    logger.log(`Result for Irace: ${iraceResult}`);
    if (iraceOutputFileName) {
      fs.writeFileSync(`${config.outDir}/${iraceOutputFileName}`, iraceResult, {
        encoding: "utf8"
      });
    }
  } else if (cluster.isWorker) {
    const solutionCountTargetPerWorker = Math.ceil(
      solutionCountTarget / agents
    );
    await workerMain({
      logger,
      instance,
      solutionCountMax: solutionCountTargetPerWorker,
      agentOptions,
      randomizeAgentOptions,
      warmupSolutions,
      pResetAfterBatch
    });
  }
};

try {
  main();
} catch (error) {
  console.error("Exception in main method", inspect(error));
}
