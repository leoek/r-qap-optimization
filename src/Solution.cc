#include "Solution.h"

using namespace v8;
using namespace std;

Nan::Persistent<v8::FunctionTemplate> Solution::constructor;

Solution::Solution(){}

Solution::Solution(Solution& old){
  for (unsigned int i = 0; i < old.GetLength(); i++){
    permutation.push_back(old.permutation.at(i));    
  }
  flowDistanceSum = old.flowDistanceSum;
  failureRiskSum = old.failureRiskSum;
  singleFactoryFailureSum = old.singleFactoryFailureSum;
  flowDistance = old.flowDistance;
  failureRisk = old.failureRisk;
  singleFactoryFailure = old.singleFactoryFailure;
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
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("flowDistanceSum").ToLocalChecked(), Solution::HandleGetters, Solution::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("failureRiskSum").ToLocalChecked(), Solution::HandleGetters, Solution::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("singleFactoryFailureSum").ToLocalChecked(), Solution::HandleGetters, Solution::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("flowDistance").ToLocalChecked(), Solution::HandleGetters, Solution::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("failureRisk").ToLocalChecked(), Solution::HandleGetters, Solution::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("singleFactoryFailure").ToLocalChecked(), Solution::HandleGetters, Solution::HandleSetters);

  Nan::SetPrototypeMethod(ctor, "add", _Add);
  Nan::SetPrototypeMethod(ctor, "getLength", _GetLength);

  target->Set(Nan::New("Solution").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Solution::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Solution::New - called without new keyword").ToLocalChecked());
  }

  // expect <8 arguments
  if(info.Length() > 8) {
    return Nan::ThrowError(Nan::New(strcat(strcat(
      "Solution::New - expected optional arguments int[] (permutation), double (quality), ",
      "int (flowDistanceSum), double (failureRiskSum), double (singleFactoryFailureSum), "),
      "double (flowDistance), double (failureRisk), double (singleFactoryFailure)"
      )).ToLocalChecked());
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
  if(info[2]->IsNumber()) {
    sol->flowDistanceSum = info[2]->NumberValue();
  }
  if(info[3]->IsNumber()) {
    sol->failureRiskSum = info[3]->NumberValue();
  }
  if(info[4]->IsNumber()) {
    sol->singleFactoryFailureSum = info[4]->NumberValue();
  }
  if(info[5]->IsNumber()) {
    sol->flowDistance = info[5]->NumberValue();
  }
  if(info[6]->IsNumber()) {
    sol->failureRisk = info[6]->NumberValue();
  }
  if(info[7]->IsNumber()) {
    sol->singleFactoryFailure = info[7]->NumberValue();
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
  /**
   * increase the permutation size accordingly
   * (values might be added out of order - eg machine 5 before 3)
   */
  while(machineIndex >= permutation.size()){
    vector<int> inner;
    permutation.push_back(inner);
  }
  /**
   * it is expected that levels are added in order
   * --> level 3 cannot be set if there is no level 2 yet
   */
  if (level == permutation[machineIndex].size()){
    permutation.at(machineIndex).push_back(value);
    return true;
  } else {
    permutation[machineIndex][level] = value;
    return true;
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
  result += string_format(
    "\n flowDistance: %f (%i); failureRiskSum: %f (%f); singleFactoryFailureSum: %f (%f)", 
    flowDistance, flowDistanceSum, failureRisk, failureRiskSum, singleFactoryFailure, singleFactoryFailureSum
  );
  return result;
}

NAN_GETTER(Solution::HandleGetters) {
  Solution* self = Nan::ObjectWrap::Unwrap<Solution>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "quality") {
    info.GetReturnValue().Set(self->quality);
  } else if (propertyName == "permutation"){
    info.GetReturnValue().Set(WrapPermutation(self->permutation));
  } else if (propertyName == "flowDistanceSum"){
    info.GetReturnValue().Set(self->flowDistanceSum);
  } else if (propertyName == "failureRiskSum"){
    info.GetReturnValue().Set(self->failureRiskSum);
  } else if (propertyName == "singleFactoryFailureSum"){
    info.GetReturnValue().Set(self->singleFactoryFailureSum);
  } else if (propertyName == "flowDistance"){
    info.GetReturnValue().Set(self->flowDistance);
  } else if (propertyName == "failureRisk"){
    info.GetReturnValue().Set(self->failureRisk);
  } else if (propertyName == "singleFactoryFailure"){
    info.GetReturnValue().Set(self->singleFactoryFailure);
  } else {
    info.GetReturnValue().Set(Nan::Undefined());
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
  } else if (propertyName == "flowDistanceSum"){
    self->flowDistanceSum = value->NumberValue();
  } else if (propertyName == "failureRiskSum"){
    self->failureRiskSum = value->NumberValue();
  } else if (propertyName == "singleFactoryFailureSum"){
    self->singleFactoryFailureSum = value->NumberValue();
  } else if (propertyName == "flowDistance"){
    self->flowDistance = value->NumberValue();
  } else if (propertyName == "failureRisk"){
    self->failureRisk = value->NumberValue();
  } else if (propertyName == "singleFactoryFailure"){
    self->singleFactoryFailure = value->NumberValue();
  }else {
    return Nan::ThrowError(Nan::New("Agent::Not implemented.").ToLocalChecked());
  }
}

Local<Object> CreateWrappedSolution(Solution& sol){
  // Create new wrapped solution instance _sol
  const int argc = 8;
  v8::Local<v8::Value> argv[argc] = {
    WrapPermutation(sol.permutation),
    Nan::New(sol.quality),
    Nan::New(sol.flowDistanceSum),
    Nan::New(sol.failureRiskSum),
    Nan::New(sol.singleFactoryFailureSum),
    Nan::New(sol.flowDistance),
    Nan::New(sol.failureRisk),
    Nan::New(sol.singleFactoryFailure)
  };

  Local<Function> constructorFunc = Nan::New(Solution::constructor)->GetFunction();
  Local<Object> _sol = Nan::NewInstance(constructorFunc, argc, argv).ToLocalChecked();
  return _sol;
}
