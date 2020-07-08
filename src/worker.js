import bindings from "bindings";
import cluster from "cluster";
import get from "lodash/get";

import { sleep } from "./helpers";
import { newMessage } from "./lib/messaging";
import config, { MESSAGE_TYPE } from "./config";
import { inspect } from "util";
import { toNativeSolution } from "./lib/nativeHelpers";

const agentaddon = bindings("agentaddon");

const reportAgentState = (
  createdSolutions,
  personalBestSolutions,
  globalBestSolutions
) => {
  process.send(
    newMessage(MESSAGE_TYPE.AGENT_STATE, {
      workerId: cluster.worker.id,
      createdSolutions,
      personalBestSolutions,
      globalBestSolutions
    })
  );
};

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
 * @param {object} options
 * @param {object} options.logger console compatible logger
 * @param {nativeInstance} options.instance native instance to initialize the agent
 * @param {number} options.solutionCountMax maximum number of solutions this worker will create
 * @param {agentOptions} options.agentOptions configuration options for this agent
 * @param {number=100} options.batchSize number of solutions to create before returning to the event loop and reporting back
 *   - a higher number decreases communication and therefore increases performance
 *   - a lower number improves synchronization between agents (global population is updated earlier)
 * @param {object} resumeOptions supply an agent state to resume a previous execution
 * @param {number} resumeOptions.solutionCount solutions created until this point
 * @param {Solution[]} resumeOptions.personalBestSolutions population with the agents personal best solutions
 * @param {Solution[]} resumeOptions.globalBestSolutions population with the global best solutions
 */
const workerMain = async (
  {
    logger,
    instance,
    solutionCountMax: inSolutionCountMax,
    agentOptions: inAgentOptions = {},
    batchSize: inBatchSize = 100,
    randomizeAgentOptions: inRandomizeAgentOptions = false,
    warmupSolutions: inWarmupSolutions = 0,
    pResetAfterBatch: inPResetAfterBatch = 0
  },
  inResumeOptions
) => {
  logger.info("Worker started");
  if (cluster.isMaster) {
    logger.warn("Executing worker main method as master");
  }

  /**
   * workerParams from env have priority, they might be set in
   * a node cluster setup. (if the worker was started as child_process
   * by the master process)
   */
  const workerParams = JSON.parse(process.env.workerParams);
  const solutionCountMax = get(
    workerParams,
    "solutionCountMax",
    inSolutionCountMax
  );
  const agentOptions = get(workerParams, "agentOptions", inAgentOptions);
  const batchSize = get(workerParams, "batchSize", inBatchSize);
  const randomizeAgentOptions = get(
    workerParams,
    "randomizeAgentOptions",
    inRandomizeAgentOptions
  );
  const warmupSolutions = get(
    workerParams,
    "warmupSolutions",
    inWarmupSolutions
  );
  const pResetAfterBatch = get(
    workerParams,
    "pResetAfterBatch",
    inPResetAfterBatch
  );
  const resumeOptions = get(workerParams, "resumeOptions", inResumeOptions);

  /**
   * This flag might be supplied by the master to stop
   * any further solution creation.
   */
  let shouldStop = false;
  // solutions created by this worker
  let solutionCount = get(resumeOptions, "solutionCount", 0);

  const {
    factories,
    machines,
    flowMatrix,
    changeOverMatrix,
    distanceMatrix
  } = instance;

  if (randomizeAgentOptions) {
    Object.keys(agentOptions).forEach(key => {
      agentOptions[key] = Math.round(
        (Math.random() * (1.5 - 0.5) + 0.5) * agentOptions[key]
      );
    });
    logger.log({ agentOptions });
  }

  const {
    maxPersonalBest,
    maxGlobalBest,
    maxPersonalHistory,
    pBestPopulationWeight,
    gBestPopulationWeight,
    rndWeight,
    pHistoryWeight
  } = agentOptions;

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
    },
    maxPersonalBest,
    maxGlobalBest,
    maxPersonalHistory,
    pBestPopulationWeight,
    gBestPopulationWeight,
    rndWeight,
    pHistoryWeight
  );

  /**
   * rehydrate the agents population if this worker resumes an agent
   */
  const initialPersonalBestSolutions = get(
    resumeOptions,
    "personalBestSolutions"
  );
  if (Array.isArray(initialPersonalBestSolutions)) {
    agent.personalBestSolutions = initialPersonalBestSolutions.map(
      toNativeSolution
    );
    logger.debug("rehydrated personal best", agent.personalBestSolutions);
  }
  const initialGlobalBestSolutions = get(resumeOptions, "globalBestSolutions");
  if (Array.isArray(initialGlobalBestSolutions)) {
    agent.globalBestSolutions = initialGlobalBestSolutions.map(
      toNativeSolution
    );
    logger.debug("rehydrated global best", agent.globalBestSolutions);
  }

  // Receive messages from the master process.
  process.on("message", msg => {
    config.logging.messages && logger.debug("received message", msg);
    if (msg.type === MESSAGE_TYPE.NEW_SOLUTION) {
      const { solution, workerId } = msg.payload;
      if (workerId !== cluster.worker.id) {
        const nativeSolution = toNativeSolution(solution);
        const result = agent.addGlobalSolution(nativeSolution);
        logger.debug("result of newly added global solution", result);
      }
    } else if (msg.type === MESSAGE_TYPE.STOP_SOLUTION_CREATION) {
      shouldStop = true;
    }
  });

  if (warmupSolutions > 0) {
    agent.pBestPopulationWeight = 0;
    agent.gBestPopulationWeight = 0;
    agent.rndWeight = 1;
    agent.pHistoryWeight = 0;
    agent.createSolutions(warmupSolutions);
    agent.pBestPopulationWeight = pBestPopulationWeight;
    agent.gBestPopulationWeight = gBestPopulationWeight;
    agent.rndWeight = rndWeight;
    agent.pHistoryWeight = pHistoryWeight;
  }

  while (
    !shouldStop &&
    (!solutionCountMax || solutionCount < solutionCountMax)
  ) {
    // Worker needs time to recv messages and callbacks before executing native code again...
    await sleep(1);
    agent.createSolutions(batchSize);
    solutionCount += batchSize;
    reportCreatedSolutionsCount(solutionCount);
    if (pResetAfterBatch && Math.random() < pResetAfterBatch) {
      agent.personalBestSolutions = [];
      agent.pBestPopulationWeight = 0;
      agent.gBestPopulationWeight = 0;
      agent.createSolutions(batchSize);
      agent.pBestPopulationWeight = pBestPopulationWeight;
      agent.gBestPopulationWeight = gBestPopulationWeight;
    }
  }
  reportCreatedSolutionsCount(solutionCount);
  reportAgentState(
    solutionCount,
    agent.personalBestSolutions,
    agent.globalBestSolutions
  );
  process.exit(0);
};

export default workerMain;
