/**
 * @typedef {object} Solution
 * @property {number[][]} permutation
 * @property {number} quality
 * @property {number} flowDistanceSum
 * @property {number} failureRiskSum
 * @property {number} singleFactoryFailureSum
 * @property {number} flowDistance
 * @property {number} failureRisk
 * @property {number} singleFactoryFailure
 */

/**
 * Js factory type definition
 * @typedef {object} jsFactory
 * @property {number} jsFactory.id
 * @property {number} jsFactory.probability
 * @property {number} jsFactory.capacity
 * @property {number} jsFactory.x
 * @property {number} jsFactory.y
 */

/**
 * @typedef {object} instance
 * @property {jsFactory[]} instance.factories object containing jsFactories
 * @property {object} instance.machines object containing jsMachines
 * @property {array} instance.flowMatrix 2-dimensional matrix array
 * @property {array} instance.changeOverMatrix 2-dimensional matrix array
 * @property {array} instance.distanceMatrix 2-dimensional matrix array
 */
