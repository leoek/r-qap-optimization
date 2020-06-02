#ifndef AGENT_ADDON_SOLUTION_H_
#define AGENT_ADDON_SOLUTION_H_

#include <nan.h>
#include "stringFormat.cc"

class Solution : public Nan::ObjectWrap {
public:
  double quality = -1;
  std::vector<int> permutation;

  Solution();
  Solution(Solution&);

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  int Add(int);
  int Add(int, bool);
  static NAN_METHOD(_Add);

  int GetLength();
  std::string ToString();
  static NAN_METHOD(_GetLength);

  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

#endif  // AGENT_ADDON_SOLUTION_H_