#include "Agent.h"

using namespace v8;
using namespace std;

Nan::Persistent<v8::FunctionTemplate> Agent::constructor;

NAN_MODULE_INIT(Agent::Init) {
  v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(Agent::New);
  constructor.Reset(ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("Agent").ToLocalChecked());

  // link our getters and setter to the object property
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("x").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("y").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("z").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("intarray").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);

  Nan::SetPrototypeMethod(ctor, "add", Add);

  target->Set(Nan::New("Agent").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Agent::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Agent::New - called without new keyword").ToLocalChecked());
  }

  // expect exactly 3 arguments
  if(info.Length() != 4) {
    return Nan::ThrowError(Nan::New("Agent::New - expected arguments x, y, z").ToLocalChecked());
  }

  // expect arguments to be numbers
  if(!info[0]->IsNumber() || !info[1]->IsNumber()) {
    return Nan::ThrowError(Nan::New("Agent::New - expected arguments to be numbers").ToLocalChecked());
  }

  // create a new instance and wrap our javascript instance
  Agent* vec = new Agent();
  vec->Wrap(info.Holder());

  // initialize it's values
  vec->x = info[0]->NumberValue();
  vec->y = info[1]->NumberValue();
  vec->z = info[2]->NumberValue();

    Local<Array> jsArray = Local<Array>::Cast(info[3]);

  vector<int> intarray;
   for (unsigned int i = 0; i < jsArray->Length(); i++)
  {
    Handle<Value> val = jsArray->Get(i);
    int numVal = val->NumberValue();
    intarray.push_back(numVal);
  }

  vec->intarray = intarray;


  // return the wrapped javascript instance
  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Agent::Add) {
  // unwrap this Agent
  Agent * self = Nan::ObjectWrap::Unwrap<Agent>(info.This());

  if (!Nan::New(Agent::constructor)->HasInstance(info[0])) {
    return Nan::ThrowError(Nan::New("Agent::Add - expected argument to be instance of Agent").ToLocalChecked());
  }
  // unwrap the Agent passed as argument
  Agent * otherVec = Nan::ObjectWrap::Unwrap<Agent>(info[0]->ToObject());

  // specify argument counts and constructor arguments
  const int argc = 3;
  v8::Local<v8::Value> argv[argc] = {
    Nan::New(self->x + otherVec->x),
    Nan::New(self->y + otherVec->y),
    Nan::New(self->z + otherVec->z)
  };

  // get a local handle to our constructor function
  v8::Local<v8::Function> constructorFunc = Nan::New(Agent::constructor)->GetFunction();
  // create a new JS instance from arguments
  v8::Local<v8::Object> jsSumVec = Nan::NewInstance(constructorFunc, argc, argv).ToLocalChecked();

  info.GetReturnValue().Set(jsSumVec);
}

NAN_GETTER(Agent::HandleGetters) {
  Agent* self = Nan::ObjectWrap::Unwrap<Agent>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "x") {
    info.GetReturnValue().Set(self->x);
  } else if (propertyName == "y") {
    info.GetReturnValue().Set(self->y);
  } else if (propertyName == "z") {
    info.GetReturnValue().Set(self->z);
  } else if (propertyName == "intarray") {
    v8::Local<v8::Array> jsArray = Nan::New<v8::Array>(self->intarray.size());
    for (size_t i = 0; i < self->intarray.size(); i++)
    {
        printf("value %lu %d", i, self->intarray[i]);
        Nan::Set(jsArray, i, Nan::New<Number>(self->intarray[i]));
    }
    info.GetReturnValue().Set(jsArray);
  } else {
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_SETTER(Agent::HandleSetters) {
  Agent* self = Nan::ObjectWrap::Unwrap<Agent>(info.This());

  if(!value->IsNumber()) {
    return Nan::ThrowError(Nan::New("expected value to be a number").ToLocalChecked());
  }

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "x") {
    self->x = value->NumberValue();
  } else if (propertyName == "y") {
    self->y = value->NumberValue();
  } else if (propertyName == "z") {
    self->z = value->NumberValue();
  } else if (propertyName == "intarray"){

  }
}