#include "Solution.h"

using namespace v8;
using namespace std;

Nan::Persistent<v8::FunctionTemplate> Solution::constructor;

Solution::Solution(){}

Solution::Solution(Solution& old){
  for (unsigned int i = 0; i < old.GetLength(); i++){
    permutation.push_back(old.permutation.at(i));    
  }
  quality = old.quality;
}

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
  vector<vector<int>> permutationArray;
  if(info[0]->IsArray()) {
    permutationArray = UnWrapPermutation(Local<Array>::Cast(info[0]));
  }
  sol->permutation = permutationArray;

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

bool Solution::Set(int machineIndex, int level, int value){
  if (machineIndex == permutation.size()){
    Add(value);
    return true;
  } else if (machineIndex < permutation.size()){
    if (level == permutation[machineIndex].size()){
      Add(machineIndex, value);
      return true;
    } else if (level < permutation[machineIndex].size()){
      permutation[machineIndex][level] = value;
      return true;
    }
  }
  printf("Invalid Solution: this shouldn't happen! machineIndex: %i, Level: %i \nSolution: %s\n", machineIndex, level, ToString().c_str());
  return false;
}

int Solution::Add(int value){
  vector<int> inner;
  inner.push_back(value);
  permutation.push_back(inner);
  return permutation.size();
}

void Solution::Add(int machineIndex, int value){
  permutation.at(machineIndex).push_back(value);
}

NAN_METHOD(Solution::_Add) {
  // unwrap this Solution
  Solution * self = Nan::ObjectWrap::Unwrap<Solution>(info.This());

  if(!info[0]->IsNumber()) {
    return Nan::ThrowError(Nan::New("Solution::Add - expected argument 0 to be an integer").ToLocalChecked());
  }
  /*
  bool check = true;
  if(info[1]->IsBoolean()) {
    check = info[1]->BooleanValue();
  }
  */
  int newVal = info[0]->IntegerValue();
  int size = (*self).Add(newVal);
  info.GetReturnValue().Set(size);
}

std::string Solution::ToString(){
  std::string result = string_format("quality: %f; length: %i; permutation: ", quality, GetLength());
  for (unsigned int i = 0; i < permutation.size(); i++){
    result += string_format(" m%i:", i);
    for (unsigned int k = 0; k < permutation.at(i).size(); k++){
      result += string_format("%i,", permutation[i][k]);
    }
    result += " ";
  }
  return result;
}

NAN_GETTER(Solution::HandleGetters) {
  Solution* self = Nan::ObjectWrap::Unwrap<Solution>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "quality") {
    info.GetReturnValue().Set(self->quality);
  } else if (propertyName == "permutation"){
    info.GetReturnValue().Set(WrapPermutation(self->permutation));
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
    self->permutation = UnWrapPermutation(Local<Array>::Cast(value));
  }
}

Local<Object> CreateWrappedSolution(Solution& sol){
  // Create new wrapped solution instance _sol
  const int argc = 2;
  v8::Local<v8::Value> argv[argc] = {
    WrapPermutation(sol.permutation),
    Nan::New(sol.quality)
  };

  Local<Function> constructorFunc = Nan::New(Solution::constructor)->GetFunction();
  Local<Object> _sol = Nan::NewInstance(constructorFunc, argc, argv).ToLocalChecked();
  return _sol;
}
