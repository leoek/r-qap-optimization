import cluster from "cluster";

import cliProgress from "cli-progress";

import { getPerformaceTools, objectValues } from "./helpers";
import config, { MESSAGE_TYPE } from "./config";
import { broadcast, newMessage } from "./lib/messaging";

const { performance } = getPerformaceTools();

const getCreatedSolutions = createdWorkerSolutions =>
  createdWorkerSolutions.reduce((sum, val) => (sum += val), 0);

/**
 *
 * @param {object} options
 * @param {object} options.logger console compatible logger
 * @param {number} options.workerCount number of workers and therefore agents
 * @param {number=} options.solutionCountTarget number of solutions to create before stopping workers and therefore agents
 */
const main = async ({ logger, workerCount = 1, solutionCountTarget }) => {
  // This will contain the best solution (this is a native instance)
  let best = null;
  let createdSolutions = 0;

  // progress bar
  const showProgressBar = config.logging.progressbar;
  const progressBar = showProgressBar
    ? new cliProgress.SingleBar(
        {
          clearOnComplete: false,
          format:
            "progress [{bar}] {percentage}% | ETA: {eta_formatted} ({eta}s) | {value}/{total}"
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
      showProgressBar && progressBar.update(createdSolutions);
      if (solutionCountTarget && createdSolutions > solutionCountTarget) {
        broadcast(newMessage(MESSAGE_TYPE.STOP_SOLUTION_CREATION));
      }
    }
    if (msg.type === MESSAGE_TYPE.NEW_SOLUTION) {
      if (!best || best.quality > msg.payload.solution.quality) {
        best = msg.payload.solution;
        if (best) {
          logger.log("new best", {
            quality: best.quality,
            createdSolutions,
            permutation: best.permutation
          });
        }
      }
      // broadcast new solutions to all workers
      broadcast(newMessage(MESSAGE_TYPE.NEW_SOLUTION, msg.payload));
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
  });

  logger.info("waiting for execution promise to resolve");
  await executionDonePromise;
  logger.info("MASTER", "execution promise resolved");

  const end = performance.now();
  showProgressBar && progressBar.stop();

  // Reuslt Output
  const solved = true;
  const runtime = end - start;
  const runlength = createdSolutions;
  const seed = null;
  const bestQuality = best ? best.quality : null;
  logger.log(`Human readable result: \n`, {
    solved,
    runtime: `${runtime} ms`,
    createdSolutions,
    bestQuality,
    best
  });
  logger.log(
    `Result for ParamILS: ${solved}, ${runtime}, ${runlength}, ${bestQuality}, ${seed}`
  );
};

export default main;
