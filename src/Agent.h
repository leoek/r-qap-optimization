
#ifndef AGENT_ADDON_AGENT_H_
#define AGENT_ADDON_AGENT_H_

//#define DEBUG_OUTPUT
//#define QAP_ONLY
#define IGNORE_DUPLICATE_SOLUTIONS

#include <nan.h>
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
  int currentMachineIndex = 0;

  // Creating Solutions
  int GetNextValue(int, std::vector<int>);
  int GetNextValue();
  void ResetFactories();

  // Rating Solutions
  int GetFlowDistanceSum(std::vector<std::vector<int>>);
  double GetFailureRiskSum(std::vector<std::vector<int>>);
  // alternative solutions (with failed factories)
  int GetAltFlowDistanceSum(std::vector<std::vector<int>>, std::vector<int>);
  double GetRelativeAltFlowDistance(
    int flowDistanceSumReference,
    std::vector<std::vector<int>> permutation,
    std::vector<int> failedFactories);
  double GetSingleFactoryFailureScore(
    int flowDistanceSumReference,
    std::vector<std::vector<int>> permutation
  );

public:
  std::vector<Machine*> machines;
  std::vector<Factory*> factories;
  Matrix * flowMatrix;
  Matrix * changeOverMatrix;
  Matrix * distanceMatrix;

  std::vector<Solution*> globalBestSolutions;
  std::vector<Solution*> personalBestSolutions;

  /**
   * Configurable Parameters for agents solution creation
   * These (reasonably) working defaults, the final values
   * should be provided at agent creation.
   */
  int maxGlobalBest = 10;
  int maxPersonalBest = 10;
  int pBestPopulationWeight = 10;
  int gBestPopulationWeight = 10;
  int rndWeight = 1;

  void ReportNewBestSolution(Solution&);
  bool HandleNewSolution(Solution&);

  void Solve(Solution&);
  int RateSolution(Solution&);
  bool UpdatePersonalPopulation(Solution&);
  bool UpdateGlobalPopulation(Solution&);

  Solution* CreateSolution();
  void CreateSolutions(int);
  
  std::string ToString();

  Nan::Callback* newBestSolutionCallback;
  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  static NAN_METHOD(AddGlobalSolution);
  static NAN_METHOD(_CreateSolution);
  static NAN_METHOD(_CreateAndReturnSolution);
  static NAN_METHOD(_CreateSolutions);

  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

#endif  // AGENT_ADDON_AGENT_H_