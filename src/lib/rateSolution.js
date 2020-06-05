const rateSolution = instance => permutation => {
  const {
    factories,
    machines,
    flowMatrix,
    changeOverMatrix,
    distanceMatrix
  } = instance;
  let flowDistanceSum = 0;

  for (let m_i = 0; m_i < machines.length; m_i++) {
    for (let m_k = 0; m_k < machines.length; m_k++) {
      let flow = flowMatrix.matrix[m_i][m_k];
      const x = permutation[m_i][0];
      const y = permutation[m_k][0];
      let distance = distanceMatrix.matrix[x][y];
      flowDistanceSum += flow * distance;
    }
  }
  return flowDistanceSum;
};

export default rateSolution;
