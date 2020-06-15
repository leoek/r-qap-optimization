import cluster from "cluster";
import { inspect } from "util";
import { get, set, random, sum, sortBy } from "lodash";
import cliProgress from "cli-progress";

import getParametersFromArgs from "./lib/parameterParser";
import createQAPParser from "./lib/qapParser";
import createRQAPParser from "./lib/rqapParser";

import createLogger from "./services/logger";
import config, { INSTANCE_TYPE } from "./config";

import masterMain from "./master";
import workerMain from "./worker";

const qapParser = createQAPParser();
const rqapParser = createRQAPParser();

const paramSpace = {
  agents: Array(18)
    .fill(0)
    .map((_, i) => i + 2), // int, 3-20
  agentOptions: {
    maxPersonalBest: Array(20)
      .fill(0)
      .map((_, i) => i + 1), // int, 1-20
    maxGlobalBest: Array(20)
      .fill(0)
      .map((_, i) => i + 1), // int, 1-20
    pBestPopulationWeight: Array(20)
      .fill(0)
      .map((_, i) => i + 1), // int, 1-20
    gBestPopulationWeight: Array(20)
      .fill(0)
      .map((_, i) => i + 1), // int, 1-20
    rndWeight: Array(20)
      .fill(0)
      .map((_, i) => i + 1) // int, 1-20
  }
};

const main = async () => {
  const logLevels =
    (cluster.isMaster && config.logging.master) ||
    (cluster.isWorker && config.logging.worker)
      ? config.logging.levels
      : [];
  const logger = createLogger({ logLevels });
  const parameters = getParametersFromArgs();
  logger.info("Parameters", parameters);
  const { solutionCountTarget, n, instanceType, instanceName } = parameters;

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
    const skipDuplicateParameterSets = false;
    const maxIterations = 10000;
    const maxBestSolutions = 10;
    const bestSolutions = [];
    const rndWeight = 1;
    const populationWeight = 10;
    const paramKeys = [
      "agents",
      ...Object.keys(paramSpace.agentOptions).map(key => `agentOptions.${key}`)
    ];

    // progress bar
    const showProgressBar = config.logging.progressbar;
    const progressBar = showProgressBar
      ? new cliProgress.SingleBar(
          {
            clearOnComplete: false,
            format: `progress [{bar}] {percentage}% | ETA: {eta_formatted} ({eta}s) | {value}/{total} | agents: {agents}`
          },
          cliProgress.Presets.shades_classic
        )
      : null;

    showProgressBar && progressBar.start(maxIterations * n, 0);
    let k = 0;
    while (k < maxIterations) {
      // determine new parameters
      const parameters = {};
      paramKeys.forEach(paramKey => {
        const selectorIndex = random(0, rndWeight + populationWeight);
        if (
          selectorIndex <= rndWeight ||
          bestSolutions.length < maxBestSolutions
        ) {
          set(
            parameters,
            paramKey,
            get(paramSpace, paramKey)[
              random(0, get(paramSpace, paramKey).length - 1)
            ]
          );
        } else {
          set(
            parameters,
            paramKey,
            get(
              bestSolutions[random(0, bestSolutions.length - 1)].parameters,
              paramKey
            )
          );
        }
      });
      const id = paramKeys.reduce(
        (result, paramKey) => `${result}_${get(parameters, paramKey)}`,
        "sol"
      );
      logger.log(
        "using\n",
        inspect(
          {
            parameters,
            id,
            best: {
              parameters: get(bestSolutions, "0.parameters"),
              found: get(bestSolutions, "0.found"),
              quality: get(bestSolutions, "0.quality")
            }
          },
          false,
          null
        )
      );
      logger.debug(
        "optimization population",
        inspect(bestSolutions, false, null)
      );
      if (
        !(
          skipDuplicateParameterSets && bestSolutions.find(sol => sol.id === id)
        )
      ) {
        const newSolution = {
          found: 1,
          parameters,
          quality: -1,
          n,
          createdSolutions: 0,
          best: 0,
          runtime: 0,
          id
        };
        const { agentOptions, agents } = parameters;

        // test these parameters
        let i = 0;
        const qualities = [];
        while (i++ < n) {
          const {
            best,
            bestQuality,
            createdSolutions,
            runtime
          } = await masterMain({
            logger,
            workerCount: agents,
            solutionCountTarget,
            showProgressBar: false,
            agentOptions
          });
          qualities.push(bestQuality);
          newSolution.createdSolutions += createdSolutions;
          if (!newSolution.best || best.quality < newSolution.best.quality) {
            newSolution.best = best;
          }
          newSolution.runtime += runtime;
          showProgressBar && progressBar.update(k * n + i, { agents });
        }
        newSolution.quality = sum(qualities) / n;

        // average the quality of the duplicates and sort the population again if duplicates are allowed
        if (!skipDuplicateParameterSets) {
          const foundIndexInBest = bestSolutions.findIndex(
            sol => sol.id === newSolution.id
          );

          if (foundIndexInBest !== -1) {
            // average with the prev results with these parameters parameters
            const found = bestSolutions[foundIndexInBest].found + 1;
            bestSolutions[foundIndexInBest] = {
              ...bestSolutions[foundIndexInBest],
              found,
              quality:
                ((found - 1) / found) *
                  bestSolutions[foundIndexInBest].quality +
                (1 / found) * newSolution.quality,
              best: Array.isArray(bestSolutions[foundIndexInBest].best)
                ? [...bestSolutions[foundIndexInBest].best, newSolution.best]
                : [bestSolutions[foundIndexInBest].best, newSolution.best],
              runtime:
                ((found - 1) / found) *
                  bestSolutions[foundIndexInBest].runtime +
                (1 / found) * newSolution.runtime
            };
          } else {
            bestSolutions.push(newSolution);
          }
          bestSolutions.sort((a, b) => a.quality - b.quality);
          //drop the worst one if maxBestSolutions is reached
          if (bestSolutions.length > maxBestSolutions) {
            bestSolutions.length = maxBestSolutions;
          }
        } else {
          i = bestSolutions.length - 1;
          while (i >= 0) {
            if (newSolution.quality < bestSolutions[i].quality) {
              if (i + 1 < maxBestSolutions) {
                bestSolutions[i + 1] = bestSolutions[i];
              }
              i--;
            } else {
              break;
            }
          }
          if (i + 1 < maxBestSolutions) {
            bestSolutions[i + 1] = newSolution;
          }
        }
        k++;
      }
    }
    showProgressBar && progressBar.stop();
    logger.log(
      "Parameter Optimization Result",
      inspect(
        {
          population: bestSolutions,
          maxIterations,
          maxBestSolutions,
          rndWeight,
          populationWeight,
          best: bestSolutions[0]
        },
        false,
        null
      )
    );
  } else if (cluster.isWorker) {
    await workerMain({
      logger,
      instance
    });
  }
};

try {
  main();
} catch (error) {
  console.error("Exception in main method", inspect(error));
}
