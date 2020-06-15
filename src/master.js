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
 * @param {function(Solution):void=} options.newSolutionCallback optional callback (Solution) => void, called if a worker reports a new solution
 * @param {function(Solution):void=} options.newBestSolutionCallback  optional callback (Solution) => void, called if a new best solution was found
 */
const main = async ({
  logger,
  workerCount = 1,
  solutionCountTarget,
  showProgressBar = config.logging.progressbar,
  addToProgressBar = "",
  newSolutionCallback,
  newBestSolutionCallback
}) => {
  // This will contain the best found solution (might be a native instance)
  let best = null;
  let createdSolutions = 0;

  // progress bar
  const progressBar = showProgressBar
    ? new cliProgress.SingleBar(
        {
          clearOnComplete: false,
          format: `progress [{bar}] {percentage}% | ETA: {eta_formatted} ({eta}s) | {value}/{total} | {bestQuality} ${addToProgressBar}`
        },
        cliProgress.Presets.shades_classic
      )
    : null;

  // Will be called if all workers exited.
  let executionDoneCallback = () => undefined;
  const executionDonePromise = new Promise(resolve => {
    executionDoneCallback = resolve;
  });

  logger.info(`about to start ${workerCount} worker`);
  const createdWorkerSolutions = Array(workerCount).fill(0);

  // Receive Solutions from Workers
  const messageHandler = msg => {
    config.logging.messages &&
      logger.debug("received message", msg, {
        solutionCountTarget,
        createdSolutions
      });
    if (
      msg.type === MESSAGE_TYPE.CREATED_SOLUTIONS_COUNT ||
      MESSAGE_TYPE.NEW_SOLUTION
    ) {
      createdWorkerSolutions[msg.payload.workerId] =
        msg.payload.createdSolutions;
      createdSolutions = getCreatedSolutions(createdWorkerSolutions);
      showProgressBar &&
        progressBar.update(createdSolutions, {
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
      newSolutionCallback && newSolutionCallback(msg.payload.solution);
      // check whether this is the new global best
      if (!best || best.quality > msg.payload.solution.quality) {
        best = msg.payload.solution;
        // call callback fn if supplied with new global best
        newBestSolutionCallback && newBestSolutionCallback(best);
        logger.log("new best solution", {
          quality: best.quality,
          createdSolutions,
          permutation: best.permutation
        });
      }
    }
  };

  showProgressBar && progressBar.start(solutionCountTarget, 0);

  const start = performance.now();
  for (let i = 0; i < workerCount; i++) {
    const worker = cluster.fork();
    worker.on("message", messageHandler);
  }

  // Handle Worker Exit
  cluster.on("exit", (worker, code, signal) => {
    logger.info(`worker ${worker.id} with pid ${worker.process.pid} died`);
    logger.info({
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
  showProgressBar && progressBar.stop();

  // Result Output
  const runtime = end - start;
  const bestQuality = best ? best.quality : null;
  logger.log(
    `Result\n`,
    inspect(
      {
        runtime: `${runtime} ms`,
        createdSolutions,
        bestQuality,
        best
      },
      { showHidden: false, depth: null }
    )
  );
  return {
    runtime,
    createdSolutions,
    bestQuality,
    best
  };
};

export default main;
