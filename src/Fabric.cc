#include "Fabric.h"

using namespace v8;
using namespace std;

Nan::Persistent<v8::FunctionTemplate> Fabric::constructor;

NAN_MODULE_INIT(Fabric::Init) {
  v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(Fabric::New);
  constructor.Reset(ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("Fabric").ToLocalChecked());

  // link our getters and setter to the object property
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("pFailure").ToLocalChecked(), Fabric::HandleGetters, Fabric::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("capacity").ToLocalChecked(), Fabric::HandleGetters, Fabric::HandleSetters);
    Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("x").ToLocalChecked(), Fabric::HandleGetters, Fabric::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("y").ToLocalChecked(), Fabric::HandleGetters, Fabric::HandleSetters);

  target->Set(Nan::New("Fabric").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Fabric::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Fabric::New - called without new keyword").ToLocalChecked());
  }

  // expect exactly <2 arguments
  if(info.Length() != 4) {
    return Nan::ThrowError(Nan::New("Fabric::New - expected 4 arguments").ToLocalChecked());
  }

  // expect arguments to be numbers
  if(!info[0]->IsNumber() || !info[1]->IsNumber() || !info[2]->IsNumber() || !info[3]->IsNumber()) {
    return Nan::ThrowError(Nan::New("Agent::New - expected arguments to be numbers").ToLocalChecked());
  }

  // create a new instance and wrap our javascript instance
  Fabric* self = new Fabric();
  self->Wrap(info.Holder());

  self->pFailure = info[0]->NumberValue();
  self->capacity = info[1]->NumberValue();
  self->x = info[2]->NumberValue();
  self->y = info[3]->NumberValue();

  // return the wrapped javascript instance
  info.GetReturnValue().Set(info.Holder());
}

NAN_GETTER(Fabric::HandleGetters) {
  Fabric* self = Nan::ObjectWrap::Unwrap<Fabric>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "pFailure") {
    info.GetReturnValue().Set(self->pFailure);
  } else if (propertyName == "capacity"){
    info.GetReturnValue().Set(self->capacity);
  } else if (propertyName == "x"){
    info.GetReturnValue().Set(self->x);
  } else if (propertyName == "y"){
    info.GetReturnValue().Set(self->y);
  }
}

NAN_SETTER(Fabric::HandleSetters) {
  Fabric* self = Nan::ObjectWrap::Unwrap<Fabric>(info.This());

  if(!value->IsNumber()) {
    return Nan::ThrowError(Nan::New("expected value to be a number").ToLocalChecked());
  }

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "pFailure") {
    self->pFailure = value->NumberValue();
  } else if (propertyName == "capacity"){
    self->capacity = value->NumberValue();
  } else if (propertyName == "x"){
    self->x = value->NumberValue();
  } else if (propertyName == "y"){
    self->y = value->NumberValue();
  }
}