#ifndef AGENT_ADDON_SOLUTION_H_
#define AGENT_ADDON_SOLUTION_H_

#include <nan.h>
#include "stringFormat.cc"
#include "WrappingHelpers.h"

class Solution : public Nan::ObjectWrap {
public:
  double quality = -1;
  std::vector<std::vector<int>> permutation;

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