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
    iraceOutputFileName,
    problemInstancesDirectory
  } = parameters;

  const outDir = parameters.outDir || config.outDir;
  const parserOptions = {
    problemInstancesDirectory
  };

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
  const parser =
    instanceType === INSTANCE_TYPE.QAP
      ? createQAPParser(parserOptions)
      : createRQAPParser(parserOptions);
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
    if (!fs.existsSync(outDir)) {
      fs.mkdirSync(outDir);
    }
    const jsonLogWriteStream = fs.createWriteStream(
      `${outDir}/${new Date().toISOString()}.jsonlog`,
      { flags: "a" }
    );
    jsonLogWriteStream.write(
      JSON.stringify({ type: "parameters", parameters })
    );
    jsonLogWriteStream.write("\n");
    jsonLogWriteStream.write(JSON.stringify({ type: "instance", instance }));
    jsonLogWriteStream.write("\n");
    let i = 0;
    const qualities = [];
    const qualityComponents = [];
    const runtimes = [];
    let overallCreatedSolutions = 0;
    let overallBest = null;
    let overallRuntime = 0;
    while (i++ < n) {
      const { best, bestQuality, createdSolutions, runtime } = await masterMain(
        {
          logger,
          workerCount: agents,
          agentOptions,
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
        flowDistance: best.flowDistanceSum,
        failureRisk: best.failureRiskSum,
        factoryFailure: best.singleFactoryFailureSum
      });
      overallCreatedSolutions += createdSolutions;
      if (!overallBest || best.quality < overallBest.quality) {
        overallBest = best;
      }
      runtimes.push(runtime);
      overallRuntime += runtime;
    }
    jsonLogWriteStream.end();
    logger.log(
      "overall result:\n",
      inspect(
        {
          runtimes,
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
      fs.writeFileSync(`${outDir}/${iraceOutputFileName}`, `${iraceResult}\n`, {
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
