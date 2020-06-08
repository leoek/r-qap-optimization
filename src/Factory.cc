#include "Factory.h"

using namespace v8;
using namespace std;

Nan::Persistent<v8::FunctionTemplate> Factory::constructor;

Factory::Factory(){
  usedCapacity = 0;
}

Factory::Factory(Factory& old){
  usedCapacity = 0;
  capacity = old.capacity;
  usedCapacity = old.usedCapacity;
  pFailure = old.pFailure;
  x = old.x;
  y = old.y;
}

std::string Factory::ToString(){
  std::string result = string_format(
    "[Factory] usedCapacity/capacity: %i/%i; pFailure: %f; x: %i; y: %i",
    usedCapacity, capacity, pFailure, x, y);
  return result;
}

NAN_MODULE_INIT(Factory::Init) {
  v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(Factory::New);
  constructor.Reset(ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("Factory").ToLocalChecked());

  // link our getters and setter to the object property
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("pFailure").ToLocalChecked(), Factory::HandleGetters, Factory::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("capacity").ToLocalChecked(), Factory::HandleGetters, Factory::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("x").ToLocalChecked(), Factory::HandleGetters, Factory::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("y").ToLocalChecked(), Factory::HandleGetters, Factory::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("usedCapacity").ToLocalChecked(), Factory::HandleGetters);

  Nan::SetPrototypeMethod(ctor, "useCapacity", _UseCapacity);
  Nan::SetPrototypeMethod(ctor, "resetUsedCapacity", _ResetUsedCapacity);
  Nan::SetPrototypeMethod(ctor, "getUnusedCapacity", _GetUnusedCapacity);
  
  target->Set(Nan::New("Factory").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Factory::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Factory::New - called without new keyword").ToLocalChecked());
  }

  // expect exactly <2 arguments
  if(info.Length() != 4) {
    return Nan::ThrowError(Nan::New("Factory::New - expected 4 arguments").ToLocalChecked());
  }

  // expect arguments to be numbers
  if(!info[0]->IsNumber() || !info[1]->IsNumber() || !info[2]->IsNumber() || !info[3]->IsNumber()) {
    return Nan::ThrowError(Nan::New("Factory::New - expected arguments to be numbers").ToLocalChecked());
  }

  // create a new instance and wrap our javascript instance
  Factory* self = new Factory();
  self->Wrap(info.Holder());

  self->pFailure = info[0]->NumberValue();
  self->capacity = info[1]->NumberValue();
  self->x = info[2]->NumberValue();
  self->y = info[3]->NumberValue();
  self->usedCapacity = 0;

  // return the wrapped javascript instance
  info.GetReturnValue().Set(info.Holder());
}

NAN_GETTER(Factory::HandleGetters) {
  Factory* self = Nan::ObjectWrap::Unwrap<Factory>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "pFailure") {
    info.GetReturnValue().Set(self->pFailure);
  } else if (propertyName == "capacity"){
    info.GetReturnValue().Set(self->capacity);
  } else if (propertyName == "x"){
    info.GetReturnValue().Set(self->x);
  } else if (propertyName == "y"){
    info.GetReturnValue().Set(self->y);
  } else if (propertyName == "usedCapacity"){
    info.GetReturnValue().Set(self->usedCapacity);
  }
}

NAN_SETTER(Factory::HandleSetters) {
  Factory* self = Nan::ObjectWrap::Unwrap<Factory>(info.This());

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

int Factory::ResetUsedCapacity(){
  usedCapacity = 0;
  return GetUnusedCapacity();
}

NAN_METHOD(Factory::_ResetUsedCapacity){
  Factory * self = Nan::ObjectWrap::Unwrap<Factory>(info.This());
  self->ResetUsedCapacity();
}

bool Factory::UseCapacity(int use){
  if (usedCapacity + use <= capacity){
    usedCapacity = usedCapacity + use;
    return True;
  } else  {
    return False;
  }
}

NAN_METHOD(Factory::_UseCapacity) {
  // unwrap this Solution
  Factory * self = Nan::ObjectWrap::Unwrap<Factory>(info.This());

  if(!info[0]->IsNumber()) {
    return Nan::ThrowError(Nan::New("Solution::Add - expected argument 0 to be an integer").ToLocalChecked());
  }

  int use = info[0]->IntegerValue();
  info.GetReturnValue().Set(self->UseCapacity(use));
}

int Factory::GetUnusedCapacity(){
  return capacity - usedCapacity;
}

NAN_METHOD(Factory::_GetUnusedCapacity) {
  // unwrap this Solution
  Factory * self = Nan::ObjectWrap::Unwrap<Factory>(info.This());
  info.GetReturnValue().Set(self->capacity - self->usedCapacity);
}