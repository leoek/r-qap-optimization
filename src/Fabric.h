#ifndef AGENT_ADDON_FABRIC_H_
#define AGENT_ADDON_FABRIC_H_

#include <nan.h>

class Fabric : public Nan::ObjectWrap {
public:
  double pFailure;
  int capacity;
  int x;
  int y;
  std::vector<int> permutation;

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);

  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

#endif  // AGENT_ADDON_FABRIC_H_