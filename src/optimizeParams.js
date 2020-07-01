import cluster from "cluster";
import { inspect } from "util";
import { get, set, random, sum, shuffle } from "lodash";
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
  /*agents: Array(18)
    .fill(0)
    .map((_, i) => i + 2),*/ // int, 3-20
  agents: [10],
  agentOptions: {
    maxPersonalBest: Array(20)
      .fill(0)
      .map((_, i) => i + 1), // int, 1-20
    maxGlobalBest: Array(20)
      .fill(0)
      .map((_, i) => i + 1), // int, 1-20
    maxPersonalHistory: Array(20)
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
      .map((_, i) => i + 1), // int, 1-20
    pHistoryWeight: Array(20)
      .fill(0)
      .map((_, i) => i + 1) // int, 1-20
  }
};

const updatePopulationMergeDuplicates = (
  population,
  newSolution,
  maxPopulationSize
) => {
  const foundIndexInPopulation = population.findIndex(
    sol => sol.id === newSolution.id
  );

  if (foundIndexInPopulation !== -1) {
    // average with the prev results with these parameters parameters
    const found = population[foundIndexInPopulation].found + 1;
    population[foundIndexInPopulation] = {
      ...population[foundIndexInPopulation],
      found,
      quality:
        ((found - 1) / found) * population[foundIndexInPopulation].quality +
        (1 / found) * newSolution.quality,
      best: Array.isArray(population[foundIndexInPopulation].best)
        ? [...population[foundIndexInPopulation].best, newSolution.best]
        : [population[foundIndexInPopulation].best, newSolution.best],
      runtime:
        ((found - 1) / found) * population[foundIndexInPopulation].runtime +
        (1 / found) * newSolution.runtime
    };
  } else {
    population.push(newSolution);
  }
  population.sort((a, b) => a.quality - b.quality);
  //drop the worst one if maxBestSolutions is reached
  if (population.length > maxPopulationSize) {
    population.length = maxPopulationSize;
  }
  return !!population.find(sol => sol.id === newSolution.id);
};

const updatePopulationIgnoreDuplicates = (
  population,
  newSolution,
  maxPopulationSize
) => {
  let i = population.length - 1;
  while (i >= 0) {
    if (newSolution.quality < population[i].quality) {
      if (i + 1 < maxPopulationSize) {
        population[i + 1] = population[i];
      }
      i--;
    } else {
      break;
    }
  }
  if (i + 1 < maxPopulationSize) {
    population[i + 1] = newSolution;
    return true;
  }
  return false;
};

const updateHistory = (population, newSolution, maxPopulationSize) => {
  if (maxPopulationSize > 0) {
    population.unshift(newSolution);
    if (population.length > maxPopulationSize) {
      population.length = maxPopulationSize;
    }
  }
};

const main = async () => {
  const logLevels =
    (cluster.isMaster && config.logging.master) ||
    (cluster.isWorker && config.logging.worker)
      ? config.logging.levels
      : [];
  const logger = createLogger({ logLevels });
  // the other params are getting optimized
  // const parameters = getParametersFromArgs();
  const parameters = {
    solutionCountTarget: 10000,
    instanceType: INSTANCE_TYPE.QAP,
    instanceName: "nug30"
  };
  // always evaluate parameters for this number of runs
  const nMin = 10;
  // if the avg is within percentageForNMax of the best so far, evaluate till this number of runs
  const nMax = 100;
  const percentageForNMax = 0.95;
  logger.info("Parameters", parameters);
  const { solutionCountTarget, instanceType, instanceName } = parameters;

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
    /**
     * These are the parameters the optimizer will use
     * to optimze the parameters
     */
    const skipDuplicateParameterSets = true;
    const maxIterations = 1000;
    const agents = 5;
    // one per worker
    const pBestSolutions = Array(agents).fill([]);
    const gBestSolutions = [];
    const pHistorySolutions = Array(agents).fill([]);
    const maxPersonalBest = 1;
    const maxGlobalBest = 1;
    const maxPersonalHistory = 1;
    const pBestPopulationWeight = 10;
    const gBestPopulationWeight = 10;
    const rndWeight = 1;
    const pHistoryWeight = 10;

    // average the quality of the duplicates and sort the population again if duplicates are allowed
    const updatePopulation = skipDuplicateParameterSets
      ? updatePopulationIgnoreDuplicates
      : updatePopulationMergeDuplicates;

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
            format: `progress [{bar}] {percentage}% | ETA: {eta_formatted} ({eta}s) | {value}/{total} | agents: {agents} | # of current workers: {workerCount}`
          },
          cliProgress.Presets.shades_classic
        )
      : null;

    showProgressBar && progressBar.start(maxIterations * nMax, 0);
    let k = 0;
    while (k < maxIterations) {
      let workerIndex = k % agents;
      // determine new parameters
      const parameters = {};
      // randomize the param order with Fisher-Yates shuffle
      shuffle(paramKeys).forEach(paramKey => {
        let selectedValue;
        while (selectedValue === null || selectedValue === undefined) {
          const selectorIndex = random(
            0,
            rndWeight +
              pBestPopulationWeight +
              gBestPopulationWeight +
              pHistoryWeight
          );
          if (selectorIndex <= rndWeight) {
            selectedValue = get(paramSpace, paramKey)[
              random(0, get(paramSpace, paramKey).length - 1)
            ];
          } else if (selectorIndex <= rndWeight + pBestPopulationWeight) {
            selectedValue = get(
              pBestSolutions[workerIndex][
                random(0, pBestSolutions[workerIndex].length - 1)
              ]?.parameters,
              paramKey
            );
          } else if (
            selectorIndex <=
            rndWeight + pBestPopulationWeight + gBestPopulationWeight
          ) {
            selectedValue = get(
              gBestSolutions[random(0, gBestSolutions.length - 1)]?.parameters,
              paramKey
            );
          } else {
            selectedValue = get(
              pHistorySolutions[workerIndex][
                random(0, pHistorySolutions[workerIndex].length - 1)
              ]?.parameters,
              paramKey
            );
          }
        }
        set(parameters, paramKey, selectedValue);
      });
      const solutionId = paramKeys.reduce(
        (result, paramKey) => `${result}_${get(parameters, paramKey)}`,
        "sol"
      );
      logger.log(
        `k=${k} (of ${maxIterations})\nusing: ${solutionId}\nbest:  ${get(
          gBestSolutions,
          "0.id"
        )} (${get(gBestSolutions, "0.quality")}, ${get(
          gBestSolutions,
          "0.n"
        )}, ${get(gBestSolutions, "0.found")})`
      );
      logger.debug("optimization population", {
        pHistorySolutions,
        pBestSolutions,
        gBestSolutions
      });
      if (
        skipDuplicateParameterSets &&
        (pHistorySolutions[workerIndex].find(sol => sol.id === solutionId) ||
          pBestSolutions[workerIndex].find(sol => sol.id === solutionId) ||
          gBestSolutions.find(sol => sol.id === solutionId))
      ) {
        // noop this is a duplicate and skipDuplicateParameterSets is true
        // k is not increaded in this case right now as the solution is omitted
      } else {
        const newSolution = {
          found: 1,
          parameters,
          quality: -1,
          createdSolutions: 0,
          best: 0,
          runtime: 0,
          id: solutionId
        };
        const { agentOptions, agents: workerCount } = parameters;

        // test these parameters
        let i = 0;
        const qualities = [];
        while (i < nMax) {
          /**
           * exit early if the current parameters solutionQuality avg is not within
           * percentageForNMax after evaluating nMin times.
           */
          if (i >= nMin && i % nMin === 0 && gBestSolutions[0]?.quality) {
            newSolution.quality = sum(qualities) / qualities.length;
            const relativeToBest =
              gBestSolutions[0]?.quality / newSolution.quality;
            if (relativeToBest < percentageForNMax) {
              logger.log(
                `\nskipping remaining evalutations for ${newSolution.id} after ${i}/${nMax}
(${gBestSolutions[0]?.quality} / ${newSolution.quality} = ${relativeToBest} < ${percentageForNMax})`
              );
              break;
            }
          }
          const {
            best,
            bestQuality,
            createdSolutions,
            runtime
          } = await masterMain({
            logger,
            workerCount,
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
          showProgressBar &&
            progressBar.update(k * nMax + i, { agents, workerCount });
          i++;
        }
        newSolution.quality = sum(qualities) / qualities.length;
        newSolution.n = i;
        updateHistory(
          pHistorySolutions[workerIndex],
          newSolution,
          maxPersonalHistory
        );
        const didUpdatePBest = updatePopulation(
          pBestSolutions[workerIndex],
          newSolution,
          maxPersonalBest
        );
        if (didUpdatePBest) {
          const didUpdateGBest = updatePopulation(
            gBestSolutions,
            newSolution,
            maxGlobalBest
          );
          if (didUpdateGBest) {
            logger.log(
              "\nupdated global population\n",
              gBestSolutions.map(({ quality, found, parameters }) => ({
                quality,
                found,
                parameters
              }))
            );
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
          pBestSolutions,
          gBestSolutions,
          maxIterations,
          agents,
          maxPersonalBest,
          maxGlobalBest,
          maxPersonalBest,
          pBestPopulationWeight,
          gBestPopulationWeight,
          rndWeight,
          pHistoryWeight,
          best: gBestSolutions[0]
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
