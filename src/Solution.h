#ifndef AGENT_ADDON_SOLUTION_H_
#define AGENT_ADDON_SOLUTION_H_

#include <nan.h>
#include "stringFormat.cc"
#include "WrappingHelpers.h"

class Solution : public Nan::ObjectWrap {
public:
  std::vector<std::vector<int>> permutation;

  // aggregation of all quality scores 
  double quality = -1;
  // distinct quality scores
  int flowDistanceSum = -1;
  double failureRiskSum = -1;
  double singleFactoryFailureSum = -1;
  // normalized quality scores
  double flowDistance = -1;
  double failureRisk = -1;
  double singleFactoryFailure = -1;

  Solution();
  Solution(Solution&);

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  bool Set(int, int, int);
  void Add(int, int);
  int Add(int);
  static NAN_METHOD(_Add);

  int GetLength();
  std::string ToString();
  static NAN_METHOD(_GetLength);

  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

v8::Local<v8::Object> CreateWrappedSolution(Solution&);

#endif  // AGENT_ADDON_SOLUTION_H_