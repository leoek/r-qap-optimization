
#ifndef AGENT_ADDON_AGENT_H_
#define AGENT_ADDON_AGENT_H_

#include <nan.h>
#include "Solution.h"
#include "Factory.h"
#include "Machine.h"
#include "rndSelector.cc"
#include "helperFunctions.h"

class Agent : public Nan::ObjectWrap {
public:
  double x;
  double y;
  double z;
  std::vector<Machine*> machines;
  std::vector<Factory*> factories;
  std::vector<int> intarray;

  std::vector<Solution*> globalBestSolutions;
  int maxPersonalBest = 3;
  std::vector<Solution*> personalBestSolutions;
  int currentMachineIndex = 0;
  std::vector<Factory*> currentFactories;

  int pBestPopulationWeight = 1;
  int gBestPopulationWeight = 1;
  int rndWeight = 1;

  int GetNextValue();
  void ResetCurrentFactories();
  void Solve(Solution&);
  int RateSolution(Solution&);

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  static NAN_METHOD(Add);
  static NAN_METHOD(CreateSolution);

  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

#endif  // AGENT_ADDON_AGENT_H_