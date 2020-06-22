import bindings from "bindings";
const agentaddon = bindings("agentaddon");

/**
 * Converts a solution object to a native agentaddon.Solution
 * @param {Solution} solution to convert to a native solution (might already be native)
 * @returns {Solution} agentaddon.solution this is always a native solution
 */
export const toNativeSolution = solution =>
  new agentaddon.Solution(solution.permutation, solution.quality);
