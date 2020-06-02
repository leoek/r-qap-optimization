import cluster from "cluster";
import { objectValues } from "../helpers";

export const newMessage = (type, payload = null) => ({
  type,
  payload,
  processId: process.pid
});

export const broadcast = msg => {
  if (cluster.isMaster) {
    objectValues(cluster.workers).forEach(worker => {
      try {
        worker.send(msg);
      } catch (ex) {}
    }, this);
  } else {
    console.error("Broadcasts can only be done by the master process");
  }
};
