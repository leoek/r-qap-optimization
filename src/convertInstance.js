import "./lib/typedefs";
import { inspect } from "util";
import { compose } from "./helpers";
import createQAPParser from "./lib/qapParser";
import createRQAPParser from "./lib/rqapParser";
import { unparseInstance, writeInstanceContent } from "./lib/rqapUnparser";

import createLogger from "./services/logger";
import { INSTANCE_TYPE } from "./config";

const qapParser = createQAPParser();
const rqapParser = createRQAPParser();

const capacity2 = instance => {
  const factories = {};
  Object.keys(instance.factories).forEach(key => {
    factories[key] = {
      ...instance.factories[key],
      capacity: 2
    };
  });
  return {
    ...instance,
    factories
  };
};

const probabilityTemplate = getPFn => instance => {
  const factories = {};
  Object.keys(instance.factories).forEach(key => {
    factories[key] = {
      ...instance.factories[key],
      probability: getPFn()
    };
  });
  return {
    ...instance,
    factories
  };
};

const probability10 = probabilityTemplate(() => 0.1);
const probability1to25 = probabilityTemplate(
  () => Math.random() * (0.25 - 0.01) + 0.01
);

const redundancy2 = instance => {
  const machines = {};
  Object.keys(instance.machines).forEach(key => {
    machines[key] = {
      ...instance.machines[key],
      redundancy: 2
    };
  });
  return {
    ...instance,
    machines
  };
};

const co1 = instance => ({
  ...instance,
  changeOverMatrix: instance.changeOverMatrix.map((row, i) =>
    row.map((_, k) => (k === i ? 0 : 1))
  )
});

/**
 *
 * @param {instance} instance
 */
const coIsDistance = instance => ({
  ...instance,
  changeOverMatrix: instance.changeOverMatrix.map((row, i) =>
    row.map((_, k) => (k === i ? 0 : instance.distanceMatrix[i][k]))
  )
});

const main = async () => {
  const logger = createLogger({});

  // read this propblem instance
  const instanceType = INSTANCE_TYPE.QAP;
  const instanceName = "nug20";

  /**
   * @type instance
   */
  let instance;
  const parser = instanceType === INSTANCE_TYPE.QAP ? qapParser : rqapParser;
  try {
    instance = parser.toInstance(
      await parser.parseFile({
        name: instanceName
      })
    );
  } catch (error) {
    console.error("Could not create problem instance", error);
    throw error;
  }
  logger.info("original instance\n", inspect(instance, false, null));

  // modify if necessary
  instance = compose(
    capacity2,
    probability1to25,
    redundancy2,
    coIsDistance
  )(instance);

  logger.info("modified instance\n", inspect(instance, false, null));

  // write the problem instance
  const content = unparseInstance(instance);
  await writeInstanceContent(content, { filename: `${instanceName}.rqap` });
};

try {
  main();
} catch (error) {
  console.error("Exception in main method", inspect(error));
}
