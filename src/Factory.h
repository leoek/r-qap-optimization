#ifndef AGENT_ADDON_FACTORY_H_
#define AGENT_ADDON_FACTORY_H_

#include <nan.h>

class Factory : public Nan::ObjectWrap {
public:
  double pFailure;
  int capacity;
  int x;
  int y;
  int usedCapacity;

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);

  bool UseCapacity(int);
  static NAN_METHOD(_UseCapacity);
  int GetUnusedCapacity();
  static NAN_METHOD(_GetUnusedCapacity);
  static NAN_METHOD(ResetUsedCapacity);

  Factory();
  Factory(Factory&);

  static NAN_GETTER(HandleGetters);
  static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

#endif  // AGENT_ADDON_FACTORY_H_