import cluster from "cluster";

import cliProgress from "cli-progress";

import { getPerformaceTools, objectValues } from "./helpers";
import config, { MESSAGE_TYPE } from "./config";
import { broadcast, newMessage } from "./lib/messaging";
import { inspect } from "util";

const { performance } = getPerformaceTools();

const getCreatedSolutions = createdWorkerSolutions =>
  createdWorkerSolutions.reduce((sum, val) => (sum += val), 0);

/**
 *
 * @param {object} options
 * @param {object} options.logger console compatible logger
 * @param {number} options.workerCount number of workers and therefore agents
 * @param {number=} options.solutionCountTarget number of solutions to create before stopping workers and therefore agents
 * @param {boolean=} options.showProgressBar whether to show a progressbar, defaults to the value defined in the config
 * @param {string=""} options.addToProgressBar string to attach to the end of the progressbar
 * @param {function({ solution: Solution, workerId: number, createdSolutions: number }):void=} options.newSolutionCallback optional callback (Solution) => void, called if a worker reports a new solution
 * @param {function({ solution: Solution, workerId: number, createdSolutions: number }):void=} options.newBestSolutionCallback  optional callback (Solution) => void, called if a new best solution was found
 */
const main = async ({
  logger,
  workerCount = 1,
  solutionCountTarget,
  qualityTarget,
  showProgressBar = config.logging.progressbar,
  addToProgressBar = "",
  newSolutionCallback,
  newBestSolutionCallback,
  agentOptions
}) => {
  // check whether the iterationBests are enabled
  const iterationBestModeEnabled =
    agentOptions.maxIterationBest && agentOptions.iterationBestWeight;

  // This will contain the best found solution (might be a native instance)
  let best = null;
  let createdSolutions = 0;

  // progress bar
  let progressMultiBar,
    solutionCountProgressBar,
    solutionQualityProgressBar,
    firstSolutionQuality;
  if (showProgressBar) {
    progressMultiBar = new cliProgress.MultiBar(
      {
        clearOnComplete: false,
        hideCursor: false,
        noTTYOutput: true,
        format: `{name} [{bar}] {percentage}% | ETA: {eta_formatted} ({eta}s) | {value}/{total} | {bestQuality} ${addToProgressBar}`
      },
      cliProgress.Presets.shades_classic
    );
    solutionCountProgressBar = progressMultiBar.create(solutionCountTarget, 0, {
      name: "solutionCount"
    });
  }

  // Will be called if all workers exited.
  let executionDoneCallback = () => undefined;
  const executionDonePromise = new Promise(resolve => {
    executionDoneCallback = resolve;
  });

  logger.info(`about to start ${workerCount} worker`);
  const createdWorkerSolutions = Array(workerCount).fill(0);

  // configure the max event listeners warning, we need this amount
  process.setMaxListeners(3 * workerCount);

  const iterationSolutions = {};

  // Receive Solutions from Workers
  const messageHandler = msg => {
    config.logging.messages &&
      logger.debug("received message", msg, {
        solutionCountTarget,
        createdSolutions
      });
    if (
      (!iterationBestModeEnabled && msg.type === MESSAGE_TYPE.NEW_SOLUTION) ||
      msg.type === MESSAGE_TYPE.CREATED_SOLUTIONS_COUNT
    ) {
      createdWorkerSolutions[msg.payload.workerId] =
        msg.payload.createdSolutions;
      createdSolutions = getCreatedSolutions(createdWorkerSolutions);
      showProgressBar &&
        solutionCountProgressBar.update(createdSolutions, {
          bestQuality: best && best.quality
        });
      if (solutionCountTarget && createdSolutions > solutionCountTarget) {
        broadcast(newMessage(MESSAGE_TYPE.STOP_SOLUTION_CREATION));
      }
    }
    if (msg.type === MESSAGE_TYPE.NEW_SOLUTION) {
      // broadcast new solutions to all workers
      broadcast(newMessage(MESSAGE_TYPE.NEW_SOLUTION, msg.payload));
      // call callback fn if supplied
      newSolutionCallback && newSolutionCallback(msg.payload);
      // check whether this is the new global best
      if (!best || best.quality > msg.payload.solution.quality) {
        best = msg.payload.solution;
        // call callback fn if supplied with new global best
        newBestSolutionCallback && newBestSolutionCallback(msg.payload);
        logger.info("new best solution", {
          quality: best.quality,
          createdSolutions,
          permutation: best.permutation
        });
        if (qualityTarget) {
          if (showProgressBar) {
            if (!solutionQualityProgressBar) {
              firstSolutionQuality = best.quality;
              solutionQualityProgressBar = progressMultiBar.create(
                firstSolutionQuality - qualityTarget,
                0,
                { name: "quality      ", bestQuality: qualityTarget }
              );
            }
            solutionQualityProgressBar.update(
              firstSolutionQuality - best.quality
            );
          }
          if (best.quality < qualityTarget) {
            broadcast(newMessage(MESSAGE_TYPE.STOP_SOLUTION_CREATION));
            // in case some workers are just starting
            setTimeout(
              () => broadcast(newMessage(MESSAGE_TYPE.STOP_SOLUTION_CREATION)),
              1000
            );
          }
        }
      }
    } else if (msg.type === MESSAGE_TYPE.AGENT_STATE) {
      logger.debug(
        "agent reported the following state",
        inspect(msg.payload, false, null)
      );
    } else if (msg.type === MESSAGE_TYPE.NEW_ITERATION_SOLUTION) {
      const { solution, workerId } = msg.payload;
      iterationSolutions[workerId] = solution;
      /**
       * check whether all solutions for that iteration are complete
       * if they are complete, determine the best and broadcast it.
       */
      if (objectValues(iterationSolutions).length >= workerCount) {
        let iterationBest;
        Object.keys(iterationSolutions).forEach(id => {
          if (!iterationBest || iterationSolutions[id].quality < best.quality) {
            iterationBest = iterationSolutions[id];
          }
          delete iterationSolutions[id];
        });
        broadcast(
          newMessage(MESSAGE_TYPE.NEW_ITERATION_BEST_SOLUTION, {
            solution: iterationBest
          })
        );
        createdSolutions = createdSolutions + workerCount;
        showProgressBar &&
          solutionCountProgressBar.update(createdSolutions, {
            bestQuality: best && best.quality
          });
      }
    }
  };

  const solutionCountTargetPerWorker = Math.ceil(
    solutionCountTarget / workerCount
  );

  const start = performance.now();
  for (let i = 0; i < workerCount; i++) {
    const worker = cluster.fork({
      workerParams: JSON.stringify({
        agentOptions,
        solutionCountMax: solutionCountTargetPerWorker
      })
    });
    worker.on("message", messageHandler);
  }

  // Handle Worker Exit
  cluster.on("exit", (worker, code, signal) => {
    logger.debug(`worker ${worker.id} with pid ${worker.process.pid} died`);
    logger.debug({
      createdSolutions,
      quality: best ? best.quality : null,
      workersLeft: objectValues(cluster.workers).length
    });
    if (objectValues(cluster.workers).length === 0) {
      executionDoneCallback();
    }
    /**
     * There is sometimes a slight delay until the worker
     * is gone from cluster.workers
     */
    setTimeout(() => {
      if (objectValues(cluster.workers).length === 0) {
        executionDoneCallback();
      }
    }, 100);
  });

  logger.info("waiting for execution promise to resolve");
  await executionDonePromise;
  logger.info("MASTER", "execution promise resolved");

  const end = performance.now();
  showProgressBar && progressMultiBar.stop();

  // Result Output
  const runtime = end - start;
  const bestQuality = best ? best.quality : null;
  return {
    runtime,
    createdSolutions,
    bestQuality,
    best
  };
};

export default main;
