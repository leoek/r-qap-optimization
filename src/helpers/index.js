export const objectValues = obj =>
  Object.keys(obj).map(key => {
    return obj[key];
  });

export const sleep = ms =>
  new Promise(resolve => {
    setTimeout(resolve, ms);
  });

/**
 * perf_hooks is avalilable with node >= 8.5 with
 * higher resolution timestamps
 */
export const getPerformaceTools = () => {
  let perf_hooks = {};
  try {
    perf_hooks = require("perf_hooks");
  } catch (ex) {
    perf_hooks = {
      performance: {
        now: Date.now
      }
    };
  }
  return perf_hooks;
};

export const newMatrix = (x, y, initVal = 0) =>
  [...Array(x)].map(_ => [...Array(y)].map(_ => initVal));

export const compose = (...fns) =>
  fns.reduceRight(
    (prevFn, nextFn) => (...args) => nextFn(prevFn(...args)),
    value => value
  );

export const asyncCompose = (...fns) =>
  fns.reduceRight(
    (prevFn, nextFn) => async (...args) => nextFn(await prevFn(...args)),
    value => value
  );
