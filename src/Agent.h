
#ifndef AGENT_ADDON_AGENT_H_
#define AGENT_ADDON_AGENT_H_

//#define DEBUG_OUTPUT

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

double GetRelativeAltFlowDistance(
  int flowDistanceSumReference,
  std::vector<std::vector<int>> permutation,
  std::vector<int> failedFactories);
double GetSingleFactoryFailureScore(
  int flowDistanceSumReference,
  std::vector<std::vector<int>> permutation
);

public:
  double x;
  double y;
  double z;
  std::vector<int> intarray;

  int maxMachineRedundancy = 1;
  std::vector<Machine*> machines;
  std::vector<Factory*> factories;
  Matrix * flowMatrix;
  Matrix * changeOverMatrix;
  Matrix * distanceMatrix; 

  int maxGlobalBest = 10;
  std::vector<Solution*> globalBestSolutions;
  int maxPersonalBest = 10;
  std::vector<Solution*> personalBestSolutions;
  int currentMachineIndex = 0;

  int pBestPopulationWeight = 10;
  int gBestPopulationWeight = 10;
  int rndWeight = 1;

  Nan::Callback* newBestSolutionCallback;
  void ReportNewBestSolution(Solution&);
  bool HandleNewSolution(Solution&);

  int GetNextValue(int, std::vector<int>);
  int GetNextValue();
  void ResetFactories();
  void Solve(Solution&);

  int GetFlowDistanceSum(std::vector<std::vector<int>>);
  double GetFailureRiskSum(std::vector<std::vector<int>>);
  int GetAltFlowDistanceSum(std::vector<std::vector<int>>, std::vector<int>);
  int RateSolution(Solution&);
  bool UpdatePersonalPopulation(Solution&);
  bool UpdateGlobalPopulation(Solution&);

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  static NAN_METHOD(AddGlobalSolution);
  Solution* CreateSolution();
  void CreateSolutions(int);
  static NAN_METHOD(_CreateSolution);
  static NAN_METHOD(_CreateAndReturnSolution);
  static NAN_METHOD(_CreateSolutions);

  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

#endif  // AGENT_ADDON_AGENT_H_