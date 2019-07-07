#ifndef AGENT_ADDON_MACHINE_H_
#define AGENT_ADDON_MACHINE_H_

#include <nan.h>

class Machine : public Nan::ObjectWrap {
public:
  int size;
  int redundancy = 1;

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  
  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

#endif  // AGENT_ADDON_MACHINE_H_