
#ifndef AGENT_ADDON_AGENT_H_
#define AGENT_ADDON_AGENT_H_

//#define DEBUG_OUTPUT
//#define QAP_ONLY

// ignore solution from history
#define IGNORE_DUPLICATE_SOLUTIONS_IN_HISTORY
// ignore solution from populations
#define IGNORE_DUPLICATE_SOLUTIONS_IN_POPULATIONS
/**
 * skip solutions which are in the history or population completely
 * --> these wont be even rated
 */
#define SKIP_DUPLICATE_SOLUTIONS
// randomize the order in which machines are assigned a factory
#define RANDOMIZE_SOLUTION_CREATION_ORDER

#include <nan.h>
#include <math.h>       /* pow */
#include "Solution.h"
#include "Factory.h"
#include "Machine.h"
#include "rndSelector.cc"
#include "helperFunctions.h"
#include "Matrix.h"
#include "WrappingHelpers.h"

class Agent : public Nan::ObjectWrap {
private:
  // This is extracted from the supplied machines,1 is just the default
  int maxMachineRedundancy = 1;

  // Creating Solutions
  int GetNextValue(int currentMachineIndex, int, std::vector<int>);
  int GetNextValue(int currentMachineIndex);
  void ResetFactories();

  // Rating Solutions
  int GetFlowDistanceSum(std::vector<std::vector<int>>);
  double GetFailureRiskSum(std::vector<std::vector<int>> permutation);
  // alternative solutions (with failed factories)
  int GetChangeOverCost(std::vector<std::vector<int>>, std::vector<int>);
  int GetAltFlowDistanceSum(std::vector<std::vector<int>> permutation, std::vector<int> failedFactories);
  double GetSingleFactoryFailureSum(
    int flowDistanceSum,
    std::vector<std::vector<int>> permutation
  );

  /**
   * These are computed once from the supplied problem instance
   */
  double averageFlow = -1;
  double averageDistance = -1;
  double averageChangeOverCost = -1;
  double averageFactoryFailureProbability = -1;
  double averageMachineRedundancy = -1;
  // calculated from the values above once
  double flowDistanceSumReference = -1;
  double failureRiskReference = -1;
  double singleFactoryFailureReference = -1;

  double getAverageFlow();
  double getAverageDistance();
  double getAverageChangeOverCost();
  double getAverageFactoryFailureProbability();
  double getAverageMachineRedundancy();

  double getFlowDistanceSumReference();
  double getFailureRiskReference();
  double getSingleFactoryFailureReference();
  
  // generates all of the values above
  void generateQualityScoreReferences();

public:
  std::vector<Machine*> machines;
  std::vector<Factory*> factories;
  Matrix * flowMatrix;
  Matrix * changeOverMatrix;
  Matrix * distanceMatrix;

  std::vector<Solution*> globalBestSolutions;
  std::vector<Solution*> personalBestSolutions;
  std::deque<Solution*> personalHistorySolutions;
  std::deque<Solution*> iterationBestSolutions;

  /**
   * Configurable Parameters for agents solution creation
   * These (reasonably) working defaults, the final values
   * should be provided at agent creation.
   */
  int maxGlobalBest = 10;
  int maxPersonalBest = 10;
  int maxPersonalHistory = 0;
  int maxIterationBest = 0;

  int pBestPopulationWeight = 10;
  int gBestPopulationWeight = 10;
  int rndWeight = 1;
  int pHistoryWeight = 0;
  int iterationBestWeight = 0;

  void ReportNewBestSolution(Solution&);
  bool HandleNewSolution(Solution&);

  void Solve(Solution&);
  int RateSolution(Solution&);
  bool UpdatePersonalPopulation(Solution&);
  bool UpdateGlobalPopulation(Solution&);
  bool UpdatePersonalHistoryPopulation(Solution&);
  bool UpdateIterationBestPopulation(Solution&);

  Solution* CreateSolution();
  void CreateSolutions(int);
  
  std::string ToString();

  Nan::Callback* newBestSolutionCallback;
  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  static NAN_METHOD(AddGlobalSolution);
  static NAN_METHOD(AddIterationBestSolution);
  static NAN_METHOD(_CreateSolution);
  static NAN_METHOD(_CreateAndReturnSolution);
  static NAN_METHOD(_CreateSolutions);

  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

#endif  // AGENT_ADDON_AGENT_H_