import bindings from "bindings";
import cluster from "cluster";

import { sleep } from "./helpers";
import { newMessage } from "./lib/messaging";
import config, { MESSAGE_TYPE } from "./config";
import { inspect } from "util";

const agentaddon = bindings("agentaddon");

const reportCreatedSolutionsCount = createdSolutions => {
  process.send(
    newMessage(MESSAGE_TYPE.CREATED_SOLUTIONS_COUNT, {
      workerId: cluster.worker.id,
      createdSolutions
    })
  );
};

const reportNewSolution = (solution, createdCount) => {
  process.send(
    newMessage(MESSAGE_TYPE.NEW_SOLUTION, {
      solution,
      workerId: cluster.worker.id,
      createdSolutions: createdCount
    })
  );
};

/**
 *
 * @param {object} options
 * @param {object} options.logger console compatible logger
 * @param {nativeInstance} options.instance native instance to initialize the agent
 * @param {number} options.solutionCountMax maximum number of solutions this worker will create
 */
const workerMain = async ({ logger, instance, solutionCountMax = 0 }) => {
  logger.info("Worker started");
  if (cluster.isMaster) {
    logger.warn("Executing worker main method as master");
  }

  /**
   * This flag might be supplied by the master to stop
   * any further solution creation.
   */
  let shouldStop = false;
  // solutions created by this worker
  let solutionCount = 0;

  const {
    factories,
    machines,
    flowMatrix,
    changeOverMatrix,
    distanceMatrix
  } = instance;

  // Every worker process holds it's own agent
  const agent = new agentaddon.Agent(
    factories,
    machines,
    flowMatrix,
    changeOverMatrix,
    distanceMatrix,
    (err, solution) => {
      logger.debug(
        "native new best solution callback",
        inspect({ err, solution }, false, null)
      );
      reportNewSolution(solution, solutionCount);
    }
  );

  // Receive messages from the master process.
  process.on("message", msg => {
    config.logging.messages && logger.debug("received message", msg);
    if (msg.type === MESSAGE_TYPE.NEW_SOLUTION) {
      const { solution, workerId } = msg.payload;
      if (workerId !== cluster.worker.id) {
        const nativeSolution = new agentaddon.Solution(
          solution.permutation,
          solution.quality
        );
        const result = agent.addGlobalSolution(nativeSolution);
        logger.debug("result of newly added global solution", result);
      }
    } else if (msg.type === MESSAGE_TYPE.STOP_SOLUTION_CREATION) {
      shouldStop = true;
    }
  });

  while (!shouldStop && solutionCount < solutionCountMax) {
    // Worker needs time to recv messages and callbacks before executing native code again...
    await sleep(1);
    agent.createSolutions(100);
    solutionCount += 100;
    reportCreatedSolutionsCount(solutionCount);
  }
  reportCreatedSolutionsCount(solutionCount);
  process.exit(0);
};

export default workerMain;
