export * from "./constants";

export default {
  logging: {
    levels: ["error", "warn"],
    messages: false,
    master: false,
    worker: true,
    progressbar: true
  },
  outDir: "out", // may be overwritten through the parameters
  convertedInstancesDir: "problems"
};
