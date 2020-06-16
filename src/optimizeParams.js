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
    const maxIterations = 1000;
    const agents = 5;
    // one per worker
    const pBestSolutions = Array(agents).fill([]);
    const gBestSolutions = [];
    const maxPersonalBest = 3;
    const maxGlobalBest = 3;
    const pBestPopulationWeight = 10;
    const gBestPopulationWeight = 10;
    const rndWeight = 1;

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

    showProgressBar && progressBar.start(maxIterations * n, 0);
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
            rndWeight + pBestPopulationWeight + gBestPopulationWeight
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
          } else {
            selectedValue = get(
              gBestSolutions[random(0, gBestSolutions.length - 1)]?.parameters,
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
        `\nusing: ${solutionId}\nbest:  ${get(gBestSolutions, "0.id")} (${get(
          gBestSolutions,
          "0.quality"
        )}, ${get(gBestSolutions, "0.found")})`
      );
      logger.debug("optimization population", {
        personalPopulationLength: pBestSolutions.map(p => p.length),
        globalPopulationLength: gBestSolutions.length
      });
      if (
        skipDuplicateParameterSets &&
        (pBestSolutions[workerIndex].find(sol => sol.id === id) ||
          gBestSolutions.find(sol => sol.id === id))
      ) {
        // noop this is a duplicate and skipDuplicateParameterSets is true
        // k is not increaded in this case right now as the solution is omitted
      } else {
        const newSolution = {
          found: 1,
          parameters,
          quality: -1,
          n,
          createdSolutions: 0,
          best: 0,
          runtime: 0,
          id: solutionId
        };
        const { agentOptions, agents: workerCount } = parameters;

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
            progressBar.update(k * n + i, { agents, workerCount });
        }
        newSolution.quality = sum(qualities) / n;
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
            logger.debug(
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
          pBestPopulationWeight,
          gBestPopulationWeight,
          rndWeight,
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
