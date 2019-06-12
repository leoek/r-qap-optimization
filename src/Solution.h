#ifndef AGENT_ADDON_SOLUTION_H_
#define AGENT_ADDON_SOLUTION_H_

#include <nan.h>

class Solution : public Nan::ObjectWrap {
public:
  double x;

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);

  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

NAN_METHOD(CreateSolution);

#endif  // AGENT_ADDON_SOLUTION_H_