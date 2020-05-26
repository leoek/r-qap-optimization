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
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("quality").ToLocalChecked(), Solution::HandleGetters, Solution::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("permutation").ToLocalChecked(), Solution::HandleGetters, Solution::HandleSetters);

  Nan::SetPrototypeMethod(ctor, "add", _Add);
  Nan::SetPrototypeMethod(ctor, "getLength", _GetLength);

  target->Set(Nan::New("Solution").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Solution::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Solution::New - called without new keyword").ToLocalChecked());
  }

  // expect <2 arguments
  if(info.Length() > 2) {
    return Nan::ThrowError(Nan::New("Solution::New - expected optional arguments int[] and double").ToLocalChecked());
  }

  // create a new instance and wrap our javascript instance
  Solution* sol = new Solution();
  sol->Wrap(info.Holder());

  // initialize it's values
  vector<int> intarray;
  if(info[0]->IsArray()) {
    Local<Array> jsArray = Local<Array>::Cast(info[0]);
    for (unsigned int i = 0; i < jsArray->Length(); i++)
    {
      Handle<Value> val = jsArray->Get(i);
      int numVal = val->NumberValue();
      intarray.push_back(numVal);
    }
  }
  sol->permutation = intarray;

  if(info[1]->IsNumber()) {
    sol->quality = info[1]->NumberValue();
  }

  // return the wrapped javascript instance
  info.GetReturnValue().Set(info.Holder());
}

int Solution::GetLength(){
  return permutation.size();
}

NAN_METHOD(Solution::_GetLength) {
  // unwrap this Solution
  Solution * self = Nan::ObjectWrap::Unwrap<Solution>(info.This());
  int size = (*self).GetLength();
  info.GetReturnValue().Set(size);
}

int Solution::Add(int value){
  return Add(value, True);
}
int Solution::Add(int value, bool check){
  /**
   * #TODO check whether this is possible if check=true
   * not sure whether this is useful here yet
   **/
  permutation.push_back(value);
  return permutation.size();
}

NAN_METHOD(Solution::_Add) {
  // unwrap this Solution
  Solution * self = Nan::ObjectWrap::Unwrap<Solution>(info.This());
  bool check = true;

  if(!info[0]->IsNumber()) {
    return Nan::ThrowError(Nan::New("Solution::Add - expected argument 0 to be an integer").ToLocalChecked());
  }
  if(info[1]->IsBoolean()) {
    check = info[1]->BooleanValue();
  }
   
  int newVal = info[0]->IntegerValue();
  int size = (*self).Add(newVal, check);
  info.GetReturnValue().Set(size);
}

std::string Solution::ToString(){
  std::string result = string_format("quality: %f; length: %i; permutation: ", quality, GetLength());
  for (unsigned int k = 0; k < permutation.size(); k++){
    result += string_format(" %d ", permutation.at(k));
  }
  return result;
}

NAN_GETTER(Solution::HandleGetters) {
  Solution* self = Nan::ObjectWrap::Unwrap<Solution>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "quality") {
    info.GetReturnValue().Set(self->quality);
  } else if (propertyName == "permutation"){
    v8::Local<v8::Array> jsArray = Nan::New<v8::Array>(self->permutation.size());
    for (size_t i = 0; i < self->permutation.size(); i++)
    {
        Nan::Set(jsArray, i, Nan::New<Number>(self->permutation[i]));
    }
    info.GetReturnValue().Set(jsArray);
  }
}

NAN_SETTER(Solution::HandleSetters) {
  Solution* self = Nan::ObjectWrap::Unwrap<Solution>(info.This());

  if(!value->IsNumber()) {
    return Nan::ThrowError(Nan::New("expected value to be a number").ToLocalChecked());
  }

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "quality") {
    self->quality = value->NumberValue();
  } else if (propertyName == "permutation"){
    Local<Array> jsArray = Local<Array>::Cast(value);
    vector<int> intarray;
    for (unsigned int i = 0; i < jsArray->Length(); i++)
    {
      Handle<Value> val = jsArray->Get(i);
      int numVal = val->IntegerValue();
      intarray.push_back(numVal);
    }
    self->permutation = intarray;
  }
}