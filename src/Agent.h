
#ifndef AGENT_ADDON_AGENT_H_
#define AGENT_ADDON_AGENT_H_

#include <nan.h>
#include "Solution.h"
#include "Factory.h"
#include "Machine.h"
#include "rndSelector.cc"
#include "helperFunctions.h"
#include "Matrix.h"

class Agent : public Nan::ObjectWrap {
public:
  double x;
  double y;
  double z;
  std::vector<int> intarray;

  std::vector<Machine*> machines;
  std::vector<Factory*> factories;
  Matrix * flowMatrix;
  Matrix * changeOverMatrix;
  Matrix * distanceMatrix; 

  std::vector<Solution*> globalBestSolutions;
  int maxPersonalBest = 3;
  std::vector<Solution*> personalBestSolutions;
  int currentMachineIndex = 0;

  int pBestPopulationWeight = 1;
  int gBestPopulationWeight = 1;
  int rndWeight = 1;

  int GetNextValue();
  void ResetFactories();
  void Solve(Solution&);
  int RateSolution(Solution&);
  bool UpdatePersonalPopulation(Solution&);

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  static NAN_METHOD(Add);
  bool CreateSolution();
  static NAN_METHOD(_CreateSolution);

  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

#endif  // AGENT_ADDON_AGENT_H_