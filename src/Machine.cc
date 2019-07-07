#include "Machine.h"

using namespace v8;
using namespace std;

Nan::Persistent<v8::FunctionTemplate> Machine::constructor;

NAN_MODULE_INIT(Machine::Init) {
  v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(Machine::New);
  constructor.Reset(ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("Machine").ToLocalChecked());

  // link our getters and setter to the object property
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("size").ToLocalChecked(), Machine::HandleGetters, Machine::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("redundancy").ToLocalChecked(), Machine::HandleGetters, Machine::HandleSetters);
  target->Set(Nan::New("Machine").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Machine::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Machine::New - called without new keyword").ToLocalChecked());
  }

  // expect exactly <2 arguments
  if(info.Length() != 2) {
    return Nan::ThrowError(Nan::New("Machine::New - expected 2 arguments").ToLocalChecked());
  }

  // expect arguments to be numbers
  if(!info[0]->IsNumber() || !info[1]->IsNumber()) {
    return Nan::ThrowError(Nan::New("Agent::New - expected arguments to be numbers").ToLocalChecked());
  }

  // create a new instance and wrap our javascript instance
  Machine* self = new Machine();
  self->Wrap(info.Holder());

  self->size = info[0]->NumberValue();
  self->redundancy = info[1]->NumberValue();

  // return the wrapped javascript instance
  info.GetReturnValue().Set(info.Holder());
}

NAN_GETTER(Machine::HandleGetters) {
  Machine* self = Nan::ObjectWrap::Unwrap<Machine>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "size") {
    info.GetReturnValue().Set(self->size);
  } else if (propertyName == "redundancy"){
    info.GetReturnValue().Set(self->redundancy);
  }
}

NAN_SETTER(Machine::HandleSetters) {
  Machine* self = Nan::ObjectWrap::Unwrap<Machine>(info.This());

  if(!value->IsNumber()) {
    return Nan::ThrowError(Nan::New("expected value to be a number").ToLocalChecked());
  }

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "size") {
    self->size = value->NumberValue();
  } else if (propertyName == "redundancy"){
    self->redundancy = value->NumberValue();
  }
}