#ifndef AGENT_ADDON_MATRIX_H_
#define AGENT_ADDON_MATRIX_H_

#include <nan.h>

class Matrix : public Nan::ObjectWrap {
public:

  std::vector<std::vector<int>> matrix;

  static NAN_MODULE_INIT(Init);
  static NAN_METHOD(New);
  
  int GetValue(int, int);
  int GetValue(int, int, bool);
  static NAN_METHOD(_GetValue);

  static NAN_GETTER(HandleGetters);
  //no setters needed, the flow matrix is immutable
  //static NAN_SETTER(HandleSetters);

  static Nan::Persistent<v8::FunctionTemplate> constructor;
};

#endif  // AGENT_ADDON_MATRIX_H_