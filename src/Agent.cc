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
  Nan::SetPrototypeMethod(ctor, "createSolution", CreateSolution);

  target->Set(Nan::New("Agent").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Agent::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Agent::New - called without new keyword").ToLocalChecked());
  }

  if(info.Length() != 5) {
    return Nan::ThrowError(Nan::New("Agent::New - unexpected number of arguments").ToLocalChecked());
  }
  // #NIT Arrays currently have no explicit type check
  if (!Nan::New(Matrix::constructor)->HasInstance(info[2])) {
    return Nan::ThrowError(Nan::New("Agent::New - expected argument 3 to be instance of Matrix").ToLocalChecked());
  }
  if (!Nan::New(Matrix::constructor)->HasInstance(info[3])) {
    return Nan::ThrowError(Nan::New("Agent::New - expected argument 4 to be instance of Matrix").ToLocalChecked());
  }
  if (!Nan::New(Matrix::constructor)->HasInstance(info[4])) {
    return Nan::ThrowError(Nan::New("Agent::New - expected argument 5 to be instance of Matrix").ToLocalChecked());
  }

  // create a new instance and wrap our javascript instance
  Agent* self = new Agent();
  self->Wrap(info.Holder());

  // initialize it's values
  Local<Array> factoryJsArray = Local<Array>::Cast(info[0]);
  vector<Factory*> factoryArray;
   for (unsigned int i = 0; i < factoryJsArray->Length(); i++)
  {
    Handle<Value> val = factoryJsArray->Get(i);
    Factory * factoryVal = Nan::ObjectWrap::Unwrap<Factory>(val->ToObject());
    factoryArray.push_back(factoryVal);
  }
  self->factories = factoryArray;

  Local<Array> machineJsArray = Local<Array>::Cast(info[1]);
  vector<Machine*> machineArray;
   for (unsigned int i = 0; i < machineJsArray->Length(); i++)
  {
    Handle<Value> val = machineJsArray->Get(i);
    Machine * machineVal = Nan::ObjectWrap::Unwrap<Machine>(val->ToObject());
    machineArray.push_back(machineVal);
  }
  self->machines = machineArray;

  self->flowMatrix = Nan::ObjectWrap::Unwrap<Matrix>(info[2]->ToObject());
  self->changeOverMatrix = Nan::ObjectWrap::Unwrap<Matrix>(info[3]->ToObject());
  self->distanceMatrix = Nan::ObjectWrap::Unwrap<Matrix>(info[4]->ToObject());
  
  // return the wrapped javascript instance
  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Agent::Add) {
  // unwrap this Agent
  Agent * self = Nan::ObjectWrap::Unwrap<Agent>(info.This());

  if (!Nan::New(Agent::constructor)->HasInstance(info[0])) {
    return Nan::ThrowError(Nan::New("Agent::Add - expected argument to be instance of Agent").ToLocalChecked());
  }
  if (!Nan::New(Solution::constructor)->HasInstance(info[1])) {
    return Nan::ThrowError(Nan::New("Agent::Add - expected argument to be instance of Solution").ToLocalChecked());
  }

  Local<Array> jsArray = Local<Array>::Cast(info[2]);

  vector<Solution*> solArray;
   for (unsigned int i = 0; i < jsArray->Length(); i++)
  {
    Handle<Value> val = jsArray->Get(i);
    Solution * solVal = Nan::ObjectWrap::Unwrap<Solution>(val->ToObject());
    printf("solval x value %f", solVal->quality);
    solArray.push_back(solVal);
  }

  // unwrap the Agent passed as argument
  Agent * otherVec = Nan::ObjectWrap::Unwrap<Agent>(info[0]->ToObject());
  Solution * otherSolution = Nan::ObjectWrap::Unwrap<Solution>(info[1]->ToObject());

  printf("sol quality %f", otherSolution->quality);

  // specify argument counts and constructor arguments
  const int argc = 4;
  v8::Local<v8::Value> argv[argc] = {
    Nan::New(self->x + otherVec->x),
    Nan::New(self->y + otherVec->y),
    Nan::New(self->z + otherVec->z)
  };

  // get a local handle to our constructor function
  //v8::Local<v8::Function> constructorFunc = Nan::New(Agent::constructor)->GetFunction();
  // create a new JS instance from arguments
  //v8::Local<v8::Object> jsSumVec = Nan::NewInstance(constructorFunc, argc, argv).ToLocalChecked();

  //info.GetReturnValue().Set(jsSumVec);
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

int Agent::GetNextValue(){

  Machine * currentMachine = machines.at(currentMachineIndex);
  // #PERFORMANCE
  // check the capacity requirement after selecting randomly
  // remove from availableFactories if not enough space
  // and select randomly again
  vector<int> availableFactories;
  for (unsigned int i = 0; i < currentFactories.size(); i++){
    if (currentFactories.at(i)->GetUnusedCapacity() >= currentMachine->size){
      availableFactories.push_back(i);
    }
  }

  vector<Solution*> chosenPopulation;
  int selected = -1;
  random_selector<> selector{};
  // try until a new index is selected
  while (selected < 0){
    int populationSelector = GetRndNumberFromRange(0, rndWeight + gBestPopulationWeight + pBestPopulationWeight);
    if (populationSelector < rndWeight){
      // Select a random Factory from the available ones
      selected = selector(availableFactories);
    } else {
      // #PERFORMANCE
      // Cache those which are not possible
      int possibleIndex = -1;
      if (populationSelector < rndWeight + gBestPopulationWeight && globalBestSolutions.size() >= 1){
        Solution * srcSol = selector(globalBestSolutions);
        possibleIndex = srcSol->permutation.at(currentMachineIndex);

      } else if (populationSelector < rndWeight + gBestPopulationWeight + pBestPopulationWeight && personalBestSolutions.size() >= 1){
        Solution * srcSol = selector(personalBestSolutions);
        possibleIndex = srcSol->permutation.at(currentMachineIndex);
      }
      // Check whether there is a possible index which was copied from a previous Solution
      if (possibleIndex >= 0){
        // Check whether the machine fits into that factory (get the factory at that index)
        // select that index if it is feasible
        if (possibleIndex >= 0 && currentFactories.at(possibleIndex)->GetUnusedCapacity() >= machines.at(currentMachineIndex)->size){
          selected = possibleIndex;
        }
      }
    }
  }
  // Use the size of currenMachine inthe chosen factory
  currentFactories.at(selected)->UseCapacity(currentMachine->size);
  return selected;
}

void Agent::ResetCurrentFactories(){
  vector<Factory*> newFactories;
  for (unsigned int i = 0; i < factories.size(); i++){
    Factory * copy = new Factory(* factories.at(i));
    newFactories.push_back(copy);
  }
  currentFactories = newFactories;
}

int Agent::RateSolution(Solution &sol){
  // Rate the generated solution
  // #TODO rate all criteria (currently only flow*distance)
  int flowDistanceSum = 0;
  for (unsigned int m_i = 0; m_i < machines.size(); m_i++){
    for (unsigned int m_k = 0; m_k < machines.size(); m_k++){
      int flow = flowMatrix->GetValue(m_i, m_k);
      int distance = distanceMatrix->GetValue(sol.permutation[m_i], sol.permutation[m_k]);
      flowDistanceSum += flow * distance;
    }
  }
  sol.quality = flowDistanceSum;
  return flowDistanceSum;
}

void Agent::Solve(Solution &sol){
  ResetCurrentFactories();
  currentMachineIndex = 0;
  while(currentMachineIndex < machines.size()){
    sol.Add(GetNextValue());
    currentMachineIndex++;
  }
}

NAN_METHOD(Agent::CreateSolution) {
  // unwrap this Agent
  Agent * self = Nan::ObjectWrap::Unwrap<Agent>(info.This());

  const int argc = 0;
  v8::Local<v8::Value> argv[argc] = {};
  // get a local handle to our constructor function
  Local<Function> constructorFunc = Nan::New(Solution::constructor)->GetFunction();
  // create a new JS instance from arguments
  Local<Object> _sol = Nan::NewInstance(constructorFunc, argc, argv).ToLocalChecked();
  Solution * sol = Nan::ObjectWrap::Unwrap<Solution>(_sol);
  self->Solve(*sol);
  self->RateSolution(*sol);

  info.GetReturnValue().Set(_sol);
}