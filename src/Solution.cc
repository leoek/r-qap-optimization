#include "Solution.h"

using namespace v8;
using namespace std;

Nan::Persistent<v8::FunctionTemplate> Solution::constructor;

NAN_MODULE_INIT(Solution::Init) {
  v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(Solution::New);
  constructor.Reset(ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("Solution").ToLocalChecked());

  // link our getters and setter to the object property
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("x").ToLocalChecked(), Solution::HandleGetters, Solution::HandleSetters);

  target->Set(Nan::New("Solution").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Solution::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Solution::New - called without new keyword").ToLocalChecked());
  }

  // expect exactly 3 arguments
  if(info.Length() != 1) {
    return Nan::ThrowError(Nan::New("Solution::New - expected arguments x, y, z").ToLocalChecked());
  }

  // expect arguments to be numbers
  if(!info[0]->IsNumber()) {
    return Nan::ThrowError(Nan::New("Solution::New - expected arguments to be numbers").ToLocalChecked());
  }

  // create a new instance and wrap our javascript instance
  Solution* vec = new Solution();
  vec->Wrap(info.Holder());

  // initialize it's values
  vec->x = info[0]->NumberValue();

  // return the wrapped javascript instance
  info.GetReturnValue().Set(info.Holder());
}

NAN_GETTER(Solution::HandleGetters) {
  Solution* self = Nan::ObjectWrap::Unwrap<Solution>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "x") {
    info.GetReturnValue().Set(self->x);
  }
}

NAN_SETTER(Solution::HandleSetters) {
  Solution* self = Nan::ObjectWrap::Unwrap<Solution>(info.This());

  if(!value->IsNumber()) {
    return Nan::ThrowError(Nan::New("expected value to be a number").ToLocalChecked());
  }

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "x") {
    self->x = value->NumberValue();
  }
}